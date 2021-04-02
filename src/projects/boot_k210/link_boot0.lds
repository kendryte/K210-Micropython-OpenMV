/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

/*
 * The OUTPUT_ARCH command specifies the machine architecture where the
 * argument is one of the names used in the Kendryte library.
 */
OUTPUT_ARCH( "riscv" )

MEMORY
{
   /* 6M SRAM */
   SRAM : ORIGIN = 0x80000000, LENGTH = 0x600000
}

ENTRY(copy_start)
SECTIONS
{
    . = 0x80000000 ;

    /* __STACKSIZE__ = 4096; */

    .copy_start :
    {
        *(.copy_start)
    } > SRAM

    .data : 
    {
        *(.data)
        *(.data.*)
    
        *(.data1)
        *(.data1.*)

        . = ALIGN(8);
        PROVIDE( __global_pointer$ = . + 0x800 );

        *(.sdata)
        *(.sdata.*)
    } > SRAM
}
