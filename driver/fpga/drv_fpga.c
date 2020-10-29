
#include "daemon_pub.h"
#include "drv_cpu.h"
#include "drv_fpga.h"



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

//cpri-r21-mul.sh
int drv_get_cpri_links(int *link_cnt)
{
    int cnt = 0;

    if (link_cnt == NULL) return VOS_ERR;
    
    for (int i = 0; i < MAX_CPRI_CNT; i++) {
        uint32 value = devmem_read(CPRI_REG_BASE + i*0x10000, AT_WORD);
        if (value & 0xF) cnt++;
    }

    *link_cnt = cnt;
    return VOS_OK;
}

//getversion
int drv_board_type()
{
    uint32 value = devmem_read(FPGA_VER_ADDRESS, AT_WORD);
    if ( (value & 0x01000000) == 0x01000000)
        return BOARD_TYPE_RHUB;
    return BOARD_TYPE_RRU;
}

