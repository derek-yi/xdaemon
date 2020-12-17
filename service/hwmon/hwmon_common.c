
#include "daemon_pub.h"

#include "hwmon_main.h"
#include "hwmon_msg.h"
#include "devm_msg.h"
#include "drv_cpu.h"
#include "drv_main.h"
#include "drv_fpga.h"


/*************************************************************************
 * cpu占有率检测
 *************************************************************************/
int check_cpu_occupy(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int top_limit;
    int cpu_usage;

    top_limit = node->base_cfg.param[0];
	if ( drv_get_cpu_usage(&cpu_usage) != VOS_OK ) {
        xlog(XLOG_ERROR, "drv_get_cpu_usage failed");
        return VOS_ERR;
    }
	
    if ( cpu_usage > top_limit ) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            if (node->fault_state == NO_FAULT) {
                mlog(XLOG_WARN, "cpu occupy warning, cpu usage %d", cpu_usage);
                hwmon_send_msg(NODE_ID_PROCESS_OVERLOAD, node->base_cfg.node_desc, MAJOR, CPU);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "cpu occupy normal, cpu usage %d", cpu_usage);
            hwmon_send_msg(NODE_ID_PROCESS_OVERLOAD, node->base_cfg.node_desc, NO_FAULT, CPU);
        }
        node->fault_state = NO_FAULT;
        node->fault_cnt = 0;
    }

	return VOS_OK;
}

/*************************************************************************
 * 内存占有率检测
 *************************************************************************/
int check_mem_occupy(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int top_limit;
    int mem_usage;

    top_limit = node->base_cfg.param[0];
	if ( drv_get_mem_usage(&mem_usage) != VOS_OK ) {
        xlog(XLOG_ERROR, "drv_get_cpu_usage failed");
        return VOS_ERR;
    }
	
    if ( mem_usage > top_limit ) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            if (node->fault_state == NO_FAULT) {
                mlog(XLOG_WARN, "mem occupy warning, mem usage %d", mem_usage);
                hwmon_send_msg(NODE_ID_MEM_OVERLOAD, node->base_cfg.node_desc, MAJOR, CPU);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "mem occupy normal, mem usage %d", mem_usage);
            hwmon_send_msg(NODE_ID_MEM_OVERLOAD, node->base_cfg.node_desc, NO_FAULT, CPU);
        }
        node->fault_cnt = 0;
        node->fault_state = NO_FAULT;
    }

	return VOS_OK;
}

/*************************************************************************
 * cpu温度检测
 *************************************************************************/
int check_cpu_temp(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int temp_limit1, temp_limit2;
    int cpu_temp;

    temp_limit1 = node->base_cfg.param[0];
    temp_limit2 = node->base_cfg.param[1];
    if (temp_limit1 >= temp_limit2) {
        return VOS_ERR;
    }

	if ( drv_get_cpu_temp(&cpu_temp) != VOS_OK ) {
        xlog(XLOG_ERROR, "drv_get_cpu_usage failed");
        return VOS_ERR;
    }

    xlog(XLOG_INFO, "cpu temp: value is %d", cpu_temp);
    if ( cpu_temp > temp_limit2 ) {
        if (node->fault_state != CRITICAL) {
            mlog(XLOG_WARN, "cpu temp warning, current temp %d", cpu_temp);
            if (node->fault_state == MINOR) 
                hwmon_send_msg(NODE_ID_HIGH_TEMP, node->base_cfg.node_desc, NO_FAULT, CPU);
            hwmon_send_msg(NODE_ID_OVER_HEATING, node->base_cfg.node_desc, CRITICAL, CPU);
        }
        node->fault_state = CRITICAL;
    } else if ( cpu_temp > temp_limit1 ) {
        if (node->fault_state != MINOR) {
            mlog(XLOG_WARN, "cpu temp warning, current temp %d", cpu_temp);
            if (node->fault_state == CRITICAL) 
                hwmon_send_msg(NODE_ID_OVER_HEATING, node->base_cfg.node_desc, NO_FAULT, CPU);
            hwmon_send_msg(NODE_ID_HIGH_TEMP, node->base_cfg.node_desc, MINOR, CPU);
        }
        node->fault_state = MINOR;
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "cpu temp normal, current temp %d", cpu_temp);
            if (node->fault_state == CRITICAL) 
                hwmon_send_msg(NODE_ID_OVER_HEATING, node->base_cfg.node_desc, NO_FAULT, CPU);
            else if (node->fault_state == MINOR) 
                hwmon_send_msg(NODE_ID_HIGH_TEMP, node->base_cfg.node_desc, NO_FAULT, CPU);
        }
        node->fault_state = NO_FAULT;
    }

	return VOS_OK;
}

