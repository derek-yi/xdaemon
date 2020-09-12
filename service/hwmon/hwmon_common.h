
#ifndef _HW_MON_H_
#define _HW_MON_H_



typedef struct _CHK_PRIV_CFG{
    struct _CHK_PRIV_CFG *next;
    char    *cfg_str;
    char    *cfg_val;
}CHK_PRIV_CFG_S;


typedef struct {
    char    *node_desc;
    int     interval;
    int     repeat_max;
    int     param1;
    int     param2;
    CHK_PRIV_CFG_S *list;
}CHK_NODE_CFG_S;

typedef int (*chk_func)(void *self, void *cookie);

#define CHK_STATUS_IDLE     0
#define CHK_STATUS_READY    1
#define CHK_STATUS_BUSY     2

typedef struct {
    CHK_NODE_CFG_S  base_cfg;

    void *      cookie;
    chk_func    chk_fun;
    int         enable;

    int     check_status;   //CHK_STATUS_IDLE
    int     check_times;
    int     fault_state;
    int     fault_cnt;      //vs base_cfg.repeat_max
    int     interval_cmp;   //vs base_cfg.interval
}CHK_NODE_INFO_S;

int hwmon_register(CHK_NODE_CFG_S *chk_node);

char *hwmon_get_priv_cfg(CHK_NODE_CFG_S *chk_node, char *cfg_str);

int hwmon_config(char *node_desc, chk_func func, void *cookie);

int hwmon_init(char *file_name);

int hwmon_exit();

int hwmon_config_list();

char* read_file(const char *filename);


#endif
