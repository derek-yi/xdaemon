
#include "daemon_pub.h"

#include "hwmon_main.h"
#include "drv_fpga.h"
#include "devm_fru.h"


#ifdef BOARD_RRU_G3

static int dbg_mode = 0;

/*************************************************************************
 * 重置检测点的配置
 *************************************************************************/
int hwmon_config_override()
{
#ifndef DAEMON_RELEASE
    CHK_NODE_CFG_S new_node;

    memset(&new_node, 0, sizeof(CHK_NODE_CFG_S));
    new_node.node_desc  = "rf0.isr.chk";
    new_node.func_name  = "adrv9009_isr_check";
    new_node.interval   = 10;
    new_node.param[0] = 0;
    hwmon_register(&new_node);
#endif    
    
#ifndef __ARM_ARCH
    hwmon_set_enable("cpu.occupy",          TRUE);
    hwmon_set_enable("mem.occupy",          TRUE);
#endif

    return VOS_OK;
}



uint32 CPRI_BASE[MAX_CPRI_CNT]= {0x43C40000};

uint32 VENDOR_WRITE_BASE[MAX_CPRI_CNT] = {0x43C90000};

uint32 VENDOR_READ_BASE[MAX_CPRI_CNT] = {0x43C90100};

int DELAY_A = 630;
int DELAY_B = 0;
int DELAY_D = 363;

uint32 CPRI_LINK_STAT[MAX_CPRI_CNT];

//cpri-r21-mul-slave.sh
int cpri_link_monitor(void *param)
{
    int i, j;
    uint32 rru_id, value;
    char temp_buf[128];
    char uuid_str[64];
    uint32 cpri_r21;
    float rx_fifo_delay, tx_fifo_delay, time_offset;

    for (i = 0; i < MAX_CPRI_CNT; i++) {
        //cpri link check
        value = fpga_read(CPRI_BASE[i]);
        if ( (value & 0xF) == 0xF) {
            if(dbg_mode) xlog(XLOG_INFO, "CRPI %d LINK IS NOT UP", i);
            CPRI_LINK_STAT[i] = 1;
        } else {
            if(dbg_mode) xlog(XLOG_INFO, "CRPI %d LINK IS NOT UP", i);
            CPRI_LINK_STAT[i] = 0;
        }

        //T_OFFSET $time_offest
        cpri_r21 = fpga_read(CPRI_BASE[i] + 0x3c);
        rx_fifo_delay = (float)(cpri_r21 >> 18);
        cpri_r21 = (cpri_r21 & 0x3FFFF) * 33;
        rx_fifo_delay = (rx_fifo_delay*33)/128;
        value = fpga_read(CPRI_BASE[i] + 0x50);
        tx_fifo_delay = (float)(value & 0x3FFF);
        tx_fifo_delay = (tx_fifo_delay*33)/128;
        time_offset = DELAY_A + DELAY_B + rx_fifo_delay + tx_fifo_delay + DELAY_D + cpri_r21;
        value = (uint32)time_offset;
        if(dbg_mode) xlog(XLOG_INFO, "CPRI %d R21 is %d, time_off is %d, rx delay is %f, tx delay is %f", i, cpri_r21, value, rx_fifo_delay, tx_fifo_delay);

        rru_id = fpga_read(VENDOR_READ_BASE[i]); //rru_id=`devmem $VENDOR_RRU_ID 32`
        fpga_read(VENDOR_READ_BASE[i] + 4); //NO_OF_LAYER
        fpga_read(VENDOR_READ_BASE[i] + 8); //TRANS_POWER

        memset(uuid_str, 0, sizeof(uuid_str));
        for(j = 0; j < 36/4; j++) {
            value = fpga_read(VENDOR_READ_BASE[i] + 12 + j*4);
            uuid_str[j*4] = (value >> 24) & 0xFF;
            uuid_str[j*4 + 1] = (value >> 16) & 0xFF;
            uuid_str[j*4 + 2] = (value >> 8) & 0xFF;
            uuid_str[j*4 + 3] = (value) & 0xFF;
        }
        if(dbg_mode) xlog(XLOG_INFO, "CPRI id is %d, rhub sn is %s", i, uuid_str);

        //echo ${CPRI_LINK_STAT[$i]} $(($rru_id + 0)) $_tmp_sn > /tmp/cpri_stat.txt
        sprintf(temp_buf, "echo %d %d %s > /tmp/cpri_stat.txt", CPRI_LINK_STAT[i], rru_id, uuid_str);
        shell_run_cmd(temp_buf);

        fpga_write(VENDOR_WRITE_BASE[i], rru_id);//RRU_ID_W $rru_id
        value = (uint32)time_offset;
        fpga_write(VENDOR_WRITE_BASE[i] + 4, value); //T_OFFSET $time_offest

        //CPRI_4T4R_W 
        value = fpga_read(FPGA_VER_ADDRESS);
        if ((value&0x00ff0000) == 0x00040000) 
            fpga_write(VENDOR_WRITE_BASE[i] + 44, 4);
        else
            fpga_write(VENDOR_WRITE_BASE[i] + 44, 2);

        //CPU_TEMPERATURE --> check_cpu_temp

        //SN `cat /tmp/uuid.txt`    
        devm_fru_get_uuid(uuid_str, sizeof(uuid_str));
        for(j = 0; j < 36/4; j++) {
            value = uuid_str[j*4];
            value = (value << 8) | uuid_str[j*4 + 1];
            value = (value << 8) | uuid_str[j*4 + 2];
            value = (value << 8) | uuid_str[j*4 + 3];
            fpga_write(VENDOR_WRITE_BASE[i] + 8 + j*4, value);
        }
    }

    return VOS_OK;
}



#endif

