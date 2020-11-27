#include <stdio.h> 

#include "zlog.h"


enum {
ZLOG_LEVEL_TRACE = 10,
/* must equals conf file setting */ 

};

#define zlog_trace(cat, format, ...) \
        zlog(cat, __FILE__, sizeof(__FILE__)-1, \
        __func__, sizeof(__func__)-1, __LINE__, \
        ZLOG_LEVEL_TRACE, format, ## __VA_ARGS__) 

int main(int argc, char** argv)
{
	int rc;
	zlog_category_t *c;

	rc = zlog_init("/etc/zlog.conf");
	if (rc) {
		printf("init failed\n");
		return -1;
	}

	c = zlog_get_category("my_cat");
	if (!c) {
		printf("get cat fail\n");
		zlog_fini();
		return -2;
	}

	zlog_trace(c, "hello, zlog_trace");
	zlog_debug(c, "hello, zlog_debug");
	zlog_info(c, "hello, zlog");
	zlog_warn(c, "hello, zlog_warn");
	zlog_error(c, "hello, zlog_error");

	zlog_fini();

	return 0;
} 

/*
$ gcc -c -o zlog_test.o zlog_test.c -I/usr/local/include
$ gcc -o zlog_test zlog_test.o -L/usr/local/lib -lzlog -lpthread
$ ./zlog_test
hello, zlog
*/