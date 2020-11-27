

#include "daemon_pub.h"

#include "hwmon_main.h"
#include "hwmon_msg.h"
#include "devm_msg.h"


#define HWMON_MAX_NODE      256

CHK_NODE_INFO_S hwmon_list[HWMON_MAX_NODE];

int check_task_enable = TRUE;

int peak_check_time = 0;

/*************************************************************************
 * 加载解析hwmon的配置脚本，see configs/hwmon_cfg_rru.json
 *************************************************************************/
int hwmon_load_script(char *file_name)
{
    CHK_NODE_CFG_S chk_node;
	char *json = NULL;
    int list_cnt;
    cJSON* root_tree;
    cJSON* checklist;

    json = read_file(file_name);
	if ((json == NULL) || (json[0] == '\0') || (json[1] == '\0')) {
		xlog(XLOG_ERROR, "file content is null");
		return VOS_ERR;
	}
 
	root_tree = cJSON_Parse(json);
	if (root_tree == NULL) {
		xlog(XLOG_ERROR, "parse json file fail");
		goto EXIT_PROC;
	}

	checklist = cJSON_GetObjectItem(root_tree, "checklist");
	if (checklist == NULL) {
		xlog(XLOG_ERROR, "parse json file fail");
		goto EXIT_PROC;
	}

	list_cnt = cJSON_GetArraySize(checklist);
	for (int i = 0; i < list_cnt; ++i) {
		cJSON* tmp_node = cJSON_GetArrayItem(checklist, i);
        int ent_size = cJSON_GetArraySize(tmp_node);

        memset(&chk_node, 0, sizeof(CHK_NODE_CFG_S));
        chk_node.node_desc = strdup(tmp_node->string);

        for (int j = 0; j < ent_size; ++j) {
            cJSON* tmp_ent = cJSON_GetArrayItem(tmp_node, j);

            if ( !strcmp(tmp_ent->string, "function") ) {
                chk_node.func_name = strdup(tmp_ent->valuestring);
            }
            else if ( !strcmp(tmp_ent->string, "interval") ) {
                chk_node.interval = tmp_ent->valueint;
            }
            else if ( !strcmp(tmp_ent->string, "repeat") ) {
                chk_node.repeat_max = tmp_ent->valueint;
            }
            else if ( !strcmp(tmp_ent->string, "param") ) {
                int p_size = cJSON_GetArraySize(tmp_ent);
                if(p_size > CHK_P_NUM) p_size = CHK_P_NUM;
                for (int k = 0; k < p_size; ++k) {
                    cJSON* tmp_param = cJSON_GetArrayItem(tmp_ent, k);
                    chk_node.param[k] = tmp_param->valueint;
                }
            }
        }

        if (hwmon_register(&chk_node) != VOS_OK) {
            break;
        }
	}

EXIT_PROC:
	if (root_tree != NULL) cJSON_Delete(root_tree);
	if (json != NULL) free(json);

    return VOS_OK;
}

/*************************************************************************
 * 注册检测节点到hwmon_list
 *************************************************************************/
int hwmon_register(CHK_NODE_CFG_S *chk_node)
{
    for (int i = 0; i < HWMON_MAX_NODE; i++) {
        if (!hwmon_list[i].base_cfg.node_desc) {
            memset(&hwmon_list[i], 0, sizeof(CHK_NODE_INFO_S));
            memcpy(&hwmon_list[i].base_cfg, chk_node, sizeof(CHK_NODE_CFG_S));

            if ( chk_node->func_name ) {
                chk_func func = hwmon_get_fun_ptr(chk_node->func_name);
                if (func != NULL) {
                    hwmon_list[i].chk_fun = func;
                    hwmon_list[i].cookie = NULL;
                    #ifdef __ARM_ARCH //上板调试
                    hwmon_list[i].enable = TRUE;
                    #endif
                }
            }
            return VOS_OK;
        }
    }

    return VOS_ERR;
}

/*************************************************************************
 * 设置指定检测点的检测函数和私有数据
 *************************************************************************/
int hwmon_config(const char *node_desc, chk_func func, void *cookie)
{
    for (int i = 0; i < HWMON_MAX_NODE; i++) {
        if (hwmon_list[i].base_cfg.node_desc) {
            if (!strcmp(hwmon_list[i].base_cfg.node_desc, node_desc)) {
                hwmon_list[i].chk_fun = func;
                hwmon_list[i].cookie = cookie;
                hwmon_list[i].enable = TRUE;
                return VOS_OK;
            }
        }
    }

    return VOS_ERR;
}

/*************************************************************************
 * 设置检测点的配置数据
 * chk_node     - 
 * cfg_str      - 
 * return       - 
 *************************************************************************/
