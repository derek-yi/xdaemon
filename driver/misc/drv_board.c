

#include "daemon_pub.h"

#include "drv_main.h"

/*
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-4/4-0048/hwmon/hwmon3/temp1_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-4/4-0049/hwmon/hwmon4/temp1_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-6/6-0048/hwmon/hwmon5/temp1_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-6/6-0049/hwmon/hwmon6/temp1_input
*/

int drv_get_board_temp(int temp_id, int *temp_val)
{
    int ret;
    int rd_val;
    char *sysfs_node[] = 
    {
        "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-4/4-0048/hwmon/hwmon3/temp1_input",
        "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-4/4-0049/hwmon/hwmon4/temp1_input",
        "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-6/6-0048/hwmon/hwmon5/temp1_input",
        "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-6/6-0049/hwmon/hwmon6/temp1_input",
    };

    if (temp_id < 0 || temp_id > 3) return VOS_ERR;
    if (temp_val == NULL) return VOS_ERR;

    ret = sys_node_read(sysfs_node[temp_id], &rd_val);
    if ( ret != VOS_OK )  return VOS_ERR;

    rd_val = rd_val/1000;
    *temp_val = rd_val;
    
    return VOS_OK;
}


