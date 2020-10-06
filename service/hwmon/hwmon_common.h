
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

#ifndef fault_severity_t
typedef enum fault_severityT {
    NO_FAULT,
    CRITICAL = 1,
    MAJOR,
    MINOR,
    WARNING
} fault_severity_t;
#endif

#define NODE_ID_CPU_OCCUPY          0x0001
#define NODE_ID_CPU_TEMP            0x0002
#define NODE_ID_MEM_OCCUPY          0x0003

#define NODE_ID_BOARD_TEMP          0x0010
#define NODE_ID_POWER_CHECK         0x0011
#define NODE_ID_FAN_SPEED           0x0012
#define NODE_ID_GNSS_LOCKED         0x0013

#define NODE_ID_AD9544_REG          0x0020
#define NODE_ID_AD9544_PLL          0x0021
#define NODE_ID_AD9528_REG          0x0022
#define NODE_ID_AD9528_PLL          0x0023
#define NODE_ID_9FGV100_REG         0x0024

#define NODE_ID_FPGA_REG            0x0030

#define NODE_ID_AD9009_REG          0x0040
#define NODE_ID_AD9009A_PLL         0x0041
#define NODE_ID_AD9009B_PLL         0x0042

typedef struct {
    char node_desc[64];
    int  node_id;
    int  fault_state;    //fault_severity_t
}HWMON_MSG_S;

int hwmon_register(CHK_NODE_CFG_S *chk_node);

char *hwmon_get_priv_cfg(CHK_NODE_CFG_S *chk_node, char *cfg_str);

int hwmon_config(const char *node_desc, chk_func func, void *cookie);

int hwmon_set_enable(const char *node_desc, int enable);

int hwmon_init(char *file_name);

int hwmon_exit();

int hwmon_config_override();

chk_func hwmon_get_fun_ptr(char *func_name);

int hwmon_send_msg(int node_id, char *node_desc, int fault_state);

#endif
