
#include "daemon_pub.h"

#include "devm_main.h"

#if T_DESC("cmd", 1)

int cli_show_sys_conf(int argc, char **argv)
{   
    DYN_CFG_S *dyn_cfg;

    vos_print("drv_conf         : %s\r\n",      sys_conf.drv_conf);
    vos_print("hwmon_conf       : %s\r\n",      sys_conf.hwmon_conf);
    vos_print("devm_conf        : %s\r\n",      sys_conf.devm_conf);
    vos_print("upcfg_conf       : %s\r\n",      sys_conf.upcfg_conf);
    vos_print("MAX_FRUID        : %d\r\n",      sys_conf.MAX_FRUID);
    vos_print("MAX_RF_CHIP      : %d\r\n",      sys_conf.MAX_RF_CHIP);
    vos_print("customer_name    : %s\r\n",      sys_conf.customer_name);
    vos_print("customer_id      : %d\r\n",      sys_conf.customer_id);

    vos_print("\r\n");
    dyn_cfg = sys_conf.dyn_cfg;
    while (dyn_cfg) {
        vos_print("%-16s : %s\r\n", dyn_cfg->cfg_str, dyn_cfg->cfg_val);
        dyn_cfg = dyn_cfg->next;
    }
    
    return VOS_OK;
}


int devm_cmd_reg()
{
    cli_cmd_reg("fru_get",          "show fru info",                &cli_show_fru_info);
    
#ifndef DAEMON_RELEASE
    cli_cmd_reg("fru_set_mac",      "set fru mac addr",             &cli_fru_set_mac);
    cli_cmd_reg("fru_set_uuid",     "set fru UUID",                 &cli_fru_set_uuid);
    cli_cmd_reg("fru_set_sku",      "set fru SKU ID",               &cli_fru_set_skuid);
    cli_cmd_reg("fru_set_rf_cal",   "set RF calibration param",     &cli_fru_set_rf_calibration);
    cli_cmd_reg("fru_load",         "load json fru",                &cli_fru_load_json);
    cli_cmd_reg("sys_conf",         "show sys conf",                &cli_show_sys_conf);
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

int devm_task_enable = TRUE;
int devm_peak_time = 0;

void* devm_main_task(void *param)  
{
    struct timeval t_start, t_end;
    uint32 delay_ms;

    while(1) {
        if (devm_task_enable != TRUE) {
            vos_msleep(100);
            continue;
        }
        gettimeofday(&t_start, NULL);

        //todo

        gettimeofday(&t_end, NULL);
        delay_ms = (t_end.tv_sec - t_start.tv_sec)*1000000+(t_end.tv_usec - t_start.tv_usec);//us
        delay_ms = delay_ms/1000; //ms
        if (devm_peak_time < delay_ms) devm_peak_time = delay_ms;
        vos_msleep(500);
    }
    
    return NULL;
}

void devm_timer_callback(union sigval param)
{
    static uint32 loop_cnt = 0;
    
    if (devm_task_enable != TRUE) {
        return ;
    }
    
    // 定时器回调函数应该简单处理
    //if(loop_cnt %3) do_sth();
    
    loop_cnt++;
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

    devm_read_fru_info(0);
    devm_read_fru_info(1);
    devm_import_fru_info();

    ret = pthread_create(&threadid, NULL, devm_main_task, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "Error at %s:%d, pthread_create failed(%s)", __FILE__, __LINE__, strerror(errno));
        return -1;  
    } 

    ret = vos_create_timer(&timer_id, 1, devm_timer_callback, NULL);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "Error at %s:%d, vos_create_timer failed", __FILE__, __LINE__);
        return -1;  
    } 
    
    return ret;    
}

int devm_exit()
{
    return 0;
}
#endif
