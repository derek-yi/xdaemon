

#include "daemon_pub.h"

#include "drv_main.h"
#include "drv_fpga.h"

#ifdef INCLUDE_ADRV9009

int adrv9009_reg_read(uint32 chip_id, uint32 reg_addr)
{
    int ret;
    int rd_value;
    char *sysfs_node;

    if (chip_id == 0) sysfs_node = "/sys/kernel/debug/iio/iio:device2/direct_reg_access";
    else if (chip_id == 1) sysfs_node = "/sys/kernel/debug/iio/iio:device3/direct_reg_access";
    else return VOS_ERR;       

    ret = sys_node_write(sysfs_node, reg_addr);
    if ( ret != VOS_OK )  return DFAULT_VALUE;

    ret = sys_node_read(sysfs_node, &rd_value);
    if ( ret != VOS_OK )  return DFAULT_VALUE;

    return rd_value;
}

int adrv9009_pll_locked  (uint32 chip_id)
{
    int ret;
    char buf[16] = {0};
    char *sysfs_node;

    if (chip_id == 0) sysfs_node = "/sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.1/iio:device2/pll_locked";
    else if (chip_id == 1) sysfs_node = "/sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.2/iio:device3/pll_locked";
    else return VOS_ERR;       

    ret = sys_node_readstr(sysfs_node, buf, sizeof(buf));
    if ( ret != VOS_OK )  return VOS_ERR;

    if (memcmp(buf, "locked", 6) != 0) {
        return VOS_ERR;
    }
    
    return VOS_OK;
}

//echo -10.5 > /sys/bus/iio/devices/iio\:device2/out_voltage1_hardwaregain
int adrv9009_set_tx_atten(uint32 chip_id, uint32 channel, float atten)
{
    int ret;
    char sysfs_node[128];
    char buf[32];

    if (chip_id >= SYS_MAX_RF_CHIP || channel > 1 ) {
        return VOS_ERR;
    }

    sprintf(sysfs_node, "/sys/bus/iio/devices/iio:device%d/out_voltage%d_hardwaregain", chip_id + 2, channel);
    sprintf(buf, "%f", atten);
    ret = sys_node_writestr(sysfs_node, buf);
    if ( ret != VOS_OK )  return VOS_ERR;
    
    return VOS_OK;
}

//cat /sys/bus/iio/devices/iio\:device2/out_voltage1_hardwaregain
int adrv9009_get_tx_atten(uint32 chip_id, uint32 channel, float *atten)
{
    int ret;
    char sysfs_node[128];
    char buf[32] = {0};

    if (chip_id >= SYS_MAX_RF_CHIP || channel > 1 || atten == NULL) {
        return VOS_ERR;
    }

    sprintf(sysfs_node, "/sys/bus/iio/devices/iio:device%d/out_voltage%d_hardwaregain", chip_id + 2, channel);
    ret = sys_node_readstr(sysfs_node, buf, sizeof(buf));
    if ( ret != VOS_OK )  return VOS_ERR;

    *atten = (float)strtof(buf, 0);
    return VOS_OK;
}

/*
TDDC_Control_Source_Register 32
    bit 31~8: ---> TDDC_sync_version: default:"zo1"(0x7a3031)
    bit 7~5: ---> reserved
    bit 4  ---> Pa_power_enable: 1:enable pa_power  0: disable pa_power
    bit 3  ---> Sync_PPS_disable: 1: not sync 1PPS  0: sync 1PPS
    bit 2  ---> TDDC_Reset 1: FPGA TDDC module reset
    bit 1  ---> TDDC_Sync_Enable(Trig) 1:enable FPGA TDDC module
    bit 0  ---> 0:GPIO control source is ARM 1:GPIO control source is FPGA
Daemon>cat /sys/bus/iio/devices/iio:device2/out_altvoltage0_TRX_LO_frequency
3500 000000    
*/
int adrv9009_set_tx_freq(uint32 chip_id, uint32 freq)
{
    uint32 real_freq;
    char buf[32];
    
    if (chip_id >= SYS_MAX_RF_CHIP || freq < 2000) {
        return VOS_ERR;
    }

    //Close radio transfer
    if (chip_id) sys_node_writestr("/sys/bus/iio/devices/iio:device3/ensm_mode", "radio_off");
    else sys_node_writestr("/sys/bus/iio/devices/iio:device2/ensm_mode", "radio_off");

    //reset the TDD module in FPGA
    fpga_write_bits(0x43c50000, 2, 0x1, 0);

    //set new carried frequency to ADRV9009
    real_freq = (uint32)( freq * 1000000 );
    sprintf(buf, "%lu", real_freq);
    if (chip_id) {
        sys_node_writestr("/sys/bus/iio/devices/iio:device3/out_altvoltage0_TRX_LO_frequency", buf);
    } else {
        sys_node_writestr("/sys/bus/iio/devices/iio:device2/out_altvoltage0_TRX_LO_frequency", buf);
    }

    real_freq -= 20*1000000; //todo: why AUX_OBS_RX
    sprintf(buf, "%lu", real_freq);
    if (chip_id) {
        sys_node_writestr("/sys/bus/iio/devices/iio:device3/out_altvoltage1_AUX_OBS_RX_LO_frequency", buf);
    } else {
        sys_node_writestr("/sys/bus/iio/devices/iio:device2/out_altvoltage1_AUX_OBS_RX_LO_frequency", buf);
    }

    //open the TDD module in FPGA, ps: set TRIG
    fpga_write_bits(0x43c50000, 2, 0x1, 1);

    //Open radio transfer
    if (chip_id) sys_node_writestr("/sys/bus/iio/devices/iio:device3/ensm_mode", "radio_on");
    else sys_node_writestr("/sys/bus/iio/devices/iio:device2/ensm_mode", "radio_on");

    //control switch to fpga
    fpga_write(0x43c50000, 0x13); //todo
        
    return VOS_OK;
}

int adrv9009_get_tx_freq(uint32 chip_id, uint32 *freq)
{
    uint32 real_freq;
    
    if (chip_id >= SYS_MAX_RF_CHIP || freq == NULL) {
        return VOS_ERR;
    }

    if (chip_id) {
        sys_node_read("/sys/bus/iio/devices/iio:device3/out_altvoltage0_TRX_LO_frequency", (int *)&real_freq);
        *freq = (real_freq/1000000);
    } else {
        sys_node_read("/sys/bus/iio/devices/iio:device2/out_altvoltage0_TRX_LO_frequency", (int *)&real_freq);
        *freq = (real_freq/1000000);
    }
    
    return VOS_OK;
}

#endif

