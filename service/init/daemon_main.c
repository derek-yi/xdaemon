

#include "daemon_pub.h"

#include "drv_main.h"
#include "hwmon_common.h"
#include "devm_common.h"
#include "devm_msg.h"


#if 1

int daemon_module_init()
{
    int ret = 0;

    ret = drv_module_init(DRV_CFG_FILE);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "drv_module_init failed!\n");  
        return -1;  
    }     

    ret = devm_msg_init(NULL);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "devm_msg_init failed!\n");  
        return -1;  
    } 

    ret = hwmon_init(HWMON_CFG_FILE);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "hwmon_init failed!\n");  
        return -1;  
    } 

    ret = devm_init(DEVM_CFG_FILE);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "hwmon_init failed!\n");  
        return -1;  
    } 

    return 0;
}

/* 该函数用于处理进程退出不会被释放的资源 //todo
参考：进程退出，哪些资源会释放
1、动态申请的内存--yes
2、创建的线程--yes
3、创建的定时器
4、信号量
5、管道
6、socket
7、打开的文件
 */
void daemon_module_exit()
{
    hwmon_exit();
    devm_exit();
}

#endif

#ifdef DEMO_THREAD_CODE //demo code

pthread_t threadid_demo;

void* thread_main_demo(void *param)  
{  
    int my_var = 0;

    for(;;)  {
        printf("pid=0x%x my_var=%d my_var_addr=0x%x \n", getpid(), my_var, &my_var);
        my_var += 1;
        sleep(300);  
    }  
    pthread_exit(0);  
    return NULL;
}  


void timer_callback_demo(union sigval param)
{
    // 定时器回调函数应该简单处理
	printf("timer_callback_demo\n");
}


#endif

#if 1


void handle_signal(int signal)
{
    if( (signal == SIGTERM) || (signal == SIGINT)) {
        xlog(XLOG_ERROR, "Caught SIGTERM, exiting now\n");
        daemon_module_exit();
        exit(0);
    }
    xlog(XLOG_ERROR, "Caught wrong signal: %d\n", signal);
}


int main(int argc, char **argv)
{
    struct sigaction sa;

	printf("hello, %s\n", argv[0]);
    daemon_module_init();

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_signal;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        xlog(XLOG_ERROR, "Error: cannot handle SIGINT"); // Should not happen
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        xlog(XLOG_ERROR, "Error: cannot handle SIGINT"); // Should not happen
    }
    
#ifdef DEMO_THREAD_CODE    
    int ret = vos_create_timer(NULL, 60, timer_callback_demo, NULL);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "vos_create_timer failed!\n");  
        return -1;  
    } 

    ret = pthread_create(&threadid_demo, NULL, thread_main_demo, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "pthread_create failed!\n");  
        return -1;  
    } 
    //pthread_join(threadid_demo, NULL);  
#endif

    //must be the last, it is deadloop
    cli_main_task();
    
	return 0; // Should not happen
}

#endif

