
#include "daemon_pub.h"

#include "xlog.h"

#include <stdarg.h>


#define LOG_FILE    0x01
#define LOG_PRINT   0x02

typedef struct _XLOG_CTRL{
    int     fd;
    char    *file_name;
    int     flags;
}XLOG_CTRL_S;

//#define XLOG_KEEP_OPEN

XLOG_CTRL_S xlog_ctrl[XLOG_MAX] = 
{
    {-1, "xlog.info.log",   LOG_PRINT},
    {-1, "xlog.error.log",  LOG_FILE|LOG_PRINT},
    {-1, "xlog.hwmon.log",  LOG_FILE|LOG_PRINT},
    {-1, "xlog.devm.log",   LOG_FILE|LOG_PRINT},
    {-1, NULL, 0},
    {-1, NULL, 0},
    {-1, NULL, 0},
    {-1, NULL, 0},
};

//created by daemon_monitor.sh
#define DAEMON_DEF_LOG_PATH     "/tmp/xlog/"

int xlog_backup(int force)
{
    static int backup_done = FALSE;
    struct tm *tp;
    time_t t = time(NULL);
    char cmd_buff[64];

    tp = localtime(&t);
    if ( (tp->tm_hour == 4) && (backup_done == TRUE) ){
        backup_done = FALSE;
        return VOS_OK;
    }

    if ( ( (tp->tm_hour == 3) && (backup_done == FALSE) )
        || (force == TRUE)  ) {
        backup_done = TRUE;

        sprintf(cmd_buff, "tar -czf xlog%d%02d%02d.tar.gz %s/* >/dev/null 2>&1", 
                tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday, DAEMON_DEF_LOG_PATH);
        shell_run_cmd(cmd_buff);
        sprintf(cmd_buff, "rm -rf %s/* >/dev/null 2>&1", DAEMON_DEF_LOG_PATH);
        shell_run_cmd(cmd_buff);
    }
        
    return VOS_OK;
}

#define XLOG_BUFF_MAX           512

int xlog_get_fd(int level)
{
    int fd;
    char file_path[64];

#ifdef XLOG_KEEP_OPEN     
    if (xlog_ctrl[level].fd > 0) {
        return xlog_ctrl[level].fd;
    }
#endif

    sprintf(file_path, "%s%s", DAEMON_DEF_LOG_PATH, xlog_ctrl[level].file_name);
    fd = open(file_path, O_CREAT|O_RDWR, 0666);
    if (fd > 0) {
        xlog_ctrl[level].fd = fd;
        lseek(fd, 0, SEEK_END);
    }
    
    return fd;
}

int xlog_print_file(int level)
{
    FILE *fp;
    char temp_buf[XLOG_BUFF_MAX];
    const char *cp_file = "temp_file";
    
    if ( (level >= XLOG_MAX) || (xlog_ctrl[level].file_name == NULL) ) {
        fprintf(stderr, "ERROR: invalid id %d\n", level);
        return VOS_ERR;
    }

    sprintf(temp_buf, "cp -f %s%s %s", DAEMON_DEF_LOG_PATH, xlog_ctrl[level].file_name, cp_file);
    shell_run_cmd(temp_buf);
    
    fp = fopen(cp_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "open failed, %s\n", strerror(errno));
        unlink(cp_file);
        return VOS_ERR;
    }

    vos_print("%s: \r\n", xlog_ctrl[level].file_name);
    memset(temp_buf, 0, sizeof(temp_buf));
    while (fgets(temp_buf, XLOG_BUFF_MAX-1, fp) != NULL) {  
        vos_print("%s", temp_buf);
        memset(temp_buf, 0, sizeof(temp_buf));
    }

    fclose(fp);
    unlink(cp_file);
    return VOS_OK;
}


void fmt_time_str(char *time_str, int max_len)
{
    struct tm *tp;
    time_t t = time(NULL);
    tp = localtime(&t);
     
    if (!time_str) return ;
    
    snprintf(time_str, max_len, "[%04d-%02d-%02d %02d:%02d:%02d]", 
            tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
}


/* 需求
1，存放目录/tmp/oran_daemon        //done
2，减少文件系统读写，使用buffer //cancel, 存放内存文件系统
3，按等级分开存放 xlog.hwmon.log, xlog.error.log,.. //done
4，每次调用，头部增加时间，尾部增加换行 //done
5，每日指定时间压缩备份，oran_daemon_log_2020xxyy.tar.gz，并清空xlog.xx.log文件                 //todo
6，避免打印过多 //todo
*/
int xlog(int level, const char *format, ...)
{
    va_list args;
    char buf[XLOG_BUFF_MAX];
    char buf2[XLOG_BUFF_MAX+64];
    char time_str[64];
    int len;

    if (level >= XLOG_MAX) {
        return VOS_ERR;
    }
    
    va_start(args, format);
    len = vsnprintf(buf, XLOG_BUFF_MAX, format, args);
    va_end(args);

    fmt_time_str(time_str, 64);
    len = sprintf(buf2, "%s %s\r\n", time_str, buf);

    if (xlog_ctrl[level].flags & LOG_FILE) {
        int fd = xlog_get_fd(level);
        if (fd < 0) {
            return VOS_ERR;
        }
        write(fd, buf2, len);
        #ifndef XLOG_KEEP_OPEN
        close(fd);
        #endif
    }

    if (xlog_ctrl[level].flags & LOG_PRINT) {
        vos_print("%s", buf2);
    }

    //backup everyday
    xlog_backup(FALSE);

    return len;    
}



