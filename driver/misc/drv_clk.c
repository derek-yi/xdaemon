

#include "daemon_pub.h"

#include "drv_main.h"


/*
echo 0x000d > /sys/devices/soc0/amba/e0007000.spi/spi_master/spi1/spi1.0/ad9544
cat /sys/devices/soc0/amba/e0007000.spi/spi_master/spi1/spi1.0/ad9544
*/    
int clk_ad9544_reg_read(uint32 chip_id, uint32 reg_addr)
{
    int ret;
    char cmd_buf[16];
    char *sys_node = "/sys/devices/soc0/amba/e0007000.spi/spi_master/spi1/spi1.0/ad9544";

    ret = sys_node_write(sys_node, reg_addr);
    if ( ret != VOS_OK )  return DFAULT_VALUE;

    ret = sys_node_readstr(sys_node, cmd_buf, sizeof(cmd_buf));
    if ( ret != VOS_OK )  return DFAULT_VALUE;

    //ad9544_attr_show: sprintf(buf, "%02x", readbuf);
    ret = (int)strtoul(cmd_buf, 0, 16);
    
    return ret;
}

int clk_ad9544_reg_write(uint32 chip_id, uint32 reg_addr, uint32 value)
{
    int ret;
    char cmd_buf[32];
    char *sys_node = "/sys/devices/soc0/amba/e0007000.spi/spi_master/spi1/spi1.0/ad9544";

    //ad9544_attr_store
    snprintf(cmd_buf, sizeof(cmd_buf), "0x%x:0x%x", reg_addr, value);
    ret = sys_node_writestr(sys_node, cmd_buf);

    return ret;
}

int drv_ad9544_pll_locked  (uint32 chip_id)
{
    int ret;
	char buf[16];

    ret = sys_node_readstr("/sys/devices/soc0/amba/e0007000.spi/spi_master/spi1/spi1.0/ad9544_locked", buf, sizeof(buf));
    if ( ret != VOS_OK )  return VOS_ERR;

    if (memcmp(buf, "locked", 6) != 0) {
        return VOS_ERR;
    }
    
    return VOS_OK;
}

int clk_ad9528_reg_read(uint32 chip_id, uint32 reg_addr)
{
    int ret;
    int rd_value;
    char *sys_node = "/sys/kernel/debug/iio/iio:device1/direct_reg_access";

    ret = sys_node_write(sys_node, reg_addr);
    if ( ret != VOS_OK )  return DFAULT_VALUE;

    ret = sys_node_read(sys_node, &rd_value);
    if ( ret != VOS_OK )  return DFAULT_VALUE;

    return rd_value;
}

int clk_ad9528_reg_write(uint32 chip_id, uint32 reg_addr, uint32 value)
{
    int ret;
    char cmd_buf[32];
    char *sys_node = "/sys/kernel/debug/iio/iio:device1/direct_reg_access";

    //iio_debugfs_write_reg
    snprintf(cmd_buf, sizeof(cmd_buf), "0x%x 0x%x", reg_addr, value);
    ret = sys_node_writestr(sys_node, cmd_buf);

    return ret;
}


/*
0xEB:  high< 1110 1011 >low
cat /sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.0/iio:device1/pll1_locked 
cat /sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.0/iio:device1/pll2_locked
cat /sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.0/iio:device1/pll1_reference_clk_a_present
cat /sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.0/iio:device1/pll1_reference_clk_b_present
cat /sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.0/iio:device1/pll1_reference_clk_ab_missing
cat /sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.0/iio:device1/vcxo_clk_present
cat /sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.0/iio:device1/pll1_feedback_clk_present
cat /sys/devices/soc0/amba/e0006000.spi/spi_master/spi0/spi0.0/iio:device1/pll2_feedback_clk_present
*/
int drv_ad9528_pll_locked  (uint32 chip_id)
{
    if (clk_ad9528_reg_read(0, 0x508) != 0XEB) {
        return VOS_ERR;
    }
    
    return VOS_OK;
}

int clk_9FGV100X_reg_read(uint32 chip_id, uint32 reg_addr)
{
    return VOS_OK;
}

/*
name-9544 = "rhub-opt8-bulitin-gps";
name-9544 = "rhub-opt8-1588-master";
name-9544 = "rhub-opt8-1588";
name-9544 = "rhub-opt8-ext-gps";

name-9544 = "rru-opt8-4t4r-4p9g";
name-9544 = "rru-opt8-4t4r-3p5g";
name-9544 = "rru-opt8-2t2r-4p9g";
name-9544 = "rru-opt8-2t2r-3p5g";
*/
int drv_board_clk_case(char *rd_buf, int buf_max)
{
    int ret;

    ret = sys_node_readstr("/sys/firmware/devicetree/base/amba/spi@e0007000/clksync-ad9544@0/name-9544", rd_buf, buf_max);
    if ( ret != VOS_OK )  return VOS_ERR;

    return VOS_OK;
}

int drv_gnss_is_locked  (void)
{
    int ret;
    char rd_buf[128];

    drv_board_clk_case(rd_buf, sizeof(rd_buf));
    if ( strstr(rd_buf, "bulitin-gps") == NULL ) {
        return TRUE;
    }

    for (int i = 0; i < 10; i++) {
        ret = sys_node_readstr("/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-8/8-0042/tim_tp_sfn", rd_buf, sizeof(rd_buf));
        if ( ret != VOS_OK )  continue;

          if (!memcmp(rd_buf, "tpsfn", 5)) {
              return TRUE;
          }
          vos_msleep(20);
    }

    return FALSE;
}



