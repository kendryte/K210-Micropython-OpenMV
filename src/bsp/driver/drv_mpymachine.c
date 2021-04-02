#include <sysctl.h>

int mp_port_get_freq(int clkid, int *freq)
{
    int ret = 0;
    uint32_t value;

    switch (clkid)
    {
    case 0:
        value = sysctl_clock_get_freq(SYSCTL_CLOCK_CPU);
        break;
    case 1:
        value = sysctl_clock_get_freq(SYSCTL_CLOCK_PLL1);
        break;
    case 2:
        value = sysctl_clock_get_freq(SYSCTL_CLOCK_PLL2);
        break;
    default:
        ret = -1;
        break;
    }

    *freq = (int)value;

    return ret;
}
