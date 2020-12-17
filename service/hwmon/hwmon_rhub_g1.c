


#include "daemon_pub.h"

#include "hwmon_main.h"
#include "drv_fpga.h"
#include "devm_fru.h"


#ifdef BOARD_RHUB_G1

static int dbg_mode = 0;

/*************************************************************************
 * 重置检测点的配置
 *************************************************************************/
int hwmon_config_override()
{
    
#ifndef __ARM_ARCH
    hwmon_set_enable("cpu.occupy",          TRUE);
    hwmon_set_enable("mem.occupy",          TRUE);
#endif

    return VOS_OK;
}

uint32 CPRI_BASE[MAX_CPRI_CNT]= {0x80000000, 0x80010000, 0x80020000, 0x80030000, 0x80040000, 0x80050000, 0x80060000, 0x80070000};

uint32 VENDOR_WRITE_BASE[MAX_CPRI_CNT] = {0x43D80000, 0x43D90000, 0x43DA0000, 0x43DB0000, 0x43DC0000, 0x43DD0000, 0x43DE0000, 0x43DF0000};

uint32 VENDOR_READ_BASE[MAX_CPRI_CNT] = {0x43D80100, 0x43D90100, 0x43DA0100, 0x43DB0100, 0x43DC0100, 0x43DD0100, 0x43DE0100, 0x43DF0100};

uint32 PHY_DETECT_BASE=0x43c30230;
uint32 PHY_LOST_SIGNAL_BASE=0x43c30234;

uint32 BBU_STATE_BASE=0x43c30238;
uint32 CAS_STATE_BASE=0x43c3023c;
uint32 SYS_STATE_BASE=0x43c30240;

uint32 RRU_STATE_BASE1=0x43c30244;
uint32 RRU_STATE_BASE2=0x43c30248;

int DELAY_A = 630;
int DELAY_B = 0;
int DELAY_D = 363;

uint32 CPRI_LINK_STAT[MAX_CPRI_CNT];
uint32 CPRI_SLAVE_4T4R[MAX_CPRI_CNT];
int CPRI_DELAY_CTRL_VALUE[MAX_CPRI_CNT];


