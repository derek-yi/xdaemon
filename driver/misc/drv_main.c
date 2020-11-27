

#include "daemon_pub.h"

#include "drv_cpu.h"
#include "drv_main.h"
#include "drv_fpga.h"

#if T_DESC("drv_cmd", 1)

int cli_show_cpu_temp(int argc, char **argv)
{
    int cpu_temp;

    int ret = drv_get_cpu_temp(&cpu_temp);
    if ( ret != VOS_OK)
        vos_print("ERROR: failed to get cpu temp \r\n");
    else
        vos_print("CPU temp is %d \r\n", cpu_temp);
    
    return 0;
}

int cli_show_board_temp(int argc, char **argv)
{
    int temp_val;

    for(int i = 0; i < 4; i++) {
        drv_get_board_temp(i, &temp_val);
        vos_print("Board temp[%d] is %d \r\n", i, temp_val);
    }
    
    return 0;
}

int cli_ad9544_read(int argc, char **argv)
{
    uint32 base_addr;
    uint32 rd_cnt = 1;

    if (argc < 2) {
        vos_print("usage: %s <addr> [<cnt>] \r\n", argv[0]);
        return 0;
    }

    base_addr = (uint32)strtoul(argv[1], 0, 0);
    if (argc > 2) {
       rd_cnt = (uint32)strtoul(argv[2], 0, 0);
    }

    for (int i = 0; i < rd_cnt; i++) {
        uint32 value = clk_ad9544_reg_read(0, base_addr + i);
        vos_print("REG: [0x%08x] = 0x%x \r\n", base_addr + i, value);
    }
    
    return 0;
}

int cli_ad9544_write(int argc, char **argv)
{
    uint32 reg_addr;
    uint32 reg_value;

    if (argc < 3) {
        vos_print("usage: %s <addr> <value> \r\n", argv[0]);
        return 0;
    }

    reg_addr = (uint32)strtoul(argv[1], 0, 0);
    reg_value = (uint32)strtoul(argv[2], 0, 0);

    clk_ad9544_reg_write(0, reg_addr, reg_value);
    reg_value = clk_ad9544_reg_read(0, reg_addr);
    vos_print("REG: [0x%08x] = 0x%x \r\n", reg_addr, reg_value);

    return 0;
}

int cli_ad9528_read(int argc, char **argv)
{
    uint32 base_addr;
    uint32 rd_cnt = 1;

    if (argc < 2) {
        vos_print("usage: %s <addr> [<cnt>] \r\n", argv[0]);
        return 0;
    }

    base_addr = (uint32)strtoul(argv[1], 0, 0);
    if (argc > 2) {
       rd_cnt = (uint32)strtoul(argv[2], 0, 0);
    }

    for (int i = 0; i < rd_cnt; i++) {
        uint32 value = clk_ad9528_reg_read(0, base_addr + i);
        vos_print("REG: [0x%08x] = 0x%x \r\n", base_addr + i, value);
    }
    
    return 0;
}

int cli_ad9528_write(int argc, char **argv)
{
    uint32 reg_addr;
    uint32 reg_value;

    if (argc < 3) {
        vos_print("usage: %s <addr> <value> \r\n", argv[0]);
        return 0;
    }

    reg_addr = (uint32)strtoul(argv[1], 0, 0);
    reg_value = (uint32)strtoul(argv[2], 0, 0);

    clk_ad9528_reg_write(0, reg_addr, reg_value);
    reg_value = clk_ad9528_reg_read(0, reg_addr);
    vos_print("REG: [0x%08x] = 0x%x \r\n", reg_addr, reg_value);

    return 0;
}

#ifdef INCLUDE_ADRV9009
int cli_ad9009_read(int argc, char **argv)
{
    uint32 chip_id;
    uint32 base_addr;
    uint32 rd_cnt = 1;

    if (argc < 2) {
        vos_print("usage: %s <chip> <addr> [<cnt>] \r\n", argv[0]);
        return 0;
    }

    chip_id = (uint32)strtoul(argv[1], 0, 0);
    base_addr = (uint32)strtoul(argv[2], 0, 0);
    if (argc > 2) {
       rd_cnt = (uint32)strtoul(argv[3], 0, 0);
    }

    for (int i = 0; i < rd_cnt; i++) {
        uint32 value = adrv9009_reg_read(chip_id, base_addr + i);
        vos_print("REG: [0x%08x] = 0x%x \r\n", base_addr + i, value);
    }
    
    return 0;
}
#endif

