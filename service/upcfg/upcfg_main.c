
#include "daemon_pub.h"

#include "upcfg_main.h"

int upcfg_init(char *cfg_file)
{
    upcfg_cli_init();
    return VOS_OK;
}

