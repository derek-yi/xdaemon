

#ifndef _XLOG_H_
#define _XLOG_H_


#define XLOG_MAX        8
#define XLOG_DEVM       3
#define XLOG_HWMON      2
#define XLOG_ERROR      1
#define XLOG_INFO       0

void fmt_time_str(char *time_str, int max_len);

int xlog_config(char *path, int max_file_size);

int xlog(int level, const char *fmt, ...);

int xlog_print_file(int level);


#endif
