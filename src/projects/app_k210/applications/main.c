/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/09/30     Bernard      The first version
 */

#include <rtthread.h>
#include <stdio.h>

static  void    py_main(void);
static  int32_t msh_wait(uint32_t no);

#define msh_jmp(no) { if(msh_wait(no)) return 0;}


int main(void)
{
    msh_jmp(3);
    
    #if   defined(PKG_USING_OPENMV_CP)
    rt_kprintf("\n**openmv mode***\n");
    omv_main();
    #elif defined(PKG_USING_MICROPYTHON)
    rt_kprintf("\n**python mode***\n");
    py_main();
    #endif

    return 0;
}

#if defined(PKG_USING_MICROPYTHON)
void    py_main(void)
{
    while(1) {

        mpy_main(NULL);   
    }
}
#endif

volatile int    msh_run = 0;

static int32_t msh_wait(uint32_t no)
{
    int i;

    for(i=no; i>0; i--) {    
    
        rt_thread_delay(RT_TICK_PER_SECOND);          
        if(msh_run == 1) break;     
    }                                   
    return (i != 0);           
}

static  int    msh(int argc, char **argv)
{
    msh_run = 1;
    return 1;
}
MSH_CMD_EXPORT(msh, enter msh skip micropython);