int cli_devmem_read(int argc, char **argv)
{
    uint32 base_addr;
    uint32 rd_cnt = 1;

    if (argc < 2) {
        vos_print("usage: %s <addr> [<cnt>]\r\n", argv[0]);
        return 0;
    }

    base_addr = (uint32)strtoul(argv[1], 0, 0);
    if (argc > 2) {
       rd_cnt = (uint32)strtoul(argv[2], 0, 0);
    }

    for (int i = 0; i < rd_cnt; i++) {
        uint32 value = devmem_read(base_addr + i*4, AT_WORD);
        vos_print("MEM_RD: [0x%08x] = 0x%x \r\n", base_addr + i*4, value);
    }
    
    return 0;
}

int cli_devmem_write(int argc, char **argv)
{
    uint32 mem_addr;
    uint32 value, rd_value;
    int access_type = AT_WORD;

    if (argc < 3) {
        vos_print("usage: %s <addr> <value> [<b|h|w>] \r\n", argv[0]);
        return 0;
    }

    mem_addr = (uint32)strtoul(argv[1], 0, 0);
    value    = (uint32)strtoul(argv[2], 0, 0);
    if (argc > 3) {
        if (argv[2][0] == 'b') access_type = AT_BYTE;
        if (argv[2][0] == 'h') access_type = AT_SHORT;
        if (argv[2][0] == 'w') access_type = AT_WORD;
    }
    
    devmem_write(mem_addr, access_type, value);
    rd_value = devmem_read(mem_addr, access_type);
    vos_print("MEM_WR: 0x%x -> [0x%08x], read 0x%x \r\n", value, mem_addr, rd_value);
    
    return 0;
}

