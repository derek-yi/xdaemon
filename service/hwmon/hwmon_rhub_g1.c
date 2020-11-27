


#include "daemon_pub.h"

#include "hwmon_main.h"

#ifdef BOARD_RHUB_G1


/*************************************************************************
 * 重置检测点的配置
 *************************************************************************/
int hwmon_config_override()
{
    
#ifndef __ARM_ARCH
    hwmon_set_enable("cpu.occupy",          TRUE);
    hwmon_set_enable("mem.occupy",          TRUE);
#endif

    return VOS_OK;
}





#endif

