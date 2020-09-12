

#include "daemon_pub.h"

#include "hwmon_common.h"

#include "cJSON.h"

#include <sys/time.h>


#define HWMON_MAX_NODE      1024

CHK_NODE_INFO_S hwmon_list[HWMON_MAX_NODE];

char* read_file(const char *filename) 
{
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';


cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}


/* json demo:
{
	"sys.param": {
		"status": "OK"
	},
	"checklist": {
		"cpu.occupy": {
			"interval": 5,
			"repeat": 10,
			"param1": 60,
			"param2": 0,
			"range_low": "0",
			"range_high": "80"
		},
		"9544.pll.lock": {
			"interval": 6,
			"repeat": 10,
			"param1": 60,
			"param2": 0
		},
		"9544.reg.dump": {
			"interval": 7,
			"repeat": 10,
			"param1": 60,
			"param2": 0
		}
	}
}
*/
int hwmon_load_script(char *file_name)
{
    CHK_NODE_CFG_S chk_node;
	char *json = NULL;
    int list_cnt;
    cJSON* root_tree;
    cJSON* checklist;

    json = read_file(file_name);
	if ((json == NULL) || (json[0] == '\0') || (json[1] == '\0')) {
		fprintf(stderr, "file content is null\n");
		return VOS_ERR;
	}
 
	root_tree = cJSON_Parse(json);
	if (root_tree == NULL) {
		fprintf(stderr, "pasre json file fail\n");
		goto EXIT_PROC;
	}

	checklist = cJSON_GetObjectItem(root_tree, "checklist");
	if (checklist == NULL) {
		fprintf(stderr, "pasre json file fail\n");
		goto EXIT_PROC;
	}

	list_cnt = cJSON_GetArraySize(checklist);
	for (int i = 0; i < list_cnt; ++i) {
		cJSON* tmp_node = cJSON_GetArrayItem(checklist, i);
        cJSON* temp_obj;
        int node_size = cJSON_GetArraySize(tmp_node);

        memset(&chk_node, 0, sizeof(CHK_NODE_CFG_S));
        chk_node.node_desc = strdup(tmp_node->string);

        temp_obj = cJSON_GetObjectItem(tmp_node, "interval");
        if (checklist == NULL) return VOS_ERR;
        chk_node.interval = temp_obj->valueint;
        
        temp_obj = cJSON_GetObjectItem(tmp_node, "repeat");
        chk_node.repeat_max = temp_obj->valueint;
        if (checklist == NULL) return VOS_ERR;
        
        temp_obj = cJSON_GetObjectItem(tmp_node, "param1");
        chk_node.param1 = temp_obj->valueint;
        if (checklist == NULL) return VOS_ERR;
        
        temp_obj = cJSON_GetObjectItem(tmp_node, "param2");
        chk_node.param2 = temp_obj->valueint;
        if (checklist == NULL) return VOS_ERR;

        if (hwmon_register(&chk_node) != VOS_OK) {
            break;
        }
	}

EXIT_PROC:
	if (root_tree != NULL) cJSON_Delete(root_tree);
	if (json != NULL) free(json);

    return VOS_OK;
}

int hwmon_register(CHK_NODE_CFG_S *chk_node)
{
    for (int i = 0; i < HWMON_MAX_NODE; i++) {
        if (!hwmon_list[i].base_cfg.node_desc) {
            memset(&hwmon_list[i], 0, sizeof(CHK_NODE_INFO_S));
            memcpy(&hwmon_list[i].base_cfg, chk_node, sizeof(CHK_NODE_CFG_S));
            return VOS_OK;
        }
    }

    return VOS_ERR;
}

int hwmon_list_show(int argc, char **argv)
{
    printf("hwmon list show: \n");
    for (int i = 0; i < HWMON_MAX_NODE; i++) {
        if (hwmon_list[i].base_cfg.node_desc) {
            printf("  %s: en=%d, interval=%d check_times=%d fault_state=%d\n", 
                    hwmon_list[i].base_cfg.node_desc, 
                    hwmon_list[i].enable,
                    hwmon_list[i].base_cfg.interval, 
                    hwmon_list[i].check_times,
                    hwmon_list[i].fault_state);
        }
    }
    
    return VOS_OK;
}

int hwmon_config(char *node_desc, chk_func func, void *cookie)
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


char *hwmon_get_priv_cfg(CHK_NODE_CFG_S *chk_node, char *cfg_str)
{
    return 0;
}

int hwmon_set_enable(char *node_desc, int enable)
{
    return 0;
}

int hwmon_set_interval(char *node_desc, int interval)
{
    return 0;
}

void hwmon_timer_callback(union sigval param)
{
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

void* hwmon_check_task(void *param)  
{
    struct timeval t_start, t_end;
    int t_used = 0;

    while(1) {
        gettimeofday(&t_start, NULL);
        
        for (int i = 0; i < HWMON_MAX_NODE; i++) {
            if ( (hwmon_list[i].enable) && (hwmon_list[i].check_status) ) {
                hwmon_list[i].check_status = CHK_STATUS_BUSY;
                hwmon_list[i].chk_fun(&hwmon_list[i], hwmon_list[i].cookie);
                hwmon_list[i].check_times++;
                hwmon_list[i].check_status = CHK_STATUS_IDLE;
            }
        }

        gettimeofday(&t_end, NULL);
        t_used = (t_end.tv_sec - t_start.tv_sec)*1000000+(t_end.tv_usec - t_start.tv_usec);//us
        t_used = t_used/1000; //ms
        //xlog(XLOG_INFO, "hwmon_check_task: t_used %d", t_used);
        sleep(1);
    }
    
    return NULL;
}


pthread_t hwmon_chk_tid;
timer_t hwmon_chk_timer;

extern int cli_json_test(int argc, char **argv);

int hwmon_cmd_reg()
{
    cli_cmd_reg("jsontest",      "json parse test",       &cli_json_test);
    cli_cmd_reg("hwmonlist",     "show hwmon list",       &hwmon_list_show);
    
    return VOS_OK;
}

int hwmon_init(char *file_name)
{
    int ret;
    
    //load cfg script
    hwmon_load_script(file_name);
    
    //config func
    hwmon_config_list();
    
    //start check task
    ret = pthread_create(&hwmon_chk_tid, NULL, hwmon_check_task, NULL);  
    if (ret != 0)  {  
        printf("pthread_create failed!\n");  
        return -1;  
    } 

    //start timer
    ret = vos_create_timer(&hwmon_chk_timer, 1, hwmon_timer_callback, NULL);
    if (ret != 0)  {  
        printf("vos_create_timer failed!\n");  
        return -1;  
    } 

    hwmon_cmd_reg();
    return 0;    
}

int hwmon_exit()
{
    //free hwmon_chk_tid
    //free hwmon_chk_timer
    return 0;
}



