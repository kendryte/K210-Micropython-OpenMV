#include <stdio.h>
#include <string.h>
#include <dfs_posix.h>

#include "py/qstr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/mphal.h"

#ifdef EXTMODS_K210_KPU
#ifdef EXTMODS_K210_DVP
#include "py_picture.h"
extern const mp_obj_type_t py_picture_type;
extern picture_t *py_picture_cobj(mp_obj_t self);
#endif //EXTMODS_K210_DVP

#include "kpu.h"
#include "prior.h"
#include "region_layer.h"

extern int load_file_from_flash(uint32_t addr, uint8_t *data_buf, uint32_t length);
extern int load_file_from_ff(const char *path, void* buffer, size_t model_size);

typedef struct _k210_kpu_obj_t
{
    mp_obj_base_t base; 
    kpu_model_context_t*  kmodel_ctx;  //sipeed_model_ctx_t
    size_t     model_size;
    mp_obj_t   model_buffer;
    mp_obj_t   model_path;
    uint32_t   inputs;
    mp_obj_t   inputs_addr;
    uint32_t   outputs;
    mp_obj_t   *output;   
    size_t     *output_size;
    region_layer_t *rl;
    box_info_t     *boxes;

} k210_kpu_obj_t;

const mp_obj_type_t k210_kpu_type;
#ifdef PKG_USING_OPENMV_CP
typedef struct image {
    int w;
    int h;
    int bpp;
    union {
        uint8_t *pixels;
        uint8_t *data;
    };
    uint8_t *ai_pixels;
} image_t;

extern const mp_obj_type_t py_image_type;
extern void *py_image_cobj(mp_obj_t img_obj);
#endif

// reference namepace nncase::runtime
// enum model_target : uint32_t
// {
//     MODEL_TARGET_CPU = 0,
//     MODEL_TARGET_K210 = 1
// };

struct model_header
{
    uint32_t identifier;
    uint32_t version;
    uint32_t flags;
    uint32_t target;    //enum model_target : uint32_t
    uint32_t constants;
    uint32_t main_mem;
    uint32_t nodes;
    uint32_t inputs;
    uint32_t outputs;
    uint32_t reserved0;
};

volatile uint32_t g_ai_done_flag = 0;
static void ai_done(void *ctx)
{
    g_ai_done_flag = 1;
}

STATIC mp_obj_t region_layer_get_rect(box_info_t *bx, mp_obj_list_t* out_box) {
    float prob;
    uint32_t out[4];
   
    for (uint32_t i= 0; i < bx->row_idx; i++) {
        prob= bx->box[bx->box_len * i + bx->crood_num];
        if (prob > bx->obj_thresh) {
            float *b= &bx->box[bx->box_len * i];
            uint32_t x1= (b[0] - (b[2] / 2)) * bx->in_w;
            uint32_t y1= (b[1] - (b[3] / 2)) * bx->in_h;
            uint32_t x2= (b[0] + (b[2] / 2)) * bx->in_w;
            uint32_t y2= (b[1] + (b[3] / 2)) * bx->in_h;

            out[0] = (x1 >= 320 ? 319 : x1); 
            out[1] = (y1 >= 224 ? 223 : y1); 
            out[2] = (x2 >= 320 ? 319 : x2); 
            out[3] = (y2 >= 224 ? 223 : y2);
             mp_obj_list_t *ret_list = m_new(mp_obj_list_t, 1);
            mp_obj_list_init(ret_list, 0);
            mp_obj_list_append(ret_list, mp_obj_new_int(out[0]));
            mp_obj_list_append(ret_list, mp_obj_new_int(out[1]));
            mp_obj_list_append(ret_list, mp_obj_new_int(out[2]));
            mp_obj_list_append(ret_list, mp_obj_new_int(out[3]));

            mp_obj_list_append(out_box, ret_list);   
        }
    }
    // m_del_obj(mp_obj_list_t,ret_list);
}


