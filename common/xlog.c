
#include "daemon_pub.h"

#include "xlog.h"

#include <stdarg.h>
#include <syslog.h>

#define LOG_FILE                0x01
#define LOG_PRINT               0x02

#define INCLUDE_ZLOG

#define XLOG_BUFF_MAX           1024

#define DAEMON_DEF_LOG_PATH     "/var/log"

typedef struct _XLOG_CTRL{
    char    *file_name;
    int     flags;
    int     facility;
    int     level;
}XLOG_CTRL_S;

XLOG_CTRL_S xlog_ctrl[XLOG_MAX] = 
{
    {"zlog_debug.log",  LOG_FILE,           LOG_LOCAL0, LOG_DEBUG},
    {"zlog_info.log",   LOG_FILE,           LOG_LOCAL0, LOG_INFO},
    {"zlog_warn.log",   LOG_FILE|LOG_PRINT, LOG_LOCAL0, LOG_WARNING},
    {"zlog_error.log",  LOG_FILE|LOG_PRINT, LOG_LOCAL0, LOG_ERR},
    {"zlog.hwmon.log",  LOG_FILE|LOG_PRINT, LOG_LOCAL1, LOG_WARNING},
    {NULL, LOG_PRINT, 0, 0},
    {NULL, LOG_PRINT, 0, 0},
    {NULL, LOG_PRINT, 0, 0}
};


int xlog_print_file(int type)
{
    FILE *fp;
    char temp_buf[XLOG_BUFF_MAX];
    const char *cp_file = "temp_file";
    
    if ( (type >= XLOG_MAX) || (xlog_ctrl[type].file_name == NULL) ) {
        fprintf(stderr, "ERROR: invalid type %d\n", type);
        return VOS_ERR;
    }

    sprintf(temp_buf, "cp -f %s/%s %s", 
            (sys_conf.xlog_path == NULL) ? DAEMON_DEF_LOG_PATH:sys_conf.xlog_path, 
            xlog_ctrl[type].file_name, cp_file);
    shell_run_cmd(temp_buf);
    
    fp = fopen(cp_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "open failed, %s\n", strerror(errno));
        unlink(cp_file);
        return VOS_ERR;
    }

    vos_print("%s: \r\n", xlog_ctrl[type].file_name);
    memset(temp_buf, 0, sizeof(temp_buf));
    while (fgets(temp_buf, XLOG_BUFF_MAX-1, fp) != NULL) {  
        vos_print("%s\r", temp_buf); //linux-\n, windows-\n\r
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

int xlog_set(int type, int print_en, int log_en)
{
    if (type >= XLOG_MAX) {
        return VOS_ERR;
    }

    if (print_en) xlog_ctrl[type].flags |= LOG_PRINT;
    else xlog_ctrl[type].flags &= ~LOG_PRINT;

    if (log_en) xlog_ctrl[type].flags |= LOG_FILE;
    else xlog_ctrl[type].flags &= ~LOG_FILE;

    return VOS_OK;
}

#ifndef INCLUDE_ZLOG
int xlog(int type, const char *format, ...)
{
    va_list args;
    char buf[XLOG_BUFF_MAX];
    int len;

    if (type >= XLOG_MAX) {
        return VOS_ERR;
    }
    
    va_start(args, format);
    len = vsnprintf(buf, XLOG_BUFF_MAX, format, args);
    va_end(args);

    if (xlog_ctrl[type].flags & LOG_FILE) {
        openlog(NULL, LOG_CONS, xlog_ctrl[type].facility);
        //setlogmask(LOG_UPTO(LOG_NOTICE));
        syslog(xlog_ctrl[type].level, "%s", buf);
        closelog();
    }

    if (xlog_ctrl[type].flags & LOG_PRINT) {
        vos_print("%s\r\n", buf);
    }

    return len;    
}
#else

#include "zlog.h"

zlog_category_t *z_hwmon = NULL; 
zlog_category_t *z_daemon = NULL;

int zlog_desc_init()
{
    int rc;
    
	rc = zlog_init("/etc/zlog.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	z_hwmon = zlog_get_category("hwmon");
	if (!z_hwmon) {
		printf("get category fail\n");
		zlog_fini();
		return -2;
	}

	z_daemon = zlog_get_category("daemon");
	if (!z_daemon) {
		printf("get category fail\n");
		zlog_fini();
		return -3;
	}
    return 0;
}

int xlog(int type, const char *format, ...)
{
    va_list args;
    char buf[XLOG_BUFF_MAX];
    int len;

    if (type >= XLOG_MAX) {
        return VOS_ERR;
    }

    if (z_daemon == NULL || z_hwmon == NULL) {
        if (zlog_desc_init() < 0) return VOS_ERR;
    }
    
    va_start(args, format);
    len = vsnprintf(buf, XLOG_BUFF_MAX, format, args);
    va_end(args);

    if (xlog_ctrl[type].flags & LOG_FILE) {
        zlog_category_t *cat = (type == XLOG_HWMON) ? z_hwmon : z_daemon;
        
        if(xlog_ctrl[type].level == LOG_DEBUG) zlog_debug(cat, "%s\r\n", buf);
        else if(xlog_ctrl[type].level == LOG_INFO) zlog_info(cat, "%s\r\n", buf);
        else if(xlog_ctrl[type].level == LOG_WARNING) zlog_warn(cat, "%s\r\n", buf);
        else if(xlog_ctrl[type].level == LOG_ERR) zlog_error(cat, "%s\r\n", buf);
    }

    if (xlog_ctrl[type].flags & LOG_PRINT) {
        char time_str[64];
        fmt_time_str(time_str, 64);
        vos_print("%s %s\r\n", time_str, buf);
    }

    return len;    
}


#endif
