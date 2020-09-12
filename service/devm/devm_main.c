
#include "daemon_pub.h"

#include "devm_common.h"

int devm_cmd_reg()
{
    return VOS_OK;
}

int devm_load_script(char *file_name)
{
    return VOS_OK;
}

int devm_init(char *file_name)
{
    int ret;
    
    //load cfg script
    devm_load_script(file_name);

    //cmd reg
    devm_cmd_reg();
    
    return 0;    
}

int devm_exit()
{
    return 0;
}

