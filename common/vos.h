#ifndef _VOS_H_
#define _VOS_H_


#ifndef uint8
typedef unsigned char uint8;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef uint32
typedef unsigned int uint32;
#endif

#define VOS_OK      0
#define VOS_ERR     (-1)

#ifndef TRUE
#define TRUE        1
#endif
#ifndef FALSE
#define FALSE       0
#endif

#ifndef T_DESC
#define T_DESC(x, y)    y
#endif

char* read_file(const char *filename);

int pipe_read(char *cmd_str, char *buff, int buf_len);

int sys_node_readstr(char *node_str, char *rd_buf, int buf_len);

int sys_node_read(char *node_str, int *value);

int sys_node_write(char *node_str, int value);

typedef void (* timer_callback)(union sigval);
int vos_create_timer(timer_t *ret_tid, int interval, timer_callback callback, void *param);

void vos_msleep(uint32 milliseconds);

int shell_run_cmd(char *cmd_str);

int shell_run_file(char *file_name);

#endif