/*************************************************************************
 * 单板温度检测
 *************************************************************************/
int check_board_temp(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int temp_id;
    int temp_limit1, temp_limit2;
    int board_temp;

    temp_id     = node->base_cfg.param[0];
    temp_limit1 = node->base_cfg.param[1];
    temp_limit2 = node->base_cfg.param[2];
    if (temp_limit1 >= temp_limit2) {
        return VOS_ERR;
    }

	if ( drv_get_board_temp(temp_id, &board_temp) != VOS_OK ) {
        xlog(XLOG_ERROR, "drv_get_cpu_usage failed");
        return VOS_ERR;
    }
	
    if ( board_temp > temp_limit2 ) {
        if (node->fault_state != CRITICAL) {
            mlog(XLOG_WARN, "board temp warning, current temp %d", board_temp);
            if (node->fault_state == MINOR) 
                hwmon_send_msg(NODE_ID_HIGH_TEMP, node->base_cfg.node_desc, NO_FAULT, PA);
            hwmon_send_msg(NODE_ID_OVER_HEATING, node->base_cfg.node_desc, CRITICAL, PA);
        }
        node->fault_state = CRITICAL;
    } else if ( board_temp > temp_limit1 ) {
        if (node->fault_state != MINOR) {
            mlog(XLOG_WARN, "board temp warning, current temp %d", board_temp);
            if (node->fault_state == CRITICAL) 
                hwmon_send_msg(NODE_ID_OVER_HEATING, node->base_cfg.node_desc, NO_FAULT, PA);
            hwmon_send_msg(NODE_ID_HIGH_TEMP, node->base_cfg.node_desc, MINOR, PA);
        }
        node->fault_state = MINOR;
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "board temp normal, current temp %d", board_temp);
            if (node->fault_state == CRITICAL) 
                hwmon_send_msg(NODE_ID_OVER_HEATING, node->base_cfg.node_desc, NO_FAULT, PA);
            else if (node->fault_state == MINOR) 
                hwmon_send_msg(NODE_ID_HIGH_TEMP, node->base_cfg.node_desc, NO_FAULT, PA);
        }
        node->fault_state = NO_FAULT;
    }

	return VOS_OK;
}

/*************************************************************************
 * INA2XX 功率电压等检测
 *************************************************************************/
int ina2xx_reg_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int chip_id, power_limit;
    int power_value;

    chip_id     = node->base_cfg.param[0];
    power_limit = node->base_cfg.param[1];

	if ( drv_power_sensor_get(chip_id, 0, &power_value) != VOS_OK ) {
        xlog(XLOG_ERROR, "drv_power_sensor_get failed");
        return VOS_ERR;
    }

    xlog(XLOG_INFO, "power sensor: power[%d] is %d", chip_id, power_value);
    if ( power_value < power_limit ) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "power sensor abnormal, value is %d", power_value);
                hwmon_send_msg(NODE_ID_INPUT_PWR_FAULT, node->base_cfg.node_desc, MAJOR, PA);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "power sensor normal, value is %d", power_value);
            hwmon_send_msg(NODE_ID_INPUT_PWR_FAULT, node->base_cfg.node_desc, NO_FAULT, PA);
        }
        node->fault_state = NO_FAULT;
        node->fault_cnt = 0;
    }

	return VOS_OK;
}

/*************************************************************************
 * 风扇速率检测
 *************************************************************************/
int fan_speed_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int fan_id, speed_limit;
    int speed_value;

    fan_id      = node->base_cfg.param[0]; //todo
    speed_limit = node->base_cfg.param[1];

    if ( drv_fan_get_speed(fan_id, &speed_value) != VOS_OK ) {
        xlog(XLOG_ERROR, "drv_fan_get_speed failed");
        return VOS_ERR;
    }

    if ( speed_value < speed_limit ) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "fan %d speed abnormal, value is %d", fan_id, speed_value);
                hwmon_send_msg(NODE_ID_FAN_BROKEN, node->base_cfg.node_desc, MAJOR, Fan);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "fan %d speed normal, value is %d", fan_id, speed_value);
            hwmon_send_msg(NODE_ID_FAN_BROKEN, node->base_cfg.node_desc, NO_FAULT, Fan);
        }
        node->fault_state = NO_FAULT;
        node->fault_cnt = 0;
    }

    return VOS_OK;
}

