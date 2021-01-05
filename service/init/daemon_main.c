

#include "daemon_pub.h"
#include <semaphore.h>

#include "drv_main.h"
#include "hwmon_main.h"
#include "devm_main.h"
#include "devm_msg.h"
#include "upcfg_main.h"

#if 1

SYS_CONF_PARAM sys_conf;

sem_t sysconf_sem;

char *sys_conf_get(char *key_str)
{
    DYN_CFG_S *p;

    if (key_str == NULL) return NULL;

    sem_wait(&sysconf_sem);
    p = sys_conf.dyn_cfg;
    while (p != NULL) {
        if ( !strcmp(key_str, p->cfg_str) ) {
            sem_post(&sysconf_sem);
            return p->cfg_val;
        }
        p = p->next;
    }

    sem_post(&sysconf_sem);
    return NULL;
}

int sys_conf_geti(char *key_str)
{
    char *cfg_val;

    cfg_val = sys_conf_get(key_str);

    if (cfg_val == NULL) return 0;

    return strtol(cfg_val, NULL, 0);
}

int sys_conf_set(char *key_str, char *key_val)
{
    DYN_CFG_S *p;

    if (key_str == NULL) return VOS_ERR;
    if (key_val == NULL) return VOS_ERR;

    sem_wait(&sysconf_sem);
    p = sys_conf.dyn_cfg;
    while (p != NULL) {
        if( !strcmp(key_str, p->cfg_str) ) {
            if (p->cfg_val) free(p->cfg_val);
            p->cfg_val = strdup(key_val);
            sem_post(&sysconf_sem);
            return VOS_OK;
        }
        p = p->next;
    }

    p = (DYN_CFG_S *)malloc(sizeof(DYN_CFG_S));
    if (p == NULL) {
        sem_post(&sysconf_sem);
        return VOS_ERR;
    }
    
    p->cfg_str = strdup(key_str);
    p->cfg_val = strdup(key_val);
    p->next = sys_conf.dyn_cfg;
    sys_conf.dyn_cfg = p;
    
    sem_post(&sysconf_sem);
    return VOS_OK;
}

int daemon_load_script(char *file_name)
{
    int ret;
	char *json = NULL;
    int i, list_cnt;
    cJSON* root_tree;
    cJSON* ent_list;

    ret = sem_init(&sysconf_sem, 0, 1);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "%d: sem_init failed(%s) \n", __LINE__, strerror(errno));
        return VOS_ERR;  
    } 

    xlog(XLOG_INFO, "load conf %s ...", file_name);
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

    memset((char *)&sys_conf, 0, sizeof(sys_conf));
    sys_conf.top_conf = strdup(file_name);
	ent_list = cJSON_GetObjectItem(root_tree, "fix.config");
	if (ent_list == NULL) {
		xlog(XLOG_ERROR, "parse json file fail");
		goto EXIT_PROC;
	}

	list_cnt = cJSON_GetArraySize(ent_list);
	for (i = 0; i < list_cnt; ++i) {
		cJSON* tmp_node = cJSON_GetArrayItem(ent_list, i);

        if ( !strcmp(tmp_node->string, "drv.conf") ) 
            sys_conf.drv_conf = strdup(tmp_node->valuestring);
        else if ( !strcmp(tmp_node->string, "hwmon.conf") ) 
            sys_conf.hwmon_conf = strdup(tmp_node->valuestring);
        else if ( !strcmp(tmp_node->string, "devm.conf") ) 
            sys_conf.devm_conf = strdup(tmp_node->valuestring);
        else if ( !strcmp(tmp_node->string, "upcfg.conf") ) 
            sys_conf.upcfg_conf = strdup(tmp_node->valuestring);
        else if ( !strcmp(tmp_node->string, "uds.file") ) 
            sys_conf.uds_file = strdup(tmp_node->valuestring);
        else if ( !strcmp(tmp_node->string, "xlog.path") ) 
            sys_conf.xlog_path = strdup(tmp_node->valuestring);
        else if ( !strcmp(tmp_node->string, "customer.name") ) 
            sys_conf.customer_name = strdup(tmp_node->valuestring);
        else if ( !strcmp(tmp_node->string, "customer.code") ) 
            sys_conf.customer_id = tmp_node->valueint;
	}

	ent_list = cJSON_GetObjectItem(root_tree, "dyn.config");
	if (ent_list == NULL) {
		xlog(XLOG_ERROR, "parse json file fail");
		goto EXIT_PROC;
	}

	list_cnt = cJSON_GetArraySize(ent_list);
	for (i = 0; i < list_cnt; ++i) {
		cJSON* tmp_node = cJSON_GetArrayItem(ent_list, i);
        DYN_CFG_S *dyn_cfg;
        char num_str[64];

        dyn_cfg = (DYN_CFG_S *)malloc(sizeof(DYN_CFG_S));
        if (dyn_cfg == NULL) {
            xlog(XLOG_ERROR, "malloc failed");
            goto EXIT_PROC;
        }
        
        dyn_cfg->cfg_str = strdup(tmp_node->string);
        if (tmp_node->valuestring) {
            dyn_cfg->cfg_val = strdup(tmp_node->valuestring);
        } else {
            sprintf(num_str, "%d", tmp_node->valueint);
            dyn_cfg->cfg_val = strdup(num_str);
        }
        dyn_cfg->next = sys_conf.dyn_cfg;
        sys_conf.dyn_cfg = dyn_cfg;
	}