void RRU_STATE()
{
    uint32 res = fpga_read(PHY_DETECT_BASE);
    uint32 rru_stat = 0;

    //((${CPRI_LINK_STAT[0]} == 1)) && rru_stat=$(($rru_stat | 0x01000000))
    if (CPRI_LINK_STAT[0] == 1) rru_stat = rru_stat | 0x01000000;
    //((${CPRI_LINK_STAT[0]} == 0)) && (($res & 4)) && rru_stat=$(($rru_stat | 0x02000000))
    if ((CPRI_LINK_STAT[0] == 0) && (res & 4)) rru_stat = rru_stat | 0x02000000;

    //((${CPRI_LINK_STAT[1]} == 1)) && rru_stat=$(($rru_stat | 0x00010000))
    if (CPRI_LINK_STAT[1] == 1) rru_stat = rru_stat | 0x00010000;
    //((${CPRI_LINK_STAT[1]} == 0)) && (($res & 8)) && rru_stat=$(($rru_stat | 0x00020000))
    if ((CPRI_LINK_STAT[1] == 0) && (res & 8)) rru_stat = rru_stat | 0x00020000;

    //((${CPRI_LINK_STAT[2]} == 1)) && rru_stat=$(($rru_stat | 0x00000100))
    if (CPRI_LINK_STAT[2] == 1) rru_stat = rru_stat | 0x00000100;
    //((${CPRI_LINK_STAT[2]} == 0)) && (($res & 16)) && rru_stat=$(($rru_stat | 0x00000200))
    if ((CPRI_LINK_STAT[2] == 0) && (res & 16)) rru_stat = rru_stat | 0x00000200;

    //((${CPRI_LINK_STAT[3]} == 1)) && rru_stat=$(($rru_stat | 0x00000001))
    if (CPRI_LINK_STAT[3] == 1) rru_stat = rru_stat | 0x00000001;
    //((${CPRI_LINK_STAT[3]} == 0)) && (($res & 32)) && rru_stat=$(($rru_stat | 0x00000002))
    if ((CPRI_LINK_STAT[3] == 0) && (res & 32)) rru_stat = rru_stat | 0x00000002;
    
    //devmem $RRU_STATE_BASE1 32 $rru_stat
    fpga_write(RRU_STATE_BASE1, rru_stat);

    rru_stat=0;
    //((${CPRI_LINK_STAT[4]} == 1)) && rru_stat=$(($rru_stat | 0x01000000))
    if (CPRI_LINK_STAT[4] == 1) rru_stat = rru_stat | 0x01000000;
    //((${CPRI_LINK_STAT[4]} == 0)) && (($res & 64)) && rru_stat=$(($rru_stat | 0x02000000))
    if ((CPRI_LINK_STAT[4] == 0) && (res & 64)) rru_stat = rru_stat | 0x02000000;

    //((${CPRI_LINK_STAT[5]} == 1)) && rru_stat=$(($rru_stat | 0x00010000))
    if (CPRI_LINK_STAT[5] == 1) rru_stat = rru_stat | 0x00010000;
    //((${CPRI_LINK_STAT[5]} == 0)) && (($res & 128)) && rru_stat=$(($rru_stat | 0x00020000))
    if ((CPRI_LINK_STAT[5] == 0) && (res & 128)) rru_stat = rru_stat | 0x00020000;

    //((${CPRI_LINK_STAT[6]} == 1)) && rru_stat=$(($rru_stat | 0x00000100))
    if (CPRI_LINK_STAT[6] == 1) rru_stat = rru_stat | 0x00000100;
    //((${CPRI_LINK_STAT[6]} == 0)) && (($res & 256)) && rru_stat=$(($rru_stat | 0x00000200))
    if ((CPRI_LINK_STAT[6] == 0) && (res & 256)) rru_stat = rru_stat | 0x00000200;

    //((${CPRI_LINK_STAT[7]} == 1)) && rru_stat=$(($rru_stat | 0x00000001))
    if (CPRI_LINK_STAT[7] == 1) rru_stat = rru_stat | 0x00000001;
    //((${CPRI_LINK_STAT[7]} == 0)) && (($res & 512)) && rru_stat=$(($rru_stat | 0x00000002))
    if ((CPRI_LINK_STAT[7] == 0) && (res & 512)) rru_stat = rru_stat | 0x00000002;

    //devmem $RRU_STATE_BASE2 32 $rru_stat
    fpga_write(RRU_STATE_BASE2, rru_stat);
}

void BBU_STATE()
{
    uint32 res = fpga_read(PHY_DETECT_BASE);
    res = res & 0x1;
    uint32 res1 = fpga_read(PHY_LOST_SIGNAL_BASE);
    res1 = res1 & 0x1;
    //res2 = `/root/xge-xlnx-smi.sh 3 1 0x43c40000`
    uint32 res2 = xlnx_smi_r(0x43c40000, 3, 1);
    res2 = res2 & 0x4;
    
    //(($res2 == 1)) && devmem $BBU_STATE_BASE 32 0x3 && return
    if (res2) {
        fpga_write(BBU_STATE_BASE, 0x3);
        return ;
    }
    //(($res1 == 0)) && devmem $BBU_STATE_BASE 32 0x4 && return
    if (!res1) {
        fpga_write(BBU_STATE_BASE, 0x4);
        return ;
    }
    //(($res == 1)) && devmem $BBU_STATE_BASE 32 0x2 && return
    if (res) {
        fpga_write(BBU_STATE_BASE, 0x2);
        return ;
    }
    //devmem $BBU_STATE_BASE 32 0x0
    fpga_write(BBU_STATE_BASE, 0x0);
}

