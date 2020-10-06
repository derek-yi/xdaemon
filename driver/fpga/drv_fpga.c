
#include "daemon_pub.h"
#include "drv_cpu.h"



/* json demo:
{
    "cfg.param": {
    	"desc": "what to do"
    },
    "cfglist": [
        { "//note": "add comment here" },
    	{ "type": "read",  "param":[100, 200, 300] },
    	{ "type": "write", "param":[100, 200] },
    	{ "type": "write", "param":[100, 200] },
    	{ "type": "shell", "param":"shell cmd str", "expect": 0 },
        { "type": "write", "param":[100, 200] },
        { "type": "write", "param":[100, 200] },
        { "type": "sleep", "param":[100] }
    ]
}
*/


int drv_fpga_run_script(char *file_name)
{

return VOS_OK;
}


//bbu_rru_CT_to_cmcc.sh
int bbu_rru_CT_to_cmcc()
{
    //shell_run_file(file_name);
    return VOS_OK;
}
