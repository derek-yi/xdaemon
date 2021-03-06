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

#ifndef VOS_OK
#define VOS_OK      0
#define VOS_ERR     (-1)
#endif

#ifndef TRUE
#define TRUE        1
#define FALSE       0
#endif

#ifndef T_DESC
#define T_DESC(x, y)    y
#endif

char* read_file(const char *filename);
int write_file(const char *filename, char *buff, int buff_size); 

int pipe_read(char *cmd_str, char *buff, int buf_len);

int sys_node_readstr(char *node_str, char *rd_buf, int buf_len);

int sys_node_read(char *node_str, int *value);

int sys_node_writestr(char *node_str, char *wr_buf);

int sys_node_write(char *node_str, int value);

typedef int (* timer_cb)(void *param);

typedef struct 
{
    uint32      enable;
    uint32      interval;
    uint32      run_cnt;
    timer_cb    cb_func;
    void       *cookie;
}TIMER_INFO_S;

int vos_create_timer(timer_t *ret_tid, int interval, timer_cb callback, void *param);

void vos_msleep(uint32 milliseconds);

int shell_run_cmd(char *cmd_str);

int vos_print(const char * format,...);

int parse_hex_string(char *hexStr, int max, uint8 *pData);

#endif