/*************************************************************************
 * 逻辑寄存器预期值检测
 *************************************************************************/
int fpga_reg_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int reg_addr, bit_mask;
     int exp_value, read_value;

    reg_addr    = node->base_cfg.param[0];
    bit_mask    = node->base_cfg.param[1];
    exp_value   = node->base_cfg.param[2];
    read_value  = fpga_read(reg_addr);

    if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
        read_value  = fpga_read(reg_addr); //try again
        if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "FPGA reg check fail, address=0x%08x read=0x%x expect=0x%x", reg_addr, read_value, exp_value);
                hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, CRITICAL, FPGA);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "FPGA reg check pass, address=0x%08x", reg_addr);
            hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, NO_FAULT, FPGA);
        }
        node->fault_state = NO_FAULT;
    }    
    
    return VOS_OK;
}

/*************************************************************************
 * 逻辑寄存器dump
 *************************************************************************/
int fpga_reg_dump(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int base_addr, reg_cnt;
    int read_value;

    base_addr = node->base_cfg.param[0];
    reg_cnt = node->base_cfg.param[1];

    for (int i = 0; i < reg_cnt; i++) {
        read_value = fpga_read(base_addr + i*4);
        mlog(XLOG_WARN, "FPGA DUMP: address=0x%08x read=0x%x", base_addr + i*4, read_value);
    }
    
    return VOS_OK;
}

/*************************************************************************
 * 时钟芯片 9FGV100X 寄存器check
 *************************************************************************/
int clk_9FGV100X_reg_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int reg_addr, bit_mask;
    int exp_value, read_value;

    reg_addr    = node->base_cfg.param[0];
    bit_mask    = node->base_cfg.param[1];
    exp_value   = node->base_cfg.param[2];
    read_value  = clk_9FGV100X_reg_read(0, reg_addr);

    if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
        read_value  = clk_9FGV100X_reg_read(0, reg_addr); //try again
        if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "9FGV100X reg check fail, address=0x%08x read=0x%x expect=0x%x", reg_addr, read_value, exp_value);
                hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, CRITICAL, _9FGV100);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "9FGV100X reg check pass, address=0x%08x", reg_addr);
            hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, NO_FAULT, _9FGV100);
        }
        node->fault_state = NO_FAULT;
    }    
    
    return VOS_OK;
}

/*************************************************************************
 * 时钟芯片 9FGV100X 寄存器dump
 *************************************************************************/
int clk_9FGV100X_reg_dump(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int base_addr, reg_cnt;
    int read_value;

    base_addr = node->base_cfg.param[0];
    reg_cnt   = node->base_cfg.param[1];

    for (int i = 0; i < reg_cnt; i++) {
        read_value = clk_9FGV100X_reg_read(0, base_addr + i);
        mlog(XLOG_WARN, "9FGV100X DUMP: address=0x%08x read=0x%x", base_addr + i, read_value);
    }
    
    return VOS_OK;
}

/*************************************************************************
 * 如果检测的是irq monitor寄存器，获取结果后清除
 *************************************************************************/
int ad9544_clear_irq_monitor(uint32 chip_id, uint32 reg_addr)
{
    //IRQ monitor registers (Register 0x300B through Register 0x3019)
    //IRQ clear registers (Register 0x2006 through Register 0x2014)
    if (reg_addr >= 0x300B && reg_addr <= 0x3019) {
        reg_addr = reg_addr - 0x300B + 0x2006;
        clk_ad9544_reg_write(chip_id, reg_addr, 0xFF);
    }
    return VOS_OK;
}

/*************************************************************************
 * 时钟芯片 AD9544 寄存器check
 *************************************************************************/