int hwmon_set_node_cfg(const char *node_desc, char *cfg_str, char *cfg_val)
{
    int i, pi;

    if (node_desc == NULL) return VOS_ERR;
    if (cfg_str == NULL) return VOS_ERR;

    for (i = 0; i < HWMON_MAX_NODE; i++) {
        if (hwmon_list[i].base_cfg.node_desc) {
            if (!strcmp(hwmon_list[i].base_cfg.node_desc, node_desc)) {
                break;
            }
        }
    }

    if (i == HWMON_MAX_NODE) {
        return VOS_ERR;
    }

    //set CHK_NODE_CFG_S.base_cfg.param[pi]
    int value = (int)strtoul(cfg_val, 0, 0);
    if (memcmp(cfg_str, "param", 5) == 0) {
        pi = (int)(cfg_str[5] - '0');
        if (pi < 0 || pi >= CHK_P_NUM) return VOS_ERR;
        hwmon_list[i].base_cfg.param[pi] = value;
        return VOS_OK;
    }

    //set CHK_NODE_CFG_S.base_cfg.interval
    if (memcmp(cfg_str, "interval", 8) == 0) {
        hwmon_list[i].base_cfg.interval = value;
        return VOS_OK;
    }

    //set CHK_NODE_CFG_S.enable
    if (memcmp(cfg_str, "enable", 6) == 0) {
        hwmon_list[i].enable = value;
        return VOS_OK;
    }

    return VOS_ERR;
}

/*************************************************************************
 * 使能指定检测点
 * node_desc    - 
 * enable       -  
 * return       - 
 *************************************************************************/
int hwmon_set_enable(const char *node_desc, int enable)
{
    for (int i = 0; i < HWMON_MAX_NODE; i++) {
        if (hwmon_list[i].base_cfg.node_desc) {
            if (!strcmp(hwmon_list[i].base_cfg.node_desc, node_desc)) {
                hwmon_list[i].enable  = enable;
                return VOS_OK;
            }
        }
    }

    return VOS_ERR;
}

/*************************************************************************
 * 设置检测点的检测周期
 * node_desc    - 
 * interval     - 
 * return       - 
 *************************************************************************/
int hwmon_set_interval(const char *node_desc, int interval)
{
    for (int i = 0; i < HWMON_MAX_NODE; i++) {
        if (hwmon_list[i].base_cfg.node_desc) {
            if (!strcmp(hwmon_list[i].base_cfg.node_desc, node_desc)) {
                hwmon_list[i].base_cfg.interval   = interval;
                return VOS_OK;
            }
        }
    }

    return VOS_ERR;
}

int hwmon_policy_update()
{
    hwmon_set_node_cfg("power.check0", "param1", "19088000"); //todo
    
    return VOS_OK;
}

int hwmon_enable_task(int enable)
{
    if (enable) {
        check_task_enable = TRUE;
    } else if (check_task_enable == TRUE) {
        check_task_enable = FALSE;
        vos_msleep(1000); //wait task goto sleep
    } 
    
    return VOS_OK;
}

/*************************************************************************
 * 发送检测消息给MP
 *************************************************************************/
int hwmon_send_msg(int node_id, char *node_desc, int fault_state)
{
    int ret;
    DEVM_MSG_S tx_msg;
    HWMON_MSG_S *msg_data = (HWMON_MSG_S *)tx_msg.msg_payload;

    tx_msg.msg_type     = MSG_TYPE_HWMON;
    tx_msg.need_ack     = FALSE;
    tx_msg.payload_len  = sizeof(HWMON_MSG_S);
    
    snprintf(msg_data->node_desc, DESC_MAX_LEN, "%s", node_desc);
    msg_data->node_id = node_id;
    msg_data->fault_state = fault_state;

 #ifdef __ARM_ARCH //上板调试    
    ret = devm_send_msg(APP_ORAN_MP, &tx_msg, NULL);
 #else
    ret = devm_send_msg(APP_ORAN_DAEMON, &tx_msg, NULL); //debug on x86
#endif    
    if (ret != VOS_OK) {  
        xlog(XLOG_ERROR, "Error at %s:%d, devm_send_msg failed(%d)", __FILE__, __LINE__, ret);
    } 
    
    return ret;
}


#if T_DESC("hwmon_cmd", 1)

static cJSON *parse_file(const char *filename)
{
    cJSON *parsed = NULL;
    char *content = read_file(filename);

    parsed = cJSON_Parse(content);
    if (content != NULL)
    {
        free(content);
    }

    return parsed;
}