STATIC mp_obj_t py_regionlayer_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    k210_kpu_obj_t *km = (k210_kpu_obj_t *)pos_args[0];
     enum
    {
        ARG_anchor_num,
        ARG_crood_num,
        ARG_landm_num,
        ARG_cls_num,
        ARG_in_w,
        ARG_in_h,
        ARG_obj_thresh,
        ARG_nms_thresh,
        ARG_variances,
        ARG_max_num,
        ARG_anchor,
    };
    static const mp_arg_t allowed_args[] = {
        {MP_QSTR_anchor_num, MP_ARG_INT, {.u_int = 3160}},
        {MP_QSTR_crood_num, MP_ARG_INT, {.u_int = 4}},
        {MP_QSTR_landm_num, MP_ARG_INT, {.u_int = 5}},
        {MP_QSTR_cls_num, MP_ARG_INT, {.u_int = 1}},
        {MP_QSTR_in_w, MP_ARG_INT, {.u_int = 320}},
        {MP_QSTR_in_h, MP_ARG_INT, {.u_int = 240}},
        {MP_QSTR_obj_thresh, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_nms_thresh, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_variances, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_max_num, MP_ARG_INT, {.u_int = 200}},
        {MP_QSTR_anchor, MP_ARG_OBJ, {.u_obj = mp_const_none}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    float obj_thresh = 0.7;
    float nms_thresh = 0.4;
    float *variances;
    float *anchor;
    if(args[ARG_obj_thresh].u_obj != mp_const_none){
        obj_thresh = mp_obj_get_float(args[ARG_obj_thresh].u_obj);
    }
    if(args[ARG_nms_thresh].u_obj != mp_const_none){
        nms_thresh = mp_obj_get_float(args[ARG_nms_thresh].u_obj);
    }
    if(args[ARG_variances].u_obj != mp_const_none){
        size_t nitems = 0;
        const mp_obj_t *items = 0;
        mp_obj_get_array(args[ARG_variances].u_obj, &nitems, (mp_obj_t **)&items);
        variances = m_new(float,nitems);
        for (int i = 0; i < nitems; i++){
            variances[i] = mp_obj_get_float(*items++);
        }
    }
    else{
        variances = m_new(float,2);
        variances[0] = 0.1;
        variances[1] = 0.2;
    }
    if(args[ARG_anchor].u_obj != mp_const_none){
        size_t nitems = 0;
        const mp_obj_t *items = 0;
        mp_obj_get_array(args[ARG_anchor].u_obj, &nitems, (mp_obj_t **)&items);
        anchor = m_new(float,nitems);
        for (int i = 0; i < nitems; i++){
            anchor[i] = mp_obj_get_float(*items++);
        }
    }
    else{
        anchor = default_anchor;
    }
    km->rl = m_new(region_layer_t,1);
    km->boxes = m_new(box_info_t, 1);
    region_layer_init(km->rl,
                     anchor, 
                     args[ARG_anchor_num].u_int, 
                     args[ARG_crood_num].u_int, 
                     args[ARG_landm_num].u_int, 
                     args[ARG_cls_num].u_int, 
                     args[ARG_in_w].u_int, 
                     args[ARG_in_h].u_int, 
                     obj_thresh, 
                     nms_thresh, 
                     variances);
    boxes_info_init(km->rl, km->boxes, args[ARG_max_num].u_int);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_regionlayer_init_obj, 1, py_regionlayer_init);

STATIC mp_obj_t py_regionlayer(mp_obj_t self_in)
{
    k210_kpu_obj_t *km = (k210_kpu_obj_t *)self_in;

    mp_obj_list_t *rect = m_new(mp_obj_list_t, 1);
    mp_obj_list_init(rect, 0);
 
    km->rl->bbox_input= (float*)(km->output[0]);
    km->rl->landm_input= (float*)(km->output[1]);
    km->rl->clses_input= (float*)(km->output[2]);
    region_layer_run(km->rl, km->boxes);
    region_layer_get_rect(km->boxes, rect);
    boxes_info_reset(km->boxes);
    
    return MP_OBJ_FROM_PTR(rect);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_regionlayer_obj, py_regionlayer);

STATIC mp_obj_t k210_kpu_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    
    k210_kpu_obj_t *self = m_new_obj(k210_kpu_obj_t);
    self->base.type=&k210_kpu_type;
    self->kmodel_ctx = m_new_obj(kpu_model_context_t);
    self->model_size = 0;
    self->model_buffer = NULL;
    self->model_path = NULL;
    self->output = NULL;
    self->output_size = NULL;
    self->inputs = 0;
    self->outputs = 0;
    self->inputs_addr = NULL;
    self->rl = NULL;
    self->boxes = NULL;

    return MP_OBJ_FROM_PTR(self);
}
/* 
test load_kmodel("/ulffd_landmark.kmodel")
 */
