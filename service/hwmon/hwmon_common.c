
#include "daemon_pub.h"

#include "hwmon_common.h"





int check_cpu_occupy(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    char *priv_cfg;
    int top_limit = 80;
    int cpu_usage;

    //method1
	priv_cfg = hwmon_get_priv_cfg(&node->base_cfg, "top_limit");
    if (priv_cfg) {
        top_limit = atoi(priv_cfg);
    }

    //method2
    top_limit = node->base_cfg.param1;

	cpu_usage = 80; // todo, get real usage
	
    if ( cpu_usage > top_limit ) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            node->fault_state = TRUE;
            node->fault_cnt = 0;
            xlog(XLOG_HWMON, "cpu occupy warning, cpu usage %d", cpu_usage);
        }
    } else {
        node->fault_state = FALSE;
        node->fault_cnt = 0;
    }

	return 0;
}



int hwmon_config_list()
{
    hwmon_config("cpu.occupy", check_cpu_occupy, NULL);

    return 0;
}