int ad9544_reg_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int reg_addr, bit_mask;
    int exp_value, read_value;

    reg_addr    = node->base_cfg.param[0];
    bit_mask    = node->base_cfg.param[1];
    exp_value   = node->base_cfg.param[2];
    read_value  = clk_ad9544_reg_read(0, reg_addr);

    if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
        read_value  = clk_ad9544_reg_read(0, reg_addr); //try again
        if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "AD9544 reg check fail, address=0x%08x read=0x%x expect=0x%x", reg_addr, read_value, exp_value);
                hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, CRITICAL, AD9544);
            }
            node->fault_state = CRITICAL;
            ad9544_clear_irq_monitor(0, reg_addr);
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "AD9544 reg check pass, address=0x%08x", reg_addr);
            hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, NO_FAULT, AD9544);
        }
        node->fault_state = NO_FAULT;
    }    
    
    return VOS_OK;
}

/*************************************************************************
 * AD9544 锁相环检测
 *************************************************************************/
int ad9544_pll_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
	int pll_locked = drv_ad9544_pll_locked(0);

    if (pll_locked != VOS_OK) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "ad9544 pll unlocked");
                hwmon_send_msg(NODE_ID_OUT_OF_SYNC, node->base_cfg.node_desc, CRITICAL, AD9544);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "ad9544 pll locked");
            hwmon_send_msg(NODE_ID_OUT_OF_SYNC, node->base_cfg.node_desc, NO_FAULT, AD9544);
        }
        node->fault_state = NO_FAULT;
        node->fault_cnt = 0;
    }

	return VOS_OK;
}

/*************************************************************************
 * 时钟芯片 AD9544 寄存器dump
 *************************************************************************/
int ad9544_reg_dump(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int base_addr, reg_cnt;
    int read_value;

    base_addr = node->base_cfg.param[0];
    reg_cnt   = node->base_cfg.param[1];

    for (int i = 0; i < reg_cnt; i++) {
        read_value = clk_ad9544_reg_read(0, base_addr + i);
        mlog(XLOG_WARN, "AD9544 DUMP: address=0x%08x read=0x%x", base_addr + i, read_value);
    }
    
    return VOS_OK;
}

#ifdef INCLUDE_AD9528
/*************************************************************************
 * 时钟芯片 AD9528 寄存器check
 *************************************************************************/
int ad9528_reg_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int reg_addr, bit_mask;
    int exp_value, read_value;

    reg_addr    = node->base_cfg.param[0];
    bit_mask    = node->base_cfg.param[1];
    exp_value   = node->base_cfg.param[2];
    read_value  = clk_ad9528_reg_read(0, reg_addr);

    if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
        read_value  = clk_ad9528_reg_read(0, reg_addr); //try again
        if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "AD9528 reg check fail, address=0x%08x read=0x%x expect=0x%x", reg_addr, read_value, exp_value);
                hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, CRITICAL, AD9528);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "AD9528 reg check pass, address=0x%08x", reg_addr);
            hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, NO_FAULT, AD9528);
        }
        node->fault_state = NO_FAULT;
    }    
    
    return VOS_OK;
}

/*************************************************************************
 * 时钟芯片 AD9528 锁相环检测
 *************************************************************************/
int ad9528_pll_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int pll_status = drv_ad9528_pll_locked(0);

    if (pll_status != VOS_OK) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "ad9528 pll abnormal");
                hwmon_send_msg(NODE_ID_OUT_OF_SYNC, node->base_cfg.node_desc, CRITICAL, AD9528);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "ad9528 pll normal");
            hwmon_send_msg(NODE_ID_OUT_OF_SYNC, node->base_cfg.node_desc, NO_FAULT, AD9528);
        }
        node->fault_state = NO_FAULT;
        node->fault_cnt = 0;
    }

    return VOS_OK;
}

/*************************************************************************
 * 时钟芯片 AD9528 寄存器dump
 *************************************************************************/
int ad9528_reg_dump(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int base_addr, reg_cnt;
    int read_value;

    base_addr = node->base_cfg.param[0];
    reg_cnt   = node->base_cfg.param[1];

    for (int i = 0; i < reg_cnt; i++) {
        read_value = clk_ad9528_reg_read(0, base_addr + i);
        mlog(XLOG_WARN, "AD9528 DUMP: address=0x%08x read=0x%x", base_addr + i, read_value);
    }
    
    return VOS_OK;
}
#endif

/*************************************************************************
 * RHUB检查cpri端口连接状态，refer to detect_cpri_state
 *************************************************************************/
