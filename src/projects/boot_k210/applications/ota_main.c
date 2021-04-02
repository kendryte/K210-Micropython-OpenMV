/*
 * File      : ota.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-27     MurphyZhao   the first version
 */
#include <rtthread.h>
#include <fal.h>
#include <rt_ota.h>
#include <string.h>

#define DBG_SECTION_NAME  "OTA"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

#define BOOT_OTA_DEBUG (1)
#define BOOT_PART_NAME  "bl"
#define DL_PART_NAME    "download"
#define APP_PART_NAME   "app"

#define CHECK_APP

int ota_main(void)
{
    int result = 0;
    const struct fal_partition *dl_part = NULL;
#ifdef CHECK_APP
    size_t i, part_table_size;
    const struct fal_partition *part_table = NULL;
#endif
    const char *dest_part_name = NULL;

    if (rt_ota_init() >= 0)
    {
        // 4. Check if the download partition exists
        dl_part = fal_partition_find(DL_PART_NAME);
        // OTA init already check download part
        /* 5. Check if the target partition name is bootloader, skip ota upgrade if yes */
        dest_part_name = rt_ota_get_fw_dest_part_name(dl_part);
        // if (dest_part_name && !strcmp(dest_part_name, BOOT_PART_NAME))
        // {
        //     LOG_E("Can not upgrade bootloader partition!");
        //     goto _app_check;
        // }

        /* do upgrade when check upgrade OK 
         * 5. CRC DL FW HDR
         * 6. Check if the dest partition exists
         * 7. CRC APP FW HDR
         * 8. Compare DL and APP HDR, containning fw version
         */
        LOG_I("check upgrade...");
        if ((result = rt_ota_check_upgrade()) == 1) // need to upgrade
        {
            if((rt_ota_get_fw_algo(dl_part) & RT_OTA_CRYPT_STAT_MASK) == RT_OTA_CRYPT_ALGO_NONE)
            {
                LOG_E("none encryption Not allow!");
                goto _app_check;
            }
            /* verify OTA download partition 
            * 9. CRC DL FW HDR
            * 10. CRC DL FW
            */
            if (rt_ota_part_fw_verify(dl_part) == 0)
            {
                // 11. rt_ota_custom_verify
                // 12. upgrade
                if (rt_ota_upgrade() < 0)
                {
                    LOG_E("OTA upgrade failed!");
                    /*
                     *  upgrade failed, goto app check. If success, jump to app to run, otherwise goto recovery factory firmware.
                     **/
                    goto _app_check;
                }
            }
            else
            {
                goto _app_check;
            }
        }
        else if (result == 0)
        {
            LOG_I("No firmware upgrade!");
        }
        else if (result == -1)
        {
            goto _app_check;
        }
        else
        {
            LOG_E("OTA upgrade failed!");
            return -1;
        }

_app_check:
#ifdef CHECK_APP
        part_table = fal_get_partition_table(&part_table_size);
        /* verify all partition */
        for (i = 0; i < part_table_size; i++)
        {
            /* ignore bootloader partition and OTA download partition */
            if (!strncmp(part_table[i].name, APP_PART_NAME, FAL_DEV_NAME_MAX))
            {
                // verify app firmware
                if (rt_ota_part_fw_verify(&part_table[i]) < 0)
                {
                    // TODO upgrade to safe image
                    LOG_E("App verify failed! Need to recovery factory firmware.");
                    return -1;
                }
                else
                {
                    result = 0;
                }
            }
        }
#else
        return 0;
#endif
    }
    else
    {
        result = -1;
    }

    return result;
}

int rt_ota_custom_verify(const struct fal_partition *cur_part, long offset, const uint8_t *buf, size_t len)
{
    return 0;
}
