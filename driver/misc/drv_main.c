#include "daemon_pub.h"

#include "drv_cpu.h"
#include "drv_main.h"
#include "drv_fpga.h"
#include "drv_i2c.h"
#include "devm_main.h"

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

    for(int i = 0; i < SYS_MAX_TEMP_ID; i++) {
        drv_get_board_temp(i, &temp_val);
        vos_print("Board temp[%d] is %d \r\n", i, temp_val);
    }
    
    return 0;
}

int cli_show_fan_info(int argc, char **argv)
{
    int temp_val;

    for (int i = 0; i < SYS_MAX_FAN_ID; i++) {
        drv_fan_get_speed(i, &temp_val);
        vos_print("FAN[%d] speed is %d \r\n", i, temp_val);
    }
    
    return 0;
}

int cli_ad9544_read(int argc, char **argv)
{
    uint32 base_addr;
    uint32 rd_cnt = 1;

    if (argc < 2) {
        vos_print("usage: %s <addr> [<cnt>] \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    base_addr = (uint32)strtoul(argv[1], 0, 0);
    if (argc > 2) {
       rd_cnt = (uint32)strtoul(argv[2], 0, 0);
    }

    for (int i = 0; i < rd_cnt; i++) {
        uint32 value = clk_ad9544_reg_read(0, base_addr + i);
        vos_print("REG: [0x%08x] = 0x%x \r\n", base_addr + i, value);
    }
    
    return CMD_OK;
}

int cli_ad9544_write(int argc, char **argv)
{
    uint32 reg_addr;
    uint32 reg_value;

    if (argc < 3) {
        vos_print("usage: %s <addr> <value> \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    reg_addr = (uint32)strtoul(argv[1], 0, 0);
    reg_value = (uint32)strtoul(argv[2], 0, 0);

    clk_ad9544_reg_write(0, reg_addr, reg_value);
    reg_value = clk_ad9544_reg_read(0, reg_addr);
    vos_print("REG: [0x%08x] = 0x%x \r\n", reg_addr, reg_value);

    return CMD_OK;
}

#ifdef INCLUDE_AD9528
int cli_ad9528_read(int argc, char **argv)
{
    uint32 base_addr;
    uint32 rd_cnt = 1;

    if (argc < 2) {
        vos_print("usage: %s <addr> [<cnt>] \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    base_addr = (uint32)strtoul(argv[1], 0, 0);
    if (argc > 2) {
       rd_cnt = (uint32)strtoul(argv[2], 0, 0);
    }

    for (int i = 0; i < rd_cnt; i++) {
        uint32 value = clk_ad9528_reg_read(0, base_addr + i);
        vos_print("REG: [0x%08x] = 0x%x \r\n", base_addr + i, value);
    }
    
    return CMD_OK;
}

int cli_ad9528_write(int argc, char **argv)
{
    uint32 reg_addr;
    uint32 reg_value;

    if (argc < 3) {
        vos_print("usage: %s <addr> <value> \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    reg_addr = (uint32)strtoul(argv[1], 0, 0);
    reg_value = (uint32)strtoul(argv[2], 0, 0);

    clk_ad9528_reg_write(0, reg_addr, reg_value);
    reg_value = clk_ad9528_reg_read(0, reg_addr);
    vos_print("REG: [0x%08x] = 0x%x \r\n", reg_addr, reg_value);

    return CMD_OK;
}
#endif

#ifdef INCLUDE_ADRV9009
int cli_ad9009_read(int argc, char **argv)
{
    uint32 chip_id;
    uint32 base_addr;
    uint32 rd_cnt = 1;

    if (argc < 2) {
        vos_print("usage: %s <chip> <addr> [<cnt>] \r\n", argv[0]);
        return CMD_ERR_PARAM;
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
    
    return CMD_OK;
}
#endif

int cli_devmem_read(int argc, char **argv)
{
    uint32 base_addr;
    uint32 rd_cnt = 1;

    if (argc < 2) {
        vos_print("usage: %s <addr> [<cnt>]\r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    base_addr = (uint32)strtoul(argv[1], 0, 0);
    if (argc > 2) {
       rd_cnt = (uint32)strtoul(argv[2], 0, 0);
    }

    for (int i = 0; i < rd_cnt; i++) {
        uint32 value = fpga_read(base_addr + i*4);
        vos_print("MEM_RD: [0x%08x] = 0x%x \r\n", base_addr + i*4, value);
    }
    
    return CMD_OK;
}

int cli_devmem_write(int argc, char **argv)
{
    uint32 mem_addr;
    uint32 value, rd_value;

    if (argc < 3) {
        vos_print("usage: %s <addr> <value> \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    mem_addr = (uint32)strtoul(argv[1], 0, 0);
    value    = (uint32)strtoul(argv[2], 0, 0);
    
    fpga_write(mem_addr, value);
    rd_value = fpga_read(mem_addr);
    vos_print("MEM_WR: 0x%x -> [0x%08x], read 0x%x \r\n", value, mem_addr, rd_value);
    
    return CMD_OK;
}

int cli_fpga_read(int argc, char **argv)
{
    uint32 reg_addr, start_bit, n_bits, mask;
    uint32 value;

    if (argc < 4) {
        vos_print("usage: %s <addr> <start_bit> <n_bits>\r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    reg_addr    = (uint32)strtoul(argv[1], 0, 0);
    start_bit   = (uint32)strtoul(argv[2], 0, 0);
    n_bits      = (uint32)strtoul(argv[3], 0, 0);
    if (start_bit > 31 || n_bits < 1 || start_bit + n_bits > 32) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }
    mask = (1<<n_bits) - 1;

    value = fpga_read_bits(reg_addr, start_bit, mask);
    vos_print("MEM_RD: [0x%08x]<%d:%d> = 0x%x \r\n", reg_addr, start_bit, start_bit + n_bits - 1, value);
    
    return CMD_OK;
}

int cli_fpga_write(int argc, char **argv)
{
    uint32 reg_addr, start_bit, n_bits, mask;
    uint32 value;

    if (argc < 5) {
        vos_print("usage: %s <addr> <start_bit> <n_bits> <value>\r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    reg_addr    = (uint32)strtoul(argv[1], 0, 0);
    start_bit   = (uint32)strtoul(argv[2], 0, 0);
    n_bits      = (uint32)strtoul(argv[3], 0, 0);
    value       = (uint32)strtoul(argv[4], 0, 0);
    if (start_bit > 31 || n_bits < 1 || start_bit + n_bits > 32) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }
    mask = (1<<n_bits) - 1;

    fpga_write_bits(reg_addr, start_bit, mask, value);
    value = fpga_read_bits(reg_addr, start_bit, mask);
    vos_print("MEM_WR: [0x%08x]<%d:%d> = 0x%x \r\n", reg_addr, start_bit, start_bit + n_bits - 1, value);
    
    return CMD_OK;
}

int cli_phy_read(int argc, char **argv)
{
    uint32 mac_base, phy_addr, reg_addr;
    uint32 value;

    if (argc < 4) {
        vos_print("usage: %s <base> <phy> <reg> \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    mac_base    = (uint32)strtoul(argv[1], 0, 0);
    phy_addr    = (uint32)strtoul(argv[2], 0, 0);
    reg_addr    = (uint32)strtoul(argv[3], 0, 0);

    value = xlnx_smi_r(mac_base, phy_addr, reg_addr);
    vos_print("PHY_RD: [0x%08x: %d, %d] = 0x%x \r\n", mac_base, phy_addr, reg_addr, value);
    
    return CMD_OK;
}

int cli_show_version(int argc, char **argv)
{
    uint32 value = fpga_read(FPGA_VER_ADDRESS);
    char buffer[256] = {0};
    int board_type = drv_board_type();
    
    vos_print("===================================================\r\n");
    vos_print("BOARD TYPE    : %s\r\n", board_type == BOARD_TYPE_RRU ? "RRU" : "RHUB");
    vos_print("FPGA Ver      : V%d.%d\r\n", (value&0x0000ff00)>>8, value&0x000000ff);
    //vos_print("Max Channel   : %s\r\n", (value&0x00ff0000) == 0x00040000? "4T4R" :"2T2R");
    vos_print("Channel       : %s\r\n", (drv_get_channel_cnt() == 4)? "4T4R" :"2T2R");
    if (board_type == BOARD_TYPE_RHUB) {
        if ((value&0xf0000000) == 0x00000000) vos_print("S-Plane Type  : Normal\r\n");
        else if ((value&0xf0000000) == 0x10000000) vos_print("S-Plane Type  : 1588\r\n");
        else if ((value&0xf0000000) == 0x20000000) vos_print("S-Plane Type  : GNSS\r\n");
    }
    vos_print("\r\n");
    
    pipe_read("uname -r", buffer, sizeof(buffer));
    vos_print("Kernel Version: %s\r", buffer);

    if (board_type != BOARD_TYPE_NONE) {
        sys_node_readstr("/sys/firmware/devicetree/base/fhk_version/branch_ver", buffer, sizeof(buffer));
        vos_print("Branch name   : %s\r\n", buffer);
        sys_node_readstr("/sys/firmware/devicetree/base/fhk_version/commit_id", buffer, sizeof(buffer));
        vos_print("Commit id     : %s\r\n", buffer);
        sys_node_readstr("/sys/firmware/devicetree/base/fhk_version/compile_date", buffer, sizeof(buffer));
        vos_print("compile_date  : %s\r\n", buffer);
        vos_print("\r\n");
    }

    sys_node_readstr("/media/sw_version.txt", buffer, sizeof(buffer));
    vos_print("MPLANE Version: %s\r", buffer);
    vos_print("\r\n");
    
    pipe_read("git branch", buffer, sizeof(buffer));
    if (strlen(buffer) > 0 ) vos_print("APP Branch: %s\r", buffer);
    pipe_read("git log -1 | head -n 4 | grep commit", buffer, sizeof(buffer));
    if (strlen(buffer) > 0 ) vos_print("%s\r", buffer);
    pipe_read("git log -1 | head -n 4 | grep Date", buffer, sizeof(buffer));
    if (strlen(buffer) > 0 ) vos_print("%s\r", buffer);
    pipe_read("git log -1 | head -n 4 | grep Author", buffer, sizeof(buffer));
    if (strlen(buffer) > 0 ) vos_print("%s\r", buffer);
    vos_print("===================================================\r\n");
    
    return CMD_OK;
}

int cli_change_dir(int argc, char **argv)
{
    if (argc < 2) {
        vos_print("usage: %s <path> \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    if ( chdir(argv[1]) == -1 ) {
        vos_print("cd failed \r\n");
    }
    
    return CMD_OK;
}

int cli_i2c_read(int argc, char **argv)
{
    uint32 rd_cnt = 1;
    int dev_bus;
    int dev_id;
    int base_addr;

    if (argc < 4) {
        vos_print("usage: %s <bus> <addr> <offset> [<cnt>] \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    dev_bus = (int)strtoul(argv[1], 0, 0);
    dev_id = (int)strtoul(argv[2], 0, 0);
    base_addr = (int)strtoul(argv[3], 0, 0);
    if (argc > 4) {
       rd_cnt = (uint32)strtoul(argv[4], 0, 0);
    }

    for (int i = 0; i < rd_cnt; i++) {
        int value = i2c_read_data(dev_bus, I2C_SMBUS_BYTE_DATA, dev_id, base_addr + i);
        vos_print("[0x%08x] = 0x%x \r\n", base_addr + i, value);
    }
    
    return CMD_OK;
}

int cli_i2c_write(int argc, char **argv)
{
    int ret;
    int dev_bus;
    int dev_id;
    int base_addr;
    uint8 value;

    if (argc < 5) {
        vos_print("usage: %s <bus> <addr> <offset> <value>\r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    dev_bus = (int)strtoul(argv[1], 0, 0);
    dev_id = (int)strtoul(argv[2], 0, 0);
    base_addr = (int)strtoul(argv[3], 0, 0);
    value = (uint8)strtoul(argv[4], 0, 0);

    ret = i2c_write_buffer(dev_bus, I2C_SMBUS_BYTE_DATA, dev_id, base_addr, &value, 1);
    vos_msleep(5);
    if (ret != VOS_OK) {
        vos_print("I2C write failed \r\n");
    }

    return CMD_OK;
}

int cli_run_dft_test(int argc, char **argv)
{
    int ret;
    
    if (argc < 2) {
        vos_print("usage: %s <param> \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    //stop bg task
    vos_print("Stopping bg task ...\r\n");
    sys_conf_set("drv_task_disable",    "1");
    sys_conf_set("drv_timer_disable",   "1");
    sys_conf_set("hwmon_disable",       "1");
    vos_msleep(1000);

    //run dft test
    vos_print("Run DFT test ...\r\n");
    ret = drv_run_dft_test(argv[1]);
    if (ret != VOS_OK) {
        vos_print("DFT test FAILED !!\r\n");
    } else {
        vos_print("DFT test PASS !! \r\n");
    }

    //recover bg task
    sys_conf_set("drv_task_disable",    "0");
    sys_conf_set("hwmon_disable",       "0");
    sys_conf_set("drv_timer_disable",   "0");

    return VOS_OK;
}

void drv_cmd_reg()
{
    cli_cmd_reg("cpu_temp",     "show cpu temp",        &cli_show_cpu_temp);
    cli_cmd_reg("board_temp",   "show board temp",      &cli_show_board_temp);
    cli_cmd_reg("fan_speed",    "show fan speed",       &cli_show_fan_info);
    cli_cmd_reg("mem_rd",       "devmem read",          &cli_devmem_read);
    cli_cmd_reg("mem_wr",       "devmem write",         &cli_devmem_write);
    cli_cmd_reg("version",      "show version",         &cli_show_version);
    cli_cmd_reg("cd",           "change dir",           &cli_change_dir);

    cli_cmd_reg("rd_ad9544",    "read ad9544 reg",      &cli_ad9544_read);
    cli_cmd_reg("wr_ad9544",    "write ad9544 reg",     &cli_ad9544_write);
#ifdef INCLUDE_AD9528    
    cli_cmd_reg("rd_ad9528",    "read ad9528 reg",      &cli_ad9528_read);
    cli_cmd_reg("wr_ad9528",    "write ad9528 reg",     &cli_ad9528_write);
#endif    
#ifdef INCLUDE_ADRV9009    
    cli_cmd_reg("rd_ad9009",    "read ad9009 reg",      &cli_ad9009_read);
#endif  

    cli_cmd_reg("rd_bits",      "read FPGA reg bits",   &cli_fpga_read);
    cli_cmd_reg("wr_bits",      "write FPGA reg bits",  &cli_fpga_write);
    cli_cmd_reg("rd_reg",       "read FPGA reg",        &cli_devmem_read);
    cli_cmd_reg("wr_reg",       "write FPGA reg",       &cli_devmem_write);
    cli_cmd_reg("rd_phy",       "read PHY reg",         &cli_phy_read);
    //cli_cmd_reg("rd_ram",       "read FPGA RAM",        &cli_devmem_read); //todo
    //cli_cmd_reg("wr_ram",       "write FPGA RAM",       &cli_devmem_write); //todo

#ifndef DAEMON_RELEASE    
    cli_cmd_reg("dft_test",     "run DFT test",         &cli_run_dft_test);
    cli_cmd_reg("i2c_rd",       "I2C Read",             &cli_i2c_read);
    cli_cmd_reg("i2c_wr",       "I2C Write",            &cli_i2c_write);
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
                ret = fpga_read(p[0]);
                if ( (ret&p[2]) != (p[1]&p[2]) ) {
                    xlog(XLOG_ERROR, "fpga read 0x%x fail, ret 0x%x, exp 0x%x", p[0], ret, p[1]);
                    return VOS_ERR;
                }
            } else if (op_type == OPTYPE_WRITE) {
                //p[0]-reg_addr, p[1]-value
                ret = fpga_write(p[0], p[1]);
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

#ifndef DAEMON_RELEASE
int drv_demo_timer(void *param)
{   
    int cpu_temp;

    drv_get_cpu_temp(&cpu_temp);
    xlog(XLOG_DEBUG, ">> cpu temp %d", cpu_temp);
    return VOS_OK;
}
#endif

TIMER_INFO_S drv_timer_list[] = 
{ 
#ifndef DAEMON_RELEASE
    {1, 60, 0, drv_demo_timer, NULL}, 
#endif
};

int drv_timer_callback(void *param)
{
    static uint32 timer_cnt = 0;
    
    if (sys_conf_geti("drv_timer_disable")) {
        return VOS_OK;
    }
    
    timer_cnt++;
    for (int i = 0; i < sizeof(drv_timer_list)/sizeof(TIMER_INFO_S); i++) {
        if ( (drv_timer_list[i].enable) && (timer_cnt%drv_timer_list[i].interval == 0) ) {
            drv_timer_list[i].run_cnt++;
            if (drv_timer_list[i].cb_func) {
                drv_timer_list[i].cb_func(drv_timer_list[i].cookie);
            }
        }
    }
    
    return VOS_OK;
}

void* drv_main_task(void *param)  
{
    //add irregular function in main loop
    while(1) {
        if (sys_conf_geti("drv_task_disable")) {
            vos_msleep(100);
            continue;
        }

        //do_sth

        vos_msleep(100);
    }
    
    return NULL;
}

int drv_module_init(char *cfg_file)
{
    int ret;
    pthread_t threadid;
    timer_t timer_id;

    xlog(XLOG_INFO, "drv_module_init: %s", cfg_file);
    drv_load_script(cfg_file);
    drv_cmd_reg();
    if ( sys_conf_geti("rm.fans.sh") ) //to be deleted
        drv_fan_init();

    ret = pthread_create(&threadid, NULL, drv_main_task, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "pthread_create failed(%s)", strerror(errno));
        return VOS_ERR;  
    } 

    ret = vos_create_timer(&timer_id, 1, drv_timer_callback, NULL);
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "vos_create_timer failed");
        return -1;  
    } 

    return VOS_OK;
}

#endif
