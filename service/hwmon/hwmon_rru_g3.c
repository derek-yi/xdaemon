
#include "daemon_pub.h"

#include "hwmon_common.h"

#ifdef BOARD_RRU_G3


/*************************************************************************
 * 重置检测点的配置
 *************************************************************************/
int hwmon_config_override()
{
    //hwmon_config("cpu.occupy",              check_cpu_occupy,       NULL);
    //hwmon_config("9544.pll.lock",         check_9544_pll,         NULL);
    //hwmon_config("fpga.reg.check1",       fpga_reg_check,         NULL);
    //hwmon_config("fpga.reg.dump2",        fpga_reg_dump,         NULL);

#ifndef __ARM_ARCH
    hwmon_set_enable("cpu.occupy",          TRUE);
    hwmon_set_enable("mem.occupy",          TRUE);
#endif

    return VOS_OK;
}





#endif

