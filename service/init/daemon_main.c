

#include "daemon_pub.h"

#include "drv_main.h"
#include "hwmon_main.h"
#include "devm_main.h"
#include "devm_msg.h"
#include "upcfg_main.h"

#if 1

int daemon_module_init()
{
    int ret = 0;

    ret = drv_module_init(DRV_CFG_FILE);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "drv_module_init failed!\n");  
        return -1;  
    }     

    ret = devm_msg_init(APP_ORAN_DAEMON);
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

    ret = upcfg_init(NULL);
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

//https://tenfy.cn/2017/09/16/only-one-instance/
int single_instance_check(char *file_path)
{
    int fd = open(file_path, O_WRONLY|O_CREAT);
    if (fd < 0) {
        printf("open foo.lock failed\n");
        return VOS_ERR;
    }

    int err = flock(fd, LOCK_EX|LOCK_NB);
    if (err == -1) {
        printf("lock failed\n");
        return VOS_ERR;
    }

    printf("lock success\n");
    //flock(fd, LOCK_UN);

    return VOS_OK;
}

extern int telnet_task_init(void);

int main(int argc, char **argv)
{
    struct sigaction sa;

    if (single_instance_check("./.applock") != VOS_OK) {
        return VOS_ERR;
    }

	printf("hello, %s\n", argv[0]);
    cli_cmd_init();
    daemon_module_init();

    signal(SIGPIPE, SIG_IGN); //socket reset
    
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_signal;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        xlog(XLOG_ERROR, "Error: cannot handle SIGINT"); // Should not happen
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        xlog(XLOG_ERROR, "Error: cannot handle SIGINT"); // Should not happen
    }
    
#ifdef INCLUDE_TELNETD
    telnet_task_init();
#endif    

#ifdef INCLUDE_CONSOLE
    cli_main_task();
#else   
    while(1) sleep(1);
#endif    
	return 0; // Should not happen
}

#endif

