
#include "daemon_pub.h"
#include "xlog.h"

#include <stdarg.h>

#define XLOG_BUFF_MAX           1024

int xlog_print_file(char *filename)
{
    FILE *fp;
    char temp_buf[XLOG_BUFF_MAX];
    const char *cp_file = "temp_file";
    
    sprintf(temp_buf, "cp -f %s/%s %s", 
            (sys_conf.xlog_path == NULL) ? "/var/log":sys_conf.xlog_path, 
            filename, cp_file);
    shell_run_cmd(temp_buf);
    
    fp = fopen(cp_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "open failed, %s\n", strerror(errno));
        unlink(cp_file);
        return VOS_ERR;
    }

    vos_print("%s: \r\n", filename);
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

#ifndef INCLUDE_ZLOG

int _xlog(char *file, int line, int level, const char *format, ...)
{
    va_list args;
    char buf[XLOG_BUFF_MAX];
    int len;
    int facility = LOG_LOCAL0;

    va_start(args, format);
    len = vsnprintf(buf, XLOG_BUFF_MAX, format, args);
    va_end(args);

    openlog(NULL, LOG_CONS, facility);
    //setlogmask(LOG_UPTO(LOG_NOTICE));
    syslog(level, "%s", buf);
    closelog();

    if ( (level == XLOG_WARN) || (level == XLOG_ERROR) ){
        vos_print("%s\r\n", buf);
    }

    return len;    
}

int xlog_desc_init()
{
    return 0;
}

#else

zlog_category_t *z_hwmon = NULL; 
zlog_category_t *z_daemon = NULL;

int xlog_desc_init()
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

#endif

