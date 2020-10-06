
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

#define DAEMON_DEF_LOG_PATH     "/tmp/oran_daemon/"
#define XLOG_BUFF_MAX           512

int xlog_get_fd(int level)
{
    int fd;
    char file_path[64];
    
    if (xlog_ctrl[level].fd > 0) {
        return xlog_ctrl[level].fd;
    }

    sprintf(file_path, "%s%s", DAEMON_DEF_LOG_PATH, xlog_ctrl[level].file_name);
    fd = open(file_path, O_CREAT|O_RDWR);
    if (fd > 0) {
        xlog_ctrl[level].fd = fd;
    }
    
    return fd;
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
    len = vsnprintf(buf, XLOG_BUFF_MAX-1, format, args);
    va_end(args);

    fmt_time_str(time_str, 64);
    len = sprintf(buf2, "%s %s\n", time_str, buf);

    if (xlog_ctrl[level].flags & LOG_FILE) {
        int fd = xlog_get_fd(level);
        if (fd < 0) {
            return VOS_ERR;
        }
        write(fd, buf2, len);
    }
    
    if (xlog_ctrl[level].flags & LOG_PRINT) {
        printf("%s", buf2);
    }

    return len;    
}



