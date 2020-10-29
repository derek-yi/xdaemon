

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


int cli_devmem_read(int argc, char **argv)
{
    uint32 mem_addr;
    uint32 value;
    int access_type = AT_WORD;

    if (argc < 2) {
        vos_print("usage: %s <addr> [<b|h|w>] \r\n", argv[0]);
        return 0;
    }

    mem_addr = (uint32)strtoul(argv[1], 0, 0);
    if (argc > 2) {
        if (argv[2][0] == 'b') access_type = AT_BYTE;
        if (argv[2][0] == 'h') access_type = AT_SHORT;
        if (argv[2][0] == 'w') access_type = AT_WORD;
    }
    
    value = devmem_read(mem_addr, access_type);
    vos_print("memread: [0x%08x] = 0x%x \r\n", mem_addr, value);
    
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
    
    value = devmem_write(mem_addr, access_type, value);
    rd_value = devmem_read(mem_addr, access_type);
    vos_print("memwrite: 0x%x -> [0x%08x], read 0x%x \r\n", value, mem_addr, rd_value);
    
    return 0;
}

int cli_run_shell(int argc, char **argv)
{
    int ret;

    if (argc < 2) {
        vos_print("usage: %s <file_path> \r\n", argv[0]);
        return 0;
    }

    ret = shell_run_file(argv[1]);
    vos_print("cli_run_shell: ret = %d \r\n", ret);
    
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

    value = adrv9009_reg_read(0, 0x200);
    UT_CHECK_VALUE(value, 0x14);

    ret = adrv9009_pll_locked(0);
    UT_CHECK_VALUE(ret, VOS_OK);

    ret = adrv9009_pll_locked(1);
    UT_CHECK_VALUE(ret, VOS_OK);

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

int cli_backup_xlog(int argc, char **argv)
{
    extern int xlog_backup(int force);   
    xlog_backup(TRUE);
    return 0;
}


void drv_cmd_reg()
{
    cli_cmd_reg("cpu_temp",     "show cpu temp",        &cli_show_cpu_temp);
    cli_cmd_reg("board_temp",   "show board temp",      &cli_show_board_temp);
    cli_cmd_reg("mem_rd",       "devmem read",          &cli_devmem_read);
    cli_cmd_reg("mem_wr",       "devmem write",         &cli_devmem_write);
    cli_cmd_reg("version",      "show version",         &cli_show_version);

    cli_cmd_reg("rd_ad9544",    "read ad9544 reg",      &cli_ad9544_read);
    cli_cmd_reg("rd_ad9528",    "read ad9528 reg",      &cli_ad9528_read);
    cli_cmd_reg("rd_ad9009",    "read ad9009 reg",      &cli_ad9009_read);
    
    cli_cmd_reg("rd_reg",       "read FPGA reg",        &cli_devmem_read);
    cli_cmd_reg("wr_reg",       "write FPGA reg",       &cli_devmem_write);
    cli_cmd_reg("rd_ram",       "read FPGA RAM",        &cli_devmem_read); //todo
    cli_cmd_reg("wr_ram",       "write FPGA RAM",       &cli_devmem_write); //todo

#ifndef DAEMON_RELEASE    
    cli_cmd_reg("shell",        "run shell file",       &cli_run_shell);
    cli_cmd_reg("ut_drv",       "drv api unittest",     &cli_drv_unit_test);
    cli_cmd_reg("xlog_bk",      "backup xlog file",     &cli_backup_xlog);
#endif
}
#endif

int drv_module_init(char *cfg_file)
{
    drv_cmd_reg();
    
    return VOS_OK;
}

