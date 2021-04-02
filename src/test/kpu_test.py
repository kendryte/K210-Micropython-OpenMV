# @fun: run:
# input : file path/ omv img
# dma   : default 5;

# @fun: regionlayer_init 
#   args        ： default
#  anchor_num   ： 3160
#  crood_num    ： 4
#  landm_num    ： 5
#  cls_num      ： 1
#  in_w         ： 320
#  in_h         ： 240
#  obj_thresh   ：  0.7
#  nms_thresh   ：  0.4
#  variances    ：  [0.1, 0.2]
#  max_num      ： 200

# @fun get_output:
# index 
#     0     : get all output (default); 
#     >0    : get index output;
# getlist   : True / False(default)

#demo : Mnist_cnn
#copy 1:
import lcd,sensor,image,k210, gc
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
lcd.init()
m = k210.KPU()
m.load_kmodel("/uint8_mnist_cnn_model.kmodel")
# m.load_kmodel(0xb00000,540*1024) #load from flash
m.run("/infer.bin")
out=m.get_output(getlist=True)
out[0].index(max(out[0]))

##########################################################
#demo : face_landmark
#copy 1:
import lcd,sensor,image,k210, gc
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
lcd.init()
m = k210.KPU()
m.load_kmodel("/ulffd_landmark.kmodel")
m.regionlayer_init(anchor_num=3160,crood_num=4,max_num=200)
m

#copy 2:
while(True):
gc.collect()
img=sensor.snapshot()
m.run(img)
m.get_output(0)
dect = m.regionlayer()
print("dect:",dect) 
for l in dect :
    img.draw_rectangle(l[0],l[1],l[2]-l[0],l[3]-l[1])

#copy 3: 
lcd.display(img)  #请注意缩进, 属于while代码内.
