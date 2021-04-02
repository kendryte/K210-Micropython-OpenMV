#include <stdio.h>
#include <string.h>
static const uint8_t iv_table[16] = "usguJdWWUugadheA";
static const uint8_t key_table[32] = "qnbGbxjnxtwaPqjuntaZyQvZepywimdg";

/**
 * Get the decryption key & iv
 *
 * @param iv_buf initialization vector
 * @param key_buf aes key
 */
void rt_ota_get_iv_key(uint8_t * iv_buf, uint8_t * key_buf)
{
    memcpy(iv_buf, iv_table, sizeof(iv_table));
    memcpy(key_buf, key_table, sizeof(key_table));
}
