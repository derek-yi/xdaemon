

#ifndef _XLOG_H_
#define _XLOG_H_


#define XLOG_HWMON      2
#define XLOG_ERROR      1
#define XLOG_INFO       0



int xlog_config(char *path, int max_file_size);

int xlog(int level, char *fmt, ...);

#endif