EXIT_PROC:
	if (root_tree != NULL) cJSON_Delete(root_tree);
	if (json != NULL) free(json);

    return VOS_OK;
}

int daemon_store_script(char *file_name)
{
    cJSON* root_tree;
    cJSON* temp;
    int ret = VOS_ERR;
    char * out;
    DYN_CFG_S *p;

    root_tree = cJSON_CreateObject();
    if (root_tree == NULL) return VOS_ERR;

    temp = cJSON_AddObjectToObject(root_tree, "fix.config");
    if (temp == NULL) goto EXIT_PROC;

    cJSON_AddItemToObject(temp, "drv.conf",         cJSON_CreateString(sys_conf.drv_conf));
    cJSON_AddItemToObject(temp, "hwmon.conf",       cJSON_CreateString(sys_conf.hwmon_conf));
    cJSON_AddItemToObject(temp, "devm.conf",        cJSON_CreateString(sys_conf.devm_conf));
    cJSON_AddItemToObject(temp, "upcfg.conf",       cJSON_CreateString(sys_conf.upcfg_conf));
    cJSON_AddItemToObject(temp, "uds.file",         cJSON_CreateString(sys_conf.uds_file));
    cJSON_AddItemToObject(temp, "xlog.path",        cJSON_CreateString(sys_conf.xlog_path));
    cJSON_AddItemToObject(temp, "customer.name",    cJSON_CreateString(sys_conf.customer_name));
    cJSON_AddItemToObject(temp, "customer.code",    cJSON_CreateNumber(sys_conf.customer_id));

    temp = cJSON_AddObjectToObject(root_tree, "dyn.config");
    if (temp == NULL) goto EXIT_PROC;

    sem_wait(&sysconf_sem);
    p = sys_conf.dyn_cfg;
    while (p != NULL) {
        cJSON_AddItemToObject(temp, p->cfg_str, cJSON_CreateString(p->cfg_val));
        p = p->next;
    }
    sem_post(&sysconf_sem);

    out = cJSON_Print(root_tree);
    if (out) {
        ret = write_file(file_name, out, strlen(out));
        vos_print("file content: \r\n %s\r\n", out);
    } 

EXIT_PROC:
    if (out != NULL) free(out);
    if (root_tree != NULL) cJSON_Delete(root_tree);
    
    return ret;
}

#endif

#if 1

int daemon_module_init()
{
    int ret = 0;

    ret = drv_module_init(sys_conf.drv_conf);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "drv_module_init failed!\n");  
        return -1;  
    }     

    ret = devm_msg_init(sys_conf.uds_file);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "devm_msg_init failed!\n");  
        return -1;  
    } 

    ret = hwmon_init(sys_conf.hwmon_conf);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "hwmon_init failed!\n");  
        return -1;  
    } 

    ret = devm_init(sys_conf.devm_conf);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "devm_init failed!\n");  
        return -1;  
    } 

    ret = upcfg_init(sys_conf.upcfg_conf);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "upcfg_init failed!\n");  
        return -1;  
    } 

    return 0;
}

/* 该函数用于处理进程退出不会被释放的资源 
 * todo
 */
void daemon_module_exit()
{
    hwmon_exit();
    devm_exit();
}


void handle_signal(int signal)
{
    if( (signal == SIGTERM) || (signal == SIGINT)) {
        xlog(XLOG_ERROR, "Caught SIGTERM, exiting now\n");
        daemon_module_exit();
        exit(0);
    }
    xlog(XLOG_ERROR, "Caught wrong signal: %d\n", signal);
}

//https://tenfy.cn/2017/09/16/only-one-instance/
#include <sys/file.h>
int single_instance_check(char *file_path)
{
    int fd = open(file_path, O_WRONLY|O_CREAT);
    if (fd < 0) {
        printf("open foo.lock failed\n");
        return VOS_ERR;
    }

    int err = flock(fd, LOCK_EX|LOCK_NB);
    if (err == -1) {
        printf("lock failed\n");
        return VOS_ERR;
    }

    printf("lock success\n");
    //flock(fd, LOCK_UN);

    return VOS_OK;
}

int main(int argc, char **argv)
{
    struct sigaction sa;
    char *cfg_file = DEF_RUNNING_CFG; //as running cfg

    if (single_instance_check("./.applock") != VOS_OK) {
        return VOS_ERR;
    }

    xlog_desc_init();
    xlog(XLOG_INFO, "starting %s ...", argv[0]);
    if (access(cfg_file, F_OK) != 0) {
        if (argc < 2) {
            xlog(XLOG_ERROR, "no cfg file\r\n");
            return VOS_ERR;
        }
        cfg_file = argv[1];  //as init cfg
    }
        
    if (daemon_load_script(cfg_file) != VOS_OK) {
        xlog(XLOG_ERROR, "daemon_load_script failed \r\n");
        return VOS_ERR;
    }

    if (daemon_module_init() != VOS_OK) {
        xlog(XLOG_ERROR, "daemon_module_init failed \r\n");
        return VOS_ERR;
    }

    signal(SIGPIPE, SIG_IGN); //socket reset
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_signal;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        xlog(XLOG_ERROR, "Error: cannot handle SIGINT"); // Should not happen
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        xlog(XLOG_ERROR, "Error: cannot handle SIGINT"); // Should not happen
    }
    
#ifdef INCLUDE_TELNETD
    telnet_task_init();
#endif    

    cli_cmd_init();
#ifdef INCLUDE_CONSOLE
    cli_main_task();
#else   
    while(1) sleep(1);
#endif    

	return 0; // Should not happen
}

#endif

