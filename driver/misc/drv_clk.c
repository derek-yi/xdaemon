

#include "daemon_pub.h"

#include "drv_main.h"


/*
echo 0x000d > /sys/devices/soc0/amba/e0007000.spi/spi_master/spi1/spi1.0/ad9544
cat /sys/devices/soc0/amba/e0007000.spi/spi_master/spi1/spi1.0/ad9544
*/    
int clk_ad9544_reg_read(int chip_id, int reg_addr)
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

int drv_ad9544_pll_locked  (int chip_id)
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

int clk_ad9528_reg_read(int chip_id, int reg_addr)
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
int drv_ad9528_pll_locked  (int chip_id)
{
    if (clk_ad9528_reg_read(0, 0x508) != 0XEB) {
        return VOS_ERR;
    }
    
    return VOS_OK;
}

int clk_9FGV100X_reg_read(int chip_id, int reg_addr)
{
    return VOS_OK;
}

int drv_gnss_is_locked  ()
{
    int ret;
    char cmd_buf[128];

    for (int i = 0; i < 100; i++) {
        ret = sys_node_readstr("/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-8/8-0042/tim_tp_sfn", cmd_buf, sizeof(cmd_buf));
        if ( ret != VOS_OK )  continue;

          if (!memcmp(cmd_buf, "tpsfn", 5)) {
              return VOS_OK;
          }
          vos_msleep(20);
    }

    return VOS_ERR;
}



