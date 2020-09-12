
#include "daemon_pub.h"

#include "xlog.h"

#include <stdarg.h>




int xlog_config(char *path, int max_file_size)
{
    return 0;
}

/* 需求
1，存放目录/var/log/oran_daemon
2，减少文件系统读写，使用buffer
3，按等级分开存放 xlog.hwmon.log, xlog.error.log,..
4，每次调用，头部增加时间，尾部增加换行
5，每日指定时间压缩备份，oran_daemon_log_2020xxyy.tar.gz，并清空xlog.xx.log文件
6，避免打印过多
*/
int xlog(int level, char *format, ...)
{
   va_list args;
   
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
   fflush(stdout);
   return 0;
}