int cli_drv_unit_test(int argc, char **argv)
{
    int ret;
    int value;

    ret = drv_get_cpu_usage(&value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) vos_print("%d: failed \r\n", __LINE__);

    ret = drv_get_mem_usage(&value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) vos_print("%d: failed \r\n", __LINE__);

    ret = drv_get_cpu_temp(&value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) vos_print("%d: failed \r\n", __LINE__);

    ret = drv_get_board_temp(0, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) vos_print("%d: failed \r\n", __LINE__);

    ret = drv_get_board_temp(1, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) vos_print("%d: failed \r\n", __LINE__);

    ret = drv_get_board_temp(2, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) vos_print("%d: failed \r\n", __LINE__);

    ret = drv_get_board_temp(3, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) vos_print("%d: failed \r\n", __LINE__);

    ret = drv_power_sensor_get(0, 0, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) vos_print("%d: failed \r\n", __LINE__);

    ret = drv_fan_get_speed(0, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    UT_CHECK_VALUE(value, 0); //todo

    value = devmem_read(0x85000010, AT_WORD);
    UT_CHECK_VALUE(value, 0x28);

#if xxx
    value = clk_9FGV100X_reg_read(0, 0x0c);
    UT_CHECK_VALUE(value, 0x56);
#endif

    ret = drv_ad9544_pll_locked(0);
    UT_CHECK_VALUE(ret, VOS_OK);

    value = clk_ad9544_reg_read(0, 0x0c);
    UT_CHECK_VALUE(value, 0x56);

    value = clk_ad9528_reg_read(0, 0x508);
    UT_CHECK_VALUE(value, 0xEB);

    ret = drv_ad9528_pll_locked(0);
    UT_CHECK_VALUE(ret, VOS_OK);

#ifdef INCLUDE_ADRV9009
    value = adrv9009_reg_read(0, 0x200);
    UT_CHECK_VALUE(value, 0x14);

    ret = adrv9009_pll_locked(0);
    UT_CHECK_VALUE(ret, VOS_OK);

    ret = adrv9009_pll_locked(1);
    UT_CHECK_VALUE(ret, VOS_OK);
#endif    

#ifdef INCLUDE_UBLOX_GNSS
    ret = drv_gnss_is_locked();
    UT_CHECK_VALUE(ret, VOS_OK);
#endif

    char *sysfs_node = "/sys/kernel/debug/iio/iio:device2/direct_reg_access";
    ret = sys_node_write(sysfs_node, 0x200);
    UT_CHECK_VALUE(ret, VOS_OK);
    ret = sys_node_read(sysfs_node, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    UT_CHECK_VALUE(value, 0x14);

    return 0;
}

//getversion
int cli_show_version(int argc, char **argv)
{
    uint32 value = devmem_read(FPGA_VER_ADDRESS, AT_WORD);
    char buffer[256] = {0};
    int board_type = drv_board_type();
    
    vos_print("===================================================\r\n");
    vos_print("BOARD TYPE  : %s\r\n", board_type == BOARD_TYPE_RRU ? "RRU" : "RHUB");
    vos_print("FPGA Ver    : V%d.%d\r\n", (value&0x0000ff00)>>8, value&0x000000ff);
    if (board_type == BOARD_TYPE_RRU) {
        vos_print("channel     : %s\r\n", (value&0x00ff0000) == 0x00040000? "4T4R" :"2T2R");
    }
    if (board_type == BOARD_TYPE_RHUB) {
        if ((value&0xf0000000) == 0x00000000) vos_print("S-Plane Type   : Normal\r\n");
        else if ((value&0xf0000000) == 0x10000000) vos_print("S-Plane Type: 1588\r\n");
        else if ((value&0xf0000000) == 0x20000000) vos_print("S-Plane Type: GNSS\r\n");
    }
    vos_print("\r\n");
    
    sys_node_readstr("/proc/version", buffer, sizeof(buffer));
    vos_print("Kernel Ver  : %s\r\n", buffer);

    if (board_type != BOARD_TYPE_NONE) {
        sys_node_readstr("/sys/firmware/devicetree/base/fw-dts-name", buffer, sizeof(buffer));
        vos_print("dts name    : %s\r\n", buffer);
        sys_node_readstr("/sys/firmware/devicetree/base/fhk_version/version", buffer, sizeof(buffer));
        vos_print("dts version : %s\r\n", buffer);
        sys_node_readstr("/sys/firmware/devicetree/base/fhk_version/compile_date", buffer, sizeof(buffer));
        vos_print("compile_date: %s\r\n", buffer);
        vos_print("\r\n");
    }
    
    vos_print("Software Ver: 0x%x\r\n", DAEMON_VERSION);
    vos_print("compile time: %s, %s\r\n", __DATE__, __TIME__);
    vos_print("===================================================\r\n");
    
    return CMD_OK;
}

int cli_change_dir(int argc, char **argv)
{
    if (argc < 2) {
        vos_print("usage: %s <path> \r\n", argv[0]);
        return 0;
    }

    if ( chdir(argv[1]) == -1 ) {
        vos_print("cd failed \r\n");
    }
    
    return CMD_OK;
}

void drv_cmd_reg()
{
    cli_cmd_reg("cpu_temp",     "show cpu temp",        &cli_show_cpu_temp);
    cli_cmd_reg("board_temp",   "show board temp",      &cli_show_board_temp);
    cli_cmd_reg("mem_rd",       "devmem read",          &cli_devmem_read);
    cli_cmd_reg("mem_wr",       "devmem write",         &cli_devmem_write);
    cli_cmd_reg("version",      "show version",         &cli_show_version);
    cli_cmd_reg("cd",           "change dir",           &cli_change_dir);

    cli_cmd_reg("rd_ad9544",    "read ad9544 reg",      &cli_ad9544_read);
    cli_cmd_reg("wr_ad9544",    "write ad9544 reg",     &cli_ad9544_write);
    cli_cmd_reg("rd_ad9528",    "read ad9528 reg",      &cli_ad9528_read);
    cli_cmd_reg("wr_ad9528",    "write ad9528 reg",     &cli_ad9528_write);
#ifdef INCLUDE_ADRV9009    
    cli_cmd_reg("rd_ad9009",    "read ad9009 reg",      &cli_ad9009_read);
#endif    
    cli_cmd_reg("rd_reg",       "read FPGA reg",        &cli_devmem_read);
    cli_cmd_reg("wr_reg",       "write FPGA reg",       &cli_devmem_write);
    cli_cmd_reg("rd_ram",       "read FPGA RAM",        &cli_devmem_read); //todo
    cli_cmd_reg("wr_ram",       "write FPGA RAM",       &cli_devmem_write); //todo

#ifndef DAEMON_RELEASE    
    cli_cmd_reg("ut_drv",       "drv api unittest",     &cli_drv_unit_test);
#endif
}
#endif

#if T_DESC("script", 1)


#define MAX_P_NUM           8
#define OPTYPE_READ         1
#define OPTYPE_WRITE        2
#define OPTYPE_MSLEEP       3

int drv_ad9544_script_init(cJSON* sub_tree)
{
    int ret;
    int step_cnt;
    int i, j, k;
    
    step_cnt = cJSON_GetArraySize(sub_tree);
    for (i = 0; i < step_cnt; ++i) {
        cJSON* step_node = cJSON_GetArrayItem(sub_tree, i); //each step
        int ent_size = cJSON_GetArraySize(step_node);

        for (j = 0; j < ent_size; ++j) {
            cJSON* line_node = cJSON_GetArrayItem(step_node, j); //each line
            cJSON *child = line_node->child;
            int op_type = 0;
            int p[MAX_P_NUM];;

            while (child != NULL) { // fetch operation param
                if ( !strcmp(child->string, "type") ) {
                    if ( !memcmp(child->valuestring, "read", 4) ) op_type = OPTYPE_READ;
                    else if ( !memcmp(child->valuestring, "write", 5) ) op_type = OPTYPE_WRITE;
                    else if ( !memcmp(child->valuestring, "sleep", 5) ) op_type = OPTYPE_MSLEEP;
                }
                else if ( !strcmp(child->string, "param") ) {
                    int p_size = cJSON_GetArraySize(child);
                    if (p_size > MAX_P_NUM) p_size = MAX_P_NUM;
                    for (k = 0; k < p_size; k++) {
                        cJSON* tmp_param = cJSON_GetArrayItem(child, k);
                        p[k] = tmp_param->valueint;
                    }
                }
                child = child->next;
            }
            
            if (op_type > 0) {
                xlog(XLOG_WARN, "AD9544: op_type %d, plist 0x%x 0x%x 0x%x", op_type, p[0], p[1], p[2]);
            }
            
            if (op_type == OPTYPE_MSLEEP) {
               vos_msleep(p[0]);
            } else if (op_type == OPTYPE_READ) {
                //p[0]-chipid, p[1]-reg_addr, p[2]-value, p[3]-mask
                ret = clk_ad9544_reg_read(p[0], p[1]);
                if ( (ret&p[3]) != (p[2]&p[3]) ) {
                    xlog(XLOG_ERROR, "ad9544 read 0x%x fail, ret 0x%x, exp 0x%x", p[1], ret, p[2]);
                    return VOS_ERR;
                }
            } else if (op_type == OPTYPE_WRITE) {
                //p[0]-chipid, p[1]-reg_addr, p[2]-value
                ret = clk_ad9544_reg_write(p[0], p[1], p[2]);
                if ( ret != VOS_OK ) {
                    xlog(XLOG_ERROR, "ad9544 write 0x%x fail, ret 0x%x", p[1], ret);
                    return VOS_ERR;
                }
            }
        }
    }
    
    return VOS_OK;
}

int drv_fpga_script_init(cJSON* sub_tree)
{
    int ret;
    int step_cnt;
    int i, j, k;
    
    step_cnt = cJSON_GetArraySize(sub_tree);
    for (i = 0; i < step_cnt; ++i) {
        cJSON* step_node = cJSON_GetArrayItem(sub_tree, i); //each step
        int ent_size = cJSON_GetArraySize(step_node);

        for (j = 0; j < ent_size; ++j) {
            cJSON* line_node = cJSON_GetArrayItem(step_node, j); //each line
            cJSON *child = line_node->child;
            int op_type = 0;
            int p[MAX_P_NUM];;

            while (child != NULL) { // fetch operation param
                if ( !strcmp(child->string, "type") ) {
                    if ( !memcmp(child->valuestring, "read", 4) ) op_type = OPTYPE_READ;
                    else if ( !memcmp(child->valuestring, "write", 5) ) op_type = OPTYPE_WRITE;
                    else if ( !memcmp(child->valuestring, "sleep", 5) ) op_type = OPTYPE_MSLEEP;
                }
                else if ( !strcmp(child->string, "param") ) {
                    int p_size = cJSON_GetArraySize(child);
                    if (p_size > MAX_P_NUM) p_size = MAX_P_NUM;
                    for (k = 0; k < p_size; k++) {
                        cJSON* tmp_param = cJSON_GetArrayItem(child, k);
                        p[k] = tmp_param->valueint;
                    }
                }
                child = child->next;
            }
            
            if (op_type > 0) {
                xlog(XLOG_WARN, "FPGA: op_type %d, plist 0x%x 0x%x 0x%x", op_type, p[0], p[1], p[2]);
            }

            if (op_type == OPTYPE_MSLEEP) {
               vos_msleep(p[0]);
            } else if (op_type == OPTYPE_READ) {
                //p[0]-reg_addr, p[1]-value, p[2]-mask
                ret = devmem_read(p[0], AT_WORD);
                if ( (ret&p[2]) != (p[1]&p[2]) ) {
                    xlog(XLOG_ERROR, "fpga read 0x%x fail, ret 0x%x, exp 0x%x", p[0], ret, p[1]);
                    return VOS_ERR;
                }
            } else if (op_type == OPTYPE_WRITE) {
                //p[0]-reg_addr, p[1]-value
                ret = devmem_write(p[0], AT_WORD, p[1]);
                if ( ret != VOS_OK ) {
                    xlog(XLOG_ERROR, "fpga write 0x%x fail, ret 0x%x", p[0], ret);
                    return VOS_ERR;
                }
            }
        }
    }
    
    return VOS_OK;
}

int drv_load_script(char *file_name)
{
	char *json = NULL;
    cJSON* root_tree;
    cJSON* sub_tree;

    json = read_file(file_name);
	if ((json == NULL) || (json[0] == '\0') || (json[1] == '\0')) {
		xlog(XLOG_ERROR, "file %s is null", file_name);
		return VOS_ERR;
	}
 
	root_tree = cJSON_Parse(json);
	if (root_tree == NULL) {
		xlog(XLOG_ERROR, "parse json file fail");
		if (json != NULL) free(json);
        return VOS_ERR;
	}

    xlog(XLOG_WARN, "load drv script %s ...", file_name);
	sub_tree = cJSON_GetObjectItem(root_tree, "9544.init");
	if (sub_tree != NULL) {
		drv_ad9544_script_init(sub_tree);
	}

	sub_tree = cJSON_GetObjectItem(root_tree, "fpga.init");
	if (sub_tree != NULL) {
		drv_fpga_script_init(sub_tree);
	}

	if (root_tree != NULL) cJSON_Delete(root_tree);
	if (json != NULL) free(json);

    return VOS_OK;
}

#endif

#if T_DESC("drv_main", 1)


int drv_task_enable = TRUE;
int drv_peak_time = 0;

void* drv_main_task(void *param)  
{
    struct timeval t_start, t_end;
    uint32 delay_ms;

    while(1) {
        if (drv_task_enable != TRUE) {
            vos_msleep(100);
            continue;
        }
        gettimeofday(&t_start, NULL);

        //todo

        gettimeofday(&t_end, NULL);
        delay_ms = (t_end.tv_sec - t_start.tv_sec)*1000000+(t_end.tv_usec - t_start.tv_usec);//us
        delay_ms = delay_ms/1000; //ms
        if (drv_peak_time < delay_ms) drv_peak_time = delay_ms;
        vos_msleep(500);
    }
    
    return NULL;
}

void drv_timer_callback(union sigval param)
{
    static uint32 loop_cnt = 0;
    
    if (drv_task_enable != TRUE) {
        return ;
    }
    
    // 定时器回调函数应该简单处理
    //if (loop_cnt%3 == 0) cpri_state_monitor();
    
    loop_cnt++;
}


int drv_module_init(char *cfg_file)
{
    int ret;
    pthread_t threadid;
    timer_t timer_id;

    xlog(XLOG_INFO, "drv_module_init: %s", cfg_file);
    drv_load_script(cfg_file);
    drv_cmd_reg();

    ret = pthread_create(&threadid, NULL, drv_main_task, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "Error at %s:%d, pthread_create failed(%s)", __FILE__, __LINE__, strerror(errno));
        return -1;  
    } 

    ret = vos_create_timer(&timer_id, 1, drv_timer_callback, NULL);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "Error at %s:%d, vos_create_timer failed(%s)", __FILE__, __LINE__, strerror(errno));
        return -1;  
    } 

    return VOS_OK;
}

#endif