int check_cpri_state(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    static int pre_rru_cnt = 0;
    int link_cnt = 0;

    if ( drv_get_cpri_links(&link_cnt) != VOS_OK ) {
        xlog(XLOG_ERROR, "drv_get_cpri_links failed");
        return VOS_ERR;
    }
    
    if ( link_cnt < pre_rru_cnt ) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            if (node->fault_state == NO_FAULT) {
                mlog(XLOG_WARN, "rru(cpri) link cnt %d", link_cnt);
                hwmon_send_msg(NODE_ID_CU_LINK_FAULT, node->base_cfg.node_desc, MAJOR, CPRI);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        pre_rru_cnt = link_cnt;
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "rru(cpri) link cnt %d", link_cnt);
            hwmon_send_msg(NODE_ID_CU_LINK_FAULT, node->base_cfg.node_desc, NO_FAULT, CPRI);
        }
        node->fault_cnt = 0;
        node->fault_state = NO_FAULT;
    }

    return VOS_OK;
}

#ifdef INCLUDE_ADRV9009
int adrv9009_id_check(int chip_id)
{
    if (chip_id < 0 || chip_id >= SYS_MAX_RF_CHIP) {
        return VOS_ERR;
    }
    
    return VOS_OK;
}

/*************************************************************************
 * ADRV9009 寄存器check
 *************************************************************************/
int adrv9009_reg_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int chip_id, reg_addr, bit_mask;
    int exp_value, read_value;

    chip_id     = node->base_cfg.param[0];
    if (adrv9009_id_check(chip_id) != VOS_OK) return VOS_ERR;
    
    reg_addr    = node->base_cfg.param[1];
    bit_mask    = node->base_cfg.param[2];
    exp_value   = node->base_cfg.param[3];
    read_value  = adrv9009_reg_read(chip_id, reg_addr);

    if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
        read_value  = adrv9009_reg_read(chip_id, reg_addr); //try again
        if ( (read_value & bit_mask) != (exp_value & bit_mask ) ) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "AD9009 reg check fail, address=0x%08x read=0x%x expect=0x%x", reg_addr, read_value, exp_value);
                hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, CRITICAL, PLL1 + chip_id);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "AD9009 reg check pass, address=0x%08x", reg_addr);
            hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, NO_FAULT, PLL1 + chip_id);
        }
        node->fault_state = NO_FAULT;
    }    
    
    return VOS_OK;
}

/*************************************************************************
 * ADRV9009 锁相环检测
 *************************************************************************/
int adrv9009_pll_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int chip_id, pll_locked;

    chip_id = node->base_cfg.param[0];
    if (adrv9009_id_check(chip_id) != VOS_OK) return VOS_ERR;
    
    pll_locked = adrv9009_pll_locked(chip_id);
    
    if ( pll_locked != VOS_OK) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "AD9009 pll unlocked");
                hwmon_send_msg(NODE_ID_LO_UNLOCK, node->base_cfg.node_desc, CRITICAL, PLL1 + chip_id);
            }
            node->fault_state = CRITICAL;
        }
    } else  {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "AD9009 pll locked");
            hwmon_send_msg(NODE_ID_LO_UNLOCK, node->base_cfg.node_desc, NO_FAULT, PLL1 + chip_id);
        }
        node->fault_state = NO_FAULT;
        node->fault_cnt = 0;
    }

    return VOS_OK;
}

/*************************************************************************
 * ADRV9009 寄存器dump
 *************************************************************************/
int adrv9009_reg_dump(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int chip_id, base_addr, reg_cnt;
    int read_value;

    chip_id   = node->base_cfg.param[0];
    if (adrv9009_id_check(chip_id) != VOS_OK) return VOS_ERR;
    
    base_addr = node->base_cfg.param[1];
    reg_cnt   = node->base_cfg.param[2];

    for (int i = 0; i < reg_cnt; i++) {
        read_value = adrv9009_reg_read(chip_id, base_addr + i);
        mlog(XLOG_WARN, "AD9009 DUMP: chip=%d address=0x%08x read=0x%x", chip_id, base_addr + i, read_value);
    }
    
    return VOS_OK;
}