void CAS_STATE()
{
    uint32 res = fpga_read(PHY_DETECT_BASE);
    res = res & 0x2;
    uint32 res1 = fpga_read(PHY_LOST_SIGNAL_BASE);
    res1 = res1 & 0x2;
    //res2 = `/root/xge-xlnx-smi.sh 3 1 0x43c90000`
    uint32 res2 = xlnx_smi_r(0x43c90000, 3, 1);
    res2 = res2 & 0x4;
    
    //(($res2 == 1)) && devmem $CAS_STATE_BASE 32 0x3 && return
    if (res2) {
        fpga_write(CAS_STATE_BASE, 0x3);
        return ;
    }
    //(($res1 == 0)) && devmem $CAS_STATE_BASE 32 0x4 && return
    if (!res1) {
        fpga_write(CAS_STATE_BASE, 0x4);
        return ;
    }
    //(($res == 1)) && devmem $CAS_STATE_BASE 32 0x2 && return
    if (res) {
        fpga_write(CAS_STATE_BASE, 0x2);
        return ;
    }
    //devmem $CAS_STATE_BASE 32 0x0
    fpga_write(CAS_STATE_BASE, 0x0);
}

void CPRI_DELAY_CTRL()
{
    uint32 tmp_value = 0;
        
    tmp_value = (CPRI_DELAY_CTRL_VALUE[0] & 0xFF);
    tmp_value = ((CPRI_DELAY_CTRL_VALUE[1] & 0xFF) << 8) | tmp_value;
    tmp_value = ((CPRI_DELAY_CTRL_VALUE[2] & 0xFF) << 16) | tmp_value;
    tmp_value = ((CPRI_DELAY_CTRL_VALUE[3] & 0xFF) << 24) | tmp_value;
    fpga_write(0x43c30168, tmp_value);
    fpga_write(0x43c30174, tmp_value);

    tmp_value = (CPRI_DELAY_CTRL_VALUE[4] & 0xFF);
    tmp_value = ((CPRI_DELAY_CTRL_VALUE[5] & 0xFF) << 8) | tmp_value;
    tmp_value = ((CPRI_DELAY_CTRL_VALUE[6] & 0xFF) << 16) | tmp_value;
    tmp_value = ((CPRI_DELAY_CTRL_VALUE[7] & 0xFF) << 24) | tmp_value;
    fpga_write(0x43c3016C, tmp_value);
    fpga_write(0x43c30178, tmp_value);
}

