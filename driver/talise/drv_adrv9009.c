

#include "daemon_pub.h"

#include "drv_main.h"


int adrv9009_reg_read(int chip_id, int reg_addr)
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

int adrv9009_pll_locked  (int chip_id)
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