int adrv9009_isr_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int file_exist;
    FILE *fp;
    int chip_id;
    char isr_log_file[32];
    char line_buff[1024];

    chip_id = node->base_cfg.param[0];
    if (adrv9009_id_check(chip_id) != VOS_OK) return VOS_ERR;

    //device id is 2-3, refer to adrv9009_irq_handler
    sprintf(isr_log_file, "/tmp/talise_isr%d.xlog", chip_id + 2); 
    file_exist = ( access(isr_log_file, F_OK) == 0);
    
    if ( file_exist ) {
        if (node->fault_state != CRITICAL) {
            mlog(XLOG_WARN, "AD9009 ISR WARN");
            hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, CRITICAL, PLL1 + chip_id);
        }
        node->fault_state = CRITICAL;
        node->fault_cnt++;

        //cat file to xlog, and delete it
        fp = fopen(isr_log_file, "r");
        if (fp != NULL) {
            memset(line_buff, 0, sizeof(line_buff));
            while (fgets(line_buff, 1000, fp) != NULL) {  
                mlog(XLOG_WARN, "%s", line_buff);
                memset(line_buff, 0, sizeof(line_buff));
            }
            fclose(fp);  
            unlink(isr_log_file);
        }
    } else  {
        if (node->fault_state != NO_FAULT) {
            hwmon_send_msg(NODE_ID_UNIT_OUT_OF_ORDER, node->base_cfg.node_desc, NO_FAULT, PLL1 + chip_id);
        }
        node->fault_state = NO_FAULT;
    }

    return VOS_OK;
}
#endif

#ifdef INCLUDE_UBLOX_GNSS 
/*************************************************************************
 * LEA-M8F
 *************************************************************************/
int ublox_gps_lock_check(void *self, void *cookie)
{
    CHK_NODE_INFO_S *node = (CHK_NODE_INFO_S *)self;
    int lock_status = drv_gnss_is_locked();

    if (lock_status != VOS_OK) {
        if (node->fault_cnt++ >= node->base_cfg.repeat_max) {
            if (node->fault_state != CRITICAL) {
                mlog(XLOG_WARN, "GNSS 1PPS UNLOCK");
                hwmon_send_msg(NODE_ID_NO_SYNC_SRC, node->base_cfg.node_desc, MAJOR, GPS);
            }
            node->fault_state = CRITICAL;
        }
    } else {
        if (node->fault_state != NO_FAULT) {
            mlog(XLOG_WARN, "GNSS 1PPS LOCKED");
            hwmon_send_msg(NODE_ID_NO_SYNC_SRC, node->base_cfg.node_desc, NO_FAULT, GPS);
        }
        node->fault_state = NO_FAULT;
        node->fault_cnt = 0;
    }

    return VOS_OK;
}
#endif

#define DECLARE_FUNC_PTR(func)      { if (!strcmp(func_name, #func)) return func; }

/*************************************************************************
 * 根据 检测函数名 获取 函数指针
 *************************************************************************/
chk_func hwmon_get_fun_ptr(char *func_name) 
{
    if (func_name == NULL) return NULL;

    DECLARE_FUNC_PTR(check_cpu_occupy);
    DECLARE_FUNC_PTR(check_mem_occupy);
    DECLARE_FUNC_PTR(check_cpu_temp);
    DECLARE_FUNC_PTR(check_board_temp);
    DECLARE_FUNC_PTR(ina2xx_reg_check);
    DECLARE_FUNC_PTR(fan_speed_check);
    DECLARE_FUNC_PTR(fpga_reg_check);
    DECLARE_FUNC_PTR(fpga_reg_dump);
    
    DECLARE_FUNC_PTR(clk_9FGV100X_reg_check);
    DECLARE_FUNC_PTR(clk_9FGV100X_reg_dump);
    DECLARE_FUNC_PTR(ad9544_reg_check);
    DECLARE_FUNC_PTR(ad9544_pll_check);
    DECLARE_FUNC_PTR(ad9544_reg_dump);

#ifdef INCLUDE_AD9528    
    DECLARE_FUNC_PTR(ad9528_reg_check);
    DECLARE_FUNC_PTR(ad9528_pll_check);
    DECLARE_FUNC_PTR(ad9528_reg_dump);
#endif    
    DECLARE_FUNC_PTR(check_cpri_state);
    
#ifdef INCLUDE_ADRV9009
    DECLARE_FUNC_PTR(adrv9009_reg_check);
    DECLARE_FUNC_PTR(adrv9009_pll_check);
    DECLARE_FUNC_PTR(adrv9009_reg_dump);
    DECLARE_FUNC_PTR(adrv9009_isr_check);
#endif    

#ifdef INCLUDE_UBLOX_GNSS
    DECLARE_FUNC_PTR(ublox_gps_lock_check);
#endif    
    
    return NULL;
}