STATIC mp_obj_t py_kpu_load_kmodel(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    k210_kpu_obj_t *km = (k210_kpu_obj_t *)pos_args[0];
      enum
    {
        ARG_path,
        ARG_size,
    };
    static const mp_arg_t allowed_args[] = {
        {MP_QSTR_path, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_size, MP_ARG_INT, {.u_int = 0}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    if(args[ARG_path].u_obj == mp_const_none){
        mp_raise_ValueError("invalid path input");
    }

    if(mp_obj_get_type(args[ARG_path].u_obj) == &mp_type_str)
    {
        const char *path = mp_obj_str_get_str(args[ARG_path].u_obj);
        km->model_path = mp_obj_new_str(path,strlen(path));     
        int ret;
        struct stat st;
        ret = stat(path, &st);
        if(ret == 0){
            km->model_size = st.st_size;
        }
        else{
            mp_raise_ValueError("Failed to open file");
        }

        km->model_buffer = m_malloc(km->model_size);
        if(load_file_from_ff(path, km->model_buffer, km->model_size) != 0)
        {
            m_free(km->model_buffer);
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to read file"));
        }
    }
    else if(mp_obj_get_type(args[ARG_path].u_obj) == &mp_type_int){
        if(mp_obj_get_int(args[ARG_path].u_obj)<=0){
            mp_raise_ValueError("path error!");
        }
        if(args[ARG_size].u_int <= 0){
            mp_raise_ValueError("size error!");
        }
        uint32_t path_addr;
        path_addr = mp_obj_get_int(args[ARG_path].u_obj);
        km->model_path = (mp_obj_t)path_addr;
        km->model_size = args[ARG_size].u_int;
        km->model_buffer = m_malloc(km->model_size);
        if(load_file_from_flash( path_addr, km->model_buffer, km->model_size) != 0)
        {
            m_free(km->model_buffer);
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to read file"));
        }
    }
    else
    {
        mp_raise_ValueError("path error!");
    }

    if (kpu_load_kmodel(km->kmodel_ctx, km->model_buffer) != 0){
        m_free(km->model_buffer);
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to init model"));
    }
    mp_printf(&mp_plat_print, "model load succeed\n");  //debug
    if(km->kmodel_ctx->is_nncase){
        km->inputs = ((struct model_header*)km->model_buffer)->inputs;
        km->outputs = ((struct model_header*)km->model_buffer)->outputs;
    }
    else{
        km->inputs = 0;
        km->outputs = ((kpu_kmodel_header_t*)km->model_buffer)->output_count;
    }
    km->output_size = m_new(size_t, km->outputs);
    km->output = m_new(mp_obj_t,km->outputs);  

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_kpu_load_kmodel_obj,2, py_kpu_load_kmodel);

STATIC mp_obj_t py_kpu_run(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    k210_kpu_obj_t *km = (k210_kpu_obj_t *)pos_args[0];

     enum
    {
        ARG_input,
        ARG_dma,
    };
    static const mp_arg_t allowed_args[] = {
        {MP_QSTR_input, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_dma, MP_ARG_INT, {.u_int = 5}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if(args[ARG_input].u_obj == mp_const_none){
        mp_raise_ValueError("invalid input");
    }
    if(mp_obj_get_type(args[ARG_input].u_obj) == &mp_type_str){
        const char *path = mp_obj_str_get_str(args[ARG_input].u_obj);
        int ret;
        struct stat st;
        ret = stat(path, &st);
        if(ret != 0){
            mp_raise_ValueError("Failed to open file");
        }
        km->inputs_addr = m_malloc(st.st_size);
        if(load_file_from_ff(path, km->inputs_addr, st.st_size) != 0)
        {
            m_free(km->inputs_addr);
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to read file"));
        }
    }
#ifdef PKG_USING_OPENMV_CP
    else if(mp_obj_get_type(args[ARG_input].u_obj) == &py_image_type){
        image_t* kimage = py_image_cobj(args[ARG_input].u_obj);
        km->inputs_addr = kimage->ai_pixels;
    }
#endif //PKG_USING_OPENMV_CP

#ifdef EXTMODS_K210_DVP
    else if(mp_obj_get_type(args[ARG_input].u_obj) == &py_picture_type){
        picture_t* kimage = py_picture_cobj(args[ARG_input].u_obj);
        km->inputs_addr = kimage->ai;
    }
#endif  //EXTMODS_K210_DVP
    else{
            mp_raise_ValueError("invalid input");
    }
    g_ai_done_flag = 0;
    kpu_run_kmodel(km->kmodel_ctx, (uint8_t *)km->inputs_addr, args[ARG_dma].u_int, ai_done, NULL);
    return mp_const_none; 
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_kpu_run_obj, 1, py_kpu_run);

STATIC mp_obj_t py_kpu_get_output(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    k210_kpu_obj_t *km = (k210_kpu_obj_t *)pos_args[0];

    if(g_ai_done_flag != 1){
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "No output of kpu run"));
    }
    g_ai_done_flag = 0;

     enum
    {
        ARG_index,
        ARG_getlist,
    };
    static const mp_arg_t allowed_args[] = {
        {MP_QSTR_index, MP_ARG_INT, {.u_int = 0}},
        {MP_QSTR_getlist, MP_ARG_BOOL, {.u_bool = 0}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_list_t *ret_list = NULL;    
    mp_obj_list_t *out = NULL;
    mp_int_t index = args[ARG_index].u_int;

    if(index <= 0){
        for(int i = 0; i < km->outputs; i++){
            kpu_get_output(km->kmodel_ctx, i, (uint8_t **)&(km->output[i]), &(km->output_size[i]));
        }  

        if(args[ARG_getlist].u_bool){
            ret_list = m_new(mp_obj_list_t, 1);
            mp_obj_list_init(ret_list, 0);

            for(int i = 0; i < km->outputs; i++){
                out = m_new(mp_obj_list_t, 1);
                mp_obj_list_init(out, 0);
                for(int j = 0; j < (km->output_size[i])/sizeof(float); j++)
                {
                    mp_obj_list_append(out, mp_obj_new_float( ((float*)km->output[i])[j] ));
                }
                mp_obj_list_append(ret_list, out);
            }  
        }            
    }
    else{
        if(index > km->outputs)
        {
            mp_raise_ValueError("excess of output");
        }

        kpu_get_output(km->kmodel_ctx, index, (uint8_t **)&(km->output[index]), &(km->output_size[index]));
        if(args[ARG_getlist].u_bool){
            ret_list = m_new(mp_obj_list_t, 1);
            mp_obj_list_init(ret_list, 0);

            for(int j = 0; j < (km->output_size[index])/sizeof(float); j++){
                mp_obj_list_append(ret_list, mp_obj_new_float( ((float*)km->output[index])[j] ));
            }
        }       
    }
    
    if(args[ARG_getlist].u_bool){
        return MP_OBJ_FROM_PTR(ret_list);
    }  
    else{
        return mp_const_none;
    }
    
}
// STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_kpu_get_output_obj, py_kpu_get_output);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_kpu_get_output_obj, 1, py_kpu_get_output);

STATIC void k210_kpu_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    k210_kpu_obj_t *km = (k210_kpu_obj_t *)self_in;
    mp_printf(print, "type        : kmodel (KPU) \n");
    mp_printf(print, "model_size  : %d\n", km->model_size);
    mp_printf(print, "inputs      : %d\n", km->inputs);
    mp_printf(print, "outputs     : %d\n", km->outputs);

}

STATIC const mp_rom_map_elem_t k210_kpu_locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR_load_kmodel), MP_ROM_PTR(&py_kpu_load_kmodel_obj)},
    {MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&py_kpu_run_obj)},
    {MP_ROM_QSTR(MP_QSTR_get_output), MP_ROM_PTR(&py_kpu_get_output_obj)},
    {MP_ROM_QSTR(MP_QSTR_regionlayer), MP_ROM_PTR(&py_regionlayer_obj)},
    {MP_ROM_QSTR(MP_QSTR_regionlayer_init), MP_ROM_PTR(&py_regionlayer_init_obj)},

};
STATIC MP_DEFINE_CONST_DICT(k210_kpu_dict, k210_kpu_locals_dict_table);

const mp_obj_type_t k210_kpu_type = {
    {&mp_type_type},
    .name = MP_QSTR_KPU,
    .print = k210_kpu_print,
    .make_new = k210_kpu_make_new,
    .locals_dict = (mp_obj_dict_t *)&k210_kpu_dict,
};

#endif
