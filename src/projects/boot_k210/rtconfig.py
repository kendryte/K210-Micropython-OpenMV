import os

# toolchains options
ARCH        ='risc-v'
CPU         ='k210'
CROSS_TOOL  ='gcc'

if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')
else:
    RTT_ROOT = r'../../rt-thread'

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')

if  CROSS_TOOL == 'gcc':
    PLATFORM    = 'gcc'
    EXEC_PATH   = r'/opt/xpack-riscv-none-embed-gcc-8.3.0-1.9/bin'
else:
    print('Please make sure your toolchains is GNU GCC!')
    exit(0)

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

BUILD = 'debug'

if PLATFORM == 'gcc':
    # toolchains
    PREFIX  = 'riscv-none-embed-'
    CC      = PREFIX + 'gcc'
    CXX     = PREFIX + 'g++'
    AS      = PREFIX + 'gcc'
    AR      = PREFIX + 'ar'
    LINK    = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE    = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY  = PREFIX + 'objcopy'

    DEVICE  = ' -mcmodel=medany -march=rv64imafc -mabi=lp64f'
    CFLAGS  = DEVICE + ' -fno-common -ffunction-sections -fdata-sections -fstrict-volatile-bitfields  -Wno-multichar'
    AFLAGS  = ' -c' + DEVICE + ' -x assembler-with-cpp -fpic'
    LFLAGS  = DEVICE + ' -nostartfiles -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,_start -T link.lds'
    CPATH   = ''
    LPATH   = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -ggdb'
        AFLAGS += ' -ggdb'
    else:
        CFLAGS += ' -O2 -Os'

    CXXFLAGS = CFLAGS + ' -std=gnu++17 -Wno-multichar'

DUMP_ACTION = OBJDUMP + ' -D -S $TARGET > rtt.asm\n'
POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'

POST_ACTION += AS + ' -o boot0.o' + AFLAGS + ' boot0.S\n' 
POST_ACTION += LINK + ' -o $TARGET' + DEVICE + ' -nostartfiles -Wl,--gc-sections,-Map=rtthread.map,-cref,-u,_start -T link_boot0.lds boot0.o\n'
POST_ACTION += OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'


def dist_handle(BSP_ROOT, dist_dir):
    from mkdist import bsp_copy_files

    src = os.path.join(BSP_ROOT, '../omv-kd233/driver')
    dst = os.path.join(dist_dir, 'driver')
    bsp_copy_files(src, dst)

    src = os.path.join(BSP_ROOT, '../omv-kd233/kendryte-sdk')
    dst = os.path.join(dist_dir, 'kendryte-sdk')
    bsp_copy_files(src, dst)
