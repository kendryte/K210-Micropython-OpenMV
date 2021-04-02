#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Project Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_USING_SMP
#define RT_CPUS_NR 2
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 1024
#define RT_DEBUG
#define RT_DEBUG_COLOR
#define RT_DEBUG_INIT_CONFIG
#define RT_DEBUG_INIT 1

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uarths"
#define RT_VER_NUM 0x40003
#define ARCH_CPU_64BIT
#define ARCH_RISCV
#define ARCH_RISCV_FPU
#define ARCH_RISCV_FPU_S
#define ARCH_RISCV64

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 8192
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */

#define RT_USING_CPLUSPLUS

/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 16384
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 10

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 4
#define DFS_FILESYSTEM_TYPES_MAX 4
#define DFS_FD_MAX 64
#define RT_USING_DFS_ELMFAT

/* elm-chan's FatFs, Generic FAT Filesystem Module */

#define RT_DFS_ELM_CODE_PAGE 437
#define RT_DFS_ELM_WORD_ACCESS
#define RT_DFS_ELM_USE_LFN_3
#define RT_DFS_ELM_USE_LFN 3
#define RT_DFS_ELM_MAX_LFN 255
#define RT_DFS_ELM_DRIVES 2
#define RT_DFS_ELM_MAX_SECTOR_SIZE 4096
#define RT_DFS_ELM_REENTRANT
#define RT_USING_DFS_DEVFS

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SERIAL
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_PIN
#define RT_USING_RTC
#define RT_USING_SOFT_RTC
#define RT_USING_SPI

/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_POSIX

/* Network */

/* Socket abstraction layer */


/* Network interface device */


/* light weight TCP/IP stack */


/* AT commands */


/* VBUS(Virtual Software BUS) */


/* Utilities */

#define RT_USING_ULOG
#define ULOG_OUTPUT_LVL_D
#define ULOG_OUTPUT_LVL 7
#define ULOG_USING_ISR_LOG
#define ULOG_ASSERT_ENABLE
#define ULOG_LINE_BUF_SIZE 128

/* log format */

#define ULOG_USING_COLOR
#define ULOG_OUTPUT_TIME
#define ULOG_OUTPUT_LEVEL
#define ULOG_OUTPUT_TAG
#define ULOG_BACKEND_USING_CONSOLE

/* RT-Thread online packages */

/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */


/* IoT Cloud */


/* security packages */


/* language packages */

#define PKG_USING_MICROPYTHON

/* Hardware Module */


/* System Module */

#define MICROPYTHON_USING_UOS
#define MICROPYTHON_USING_FILE_SYNC_VIA_IDE

/* Tools Module */


/* Network Module */


/* User Extended Module */

#define MICROPYTHON_USING_USEREXTMODS
#define PKG_MICROPYTHON_HEAP_SIZE 1500000
#define PKG_USING_MICROPYTHON_LATEST_VERSION
#define PKG_MICROPYTHON_VER_NUM 0x99999

/* multimedia packages */


/* tools packages */


/* system packages */

#define PKG_USING_FAL
#define FAL_DEBUG_CONFIG
#define FAL_DEBUG 1
#define FAL_PART_HAS_TABLE_CFG
#define PKG_USING_FAL_LATEST_VERSION
#define PKG_FAL_VER_NUM 0x99999

/* Micrium: Micrium software products porting for RT-Thread */


/* peripheral libraries and drivers */


/* miscellaneous packages */


/* samples: kernel and components samples */


/* games: games run on RT-Thread console */

#define BOARD_K210_EVB

/* Hardware Drivers */

/* IO Groups Power Supply Settings */

#define BSP_GROUPA_POWER_SUPPLY_1V8
#define BSP_GROUPB_POWER_SUPPLY_3V3
#define BSP_GROUPC_POWER_SUPPLY_3V3
#define BSP_USING_UART_HS

/* General Purpose UARTs */

#define BSP_USING_LCD
#define BSP_LCD_CS_PIN 6
#define BSP_LCD_WR_PIN 7
#define BSP_LCD_DC_PIN 8
#define BSP_LCD_RST_PIN -1
#define BSP_LCD_BACKLIGHT_PIN -1
#define BSP_LCD_BACKLIGHT_ACTIVE_LOW
#define BSP_LCD_CLK_FREQ 18000000
#define BSP_BOARD_K210_OPENMV_TEST
#define BSP_LCD_X_MAX 240
#define BSP_LCD_Y_MAX 320
#define BSP_USING_CAMERA
#define BSP_CAMERA_SCCB_SDA0_PIN 13
#define BSP_CAMERA_SCCB_SDA_PIN 9
#define BSP_CAMERA_SCCB_SCLK_PIN 10
#define BSP_CAMERA_CMOS_RST_PIN 11
#define BSP_CAMERA_CMOS_VSYNC_PIN 12
#define BSP_CAMERA_CMOS_PWDN_PIN 13
#define BSP_CAMERA_CMOS_XCLK_PIN 14
#define BSP_CAMERA_CMOS_PCLK_PIN 15
#define BSP_CAMERA_CMOS_HREF_PIN 17
#define BSP_CAMERA_CMOS_PWDN_PIN_GC0328 11
#define __STACKSIZE__ 4096

/* OpenMV */

#define PKG_USING_OPENMV_CP
#define OMV_BOARD_KD233
#define OMV_HAL_K210
#define OMV_RAW_BUF_SIZE 230400

/* extmods */


#endif
