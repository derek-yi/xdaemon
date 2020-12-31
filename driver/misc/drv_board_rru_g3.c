

#include "daemon_pub.h"

#include "drv_main.h"
#include "drv_cpu.h"
#include "drv_fpga.h"

#ifdef BOARD_RRU_G3    

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

int drv_get_eeprom_info(int fru_id, EEPROM_INFO *info)
{
    if (fru_id == 1) {
        info->i2c_bus       = 6;
        info->dev_id        = 0x50;
        info->chip_size     = 256;
    } else if (fru_id == 0) {
        info->i2c_bus       = 3;
        info->dev_id        = 0x54;
        info->chip_size     = 1024;
    } else {
        return VOS_ERR;
    }
    
    return VOS_OK;
}

/*
function CPU_TEMPERATURE()
{
    tmp=`devmem 0x43ca0200`
    tmp=$(($tmp >> 4))
    tmp=`echo "scale=0; ($tmp*503.975/4096 - 273.15)/1" | bc`
    echo $tmp > /tmp/cpu_temperature.txt
}
*/
/*************************************************************************
 * 获取cpu温度
 *************************************************************************/
int drv_get_cpu_temp(int *temp)
{
    int cpu_temp;
    float float_tmp;

    if (temp == NULL) return VOS_ERR;
    
    cpu_temp = devmem_read(0x43ca0200, 4);
    cpu_temp = devmem_read(0x43ca0200, 4); //repeat
    float_tmp = (float)(cpu_temp >> 4);
    float_tmp = (float_tmp*503.975/4096 - 273.15);
    *temp = (int)float_tmp;
    
    return VOS_OK;
}

/*
0x43c30018
Bit[0]  decompress_dis
0 : Send the decompressed data to dfe // 对应4T4R   
1: Send the uncompressed data to dfe// 对应2T2
*/
int drv_get_channel_cnt(void)
{
    if ( fpga_read_bits(0x43c30018, 0, 0x1) ) return 2;
    
    return 4;
}

#endif 

