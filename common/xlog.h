

#ifndef _XLOG_H_
#define _XLOG_H_


#define XLOG_DEBUG      0
#define XLOG_INFO       1
#define XLOG_WARN       2
#define XLOG_ERROR      3
#define XLOG_HWMON      4

#define XLOG_MAX        8

void fmt_time_str(char *time_str, int max_len);

int xlog_config(char *path, int max_file_size);

int xlog(int type, const char *fmt, ...);

int xlog_print_file(int type);


#endif
