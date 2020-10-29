
#ifndef _HW_MON_H_
#define _HW_MON_H_


#define CHK_P_NUM           8

#define CHK_STATUS_IDLE     0
#define CHK_STATUS_READY    1
#define CHK_STATUS_BUSY     2

typedef int (*chk_func)(void *self, void *cookie);

typedef struct _CHK_PRIV_CFG{
    struct _CHK_PRIV_CFG *next;
    char    *cfg_str;
    char    *cfg_val;
}CHK_PRIV_CFG_S;


typedef struct {
    char    *node_desc;
    char    *func_name;
    int     interval;
    int     repeat_max;
    int     param[CHK_P_NUM];
    CHK_PRIV_CFG_S *priv_cfg;
}CHK_NODE_CFG_S;


typedef struct {
    //config data
    CHK_NODE_CFG_S  base_cfg;
    void *      cookie;
    chk_func    chk_fun;
    int         enable;

    //running data
    int     check_status;   //CHK_STATUS_IDLE
    int     check_times;
    int     fault_state;
    int     fault_cnt;      //vs base_cfg.repeat_max
    int     interval_cmp;   //vs base_cfg.interval
}CHK_NODE_INFO_S;

int hwmon_register(CHK_NODE_CFG_S *chk_node);

char *hwmon_get_node_cfg(CHK_NODE_CFG_S *chk_node, char *cfg_str);

int hwmon_set_node_cfg(const char *node_desc, char *cfg_str, char *cfg_val);

int hwmon_config(const char *node_desc, chk_func func, void *cookie);

int hwmon_set_enable(const char *node_desc, int enable);

int hwmon_init(char *file_name);

int hwmon_exit();

int hwmon_config_override();

chk_func hwmon_get_fun_ptr(char *func_name);

int hwmon_send_msg(int node_id, char *node_desc, int fault_state);

int hwmon_enable_task(int enable);

#endif
