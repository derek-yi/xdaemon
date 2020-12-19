#include "daemon_pub.h"

#include "drv_cpu.h"
#include "drv_i2c.h"
#include "drv_main.h"
#include "drv_fpga.h"
#include "devm_main.h"

typedef int (* test_func)(void *cookie, char *out_info);

typedef struct 
{
    char        *tc_desc;
    int         enable;    
    test_func   tc_func;
    void        *cookie;
}DFT_TC_INFO;


int dft_test_cpu(void *cookie, char *out_info)
{
    int value;
    int offset = 0;
    
    drv_get_cpu_temp(&value);
    if (value < 0) { //todo
        offset += sprintf(out_info + offset, ">> Invalid CPU Temp %d \r\n", value);
    }

    return VOS_OK;
}

int dft_check_board_temp(void *cookie, char *out_info)
{
    int ret, value;
    int offset = 0;

    for(int i = 0; i < SYS_MAX_TEMP_ID; i++) {
        ret = drv_get_board_temp(i, &value);
        if (ret != VOS_OK) {
            offset += sprintf(out_info + offset, ">> Sensor %d read failed \r\n", i);
        }
    }
    
    return VOS_OK;
}

int dft_test_ad9544(void *cookie, char *out_info)
{
    int value;
    int offset = 0;

    value = clk_ad9544_reg_read(0, 0xC);
    if (value != 0x56) {
        offset += sprintf(out_info + offset, ">> invalid register 0x000C \r\n");
    }

    if (drv_ad9544_pll_locked(0) != VOS_OK) {
        offset += sprintf(out_info + offset, ">> PLL Unlock \r\n");
    }

    return VOS_OK;
}

int dft_test_fpga(void *cookie, char *out_info)
{
    int value;
    int offset = 0;

    value = fpga_read(FPGA_VER_ADDRESS);
    if (value != 0x40108) {
        offset += sprintf(out_info + offset, ">> invalid register 0x%x \r\n", FPGA_VER_ADDRESS);
    }

    return VOS_OK;
}

int dft_test_eeprom(void *cookie, char *out_info)
{
    uint8 value;
    uint8 old_val;
    EEPROM_INFO info;
    int offset = 0;

    drv_get_eeprom_info(0, &info);
    old_val = (uint8)i2c_read_data(info.i2c_bus, I2C_SMBUS_BYTE_DATA, info.dev_id, 0x10);

    value = 0x5a;
    i2c_write_buffer(info.i2c_bus, I2C_SMBUS_BYTE_DATA, info.dev_id, 0x10, &value, 1);
    vos_msleep(5);
    value = i2c_read_data(info.i2c_bus, I2C_SMBUS_BYTE_DATA, info.dev_id, 0x10);
    i2c_write_buffer(info.i2c_bus, I2C_SMBUS_BYTE_DATA, info.dev_id, 0x10, &old_val, 1);
    vos_msleep(5);
    if ( value != 0x5a ) {
        offset += sprintf(out_info + offset, ">> EEPROM 0 rw failed \r\n");
    }

    return VOS_OK;
}

int dft_test_gps(void *cookie, char *out_info)
{
    //todo
    return VOS_OK;
}

int dft_test_pmon(void *cookie, char *out_info)
{
    int value;
    int offset = 0;

    drv_power_sensor_get(0, 0, &value);
    if (value == 0) {
        offset += sprintf(out_info + offset, ">> PMON 0 check failed \r\n");
    }

    drv_power_sensor_get(1, 0, &value);
    if (value == 0) {
        offset += sprintf(out_info + offset, ">> PMON 1 check failed \r\n");
    }

    return VOS_OK;
}

int dft_test_fan(void *cookie, char *out_info)
{
    int value;
    int offset = 0;

    drv_fan_get_speed(0, &value);
    if (value == 0) {
        offset += sprintf(out_info + offset, ">> FAN 0 check failed \r\n");
    }

    drv_fan_get_speed(1, &value);
    if (value == 0) {
        offset += sprintf(out_info + offset, ">> FAN 1 check failed \r\n");
    }

    return VOS_OK;
}

DFT_TC_INFO dft_tc_list[] = 
{
    //CPU内部各状态自检, 包括cpu温度，需使用的模块
    {"CPU Test",            1,      dft_test_cpu,           NULL},  
    //IIC链路自检——SENSOR，可访问性测试，取值否异常
    {"SENSOR Test",         1,      dft_check_board_temp,   NULL},  
    //AD9544状态检测，可访问性测试，锁定状态
    {"AD9544 Test",         1,      dft_test_ad9544,        NULL},  
    //FPGA关键寄存器检测，关注器件损伤
    {"FPGA Test",           1,      dft_test_fpga,          NULL},  
    //IIC链路自检——EEPROM，可访问性测试
    {"ERPROM Test",         1,      dft_test_eeprom,        NULL},  
    //IIC链路自检——PMON，可访问性测试，取值否异常
    {"PMON Test",           1,      dft_test_pmon,          NULL},  
#ifdef BOARD_RHUB_G1
    //IIC链路自检——GPS，可访问性测试，输出1pps及reference clock是否正常
    {"GPS Test",            1,      dft_test_gps,           NULL},
    //风扇控制状态自检，设置和查询，测试多个挡位
    {"FAN Test",            1,      dft_test_fan,           NULL},  
#endif    
};

int drv_run_dft_test(char *param)
{
    int ret;
    int fail_cnt = 0;
    char detail_info[512];

    for (int i = 0; i < sizeof(dft_tc_list)/sizeof(DFT_TC_INFO); i++) {
        if (!dft_tc_list[i].enable) continue;
        
        if (dft_tc_list[i].tc_func != NULL) {
            memset(detail_info, 0, sizeof(detail_info));
            ret = dft_tc_list[i].tc_func(dft_tc_list[i].cookie, detail_info);
            if ( (ret != VOS_OK) || (detail_info[0]) ){
                fail_cnt++;
                vos_print("Step %2d: <%-16s>------------------------------------------ FAILED.\r\n", i + 1, dft_tc_list[i].tc_desc);
                if (detail_info[0]) vos_print("%s\r\n", detail_info);
            } else {
                vos_print("Step %2d: <%-16s>------------------------------------------ PASS.\r\n", i + 1, dft_tc_list[i].tc_desc);
            }
        }
    }

    if (fail_cnt) return VOS_ERR;
    return VOS_OK;
}

