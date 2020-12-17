
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
    {0, NULL, NULL}, 
};

void* upcfg_main_task(void *param)  
{
    TIMER_INFO_S *tc;
    timer_t timer_id;

    for (int i = 0; i < sizeof(up_timer_list)/sizeof(TIMER_INFO_S); i++) {
        tc = &up_timer_list[i];
        if ( (tc->interval > 0) && (tc->cb_func != NULL) ) {
            if (vos_create_timer(&timer_id, tc->interval, tc->cb_func, tc->cookie) != VOS_OK)  {  
                xlog(XLOG_ERROR, "vos_create_timer failed");
                //return NULL;  
            } 
        }
    }

    //add irregular function in main loop
    while(1) {
        if (sys_conf_geti("upcfg_task_disable")) {
            vos_msleep(100);
            continue;
        }

        //todo

        vos_msleep(500);
    }
    
    return NULL;
}

int upcfg_init(char *cfg_file)
{
    int ret = VOS_OK;
    pthread_t threadid;

    //load cfg script
    xlog(XLOG_INFO, "upcfg_init: %s", cfg_file);
    upcfg_load_script(cfg_file);
    upcfg_cli_init();

    ret = pthread_create(&threadid, NULL, upcfg_main_task, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "Error at %s:%d, pthread_create failed(%s)", __FILE__, __LINE__, strerror(errno));
        return -1;  
    } 

    return VOS_OK;
}

#endif