int cli_json_test(int argc, char **argv)
{
    char *actual = NULL;
    cJSON *tree = NULL;

    if (argc < 2) {
        vos_print("usage: %s <file> \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    /* read and parse test */
    tree = parse_file(argv[1]);
    if (tree == NULL ) {
        vos_print("Failed to read of parse test. \r\n");
        return 0;
    }

    /* print the parsed tree */
    actual = cJSON_Print(tree);
    if (actual != NULL ) {
        vos_print("file %s: \r\n", argv[1]);
        vos_print("%s \r\n", actual);
    }

    if (tree != NULL)
    {
        cJSON_Delete(tree);
    }
    
    if (actual != NULL)
    {
        free(actual);
    }

    return 0;
}

/*************************************************************************
 * 打印所有检测节点信息
 *************************************************************************/
int cli_show_hwmon_list(int argc, char **argv)
{
    int count = 0;
    
    for (int i = 0; i < HWMON_MAX_NODE; i++) {
        if (hwmon_list[i].base_cfg.node_desc) {
            count++;
            vos_print("%s: \r\n", hwmon_list[i].base_cfg.node_desc);
            vos_print("  => interval=%d repeat=%d param=0x%x|0x%x|0x%x|0x%x", 
                      hwmon_list[i].base_cfg.interval, hwmon_list[i].base_cfg.repeat_max, 
                      hwmon_list[i].base_cfg.param[0], hwmon_list[i].base_cfg.param[1],
                      hwmon_list[i].base_cfg.param[2], hwmon_list[i].base_cfg.param[3]);

            vos_print("\r\n  => enable=%d check_times=%d fault_state=%d \r\n", 
                    hwmon_list[i].enable, 
                    hwmon_list[i].check_times, hwmon_list[i].fault_state);
        }
    }
    vos_print("hwmon list count: %d \r\n", count);
#ifndef DAEMON_RELEASE    
    vos_print("peak check time : %d(ms) \r\n", peak_check_time);
#endif  

    return VOS_OK;
}

int cli_show_hwmon_history(int argc, char **argv)
{
    xlog_print_file(XLOG_HWMON);
    return VOS_OK;
}

int cli_enable_hwmon_task(int argc, char **argv)
{
    if (argc < 2) {
        vos_print("usage: %s <enable|disable> \r\n", argv[0]);
        return VOS_OK;
    }

    if (!strcmp(argv[1], "enable")) {
        hwmon_enable_task(TRUE);
    } else if (!strcmp(argv[1], "disable")) {
        hwmon_enable_task(FALSE);
    } 
    
    return VOS_OK;
}

int cli_set_hwmon_node(int argc, char **argv)
{
    if (argc < 4) {
        vos_print("usage: hm_config <node> <param> <value> \r\n");
        return VOS_OK;
    }

    if ( hwmon_set_node_cfg(argv[1], argv[2], argv[3]) != VOS_OK ) {
         return VOS_ERR;
    } 
    
    return VOS_OK;
}

int cli_send_echo_cmd(int argc, char **argv)
{
    int ret;
    DEVM_MSG_S tx_msg;
    DEVM_MSG_S rx_msg;
    char time_str[64];

    tx_msg.msg_type     = MSG_TYPE_ECHO;
    tx_msg.need_ack     = TRUE;
    fmt_time_str(time_str, 64);
    sprintf(tx_msg.msg_payload, "%s echo msg", time_str);
    tx_msg.payload_len  = strlen(tx_msg.msg_payload) + 1;

    char *app_id = (argc < 2) ? APP_ORAN_DAEMON : argv[1];
    ret = devm_send_msg(app_id, &tx_msg, &rx_msg);
    if (ret != VOS_OK) {  
        xlog(XLOG_ERROR, "Error at %s:%d, devm_send_msg failed(%d)", __FILE__, __LINE__, ret);
        return ret;
    } 

    vos_print("echo: %d, %s \r\n", rx_msg.ack_value, rx_msg.msg_payload);
    
    return VOS_OK;
}

/*************************************************************************
 * 检测模块命令行注册
 *************************************************************************/
int hwmon_cmd_reg()
{
    cli_cmd_reg("hm_list",          "show hwmon list",          &cli_show_hwmon_list);
    cli_cmd_reg("hm_history",       "show hwmon history",       &cli_show_hwmon_history);
    cli_cmd_reg("hm_enable",        "enable hwmon task",        &cli_enable_hwmon_task);
    cli_cmd_reg("hm_config",        "config hwmon param",       &cli_set_hwmon_node);
    
#ifndef DAEMON_RELEASE    
    cli_cmd_reg("jsontest",        "json parse test",          &cli_json_test);
    cli_cmd_reg("echotest",        "send echo cmd",            &cli_send_echo_cmd);
#endif

    return VOS_OK;
}
#endif

#ifndef DAEMON_RELEASE 

int hwmon_msg_proc(DEVM_MSG_S *rx_msg, DEVM_MSG_S *tx_msg)
{
    HWMON_MSG_S *hwmon_msg;

    if (!rx_msg) return VOS_ERR;
    
    hwmon_msg = (HWMON_MSG_S *)rx_msg->msg_payload;
    vos_print("hwmon_msg_proc: %d(%s) fault %d \r\n", hwmon_msg->node_id, hwmon_msg->node_desc, hwmon_msg->fault_state);

    return VOS_OK;
}

int echo_msg_proc(DEVM_MSG_S *rx_msg, DEVM_MSG_S *tx_msg)
{
    if (!rx_msg) return VOS_ERR;
    if (!tx_msg) return VOS_ERR;
    
    vos_print("echo_msg_proc: %s \r\n", rx_msg->msg_payload);
    memcpy((char *)tx_msg, (char *)rx_msg, sizeof(DEVM_MSG_S));
    tx_msg->need_ack    = FALSE;
    tx_msg->ack_value   = 1234;

    return VOS_OK;
}

#endif

#if T_DESC("hwmon_main", 1)

pthread_t hwmon_chk_tid;

timer_t hwmon_chk_timer;


/*************************************************************************
 * 定时器回调，检测各检测点是否超时
 *************************************************************************/
void hwmon_timer_callback(union sigval param)
{
    if (check_task_enable != TRUE) {
        return ;
    }
    
    // 定时器回调函数应该简单处理
    for (int i = 0; i < HWMON_MAX_NODE; i++) {
        if (hwmon_list[i].enable) {
            if (hwmon_list[i].interval_cmp++ >= hwmon_list[i].base_cfg.interval) {
                hwmon_list[i].interval_cmp = 0;
                hwmon_list[i].check_status = CHK_STATUS_READY;
            }
        }
    }
}

/*************************************************************************
 * 检测主函数，轮询检测所有已超时的检测点
 *************************************************************************/
void* hwmon_check_task(void *param)  
{
    struct timeval t_start, t_end;
    int t_used = 0;

    while(1) {
        if (check_task_enable != TRUE) {
            vos_msleep(100);
            continue;
        }
        gettimeofday(&t_start, NULL);
        
        for (int i = 0; i < HWMON_MAX_NODE; i++) {
            if ( (hwmon_list[i].enable)
                && (hwmon_list[i].check_status == CHK_STATUS_READY) 
                && (hwmon_list[i].chk_fun) ) {
                //hwmon_list[i].check_status = CHK_STATUS_BUSY;
                hwmon_list[i].chk_fun(&hwmon_list[i], hwmon_list[i].cookie);
                hwmon_list[i].check_times++;
                hwmon_list[i].check_status = CHK_STATUS_IDLE;
            }
        }

        hwmon_policy_update();
        
        gettimeofday(&t_end, NULL);
        t_used = (t_end.tv_sec - t_start.tv_sec)*1000000+(t_end.tv_usec - t_start.tv_usec);//us
        t_used = t_used/1000; //ms
        if (peak_check_time < t_used) peak_check_time = t_used;
        vos_msleep(500);
    }
    
    return NULL;
}

/*************************************************************************
 * 检测模块初始化函数
 *************************************************************************/
int hwmon_init(char *cfg_file)
{
    int ret;
    
    //load cfg script
    xlog(XLOG_INFO, "hwmon_init: %s", cfg_file);
    hwmon_load_script(cfg_file);
    
    //override check node
    hwmon_config_override();

#ifndef DAEMON_RELEASE 
    devm_set_msg_func(MSG_TYPE_HWMON,   hwmon_msg_proc);
    devm_set_msg_func(MSG_TYPE_ECHO,    echo_msg_proc);
#endif
    
    //start check task
    ret = pthread_create(&hwmon_chk_tid, NULL, hwmon_check_task, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "Error at %s:%d, pthread_create failed(%s)", __FILE__, __LINE__, strerror(errno));
        return -1;  
    } 

    //start timer
    ret = vos_create_timer(&hwmon_chk_timer, 1, hwmon_timer_callback, NULL);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "Error at %s:%d, vos_create_timer failed(%s)", __FILE__, __LINE__, strerror(errno));
        return -1;  
    } 

    hwmon_cmd_reg();
    return 0;    
}

/*************************************************************************
 * 检测模块退出函数
 *************************************************************************/
int hwmon_exit()
{
    //free hwmon_chk_tid
    //free hwmon_chk_timer
    return 0;
}

#endif

