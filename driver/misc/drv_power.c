
#include "daemon_pub.h"

#include "drv_main.h"


/*
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0040/hwmon/hwmon1/curr1_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0040/hwmon/hwmon1/in0_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0040/hwmon/hwmon1/in1_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0040/hwmon/hwmon1/power1_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0040/hwmon/hwmon1/shunt_resistor

cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0041/hwmon/hwmon2/curr1_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0041/hwmon/hwmon2/in0_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0041/hwmon/hwmon2/in1_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0041/hwmon/hwmon2/power1_input
cat /sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0041/hwmon/hwmon2/shunt_resistor
*/

int drv_power_sensor_get(int chip, int type, int *value)
{
    int ret;
    int rd_val = 0;
    char *sysfs_node[] = 
    {
        "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0040/hwmon/hwmon1/power1_input",
        "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0041/hwmon/hwmon2/power1_input",
        "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0040/hwmon/hwmon1/curr1_input",
        "/sys/devices/soc0/amba/e0004000.i2c/i2c-0/i2c-7/7-0041/hwmon/hwmon2/curr1_input",
    };

    if (chip < 0 || chip > 1) return VOS_ERR;
    if (value == NULL) return VOS_ERR;

    if ( type == 0) { //power
        ret = sys_node_read(sysfs_node[chip], &rd_val);
        if ( ret != VOS_OK )  return VOS_ERR;
    }

    if ( type == 1) { //current
        ret = sys_node_read(sysfs_node[chip], &rd_val);
        if ( ret != VOS_OK )  return VOS_ERR;
    }

    *value = rd_val;
    return VOS_OK;
}


