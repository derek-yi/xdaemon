

#include "daemon_pub.h"

#include "drv_main.h"
#include "drv_cpu.h"


/* fan.sh
tmp=`devmem $FAN_RPS 32`  //0x43c3026c
fan1=$(( ($tmp & 0xffff) ))
fan2=$(( ($tmp>>16) ))
fan1=`echo "scale=0; ($fan1*100)/320" | bc`
fan2=`echo "scale=0; ($fan2*100)/320" | bc`

*/
int drv_fan_get_speed(int fan_id, int *ret_value)
{
    int reg_val;
    int speed;
    
    if (fan_id < 0 || fan_id > 1) return VOS_ERR;
    if (ret_value == NULL) return VOS_ERR;

    reg_val = devmem_read(0x43c3026c, AT_WORD);
    if (fan_id == 0) speed = reg_val & 0xffff;
    else speed = reg_val >> 16;

    speed = (speed*100)/320;
    *ret_value = speed;
    
    return VOS_OK;
}


