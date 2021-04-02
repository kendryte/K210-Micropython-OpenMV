#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <stdio.h>

#include <rtthread.h>

#define JAN_1970 0x83aa7e80 /* 2208988800 1970 - 1900 in seconds */
#define NTP_PORT (123)

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) (4294 * (x) + ((1981 * (x)) >> 11))

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via settimeofday) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x) ((x)*15.2587890625)

struct ntptime
{
    unsigned int coarse;
    unsigned int fine;
};

static int send_packet(int usd)
{
    uint32_t data[12];
    struct timeval now;
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6

    memset((char *)data, 0, sizeof(data));
    data[0] = htonl(
        (LI << 30) | (VN << 27) | (MODE << 24) |
        (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
    data[1] = htonl(1 << 16); /* Root Delay (seconds) */
    data[2] = htonl(1 << 16); /* Root Dispersion (seconds) */
    gettimeofday(&now, NULL);
    data[10] = htonl(now.tv_sec + JAN_1970); /* Transmit Timestamp coarse */
    data[11] = htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */

    if (send(usd, data, 48, 0) != 48)
        return -1;

    return 0;
}

static int rfc1305(uint32_t *data, struct timeval *tv_set)
{
    tv_set->tv_sec = ntohl(((uint32_t *)data)[10]) - JAN_1970;
    tv_set->tv_usec = USEC(ntohl(((uint32_t *)data)[11]));

    return 0;
}

static int get_packet(const char *ns, uint16_t port, uint32_t packet[12])
{
    int ret = -1;
    int usd;
    struct sockaddr_in sa;
    struct hostent *he;
    fd_set fds;
    struct timeval tv;

    if ((he = gethostbyname(ns)) == NULL)
    {
        printf("get host fail\n");
        return -1;
    }

    if ((usd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("open socket fail\n");
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    memcpy(&sa.sin_addr, he->h_addr_list[0], sizeof(sa.sin_addr));
    sa.sin_port = htons(port);
    sa.sin_family = AF_INET;

    if (connect(usd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    {
        printf("connect server fail\n");
        goto _out;
    }

    if (send_packet(usd) != 0)
    {
        printf("send fail\n");
        goto _out;
    }

    FD_ZERO(&fds);
    FD_SET(usd, &fds);

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    memset(packet, 0, 48);
    if (select(usd + 1, &fds, NULL, NULL, &tv) != 1)
    {
        printf("wait recv fail\n");
        goto _out;
    }

    ret = (int)recv(usd, packet, 48, 0);
    if (ret != 48)
    {
        printf("recv(%d) fail\n", ret);
        ret = -1;
    }

_out:
    close(usd);

    return ret;
}

static int ntpd_gettime(const char *ns, unsigned short port, struct timeval *tv)
{
    int ret;
    uint32_t packet[12];

    if (get_packet(ns, port, packet) == -1)
        return -1;

    ret = rfc1305(packet, tv);

    return ret;
}

void ngtime(int argc, char *argv[])
{
    struct timeval tv;

    if (ntpd_gettime("ntp1.aliyun.com", 123, &tv) == 0)
    {
        printf("Time: %s\n", ctime(&tv.tv_sec));
    }
    else
    {
        printf("get time fail\n");
    }
}
MSH_CMD_EXPORT(ngtime, NTP get time);