//cpri-r21-mul.sh
int cpri_link_monitor(void *param)
{
    int i, j;
    uint32 value, state_reg;
    uint32 cpri_r21, time_offset;
    float rx_fifo_delay, tx_fifo_delay, _r21, r21;
    char uuid_str[64];
    char temp_buf[128];
    
    shell_run_cmd("rm -f /tmp/cpri_stat.tmp");
    for (i = 0; i < MAX_CPRI_CNT; i++) {
        //cpri link check
        value = fpga_read(CPRI_BASE[i]);
        if ( (value & 0xF) == 0xF) {
            if(dbg_mode) xlog(XLOG_INFO, "CRPI %d LINK IS NOT UP", i);
            CPRI_LINK_STAT[i] = 1;

            state_reg = fpga_read(CPRI_BASE[i] + 4);
            vos_msleep(500);
            state_reg = fpga_read(CPRI_BASE[i] + 4);
            vos_msleep(500);
            state_reg = fpga_read(CPRI_BASE[i] + 4);
            if ( (state_reg& 0xff03) != 0) {
                if(dbg_mode) xlog(XLOG_WARN, "cpri %d reg 0x4 is %x, need reset", i, state_reg);
                fpga_write(0x43c30294, 0x0);
                fpga_write(0x43c30294, 0x0);
                fpga_write(0x43c30294, 0x1);
                vos_msleep(3000);
            }
            
        } else {
            if(dbg_mode) xlog(XLOG_INFO, "CRPI %d LINK IS NOT UP", i);
            CPRI_LINK_STAT[i] = 0;
        }

        //cal fifo delay
        cpri_r21 = fpga_read(CPRI_BASE[i] + 0x3C);
        rx_fifo_delay = (float)(cpri_r21 >> 18);
        cpri_r21 = (cpri_r21 & 0x3FFFF) * 33;
        if(dbg_mode) xlog(XLOG_INFO, "cpri_rx_fifo_delay is %f", rx_fifo_delay);
        rx_fifo_delay = (rx_fifo_delay*33)/128;
        value = fpga_read(CPRI_BASE[i] + 0x50);
        tx_fifo_delay = (float)(value & 0x3FFF);
        tx_fifo_delay = (tx_fifo_delay*33)/128;
        _r21 = cpri_r21 - DELAY_A - DELAY_B - rx_fifo_delay - tx_fifo_delay - DELAY_D;
        if(dbg_mode) xlog(XLOG_INFO, "CPRI %d _r21 is %f, rx delay is %f, tx delay is %f", i, _r21, rx_fifo_delay, tx_fifo_delay);

        fpga_write(VENDOR_WRITE_BASE[i], i + 1); //RRU_ID_W $(($i+1))
        fpga_write(VENDOR_WRITE_BASE[i] + 4, 11); //NO_OF_LAYER 11
        fpga_write(VENDOR_WRITE_BASE[i] + 8, 123456);//TRANS_POWER 123456
        
        //SN `cat /tmp/uuid.txt`
        devm_fru_get_uuid(uuid_str, sizeof(uuid_str));
        for(j = 0; j < 36/4; j++) {
            value = uuid_str[j*4];
            value = (value << 8) | uuid_str[j*4 + 1];
            value = (value << 8) | uuid_str[j*4 + 2];
            value = (value << 8) | uuid_str[j*4 + 3];
            fpga_write(VENDOR_WRITE_BASE[i] + 12 + j*4, value);
        }

        CPRI_SLAVE_4T4R[i] = fpga_read(VENDOR_READ_BASE[i] + 44);

        memset(uuid_str, 0, sizeof(uuid_str));
        for(j = 0; j < 36/4; j++) {
            value = fpga_read(VENDOR_READ_BASE[i] + 8 + j*4);
            uuid_str[j*4] = (value >> 24) & 0xFF;
            uuid_str[j*4 + 1] = (value >> 16) & 0xFF;
            uuid_str[j*4 + 2] = (value >> 8) & 0xFF;
            uuid_str[j*4 + 3] = (value) & 0xFF;
        }
        if(dbg_mode) xlog(XLOG_INFO, "CPRI id is %d, rhub sn is %s", i, uuid_str);

        fpga_read(VENDOR_READ_BASE[i]);//RRU_ID
        fpga_read(VENDOR_READ_BASE[i] + 4);//T_OFFSET

        time_offset = fpga_read(VENDOR_READ_BASE[i] + 4);
        r21 = 98.643*(_r21 - time_offset)/2;
        if(dbg_mode) xlog(XLOG_INFO, "CPRI %d time_off is %d, r21 is %f", i, time_offset, r21);
        r21 = r21/8138.02083;
        value = (uint32)r21;
        if (value >= 10) CPRI_DELAY_CTRL_VALUE[i] = 246 - value;
        else CPRI_DELAY_CTRL_VALUE[i] = 246;
        if (CPRI_DELAY_CTRL_VALUE[i] < 0) CPRI_DELAY_CTRL_VALUE[i] = 0xfe;
        if (CPRI_LINK_STAT[i] == 0) CPRI_DELAY_CTRL_VALUE[i] = 0xff;
        if(dbg_mode) xlog(XLOG_INFO, "CPRI %d r21 register is %d, CPRI_DELAY_CTRL_VALUE %d", i, value, CPRI_DELAY_CTRL_VALUE[i]);
        
        //echo $i ${CPRI_LINK_STAT[$i]} ${CPRI_SLAVE_SN[$i]} $((${CPRI_SLAVE_4T4R[$i]} + 0))>> /tmp/cpri_stat.tmp
        sprintf(temp_buf, "echo %d %d %s %d >> /tmp/cpri_stat.tmp", i, CPRI_LINK_STAT[i], uuid_str, CPRI_SLAVE_4T4R[i]);
        shell_run_cmd(temp_buf);
    }

    shell_run_cmd("cp -f /tmp/cpri_stat.tmp /tmp/cpri_stat.txt");

    BBU_STATE();
    RRU_STATE();
    CAS_STATE();
    CPRI_DELAY_CTRL();
    return VOS_OK;
}



#endif

