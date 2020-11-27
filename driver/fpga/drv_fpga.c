
#include "daemon_pub.h"
#include "drv_cpu.h"
#include "drv_fpga.h"
#include "devm_fru.h"


//bbu_rru_CT_to_cmcc.sh
int bbu_rru_CT_to_cmcc()
{
    return VOS_OK;
}

//cpri-r21-mul.sh
int drv_get_cpri_links(int *link_cnt)
{
    int cnt = 0;

    if (link_cnt == NULL) return VOS_ERR;
    
    for (int i = 0; i < MAX_CPRI_CNT; i++) {
        uint32 value = devmem_read(CPRI_REG_BASE + i*0x10000, AT_WORD);
        if (value & 0xF) cnt++;
    }

    *link_cnt = cnt;
    return VOS_OK;
}

//getversion
int drv_board_type()
{
    uint32 value = devmem_read(FPGA_VER_ADDRESS, AT_WORD);
    if ( (value & 0x01000000) == 0x01000000)
        return BOARD_TYPE_RHUB;
    return BOARD_TYPE_RRU;
}

int CPRI_BASE[MAX_CPRI_CNT]= {0x43C40000};

int VENDOR_WRITE_BASE[MAX_CPRI_CNT] = {0x43C90000};

int VENDOR_READ_BASE[MAX_CPRI_CNT] = {0x43C40100};

int DELAY_A = 630;
int DELAY_B = 0;
int DELAY_D = 363;

//cpri-r21-mul-slave.sh
int cpri_state_monitor()
{
    int i;
    uint32 rru_id, value;
    char temp_data[64];
    uint32 cpri_r21;
    float rx_fifo_delay, tx_fifo_delay, time_offset;

    for (i = 0; i < MAX_CPRI_CNT; i++) {
        //RRU_ID_W $rru_id
        rru_id = devmem_read(VENDOR_READ_BASE[i], AT_WORD);
        devmem_write(VENDOR_WRITE_BASE[i], AT_WORD, rru_id);

        //T_OFFSET $time_offest
        cpri_r21 = devmem_read(CPRI_BASE[i] + 0x3c, AT_WORD);
        rx_fifo_delay = (float)(cpri_r21 >> 18);
        cpri_r21 = (cpri_r21 & 0x3FFFF) * 33;
        rx_fifo_delay = (rx_fifo_delay*33)/128;
        value = devmem_read(CPRI_BASE[i] + 0x50, AT_WORD);
        tx_fifo_delay = (float)(value & 0x3FFF);
        tx_fifo_delay = (tx_fifo_delay*33)/128;
        time_offset = DELAY_A + DELAY_B + rx_fifo_delay + tx_fifo_delay + DELAY_D + cpri_r21;
        value = (uint32)time_offset;
        devmem_write(VENDOR_WRITE_BASE[i] + 4, AT_WORD, value);

        //CPRI_4T4R_W $((${VENDOR_WRITE_BASE[$i]} + 4 + 4 + 36))
        value = devmem_read(FPGA_VER_ADDRESS, AT_WORD);
        if ((value&0x00ff0000) == 0x00040000) 
            devmem_write(VENDOR_WRITE_BASE[i] + 44, AT_WORD, 4);
        else
            devmem_write(VENDOR_WRITE_BASE[i] + 44, AT_WORD, 2);
        
        //SN `cat /tmp/uuid.txt`    
        devm_fru_get_uuid(temp_data, sizeof(temp_data));
        for(i = 0; i < 36; i+=4) {
            value = temp_data[i];
            value = (value << 8) | temp_data[i + 1];
            value = (value << 8) | temp_data[i + 2];
            value = (value << 8) | temp_data[i + 3];
            devmem_write(VENDOR_WRITE_BASE[i] + 8 + i*4, AT_WORD, value);
        }

        //echo ${CPRI_LINK_STAT[$i]} $(($rru_id + 0)) $_tmp_sn > /tmp/cpri_stat.txt
        value = devmem_read(CPRI_BASE[i], AT_WORD);
        value = ((value & 0xF) == 0xF);
        sprintf(temp_data, "echo %d %d %s > /tmp/cpri_stat.txt", value, rru_id, temp_data);
        shell_run_cmd(temp_data);
    }
    
    //CPU_TEMPERATURE --> check_cpu_temp
    //drv_get_cpu_temp(&cpu_temp);
    //sprintf(temp_data, "echo %d > /tmp/cpu_temperature.txt", cpu_temp);
    //shell_run_cmd(temp_data);
    
    return VOS_OK;
}

