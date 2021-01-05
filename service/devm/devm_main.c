
#include "daemon_pub.h"

#include "devm_main.h"

#if T_DESC("cmd", 1)

int cli_sys_cfg_list(int argc, char **argv)
{   
    DYN_CFG_S *dyn_cfg;

    vos_print("top_conf             : %s\r\n",      sys_conf.top_conf);
    vos_print("drv_conf             : %s\r\n",      sys_conf.drv_conf);
    vos_print("hwmon_conf           : %s\r\n",      sys_conf.hwmon_conf);
    vos_print("devm_conf            : %s\r\n",      sys_conf.devm_conf);
    vos_print("upcfg_conf           : %s\r\n",      sys_conf.upcfg_conf);
    vos_print("customer_name        : %s\r\n",      sys_conf.customer_name);
    vos_print("customer_id          : %d\r\n",      sys_conf.customer_id);
    vos_print("\r\n");
    
    dyn_cfg = sys_conf.dyn_cfg;
    while (dyn_cfg) {
        vos_print("%-20s : %s\r\n", dyn_cfg->cfg_str, dyn_cfg->cfg_val);
        dyn_cfg = dyn_cfg->next;
    }
    
    return VOS_OK;
}

int cli_sys_cfg_set(int argc, char **argv)
{
    if (argc < 3) {
        vos_print("usage: %s <key> <value> \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    sys_conf_set(argv[1], argv[2]);
    return VOS_OK;
}

int cli_sys_cfg_store(int argc, char **argv)
{
    if ( daemon_store_script(DEF_RUNNING_CFG) != VOS_OK) {
        vos_print("Failed to save sys config\r\n");
    }
    return VOS_OK;
}

int cli_sys_cfg_clear(int argc, char **argv)
{
    unlink(DEF_RUNNING_CFG);    
    return VOS_OK;
}

int devm_cmd_reg()
{
    cli_cmd_reg("fru_show",         "show fru info",                &cli_show_fru_info);
    cli_cmd_reg("cfg_list",         "show sys conf",                &cli_sys_cfg_list);
    
#ifndef DAEMON_RELEASE
    cli_cmd_reg("fru_set_mac",      "set board mac addr",           &cli_fru_set_mac);
    cli_cmd_reg("fru_set_uuid",     "set board UUID",               &cli_fru_set_uuid);
    cli_cmd_reg("fru_set_sku",      "set board SKU ID",             &cli_fru_set_skuid);
    cli_cmd_reg("fru_set_rf_cal",   "set RF calibration param",     &cli_fru_set_rf_calibration);
    cli_cmd_reg("fru_set_sn",       "set board sn",                 &cli_fru_set_sn);
    cli_cmd_reg("fru_load",         "load json fru",                &cli_fru_load_json);

    cli_cmd_reg("cfg_set",          "set sys conf",                 &cli_sys_cfg_set);
    cli_cmd_reg("cfg_save",         "save sys conf",                &cli_sys_cfg_store);
    cli_cmd_reg("cfg_clear",        "reset to init conf",           &cli_sys_cfg_clear);
#endif

    return VOS_OK;
}
#endif

#if T_DESC("init_script", 1)

int devm_load_script(char *file_name)
{
    return VOS_OK;
}

#endif

#if T_DESC("main", 1)

void* devm_main_task(void *param)  
{
    //add irregular function in main loop
    while(1) {
        if (sys_conf_geti("devm_task_disable")) {
            vos_msleep(100);
            continue;
        }

        //todo

        vos_msleep(100);
    }
    
    return NULL;
}

TIMER_INFO_S devm_timer_list[] = 
{
    {0, 10, 0, NULL, NULL}, 
};

int devm_timer_callback(void *param)
{
    static uint32 timer_cnt = 0;
    
    if (sys_conf_geti("devm_timer_disable")) {
        return VOS_OK;
    }
    
    timer_cnt++;
    for (int i = 0; i < sizeof(devm_timer_list)/sizeof(TIMER_INFO_S); i++) {
        if ( (devm_timer_list[i].enable) && (timer_cnt%devm_timer_list[i].interval == 0) ) {
            devm_timer_list[i].run_cnt++;
            if (devm_timer_list[i].cb_func) {
                devm_timer_list[i].cb_func(devm_timer_list[i].cookie);
            }
        }
    }
    
    return VOS_OK;
}

int devm_init(char *cfg_file)
{
    int ret = VOS_OK;
    pthread_t threadid;
    timer_t timer_id;
    
    //load cfg script
    xlog(XLOG_INFO, "devm_init: %s", cfg_file);
    devm_load_script(cfg_file);

    //cmd reg
    devm_cmd_reg();
    devm_import_fru_info();

    ret = pthread_create(&threadid, NULL, devm_main_task, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "pthread_create failed(%s)", strerror(errno));
        return VOS_ERR;  
    } 

    ret = vos_create_timer(&timer_id, 1, devm_timer_callback, NULL);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "vos_create_timer failed");
        return -1;  
    } 

    return ret;    
}

int devm_exit()
{
    return 0;
}

#endif
