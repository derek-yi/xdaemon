
#include "daemon_pub.h"

#include "devm_main.h"

int devm_cmd_reg()
{
    cli_cmd_reg("fru_get",          "show fru info",                &cli_show_fru_info);
    cli_cmd_reg("fru_set_mac",      "set fru mac addr",             &cli_fru_set_mac);
    cli_cmd_reg("fru_set_uuid",     "set fru UUID",                 &cli_fru_set_uuid);
    cli_cmd_reg("fru_set_sku",      "set fru SKU ID",               &cli_fru_set_skuid);
    cli_cmd_reg("fru_set_rf_cal",   "set RF calibration param",     &cli_fru_set_rf_calibration);

    return VOS_OK;
}

int devm_load_script(char *file_name)
{
    return VOS_OK;
}

int devm_init(char *file_name)
{
    int ret = VOS_OK;
    
    //load cfg script
    devm_load_script(file_name);

    //cmd reg
    devm_cmd_reg();

#ifndef DAEMON_RELEASE
    devm_store_fru_info(FALSE);
#endif

    devm_load_fru_info(0);
    devm_load_fru_info(1);
    devm_import_fru_info();
    
    return ret;    
}

int devm_exit()
{
    return 0;
}

