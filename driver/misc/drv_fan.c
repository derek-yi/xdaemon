

#include "daemon_pub.h"

#include "drv_main.h"
#include "drv_cpu.h"
#include "drv_fpga.h"

uint32 FAN_PWM_CTRL=0x43c30258;
uint32 FAN_T_BDRY=0x43c3025c;
uint32 FAN_T_CTRL=0x43c30260;
uint32 FAN_T_THRESHOLD=0x43c30264;
uint32 FAN_TEMPRATURE=0x43c30268;
uint32 FAN_RPS=0x43c3026c;
uint32 FAN_RPS1=0x43c30270;
uint32 FAN_RPS2=0x43c30274;
uint32 FAN_RT_TR=0x43c30278;
uint32 FAN_FW_TEMP=0x43c30280;
//uint32 SYS_STATE_BASE=0x43c30240;

uint32 pwm_max=125;
uint32 pwm_min=15;
uint32 pwm_step=11;
uint32 pwm_n_step=10;
uint32 t_max=358;  //#80     `echo "scale=0; (80+273.15)*4096/503.975/8" | bc`
uint32 t_min=338;  //#60     `echo "scale=0; (60+273.15)*4096/503.975/8" | bc`
uint32 t_step=2;
uint32 t_tolerance=2;
uint32 t_thr_up=3;
uint32 t_thr_down=3;

//function FAN_PWM_SET()
//function FAN_T_SET()
int drv_fan_init(void)
{
    uint32 tmp;
    
    //tmp=$((${pwm_n_step} | (${pwm_step}<<8) | (${pwm_min}<<16) | (${pwm_max}<<24)))
    //devmem $FAN_PWM_CTRL 32 $tmp    
    tmp = pwm_n_step | (pwm_step << 8) | (pwm_min << 16) | (pwm_max << 24);
    fpga_write(FAN_PWM_CTRL, tmp);

    //tmp=$(((${t_max}<<16) | ${t_min}))
    //devmem $FAN_T_BDRY 32 $tmp
    tmp = (t_max << 16) | t_min;
    fpga_write(FAN_T_BDRY, tmp);

    //tmp=$(((${t_step}<<16) | ${t_tolerance}))
    //devmem $FAN_T_CTRL 32 $tmp
    tmp = (t_step << 16) | t_tolerance;
    fpga_write(FAN_T_CTRL, tmp);

    //tmp=$(((${t_thr_up}<<16) | ${t_thr_down}))
    //devmem $FAN_T_THRESHOLD 32 $tmp
    tmp = (t_thr_up << 16) | t_thr_down;
    fpga_write(FAN_T_THRESHOLD, tmp);

    return VOS_OK;
}

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

    reg_val = (int)fpga_read(FAN_RPS);
    if (fan_id == 0) speed = reg_val & 0xffff;
    else speed = reg_val >> 16;

    speed = (speed*100)/320;
    *ret_value = speed;
    
    return VOS_OK;
}

