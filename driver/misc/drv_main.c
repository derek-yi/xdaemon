

#include "daemon_pub.h"

#include "drv_cpu.h"
#include "drv_main.h"

#if T_DESC("drv_cmd", 1)

int cli_show_cpu_temp(int argc, char **argv)
{
    int cpu_temp;

    int ret = drv_get_cpu_temp(&cpu_temp);
    if ( ret != VOS_OK)
        printf("ERROR: failed to get cpu temp\n");
    else
        printf("CPU temp is %d\n", cpu_temp);
    
    return 0;
}

int cli_devmem_read(int argc, char **argv)
{
    uint32 mem_addr;
    uint32 value;
    int access_type = AT_WORD;

    if (argc < 2) {
        printf("usage: %s <addr> [<b|h|w>]\n", argv[0]);
        return 0;
    }

    mem_addr = (uint32)strtoul(argv[1], 0, 0);
    if (argc > 2) {
        if (argv[2][0] == 'b') access_type = AT_BYTE;
        if (argv[2][0] == 'h') access_type = AT_SHORT;
        if (argv[2][0] == 'w') access_type = AT_WORD;
    }
    
    value = devmem_read(mem_addr, access_type);
    printf("memread: [0x%8x] = 0x%x\n", mem_addr, value);
    
    return 0;
}

int cli_devmem_write(int argc, char **argv)
{
    uint32 mem_addr;
    uint32 value, rd_value;
    int access_type = AT_WORD;

    if (argc < 3) {
        printf("usage: %s <addr> <value> [<b|h|w>]\n", argv[0]);
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
    printf("memwrite: 0x%x -> [0x%8x], read 0x%x\n", value, mem_addr, rd_value);
    
    return 0;
}

int cli_run_shell(int argc, char **argv)
{
    int ret;

    if (argc < 2) {
        printf("usage: %s <file_path>\n", argv[0]);
        return 0;
    }

    ret = shell_run_file(argv[1]);
    printf("cli_run_shell: ret = %d\n", ret);
    
    return 0;
}

#ifdef INCLUDE_UT_CODE
int cli_drv_unit_test(int argc, char **argv)
{
    int ret;
    int value;

    ret = drv_get_cpu_usage(&value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) printf("%d: failed\n", __LINE__);

    ret = drv_get_mem_usage(&value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) printf("%d: failed\n", __LINE__);

    ret = drv_get_cpu_temp(&value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) printf("%d: failed\n", __LINE__);

    ret = drv_get_board_temp(0, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) printf("%d: failed\n", __LINE__);

    ret = drv_get_board_temp(1, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) printf("%d: failed\n", __LINE__);

    ret = drv_get_board_temp(2, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) printf("%d: failed\n", __LINE__);

    ret = drv_get_board_temp(3, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) printf("%d: failed\n", __LINE__);

    ret = drv_power_sensor_get(0, 0, &value);
    UT_CHECK_VALUE(ret, VOS_OK);
    if (value == 0) printf("%d: failed\n", __LINE__);

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

#if xxx
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
#endif

void drv_cmd_reg()
{
    cli_cmd_reg("cputemp",      "show cpu temp",        &cli_show_cpu_temp);
    cli_cmd_reg("memread",      "devmem read",          &cli_devmem_read);
    cli_cmd_reg("memwrite",     "devmem write",         &cli_devmem_write);
    cli_cmd_reg("shell",        "run shell file",       &cli_run_shell);
#ifdef INCLUDE_UT_CODE    
    cli_cmd_reg("ut.drv",       "drv api unittest",     &cli_drv_unit_test);
#endif
}
#endif

int drv_module_init(char *cfg_file)
{
    drv_cmd_reg();
    
    return VOS_OK;
}

