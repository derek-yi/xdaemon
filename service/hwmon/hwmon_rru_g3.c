
#include "daemon_pub.h"

#include "hwmon_common.h"

#ifdef BOARD_RRU_G3


/*************************************************************************
 * 重置检测点的配置
 *************************************************************************/
int hwmon_config_override()
{
    CHK_NODE_CFG_S new_node;

    memset(&new_node, 0, sizeof(CHK_NODE_CFG_S));
    new_node.node_desc  = "talise.isr.chk";
    new_node.func_name  = "adrv9009_isr_check";
    new_node.interval   = 10;
    new_node.repeat_max = 1;
    hwmon_register(&new_node);
    
#ifndef __ARM_ARCH
    hwmon_set_enable("cpu.occupy",          TRUE);
    hwmon_set_enable("mem.occupy",          TRUE);
#endif

    return VOS_OK;
}





#endif

