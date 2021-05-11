#include <dfs_posix.h>
#include <stdint.h>
#include "drv_qspiflash.h"

int load_file_from_flash(uint32_t addr, uint8_t *data_buf, uint32_t length){
    w25qxx_init(3, 0, 25000000);
    w25qxx_read_data(addr, data_buf, length);
    return 0;
}

int load_file_from_ff(const char *path, void* buffer, size_t model_size){
 
    int fd ;
    size_t rdbytes= 0;
    fd = open(path,O_RDONLY);
    if(fd <= -1)
    {
        return -1;
    }
   
    rdbytes = read(fd, buffer, model_size);
    close(fd);
    if(rdbytes != model_size)
    {
        return -2;
    }                
    return 0;
}
