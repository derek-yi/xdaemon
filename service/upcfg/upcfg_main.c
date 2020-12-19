
#include "daemon_pub.h"

#include "devm_main.h"
#include "upcfg_main.h"

#if T_DESC("init_script", 1)

int upcfg_load_script(char *file_name)
{
    return VOS_OK;
}

#endif

#if T_DESC("main", 1)

TIMER_INFO_S up_timer_list[] = 
{ 
    {0, 0, 0, NULL, NULL}, 
};

int upcfg_timer_callback(void *param)
{
    static uint32 timer_cnt = 0;
    
    if (sys_conf_geti("upcfg_timer_disable")) {
        return VOS_OK;
    }
    
    timer_cnt++;
    for (int i = 0; i < sizeof(up_timer_list)/sizeof(TIMER_INFO_S); i++) {
        if ( (up_timer_list[i].enable) && (timer_cnt%up_timer_list[i].interval == 0) ) {
            up_timer_list[i].run_cnt++;
            if (up_timer_list[i].cb_func) {
                up_timer_list[i].cb_func(up_timer_list[i].cookie);
            }
        }
    }
    
    return VOS_OK;
}

void* upcfg_main_task(void *param)  
{
    //add irregular function in main loop
    while(1) {
        if (sys_conf_geti("upcfg_task_disable")) {
            vos_msleep(100);
            continue;
        }

        //todo

        vos_msleep(100);
    }
    
    return NULL;
}

int upcfg_init(char *cfg_file)
{
    int ret = VOS_OK;
    pthread_t threadid;
    timer_t timer_id;

    //load cfg script
    xlog(XLOG_INFO, "upcfg_init: %s", cfg_file);
    upcfg_load_script(cfg_file);
    upcfg_cli_init();

    ret = pthread_create(&threadid, NULL, upcfg_main_task, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "pthread_create failed(%s)", strerror(errno));
        return -1;  
    } 

    ret = vos_create_timer(&timer_id, 1, upcfg_timer_callback, NULL);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "vos_create_timer failed");
        return -1;  
    } 

    return VOS_OK;
}

#endif
