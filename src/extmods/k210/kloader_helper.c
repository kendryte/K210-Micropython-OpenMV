#include <dfs_posix.h>
#include <stdint.h>
#include "fpioa.h"
#include "spi.h"
#include "sysctl.h"
#include "dmac.h"
/* clang-format off */
#define DATALENGTH                          8
#define FAST_READ_QUAL_IO                   0xEB
/**
 * @brief      w25qxx operating status enumerate
 */
typedef enum _w25qxx_status
{
    W25QXX_OK = 0,
    W25QXX_BUSY,
    W25QXX_ERROR,
} w25qxx_status_t;


static uint32_t spi_bus_no = 0;
static uint32_t spi_chip_select = 0;
static w25qxx_status_t __w25qxx_init(uint8_t spi_index, uint8_t spi_ss);
static w25qxx_status_t __w25qxx_read_data(uint32_t addr, uint8_t *data_buf, uint32_t length);

static w25qxx_status_t _w25qxx_receive_data_enhanced_dma(uint32_t *cmd_buff, uint8_t cmd_len, uint8_t *rx_buff, uint32_t rx_len)
{
    spi_receive_data_multiple_dma(DMAC_CHANNEL0, DMAC_CHANNEL1, spi_bus_no, spi_chip_select, cmd_buff, cmd_len, rx_buff, rx_len);
    return W25QXX_OK;
}

static w25qxx_status_t __w25qxx_init(uint8_t spi_index, uint8_t spi_ss)
{
    spi_bus_no = spi_index;
    spi_chip_select = spi_ss;
    spi_init(spi_bus_no, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
    spi_set_clk_rate(spi_bus_no, 25000000);
    // w25qxx_page_program_fun = w25qxx_page_program;
    // w25qxx_read_fun = w25qxx_stand_read_data;
    return W25QXX_OK;
}

static w25qxx_status_t ___w25qxx_read_data(uint32_t addr, uint8_t *data_buf, uint32_t length)
{
    uint32_t cmd[2] = {0};
    cmd[0] = FAST_READ_QUAL_IO;
    cmd[1] = addr << 8;
    spi_init(spi_bus_no, SPI_WORK_MODE_0, SPI_FF_QUAD, DATALENGTH, 0);
    spi_init_non_standard(spi_bus_no, 8/*instrction length*/, 32/*address length*/, 4/*wait cycles*/,
                            SPI_AITM_ADDR_STANDARD/*spi address trans mode*/);
    _w25qxx_receive_data_enhanced_dma(cmd, 2, data_buf, length);

    return W25QXX_OK;
}
static w25qxx_status_t __w25qxx_read_data(uint32_t addr, uint8_t *data_buf, uint32_t length)
{
    uint32_t len = 0;

    while (length)
    {
        len = ((length >= 0x010000) ? 0x010000 : length);
        ___w25qxx_read_data(addr, data_buf, len);
        addr += len;
        data_buf += len;
        length -= len;
    }
    return W25QXX_OK;
}

int load_file_from_flash(uint32_t addr, uint8_t *data_buf, uint32_t length){
    __w25qxx_init(3, 0);
    __w25qxx_read_data(addr, data_buf, length);
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
