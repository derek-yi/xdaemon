#!/bin/bash

# version V1.0.1

#temperature adc value is 12bit ,but we only take the up 9bit for easy, so *8 or /8 before calculate
#the function is temperature = adc*503.975/4096 - 273.15

export PATH=$PATH:/usr/local/sbin/:/usr/local/bin/

FAN_PWM_CTRL=0x43c30258
FAN_T_BDRY=0x43c3025c
FAN_T_CTRL=0x43c30260
FAN_T_THRESHOLD=0x43c30264
FAN_TEMPRATURE=0x43c30268
FAN_RPS=0x43c3026c
FAN_RPS1=0x43c30270
FAN_RPS2=0x43c30274
FAN_RT_TR=0x43c30278
FAN_FW_TEMP=0x43c30280
SYS_STATE_BASE=0x43c30240

pwm_max=125
pwm_min=15
pwm_step=11
pwm_n_step=10
t_max=358  #80     `echo "scale=0; (80+273.15)*4096/503.975/8" | bc`
t_min=338  #60     `echo "scale=0; (60+273.15)*4096/503.975/8" | bc`
t_step=2
t_tolerance=2
t_thr_up=3
t_thr_down=3

#devmem ${FAN_FW_TEMP} $(( (1<<24) | (1<<16) | 50 ))

function FAN_PWM_SET()
{
    tmp=$((${pwm_n_step} | (${pwm_step}<<8) | (${pwm_min}<<16) | (${pwm_max}<<24)))
    devmem $FAN_PWM_CTRL 32 $tmp

    devmem $FAN_PWM_CTRL 32
    printf "  FAN_PWM_CTRL %X\r\n" ${tmp}
}

function FAN_T_SET()
{
    tmp=$(((${t_max}<<16) | ${t_min}))
    devmem $FAN_T_BDRY 32 $tmp

    devmem $FAN_T_BDRY 32
    printf "  FAN_T_BDRY %X\r\n" ${tmp}

    tmp=$(((${t_step}<<16) | ${t_tolerance}))
    devmem $FAN_T_CTRL 32 $tmp

    devmem $FAN_T_CTRL 32
    printf "  FAN_T_CTRL %X\r\n" ${tmp}

    tmp=$(((${t_thr_up}<<16) | ${t_thr_down}))
    devmem $FAN_T_THRESHOLD 32 $tmp

    devmem $FAN_T_THRESHOLD 32
    printf "  FAN_T_THRESHOLD %X\r\n" ${tmp}
}

FAN_PWM_SET
FAN_T_SET

i=0
test_t=0

while ((1))
do
    time_l=`date +%s%N`

    tmp=`devmem $FAN_TEMPRATURE 32`
    tmp1=$(( ($tmp & 0xffff)*8 ))
    tmp1=`echo "scale=0; (${tmp1}*503.975/4096-273.15)/1" | bc`
    tmp2=$(( ( ($tmp>>16)& 0xffff)*8 ))
    tmp2=`echo "scale=0; (${tmp2}*503.975/4096-273.15)/1" | bc`
    printf " FAN_TEMPRATURE is %d %d\r\n"  ${tmp1} ${tmp2}

    echo $tmp1 > /tmp/cpu_temperature.txt
    (( $tmp1 > 90 )) && devmem ${SYS_STATE_BASE} 32 1
    (( $tmp1 > 90 )) || devmem ${SYS_STATE_BASE} 32 4

    tmp=`devmem $FAN_RPS 32`
    fan1=$(( ($tmp & 0xffff) ))
    fan2=$(( ($tmp>>16) ))
    fan1=`echo "scale=0; ($fan1*100)/320" | bc`
    fan2=`echo "scale=0; ($fan2*100)/320" | bc`

    printf " FAN_RPS is %d %d %d%% %d%%\r\n" $((($tmp & 0xffff))) $((($tmp>>16))) $fan1 $fan2

    tmp=`devmem $FAN_RT_TR 32`
    fp1=$((($tmp>>16)))
    fp1=`echo "scale=0; ($fp1*100)/125" | bc`
    printf " FAN_RT_TR is %d %d%%\r\n" $((($tmp>>16))) $fp1

    echo $fp1"%" > /tmp/fan_power.txt
    echo $fan1"%" > /tmp/fan1.txt
    echo $fan2"%" > /tmp/fan2.txt
    (($fp1 > 30)) && (( (${fp1}/2) > $fan1 )) && echo "warning" >> /tmp/fan1.txt
    (($fp1 > 30)) && (( (${fp1}/2) > $fan2 )) && echo "warning" >> /tmp/fan2.txt
    echo "========"

    i=$(($i+1))
    (( ($i%5) == 0 )) && test_t=$(( ($test_t+1) % 90 ))

    #devmem ${FAN_FW_TEMP} $(( (1<<24) | (1<<16) | ${test_t} ))

    time_n=`date +%s%N`

    gap=$((1000000 - ($time_n-$time_l)/1000))

    gap=`echo "scale=6; $gap/1000000" | bc`

    #sleep $gap
    sleep 1

done

