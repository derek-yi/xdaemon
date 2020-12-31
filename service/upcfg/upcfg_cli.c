#include "daemon_pub.h"

#include "drv_fpga.h"
#include "drv_main.h"
#include "hwmon_main.h"
#include "upcfg_main.h"

uint32 tdd_fmt = 0;
uint32 slot_num[2];
uint32 dl_num[2];
uint32 ul_num[2];
uint32 sp_dl_num[2];
uint32 sp_gap_num[2];
uint32 sp_ul_num[2];

int field_info_help(FIELD_INFO *list, int cnt)
{
    vos_print("%-20s: %s\r\n", "show", "show current config");
    for(int i = 0; i < cnt; i++) {
        vos_print("\r\n%-20s: %s //%s\r\n", list[i].field_name, list[i].field_desc, list[i].value_desc);
    }
    
    return VOS_OK;
}

int field_info_show(FIELD_INFO *list, int cnt)
{
    uint32 value;
    
    for (int i = 0; i < cnt; i++) {
        value = fpga_read_bits(list[i].reg_addr, list[i].start_bit, list[i].mask);
        vos_print("%-24s: 0x%-8x //%s \r\n", list[i].field_name, value, list[i].value_desc);
    }
    
    return VOS_OK;
}

int field_info_set(FIELD_INFO *list, int cnt, char *field, uint32 value)
{
    uint32 old_value;
    
    for (int i = 0; i < cnt; i++) {
        if (!strncasecmp(list[i].field_name, field, strlen(field))) {
            old_value = fpga_read(list[i].reg_addr);
            fpga_write_bits(list[i].reg_addr, list[i].start_bit, list[i].mask, value);
            value = fpga_read(list[i].reg_addr);
            vos_print("0x%x: old 0x%x, new 0x%x \r\n", list[i].reg_addr, old_value, value);
            return VOS_OK;
        }
    }

    return CMD_ERR_PARAM;
}

int cli_do_field_info(int argc, char **argv, FIELD_INFO *list, uint32 list_cnt)
{
    uint32 value;

    if (argc < 2) {
        field_info_help(list, list_cnt);
        return CMD_ERR_PARAM;
    }

    if (!strncasecmp(argv[1], "show", strlen(argv[1]))) {
        field_info_show(list, list_cnt);
        return CMD_OK_NO_LOG;
    }

    if (argc < 3) {
        field_info_help(list, list_cnt);
        return CMD_ERR_PARAM;
    }

    value = (uint32)strtoul(argv[2], 0, 0);
    if ( field_info_set(list, list_cnt, argv[1], value) != VOS_OK) {
        field_info_help(list, list_cnt);
        return CMD_ERR_PARAM;
    }

    return CMD_OK;    
}

int cli_show_field_info(int argc, char **argv, FIELD_INFO *list, uint32 list_cnt)
{
    uint32 value;

    if (argc < 2) {
        vos_print("%s show - show field value \r\n", argv[0]);
        return CMD_ERR_PARAM;
    }

    if ( !strncasecmp(argv[1], "show", strlen(argv[1])) ) {
        field_info_show(list, list_cnt);
    } else {
        vos_print("%s show - show field value \r\n", argv[0]);
    }

    return CMD_OK_NO_LOG;    
}

int cli_tdd_enable(int argc, char **argv)
{
    uint32 value = 0;
    
    if (argc < 2) {
        vos_print("tdd_enable show     -- show tdd status\r\n");
        vos_print("tdd_enable <enable> -- 0-disable, 1-enable\r\n");
        return CMD_ERR_PARAM;
    }

    if (!strncasecmp(argv[1], "show", strlen(argv[1]))) {
        value = fpga_read_bits(0x43c50000, 1, 0x1);
        if (value == 0x0) vos_print("TDD Status: OFF \r\n");
        else vos_print("TDD Status: ON \r\n");
        return CMD_OK_NO_LOG;
    }

    value = (uint32)strtoul(argv[1], 0, 0);
    fpga_write_bits(0x43c50000, 1, 0x1, value);
    return CMD_OK;
}

FIELD_INFO tddc_ctrl[] = 
{
    {"rx_advance",      0x43c50014, 0,   0xFFFF,     "rx advance",      "0 ~ 65535"},
    {"rx_delay",        0x43c50018, 0,   0xFFFF,     "rx delay",        "0 ~ 65535"},
    {"tx_advance",      0x43c5001c, 0,   0xFFFF,     "tx advance",      "0 ~ 65535"},
    {"tx_delay",        0x43c50020, 0,   0xFFFF,     "tx delay",        "0 ~ 65535"},
};

int cli_tddc_ctrl(int argc, char **argv)
{
    FIELD_INFO *list = &tddc_ctrl[0];
    uint32 list_cnt = sizeof(tddc_ctrl)/sizeof(FIELD_INFO);
    
    return cli_do_field_info(argc, argv, list, list_cnt);
}

#ifdef BOARD_RRU_G3

int cli_pps_select(int argc, char **argv)
{
    uint32 value = 0;
    int cmp_len;
    
    if (argc < 2) {
        vos_print("pps_select show -- show pps select\r\n");
        vos_print("pps_select <normal|inside_fpga|outside_gps> -- pps select\r\n");
        return CMD_ERR_PARAM;
    }

    if (!strncasecmp(argv[1], "show", strlen(argv[1]))) {
        value = fpga_read_bits(0x43c30004, 0, 0x3);
        if (value == 0x0) vos_print("PPS Select: %u (Normal, From cpri 1pps) \r\n", value);
        else if (value == 0x1) vos_print("PPS Select: %u (From inside 1pps) \r\n", value);
        else if (value == 0x3) vos_print("PPS Select: %u (From outside GPS 1pps) \r\n", value);
        else vos_print("PPS Select: %u (Unknown) \r\n", value);
        return CMD_OK_NO_LOG;
    }

    cmp_len = strlen(argv[1]);
    if (!strncasecmp(argv[1], "normal", cmp_len)) value = 0x0;
    else if (!strncasecmp(argv[1], "inside_fpga", cmp_len)) value = 0x1;
    else if (!strncasecmp(argv[1], "outside_gps", cmp_len)) value = 0x3;
    else {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }        

    fpga_write_bits(0x43c30004, 0, 0x3, value);
    return CMD_OK;
}

FIELD_INFO dfe_ctrl[] = 
{
    {"dma_enable",          0x43c30018, 31, 0x1,   "config data source",    "0-from cpri, 1-from tx_dma"},
    {"dfe_enable",          0x43c30018, 15, 0x1,   "dfe function control",  "0-tx_disable, 1-tx_enable"},
    {"decompress_dis",      0x43c30018, 0,   0x1,  "send data format",      "0-send decompressed data, 1-send uncompressed data"},
};

int cli_dfe_ctrl(int argc, char **argv)
{
    FIELD_INFO *list = &dfe_ctrl[0];
    uint32 list_cnt = sizeof(dfe_ctrl)/sizeof(FIELD_INFO);
    
    return cli_do_field_info(argc, argv, list, list_cnt);
}

FIELD_INFO dfe_out_ctrl[] = 
{
    {"hbf2_out_enable",     0x43c30028, 8, 0xF,     "send the data of hbf2 module to RX_DMA and JESD",    "0-disable, 1-enable"},
    {"cfr_out_enable",      0x43c30028, 4,  0xF,    "send the data of cfr/aute_gain module to RX_DMA and JESD", "0-disable, 1-enable" },
    {"dpd_out_enable",      0x43c30028, 0,  0xF,    "send the data of dfe(dpd) module to RX_DMA and JESD ",  "0-disable, 1-enable"    },
};

int cli_dfe_out_ctrl(int argc, char **argv)
{
    FIELD_INFO *list = &dfe_out_ctrl[0];
    uint32 list_cnt = sizeof(dfe_out_ctrl)/sizeof(FIELD_INFO);
    
    return cli_do_field_info(argc, argv, list, list_cnt);
}

FIELD_INFO gain_ctrl[] = 
{
    {"n0_ant0_pre",        0x43c3002c, 0,   0xFFFF,  "N0_Ant0_pre_gain_factor",    "gain factor"},
    {"n0_ant1_pre",        0x43c3002c, 16, 0xFFFF,   "N0_Ant1_pre_gain_factor",    "gain factor"},
    {"n0_ant0_auto",       0x43c30030, 0,   0xFFFF,  "N0_Ant0_auto_gain_factor",   "gain factor"},
    {"n0_ant1_auto",       0x43c30030, 16, 0xFFFF,   "N0_Ant1_auto_gain_factor",   "gain factor"},
    {"n0_ant0_post",       0x43c30034, 0,   0xFFFF,  "N0_Ant0_post_gain_factor",   "gain factor"},
    {"n0_ant1_post",       0x43c30034, 16, 0xFFFF,   "N0_Ant1_post_gain_factor",   "gain factor"},
    {"n1_ant0_pre",        0x43c30038, 0,   0xFFFF,  "N0_Ant0_pre_gain_factor",    "gain factor"},
    {"n1_ant1_pre",        0x43c30038, 16, 0xFFFF,   "N0_Ant1_pre_gain_factor",    "gain factor"},
    {"n1_ant0_auto",       0x43c3003c, 0,   0xFFFF,  "N0_Ant0_auto_gain_factor",   "gain factor"},
    {"n1_ant1_auto",       0x43c3003c, 16, 0xFFFF,   "N0_Ant1_auto_gain_factor",   "gain factor"},
    {"n1_ant0_post",       0x43c30040, 0,   0xFFFF,  "N0_Ant0_post_gain_factor",   "gain factor"},
    {"n1_ant1_post",       0x43c30040, 16, 0xFFFF,   "N0_Ant1_post_gain_factor",   "gain factor"},
};

int cli_gain_ctrl(int argc, char **argv)
{
    FIELD_INFO *list = &gain_ctrl[0];
    uint32 list_cnt = sizeof(gain_ctrl)/sizeof(FIELD_INFO);
    
    return cli_do_field_info(argc, argv, list, list_cnt);
}

//tdd_format.sh
int cli_tdd_format(int argc, char **argv)
{
    uint32 value;
    uint32 cmp_len;
    
    if (argc < 2) {
        vos_print("tdd_format show -- show tdd frame format\r\n");
        vos_print("tdd_format single <slot_num1> <dl_num1> <ul_num1> <sp_dl_num1> <sp_gap_num1> <sp_ul_num1> -- single cycle config\r\n");
        vos_print("tdd_format double <slot_num1> <dl_num1> <ul_num1> <sp_dl_num1> <sp_gap_num1> <sp_ul_num1> \r\n"
                  "                  <slot_num2> <dl_num2> <ul_num2> <sp_dl_num2> <sp_gap_num2> <sp_ul_num2> -- double cycle config\r\n");
        return CMD_ERR_PARAM;
    }

    cmp_len = strlen(argv[1]);
    if ( !strncasecmp(argv[1], "show", cmp_len) ) {
        if (tdd_fmt == 1) {
            vos_print("TDD Format: Single Cycle\r\n");
            vos_print("-> slot_num1 %d, dl_num1 %d, ul_num1 %d, sp_dl_num1 %d, sp_gap_num1 %d sp_ul_num1 %d \r\n",
                       slot_num[0], dl_num[0], ul_num[0], sp_dl_num[0], sp_gap_num[0], sp_ul_num[0]);
        } else if (tdd_fmt == 2) {
            vos_print("TDD Format: Double Cycle\r\n");
            vos_print("-> slot_num1 %d, dl_num1 %d, ul_num1 %d, sp_dl_num1 %d, sp_gap_num1 %d sp_ul_num1 %d \r\n",
                       slot_num[0], dl_num[0], ul_num[0], sp_dl_num[0], sp_gap_num[0], sp_ul_num[0]);
            vos_print("-> slot_num2 %d, dl_num2 %d, ul_num2 %d, sp_dl_num2 %d, sp_gap_num2 %d sp_ul_num2 %d \r\n",
                       slot_num[1], dl_num[1], ul_num[1], sp_dl_num[1], sp_gap_num[1], sp_ul_num[1]);
        } else {
            vos_print("NOT config TDD format\r\n");
        }
        
        return CMD_OK_NO_LOG;
    }

    if ( strncasecmp(argv[1], "single", cmp_len) && strncasecmp(argv[1], "double", cmp_len) ) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    if ( (argc < 8) || ( !strncasecmp(argv[1], "double", cmp_len) && (argc < 14) ) ) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    slot_num[0]     = (uint32)strtoul(argv[2], 0, 0);
    dl_num[0]       = (uint32)strtoul(argv[3], 0, 0);
    ul_num[0]       = (uint32)strtoul(argv[4], 0, 0);
    sp_dl_num[0]    = (uint32)strtoul(argv[5], 0, 0);
    sp_gap_num[0]   = (uint32)strtoul(argv[6], 0, 0);
    sp_ul_num[0]    = (uint32)strtoul(argv[7], 0, 0);

    //devmem 0x43c50000 32 0x0 
    fpga_write(0x43c50000, 0); //close tdd
    
    //devmem 0x43c50034 32 $[$3*61440] 
    fpga_write(0x43c50034, slot_num[0]*61440); //duplex_tdd_period_0

    //devmem 0x43c50008 32 $[$3*(1<<24)+$4*(1<<18)+$5*(1<<12)+$6*(1<<8)+$7*(1<<4)+$8]
    value = slot_num[0]*(1<<24) + dl_num[0]*(1<<18) + ul_num[0]*(1<<12) + sp_dl_num[0]*(1<<8) + sp_gap_num[0]*(1<<4) + sp_ul_num[0];
    fpga_write(0x43c50008, value); //frame_comprise_0

    //devmem 0x43c50024 32 $[$4*61440+($6-1)*(4096+288)+4096+352]
    value = dl_num[0]*61440 + (sp_dl_num[0] - 1)*(4096+288) + 4096 + 352;
    fpga_write(0x43c50024, value); //tx_time_0

    //devmem 0x43c50028 32 $[$5*61440+$8*(4096+288)]
    value = ul_num[0]*61440 + sp_ul_num[0]*(4096+288);
    fpga_write(0x43c50028, value); //rx_time_0

    //devmem 0x43c5002c 32 $[$7*(4096+288)]
    fpga_write(0x43c5002c, sp_gap_num[0]*(4096+288)); //gap_time_0

    if (! strncasecmp(argv[1], "single", cmp_len) ) {
        //devmem 0x43c500c0 32 0
        fpga_write(0x43c500c0, 0); //single cycle
        tdd_fmt = 1;

        fpga_write(0x43c500d4, 0); //duplex_tdd_period_1
        fpga_write(0x43c500c4, 0); //frame_comprise_1
        fpga_write(0x43c500c8, 0); //tx_time_1
        fpga_write(0x43c500cc, 0); //rx_time_1
        fpga_write(0x43c500d0, 0); //gap_time_1
    } else {
        slot_num[1]     = (uint32)strtoul(argv[8], 0, 0);
        dl_num[1]       = (uint32)strtoul(argv[9], 0, 0);
        ul_num[1]       = (uint32)strtoul(argv[10], 0, 0);
        sp_dl_num[1]    = (uint32)strtoul(argv[11], 0, 0);
        sp_gap_num[1]   = (uint32)strtoul(argv[12], 0, 0);
        sp_ul_num[1]    = (uint32)strtoul(argv[13], 0, 0);

        //devmem 0x43c500c0 32 1
        fpga_write(0x43c500c0, 1); //double cycle
        tdd_fmt = 2;

        //devmem 0x43c500d4 32 $[$9*61440]
        fpga_write(0x43c500d4, slot_num[1]*61440); //duplex_tdd_period_1

        //devmem 0x43c500c4 32 $[$9*(1<<24)+${10}*(1<<18)+${11}*(1<<12)+${12}*(1<<8)+${13}*(1<<4)+${14}]
        value = slot_num[0]*(1<<24) + dl_num[0]*(1<<18) + ul_num[0]*(1<<12) + sp_dl_num[0]*(1<<8) + sp_gap_num[0]*(1<<4) + sp_ul_num[0];
        fpga_write(0x43c500c4, value); //frame_comprise_1

        //devmem 0x43c500c8 32 $[${10}*61440+(${12}-1)*(4096+288)+4096+352]
        value = dl_num[0]*61440 + (sp_dl_num[0] - 1)*(4096+288) + 4096 + 352;
        fpga_write(0x43c500c8, value); //tx_time_1

        //devmem 0x43c500cc 32 $[${11}*61440+${14}*(4096+288)]
        value = ul_num[0]*61440 + sp_ul_num[0]*(4096+288);
        fpga_write(0x43c500cc, value); //rx_time_1

        //devmem 0x43c500d0 32 $[${13}*(4096+288)]
        fpga_write(0x43c500d0, sp_gap_num[0]*(4096+288)); //gap_time_1        
    }
    
    //devmem 0x43c50000 32 0x13
    fpga_write(0x43c50000, 0x13); //open tdd

    return CMD_OK;
}

int cli_rf_tx_attenuation(int argc, char **argv)
{
    uint32 chip_id;
    uint32 chn_id;
    float tx_atten;
    
    if (argc < 2) {
        vos_print("tx_atten show                        -- show tx attenuation\r\n");
        vos_print("tx_atten <chip-id> <channel> <value> -- set tx attenuation\r\n");
        return CMD_ERR_PARAM;
    }

    if (!strncasecmp(argv[1], "show", strlen(argv[1]))) {
        vos_print("chip  channel  tx_attenuation \r\n");
        for (int i = 0; i < SYS_MAX_RF_CHIP; i++) {
            adrv9009_get_tx_atten(i, 0, &tx_atten);
            vos_print("%d     %d        %f \r\n", i, 0, tx_atten);
            adrv9009_get_tx_atten(i, 1, &tx_atten);
            vos_print("%d     %d        %f \r\n", i, 1, tx_atten);
        }
        return CMD_OK_NO_LOG;
    }

    if (argc < 4) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    chip_id  = (uint32)strtoul(argv[1], 0, 0);
    chn_id   = (uint32)strtoul(argv[2], 0, 0);
    tx_atten = (float)strtof(argv[3], 0);
    //vos_print("tx_atten %f \r\n", tx_atten);
    if (adrv9009_set_tx_atten(chip_id, chn_id, tx_atten) != VOS_OK) {
        vos_print("Failed to set tx_atten \r\n");
    }

    return CMD_OK;
}

int cli_rf_frequence(int argc, char **argv)
{
    uint32 freq;
    
    if (argc < 2) {
        vos_print("rf_freq show   -- show RF frequence\r\n");
        vos_print("rf_freq <freq> -- set RF frequence in Mhz\r\n");
        return CMD_ERR_PARAM;
    }

    if (!strncasecmp(argv[1], "show", strlen(argv[1]))) {
        adrv9009_get_tx_freq(0, &freq);
        vos_print("RF frequence: %d (Mhz) \r\n", freq);
        return CMD_OK_NO_LOG;
    }

    freq  = (uint32)strtoul(argv[1], 0, 0);
    adrv9009_set_tx_freq(0, freq);
    adrv9009_set_tx_freq(1, freq);

    return CMD_OK;
}
extern int cli_cpri_link_show(int argc, char **argv);

#endif

//#define BOARD_RHUB_G1
#ifdef BOARD_RHUB_G1

int cli_pps_select(int argc, char **argv)
{
    uint32 value = 0;
    int cmp_len;
    
    if (argc < 2) {
        vos_print("pps_select show -- show pps select\r\n");
        vos_print("pps_select <normal|inside_fpga> -- pps select\r\n");
        return CMD_ERR_PARAM;
    }

    cmp_len = strlen(argv[1]);
    if (!strncasecmp(argv[1], "show", cmp_len)) {
        value = fpga_read_bits(0x43c30004, 0, 0x1);
        if (value == 0x0) vos_print("PPS Select: %u (Normal, outside pps) \r\n", value);
        else vos_print("PPS Select: %u (From inside pps) \r\n", value);
        return CMD_OK_NO_LOG;
    }

    if (!strncasecmp(argv[1], "normal", cmp_len)) value = 0x0;
    else if (!strncasecmp(argv[1], "inside_fpga", cmp_len)) value = 0x1;
    else {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }        

    fpga_write_bits(0x43c30004, 0, 0x1, value);
    return CMD_OK;
}

//tdd_format.sh
int cli_tdd_format(int argc, char **argv)
{
    uint32 value;
    uint32 cmp_len;
    
    if (argc < 2) {
        vos_print("tdd_format show -- show tdd frame format\r\n");
        vos_print("tdd_format single <slot_num1> <dl_num1> <ul_num1> <sp_dl_num1> <sp_gap_num1> <sp_ul_num1> -- single cycle config\r\n");
        vos_print("tdd_format double <slot_num1> <dl_num1> <ul_num1> <sp_dl_num1> <sp_gap_num1> <sp_ul_num1> \r\n"
                  "                  <slot_num2> <dl_num2> <ul_num2> <sp_dl_num2> <sp_gap_num2> <sp_ul_num2> -- double cycle config\r\n");
        return CMD_ERR_PARAM;
    }

    cmp_len = strlen(argv[1]);
    if ( !strncasecmp(argv[1], "show", cmp_len) ) {
        if (tdd_fmt == 1) {
            vos_print("TDD Format: Single Cycle\r\n");
            vos_print("-> slot_num1 %d, dl_num1 %d, ul_num1 %d, sp_dl_num1 %d, sp_gap_num1 %d sp_ul_num1 %d \r\n",
                       slot_num[0], dl_num[0], ul_num[0], sp_dl_num[0], sp_gap_num[0], sp_ul_num[0]);
        } else if (tdd_fmt == 2) {
            vos_print("TDD Format: Double Cycle\r\n");
            vos_print("-> slot_num1 %d, dl_num1 %d, ul_num1 %d, sp_dl_num1 %d, sp_gap_num1 %d sp_ul_num1 %d \r\n",
                       slot_num[0], dl_num[0], ul_num[0], sp_dl_num[0], sp_gap_num[0], sp_ul_num[0]);
            vos_print("-> slot_num2 %d, dl_num2 %d, ul_num2 %d, sp_dl_num2 %d, sp_gap_num2 %d sp_ul_num2 %d \r\n",
                       slot_num[1], dl_num[1], ul_num[1], sp_dl_num[1], sp_gap_num[1], sp_ul_num[1]);
        } else {
            vos_print("NOT config TDD format\r\n");
        }
        
        return CMD_OK_NO_LOG;
    }

    if ( strncasecmp(argv[1], "single", cmp_len) && strncasecmp(argv[1], "double", cmp_len) ) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    if ( (argc < 8) || ( !strncasecmp(argv[1], "double", cmp_len) && (argc < 14) ) ) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    slot_num[0]     = (uint32)strtoul(argv[2], 0, 0);
    dl_num[0]       = (uint32)strtoul(argv[3], 0, 0);
    ul_num[0]       = (uint32)strtoul(argv[4], 0, 0);
    sp_dl_num[0]    = (uint32)strtoul(argv[5], 0, 0);
    sp_gap_num[0]   = (uint32)strtoul(argv[6], 0, 0);
    sp_ul_num[0]    = (uint32)strtoul(argv[7], 0, 0);

    //devmem 0x43c50000 32 0x0 
    fpga_write(0x43c50000, 0); //close tdd
    
    //devmem 0x43c5002c 32 $[$3*61440] 
    fpga_write(0x43c5002c, slot_num[0]*61440); //duplex_tdd_period_0

    //devmem 0x43c50008 32 $[$3*(1<<24)+$4*(1<<18)+$5*(1<<12)+$6*(1<<8)+$7*(1<<4)+$8]
    value = slot_num[0]*(1<<24) + dl_num[0]*(1<<18) + ul_num[0]*(1<<12) + sp_dl_num[0]*(1<<8) + sp_gap_num[0]*(1<<4) + sp_ul_num[0];
    fpga_write(0x43c50008, value); //frame_comprise_0

    //devmem 0x43c5001c 32 $[$4*61440+($6-1)*(4096+288)+4096+352]
    value = dl_num[0]*61440 + (sp_dl_num[0] - 1)*(4096+288) + 4096 + 352;
    fpga_write(0x43c5001c, value); //tx_time_0

    //devmem 0x43c50020 32 $[$5*61440+$8*(4096+288)]
    value = ul_num[0]*61440 + sp_ul_num[0]*(4096+288);
    fpga_write(0x43c50020, value); //rx_time_0

    //devmem 0x43c50024 32 $[$7*(4096+288)]
    fpga_write(0x43c50024, sp_gap_num[0]*(4096+288)); //gap_time_0

    if (! strncasecmp(argv[1], "single", cmp_len) ) {
        //devmem 0x43c500c0 32 0
        fpga_write(0x43c5003c, 0); //single cycle
        tdd_fmt = 1;

        fpga_write(0x43c50050, 0); //duplex_tdd_period_1
        fpga_write(0x43c50040, 0); //frame_comprise_1
        fpga_write(0x43c50044, 0); //tx_time_1
        fpga_write(0x43c50048, 0); //rx_time_1
        fpga_write(0x43c5004c, 0); //gap_time_1
    } else {
        slot_num[1]     = (uint32)strtoul(argv[8], 0, 0);
        dl_num[1]       = (uint32)strtoul(argv[9], 0, 0);
        ul_num[1]       = (uint32)strtoul(argv[10], 0, 0);
        sp_dl_num[1]    = (uint32)strtoul(argv[11], 0, 0);
        sp_gap_num[1]   = (uint32)strtoul(argv[12], 0, 0);
        sp_ul_num[1]    = (uint32)strtoul(argv[13], 0, 0);

        //devmem 0x43c5003c 32 1
        fpga_write(0x43c5003c, 1); //double cycle
        tdd_fmt = 2;

        //devmem 0x43c50050 32 $[$9*61440]
        fpga_write(0x43c50050, slot_num[1]*61440); //duplex_tdd_period_1

        //devmem 0x43c50040 32 $[$9*(1<<24)+${10}*(1<<18)+${11}*(1<<12)+${12}*(1<<8)+${13}*(1<<4)+${14}]
        value = slot_num[0]*(1<<24) + dl_num[0]*(1<<18) + ul_num[0]*(1<<12) + sp_dl_num[0]*(1<<8) + sp_gap_num[0]*(1<<4) + sp_ul_num[0];
        fpga_write(0x43c50040, value); //frame_comprise_1

        //devmem 0x43c50044 32 $[${10}*61440+(${12}-1)*(4096+288)+4096+352]
        value = dl_num[0]*61440 + (sp_dl_num[0] - 1)*(4096+288) + 4096 + 352;
        fpga_write(0x43c50044, value); //tx_time_1

        //devmem 0x43c50048 32 $[${11}*61440+${14}*(4096+288)]
        value = ul_num[0]*61440 + sp_ul_num[0]*(4096+288);
        fpga_write(0x43c50048, value); //rx_time_1

        //devmem 0x43c5004c 32 $[${13}*(4096+288)]
        fpga_write(0x43c5004c, sp_gap_num[0]*(4096+288)); //gap_time_1        
    }
    
    //devmem 0x43c50000 32 0x3
    fpga_write(0x43c50000, 0x3); //open tdd

    return CMD_OK;
}

int cli_aggregator_weight(int argc, char **argv)
{
    uint32 rru_id, chip_id, value;
    uint32 weight;
    
    if (argc < 2) {
        vos_print("agg_weight <rru-id> <chip-id> <weight> -- config aggregator weight(0-100)\r\n");
        vos_print("agg_weight show                        -- show aggregator weight\r\n");
        return CMD_ERR_PARAM;
    }    

    if (!strncasecmp(argv[1], "show", strlen(argv[1]))) {
        for(int i = 0; i < 8; i++) {
            value = fpga_read(0x43c30128 + i*4);
            weight = (value*100)/0x8000;
            if ( (value*100)%0x8000 ) weight += 1;
            vos_print("N0 rru_%d weight(%): %d \r\n", i, weight);
        }

        for(int i = 0; i < 8; i++) {
            value = fpga_read(0x43c30148 + i*4);
            weight = (value*100)/0x8000;
            if ( (value*100)%0x8000 ) weight += 1;
            vos_print("N1 rru_%d weight(%): %d \r\n", i, weight);
        }
        return CMD_OK_NO_LOG;        
    } else if (argc < 4) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    rru_id  = (uint32)strtoul(argv[1], 0, 0);
    chip_id = (uint32)strtoul(argv[2], 0, 0);
    weight  = (uint32)strtoul(argv[3], 0, 0);
    if (rru_id > 7 || chip_id > 1 || weight > 100) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    value = (weight*0x8000)/100; 
    if (chip_id) fpga_write(0x43c30148 + rru_id*4, value);
    else fpga_write(0x43c30128 + rru_id*4, value);
    
    return CMD_OK;
}

FIELD_INFO ant_enable[] = 
{
    {"ant0-tx",         0x43c30118, 0, 0x1,      "ANTE0_Tx_enable", "0-disable, 1-enable"},
    {"ant1-tx",         0x43c30118, 1, 0x1,      "ANTE1_Tx_enable", "0-disable, 1-enable"},
    {"ant2-tx",         0x43c30118, 2, 0x1,      "ANTE2_Tx_enable", "0-disable, 1-enable"},
    {"ant3-tx",         0x43c30118, 3, 0x1,      "ANTE3_Tx_enable", "0-disable, 1-enable"},
    
    {"ant0-rx",         0x43c30118, 4, 0x1,      "ANTE0_Rx_enable", "0-disable, 1-enable"},
    {"ant1-rx",         0x43c30118, 5, 0x1,      "ANTE1_Rx_enable", "0-disable, 1-enable"},
    {"ant2-rx",         0x43c30118, 6, 0x1,      "ANTE2_Rx_enable", "0-disable, 1-enable"},
    {"ant3-rx",         0x43c30118, 7, 0x1,      "ANTE3_Rx_enable", "0-disable, 1-enable"},
};

int cli_antenna_enable(int argc, char **argv)
{
    FIELD_INFO *list = &ant_enable[0];
    uint32 list_cnt = sizeof(ant_enable)/sizeof(FIELD_INFO);
    
    return cli_do_field_info(argc, argv, list, list_cnt);
}

FIELD_INFO ant_remapping[] = 
{
    {"remap-rx0",       0x43c30120, 0,   0xF,     "rx signal source antenna number of antenna_0 after remapping module", "0-3, mapping index"},
    {"remap-rx1",       0x43c30120, 4,   0xF,     "rx signal source antenna number of antenna_1 after remapping module", "0-3, mapping index"},
    {"remap-rx2",       0x43c30120, 8,   0xF,     "rx signal source antenna number of antenna_2 after remapping module", "0-3, mapping index"},
    {"remap-rx3",       0x43c30120, 12, 0xF,      "rx signal source antenna number of antenna_3 after remapping module", "0-3, mapping index"},
    
    {"remap-tx0",       0x43c30124, 0,   0xF,     "tx signal source index of cpri_data_0(8 bit) after remapping module", "0-7, mapping index"},
    {"remap-tx1",       0x43c30124, 4,   0xF,     "tx signal source index of cpri_data_1(8 bit) after remapping module", "0-7, mapping index"},
    {"remap-tx2",       0x43c30124, 8,   0xF,     "tx signal source index of cpri_data_2(8 bit) after remapping module", "0-7, mapping index"},
    {"remap-tx3",       0x43c30124, 12, 0xF,      "tx signal source index of cpri_data_3(8 bit) after remapping module", "0-7, mapping index"},
    {"remap-tx4",       0x43c30124, 16, 0xF,      "tx signal source index of cpri_data_4(8 bit) after remapping module", "0-7, mapping index"},
    {"remap-tx5",       0x43c30124, 20, 0xF,      "tx signal source index of cpri_data_5(8 bit) after remapping module", "0-7, mapping index"},
    {"remap-tx6",       0x43c30124, 24, 0xF,      "tx signal source index of cpri_data_6(8 bit) after remapping module", "0-7, mapping index"},
    {"remap-tx7",       0x43c30124, 28, 0xF,      "tx signal source index of cpri_data_7(8 bit) after remapping module", "0-7, mapping index"},
};

int cli_antenna_remapping(int argc, char **argv)
{
    FIELD_INFO *list = &ant_remapping[0];
    uint32 list_cnt = sizeof(ant_remapping)/sizeof(FIELD_INFO);
    
    return cli_do_field_info(argc, argv, list, list_cnt);
}

int module_name_to_bit(int flag, char *mod_name)
{
    int cmp_len = strlen(mod_name);
    
    if (flag) { // tx
        if ( !strncasecmp(mod_name, "src_data", cmp_len) ) return 0x01;
        else if ( !strncasecmp(mod_name, "cp_insertion", cmp_len) ) return 0x02;
        else if ( !strncasecmp(mod_name, "phase_comp", cmp_len) ) return 0x04;
        else if ( !strncasecmp(mod_name, "ifft", cmp_len) ) return 0x08;
        else if ( !strncasecmp(mod_name, "deccompress", cmp_len) ) return 0x10;
        else if ( !strncasecmp(mod_name, "sc_map", cmp_len) ) return 0x20;
    } else {
        if ( !strncasecmp(mod_name, "src_data", cmp_len) ) return 0x01;
        else if ( !strncasecmp(mod_name, "sc_demap", cmp_len) ) return 0x02;
        else if ( !strncasecmp(mod_name, "compression", cmp_len) ) return 0x04;
        else if ( !strncasecmp(mod_name, "fft", cmp_len) ) return 0x08;
        else if ( !strncasecmp(mod_name, "phase_comp", cmp_len) ) return 0x10;
        else if ( !strncasecmp(mod_name, "cp_remove", cmp_len) ) return 0x20;
    }

    return 0;
}

int cli_antenna_capture(int argc, char **argv)
{
    uint32 tdd_enable;
    uint32 ant_id;
    uint32 value, tmp;
    
    if (argc < 2) {
        vos_print("capture tx <ant-id> <module1> [<module2>] -- tx module(src_data sc_map deccompress ifft phase_comp cp_insertion) \r\n");
        vos_print("capture rx <ant-id> <module1> [<module2>] -- rx module(src_data cp_remove phase_comp fft compression sc_demap) \r\n");
        vos_print("capture reset                             -- clear capture config \r\n");
        return CMD_ERR_PARAM;
    }    

    if (!strncasecmp(argv[1], "reset", strlen(argv[1]))) {
        fpga_write_bits(0x43c30110, 0,   0x1, 0); //clear tx
        fpga_write_bits(0x43c30050, 16, 0x1, 0); //clear tx
        fpga_write_bits(0x43c30050, 25, 0x3F, 0); //clear tx
        
        fpga_write_bits(0x43c30114, 0,   0x1, 0); //clear rx
        fpga_write_bits(0x43c30054, 16, 0x1, 0); //clear rx
        fpga_write_bits(0x43c30054, 25, 0x3F, 0); //clear rx
        return CMD_OK;
    } else if (argc < 4) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    ant_id  = (uint32)strtoul(argv[2], 0, 0);
    if (ant_id > 3) {
        vos_print("invalid ant_id \r\n");
        return CMD_ERR_PARAM;
    }
    
    if ( !strncasecmp(argv[1], "tx", strlen(argv[1])) ) {
        value = module_name_to_bit(1, argv[3]);
        if (value == 0) {
            vos_print("invalid module_name \r\n");
            return CMD_ERR_PARAM;
        }
        
        if (argc > 4) {
            tmp = module_name_to_bit(1, argv[4]);
            if (tmp == 0) {
                vos_print("invalid module_name \r\n");
                return CMD_ERR_PARAM;
            }

            value |= tmp;
            if ( (value & 0x1) == 0 ) {
                vos_print("src_data must be selected while capture two modules \r\n");
                return CMD_ERR_PARAM;
            }
        }

        fpga_write_bits(0x43c50000, 1, 0x1, 0); //close tdd
        fpga_write_bits(0x43c30114, 0,   0x1, 0); //clear rx
        fpga_write_bits(0x43c30054, 16, 0x1, 0); //clear rx
        fpga_write_bits(0x43c30054, 25, 0x3F, 0); //clear rx
        fpga_write_bits(0x43c30050, 25, 0x3F, value); //select module
        
        if ( (ant_id == 0) || (ant_id == 1) ) fpga_write_bits(0x43c30050, 0, 0x1, 0);
        else fpga_write_bits(0x43c30050, 0, 0x1, 1);
        
        if ( (ant_id == 0) || (ant_id == 2) ) fpga_write_bits(0x43c30110, 0, 0x1, 0);
        else fpga_write_bits(0x43c30110 , 0, 0x1, 1);
        
        fpga_write_bits(0x43c50000, 1, 0x1, 1); //open tdd
        
        return CMD_OK;
    }
    else if ( !strncasecmp(argv[1], "rx", strlen(argv[1])) ) {
        value = module_name_to_bit(0, argv[3]);
        if (value == 0) {
            vos_print("invalid module_name \r\n");
            return CMD_ERR_PARAM;
        }
        
        if (argc > 4) {
            tmp = module_name_to_bit(0, argv[4]);
            if (tmp == 0) {
                vos_print("invalid module_name \r\n");
                return CMD_ERR_PARAM;
            }

            value |= tmp;
            if ( (value & 0x1) == 0 ) {
                vos_print("src_data must be selected while capture two modules \r\n");
                return CMD_ERR_PARAM;
            }
        }

        fpga_write_bits(0x43c50000, 1, 0x1, 0); //close tdd
        fpga_write_bits(0x43c30110, 0,   0x1, 0); //clear tx
        fpga_write_bits(0x43c30050, 16, 0x1, 0); //clear tx
        fpga_write_bits(0x43c30050, 25, 0x3F, 0); //clear tx
        fpga_write_bits(0x43c30054, 25, 0x3F, value); //select module
        
        if ( (ant_id == 0) || (ant_id == 1) ) fpga_write_bits(0x43c30054, 0, 0x1, 0);
        else fpga_write_bits(0x43c30054, 0, 0x1, 1);
        
        if ( (ant_id == 0) || (ant_id == 2) ) fpga_write_bits(0x43c30114, 0, 0x1, 0);
        else fpga_write_bits(0x43c30114 , 0, 0x1, 1);
        
        fpga_write_bits(0x43c50000, 1, 0x1, 1); //open tdd
        
        return CMD_OK;
    }
    else {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    return CMD_OK;
}

FIELD_INFO cpri_delay[] = 
{
    {"rru0_tx_delay",         0x43c30168, 0,   0xFF,      NULL, "Cpri_tx_delay of rru0"},
    {"rru1_tx_delay",         0x43c30168, 8,   0xFF,      NULL, "Cpri_tx_delay of rru1"},
    {"rru2_tx_delay",         0x43c30168, 16, 0xFF,       NULL, "Cpri_tx_delay of rru2"},
    {"rru3_tx_delay",         0x43c30168, 24, 0xFF,       NULL, "Cpri_tx_delay of rru3"},
    {"rru4_tx_delay",         0x43c3016C, 0,   0xFF,      NULL, "Cpri_tx_delay of rru4"},
    {"rru5_tx_delay",         0x43c3016C, 8,   0xFF,      NULL, "Cpri_tx_delay of rru5"},
    {"rru6_tx_delay",         0x43c3016C, 16, 0xFF,       NULL, "Cpri_tx_delay of rru6"},
    {"rru7_tx_delay",         0x43c3016C, 24, 0xFF,       NULL, "Cpri_tx_delay of rru7"},
    
    {"rru0_rx_delay",         0x43c30174, 0,   0xFF,      NULL, "Cpri_rx_delay of rru0"},
    {"rru1_rx_delay",         0x43c30174, 8,   0xFF,      NULL, "Cpri_rx_delay of rru1"},
    {"rru2_rx_delay",         0x43c30174, 16, 0xFF,       NULL, "Cpri_rx_delay of rru2"},
    {"rru3_rx_delay",         0x43c30174, 24, 0xFF,       NULL, "Cpri_rx_delay of rru3"},
    {"rru4_rx_delay",         0x43c30178, 0,   0xFF,      NULL, "Cpri_rx_delay of rru4"},
    {"rru5_rx_delay",         0x43c30178, 8,   0xFF,      NULL, "Cpri_rx_delay of rru5"},
    {"rru6_rx_delay",         0x43c30178, 16, 0xFF,       NULL, "Cpri_rx_delay of rru6"},
    {"rru7_rx_delay",         0x43c30178, 24, 0xFF,       NULL, "Cpri_rx_delay of rru7"},
};

int cli_cpri_delay_show(int argc, char **argv)
{
    FIELD_INFO *list = &cpri_delay[0];
    uint32 list_cnt = sizeof(cpri_delay)/sizeof(FIELD_INFO);
    
    return cli_show_field_info(argc, argv, list, list_cnt);
}

int cli_cpri_link_show(int argc, char **argv)
{
    if (argc < 2) {
        vos_print("cpri_link show -- show cpri link status\r\n");
        return CMD_ERR_PARAM;
    }    

    if (!strncasecmp(argv[1], "show", strlen(argv[1]))) {
        char *phy_desc[] = {"BBU ", "CAS ", "RRU0", "RRU1", "RRU2", "RRU3", "RRU4", "RRU5", "RRU6", "RRU7"};
        uint32 link_state;
        
        vos_print("        Optic_Online  Optic_LOS  Link_state \r\n");
        for(int i = 0; i < 10; i++) {
            get_cpri_link(i, &link_state);
            vos_print("%s    %-8s      %-8s   %s \r\n",
                        phy_desc[i], 
                        fpga_read_bits(0x43c30230, i, 0x1) ? "Online" : "Offline",
                        fpga_read_bits(0x43c30234, i, 0x1) ? "LOS" : "Normal",
                        link_state ? "Up" : "Down");
        }
        return CMD_OK_NO_LOG;        
    }

    vos_print("invalid param \r\n");
    return CMD_ERR_PARAM;
}

int cli_time_sync_config(int argc, char **argv)
{
    char rd_buf[128];
    uint32 value1, value2;
    int cmp_len;

    if (argc < 2) {
        vos_print("time_sync show                -- show time sync info\r\n");
        vos_print("time_sync mode <master|slave> -- set 1588 role\r\n");
        vos_print("time_sync offset <value>      -- adjust gps offset(ms)\r\n");
        return CMD_ERR_PARAM;
    }    

    cmp_len = strlen(argv[1]);
    if (!strncasecmp(argv[1], "show", cmp_len)) {
        drv_board_clk_case(rd_buf, sizeof(rd_buf));
        vos_print("Clock Case: %s \r\n", rd_buf);
        if ( strstr(rd_buf, "bulitin-gps") != NULL ) {
            vos_print("GPS Status: %s \r\n", drv_gnss_is_locked() ? "Locked" : "Unlocked");
        }
        value1 = fpga_read_bits(0x43c30008, 0, 0x1);
        vos_print("1588 mode : %s \r\n", value1 ? "Master" : "Slave");
        value1 = fpga_read(0x43c302dc) & 0xFFFF;
        vos_print("1588 SFN  : %d \r\n", value1);
        value1 = fpga_read(0x43c302d4);
        value2 = fpga_read(0x43c302d8);
        vos_print("Master TS : %d (ns %d)\r\n", value1, value2);
        value1 = fpga_read(0x43c302e4);
        value2 = fpga_read(0x43c302e8);
        vos_print("Slave TS  : %d (ns %d)\r\n", value1, value2);

        return CMD_OK_NO_LOG;        
    }

    if (argc < 3) {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    if (!strncasecmp(argv[1], "mode", cmp_len)) {
        if (!strncasecmp(argv[2], "master", strlen(argv[2]))) value1 = 1;
        else if (!strncasecmp(argv[2], "slave", strlen(argv[2]))) value1 = 0;
        else {
            vos_print("invalid param \r\n");
            return CMD_ERR_PARAM;
        }
            
        vos_print("mode is %d \r\n", value1); //todo
        return CMD_OK;
    }

    if (!strncasecmp(argv[1], "offset", cmp_len)) {
        int offset = atoi(argv[2]);
        vos_print("offset is %d \r\n", offset); //todo
        return CMD_OK;
    }
    
    vos_print("invalid param \r\n");
    return CMD_OK;
}


FIELD_INFO ifft_stat[] = 
{
    {"N0_exp_overflow_cnt",         0x43c300e4, 0, 0xFFFFFFFF,      NULL, "N0 ifft exp_overflow cnt"},
    {"N1_exp_overflow_cnt",         0x43c301fc, 0, 0xFFFFFFFF,      NULL, "N1 ifft exp_overflow cnt"},
};

int cli_ifft_stat(int argc, char **argv)
{
    FIELD_INFO *list = &ifft_stat[0];
    uint32 list_cnt = sizeof(ifft_stat)/sizeof(FIELD_INFO);
    
    return cli_show_field_info(argc, argv, list, list_cnt);
}

int cli_mac_config(int argc, char **argv)
{
    uint32 value;
    uint8 mac[6];
    int cp_len;
    
    if (argc < 2) {
        vos_print("mac_cfg show           -- show mac address\r\n");
        vos_print("mac_cfg bbu_mac <mac>  -- set bbu mac \r\n");
        vos_print("mac_cfg rhub_mac <mac> -- set rhub mac \r\n");
        return CMD_ERR_PARAM;
    }

    cp_len = strlen(argv[1]);
    if (!strncasecmp(argv[1], "show", cp_len)) {
        value = fpga_read(0x43c30020);
        mac[0] = value & 0xFF; mac[1] = (value >> 8) & 0xFF;
        mac[2] = (value >> 16) & 0xFF; mac[3] = (value >> 24) & 0xFF;
        value = fpga_read(0x43c30024);
        mac[4] = value & 0xFF; mac[5] = (value >> 8) & 0xFF;
        vos_print("BBU Mac Address : %02x%02x-%02x%02x-%02x%02x\r\n", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);

        value = fpga_read(0x43c30028);
        mac[0] = value & 0xFF; mac[1] = (value >> 8) & 0xFF;
        mac[2] = (value >> 16) & 0xFF; mac[3] = (value >> 24) & 0xFF;
        value = fpga_read(0x43c3002c);
        mac[4] = value & 0xFF; mac[5] = (value >> 8) & 0xFF;
        vos_print("RHUB Mac Address: %02x%02x-%02x%02x-%02x%02x\r\n", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);

        return CMD_OK_NO_LOG;
    }
    else if (!strncasecmp(argv[1], "bbu_mac", cp_len) && argc > 2) {
        parse_hex_string(argv[2], 6, mac);
        value = mac[2]; value = (value << 8) | mac[3];
        value = (value << 8) | mac[4]; value = (value << 8) | mac[5];
        fpga_write(0x43c30020, value);
        value = mac[0]; value = (value << 8) | mac[1];
        fpga_write_bits(0x43c30024, 0, 0xFFFF, value);
    }
    else if (!strncasecmp(argv[1], "rhub_mac", cp_len) && argc > 2) {
        parse_hex_string(argv[2], 6, mac);
        value = mac[2]; value = (value << 8) | mac[3];
        value = (value << 8) | mac[4]; value = (value << 8) | mac[5];
        fpga_write(0x43c30028, value);
        value = mac[0]; value = (value << 8) | mac[1];
        fpga_write_bits(0x43c3002c, 0, 0xFFFF, value);
    } 
    else {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    return CMD_OK;
}

int cli_phase_compensation(int argc, char **argv)
{
    if (argc < 2) {
        vos_print("phase_comp show -- show phase compensation\r\n");
        vos_print("phase_comp <sym_num> <fft_len> <cp_len_l> <cp_len_s> <sample_rate> <fo> -- set phase compensation\r\n");
        return CMD_ERR_PARAM;
    }

    if (!strncasecmp(argv[1], "show", 4)) {
        
        return CMD_OK_NO_LOG;
    }

    return CMD_OK;
}

FIELD_INFO pxsch_stat[] = 
{
    {"N0_sc_map_in_cnt",        0x43c300c8, 0, 0xFFFFFFFF,       NULL, "BBU send to N0, harden_tx pusch pkt number"},
    {"N1_sc_map_in_cnt",        0x43c301e0, 0, 0xFFFFFFFF,       NULL, "BBU send to N1, harden_tx pusch pkt number"},
    {"N0_sc_demap_out_cnt",     0x43c300ec, 0,  0xFFFFFFFF,      NULL, "N0 harden_rx sent to BBU, pdsch pkt number"},
    {"N1_sc_demap_out_cnt",     0x43c30204, 0, 0xFFFFFFFF,       NULL, "N1 harden_rx sent to BBU, pdsch pkt number"},
};

int cli_pxsch_stat(int argc, char **argv)
{
    FIELD_INFO *list = &pxsch_stat[0];
    uint32 list_cnt = sizeof(pxsch_stat)/sizeof(FIELD_INFO);
    
    return cli_show_field_info(argc, argv, list, list_cnt);
}

FIELD_INFO harden_rx_ctrl[] = 
{
    {"enable_rx",           0x43c30054, 15, 0x1,   "enable harden_rx function", "0-disable, 1-enable"},
    {"bypass_fft",          0x43c30054, 14, 0x1,   "disable fft function of harden_rx", "0-no bypass, 1-bypass"},
    {"bypass_phs",          0x43c30054, 11, 0x1,   "disable phase comps function of harden_tx", "0-no bypass, 1-bypass"},
    {"scaler_enable",       0x43c30054, 10, 0x1,   "enable ifft scalar function of harden_rx", "0-disable, 1-enable"},
    {"dc_disable",          0x43c30054, 8,   0x1,  "if dc_disable = 0 ,The dc position is not zero", "0-enable, 1-disable"},
    {"cp_remove_enable",    0x43c30054, 30, 0x1,   "send the data of cp_remove module to debug_xge", "0-disable, 1-enable"},
    {"phs_enable",          0x43c30054, 29, 0x1,   "send the data of phase compensation module to debug_xge", "0-disable, 1-enable"},
    {"fft_enable",          0x43c30054, 28, 0x1,   "send the data of fft module to debug_xge", "0-disable, 1-enable"},
    {"compression_enable",  0x43c30054, 27, 0x1,   "send the data of compression module to debug_xge", "0-disable, 1-enable"},
    {"sc_demap_enable",     0x43c30054, 26, 0x1,   "send the data of sc_demap module to debug_xge", "0-disable, 1-enable"},
    {"source_enable",       0x43c30054, 25, 0x1,   "Send source data directly to debug_xge", "0-disable, 1-enable"},
    {"antena1_enable",      0x43c30054, 16, 0x1,   "select which antenna data to debug_xge", "0:antenna0, 1:antenna1"},
    {"piece_select",        0x43c30114, 0,   0x1,  "debug harden_rx piece select", "0:piece num0, 1:piece num1"},
};

int cli_harden_rx_ctrl(int argc, char **argv)
{
    FIELD_INFO *list = &harden_rx_ctrl[0];
    uint32 list_cnt = sizeof(harden_rx_ctrl)/sizeof(FIELD_INFO);
    
    return cli_do_field_info(argc, argv, list, list_cnt);
}

FIELD_INFO harden_tx_ctrl[] = 
{
    {"enable_tx",           0x43c30050, 15, 0x1,   "enable harden_tx function", "0-disable, 1-enable"},
    {"bypass_fft",          0x43c30050, 14, 0x1,   "disable ifft function of harden_tx", "0-no bypass, 1-bypass"},
    {"sc_sync_dis",         0x43c30050, 13, 0x1,   "Turn off sc_map module synchronization sync_index function", "0-turn on, 1-turn off"},
    {"cp_sync_dis",         0x43c30050, 12, 0x1,   "Turn off cp_insertion module synchronization sync_index function", "0-turn on, 1-turn off"},
    {"bypass_phs",          0x43c30050, 11, 0x1,   "disable phase comps function of harden_tx", "0-no bypass, 1-bypass"},
    {"scaler_enable",       0x43c30050, 10, 0x1,   "enable ifft scalar function of harden_tx", "0-disable, 1-enable"},
    {"dc_disable",          0x43c30050, 8,   0x1,  "if dc_disable = 0 ,The dc position is not zero", "0-enable, 1-disable"},
    {"ref_exp",             0x43c30050, 0,   0x1F, "if enable scaler function,this value must to be set", "5 bits of ref exp"},
    {"sc_map_enable",       0x43c30050, 30, 0x1,   "send the data of sc_map module to debug_xge", "0-disable, 1-enable"},
    {"decompress_enable",   0x43c30050, 29, 0x1,   "send the data of deccompress module to debug_xge", "0-disable, 1-enable"},
    {"fft_enable",          0x43c30050, 28, 0x1,   "send the data of ifft module to debug_xge", "0-disable, 1-enable"},
    {"phs_enable",          0x43c30050, 27, 0x1,   "send the data of phase compensation module to debug_xge", "0-disable, 1-enable"},
    {"cpi_enable",          0x43c30050, 26, 0x1,   "send the data of cp_insertion module to debug_xge", "0-disable, 1-enable"},
    {"source_enable",       0x43c30050, 25, 0x1,   "Send source data directly to debug_xge", "0-disable, 1-enable"},
    {"antena1_enable",      0x43c30050, 16, 0x1,   "select which antenna data to debug_xge", "0:antenna0, 1:antenna1"},
    {"piece_select",        0x43c30110, 0,   0x1,  "debug harden_tx piece select", "0:piece num0, 1:piece num1"},
};

int cli_harden_tx_ctrl(int argc, char **argv)
{
    FIELD_INFO *list = &harden_tx_ctrl[0];
    uint32 list_cnt = sizeof(harden_tx_ctrl)/sizeof(FIELD_INFO);
    
    return cli_do_field_info(argc, argv, list, list_cnt);
}

FIELD_INFO tx_timeout[] = 
{
    {"N0_sc_map_timeout_cnt",       0x43c300cc, 0, 0xFFFFFFFF,      NULL, "first 9009 ANT0&ANT1 SC timout cnt"},
    {"N0_cpi0_timeout_cnt",         0x43c300d0, 0, 0xFFFFFFFF,      NULL, "first 9009 ANT0 CPI timout cnt"},
    {"N0_cpi1_timeout_cnt",         0x43c300d4, 0,  0xFFFFFFFF,     NULL, "first 9009 ANT1 CPI timout cnt"},
    {"N1_sc_map_timeout_cnt",       0x43c301e4, 0, 0xFFFFFFFF,      NULL, "second 9009 ANT0&ANT1 SC timout cnt"},
    {"N1_cpi0_timeout_cnt",         0x43c301e8, 0, 0xFFFFFFFF,      NULL, "second 9009 ANT0 CPI timout cnt"},
    {"N1_cpi1_timeout_cnt",         0x43c301ec, 0,  0xFFFFFFFF,     NULL, "second 9009 ANT1 CPI timout cnt"},
};

int cli_tx_timeout_stat(int argc, char **argv)
{
    FIELD_INFO *list = &tx_timeout[0];
    uint32 list_cnt = sizeof(tx_timeout)/sizeof(FIELD_INFO);
    
    return cli_show_field_info(argc, argv, list, list_cnt);
}

FIELD_INFO xge_stat[] = 
{                                           //base  offset1 offset2
    {"Received bytes",                      0x0,    0x200,  0x204,      NULL, NULL},
    {"Transmitted bytes",                   0x0,    0x208,  0x20c,      NULL, NULL},
    {"Undersize frames received",           0x0,    0x210,  0x214,      NULL, NULL},
    {"Fragment frames received",            0x0,    0x218,  0x21c,      NULL, NULL},
    {"Oversize frames received OK",         0x0,    0x250,  0x254,      NULL, NULL},
    {"Oversize frames transmitted OK",      0x0,    0x288,  0x28c,      NULL, NULL},
    {"Frames received OK",                  0x0,    0x290,  0x294,      NULL, NULL},
    {"Frame Check Sequence errors",         0x0,    0x298,  0x29c,      NULL, NULL},
    {"Broadcast frames received OK",        0x0,    0x2a0,  0x2a4,      NULL, NULL},
    {"Multicast frames received OK",        0x0,    0x2a8,  0x2ac,      NULL, NULL},
    {"Control frames received OK",          0x0,    0x2b0,  0x2b4,      NULL, NULL},
    {"Length/Type out of range",            0x0,    0x2b8,  0x2bc,      NULL, NULL},
    {"PAUSE frames received OK",            0x0,    0x2c8,  0x2cc,      NULL, NULL},
    {"Frames transmitted OK",               0x0,    0x2d8,  0x2dc,      NULL, NULL},
    {"Broadcast frames transmitted OK",     0x0,    0x2e0,  0x2e4,      NULL, NULL},
    {"Multicast frames transmitted OK",     0x0,    0x2e8,  0x2ec,      NULL, NULL},
    {"Underrun errors",                     0x0,    0x2f0,  0x2f4,      NULL, NULL},
    {"Control frames transmitted OK",       0x0,    0x2f8,  0x2fc,      NULL, NULL},
    {"PAUSE frames transmitted OK",         0x0,    0x308,  0x30c,      NULL, NULL},
};

int cli_xge_stat(int argc, char **argv)
{
    FIELD_INFO *list = &xge_stat[0];
    uint32 list_cnt = sizeof(xge_stat)/sizeof(FIELD_INFO);
    uint32 i, value1, value2;

    if (argc < 2) {
        vos_print("xge_stat <bbu|cas> -- show xge statistics\r\n");
        return CMD_ERR_PARAM;
    }    

    //reset base address
    if (!strncasecmp(argv[1], "bbu", strlen(argv[1]))) {
        for (i = 0; i < list_cnt; i++) list[i].reg_addr = 0x43c40000; 
    } else  if (!strncasecmp(argv[1], "cas", strlen(argv[1]))) {
        for (i = 0; i < list_cnt; i++) list[i].reg_addr = 0x43c90000;
    } else {
        vos_print("invalid param \r\n");
        return CMD_ERR_PARAM;
    }

    for (i = 0; i < list_cnt; i++) {
        value1 = fpga_read(list[i].reg_addr + list[i].start_bit); //LSW
        value2 = fpga_read(list[i].reg_addr + list[i].mask); //MSW
        vos_print("%-36s: 0x%08x 0x%08x \r\n", list[i].field_name, value2, value1);
    }
    
    return CMD_OK_NO_LOG;
}

#endif

int upcfg_cli_init()
{
    cli_cmd_reg("pps_select",           "pps select config",                &cli_pps_select);
    cli_cmd_reg("tdd_enable",           "tdd function enable",              &cli_tdd_enable);
    cli_cmd_reg("tdd_format",           "tdd format config",                &cli_tdd_format);
    cli_cmd_reg("tddc_ctrl",            "tddc control",                     &cli_tddc_ctrl);

#ifdef BOARD_RRU_G3
    cli_cmd_reg("cpri_link",            "show CPRI link status",            &cli_cpri_link_show);
    cli_cmd_reg("dfe_ctrl",             "dfe control",                      &cli_dfe_ctrl);
    cli_cmd_reg("dfe_out_ctrl",         "dfe out control",                  &cli_dfe_out_ctrl);
    cli_cmd_reg("gain_ctrl",            "gain control",                     &cli_gain_ctrl);
    cli_cmd_reg("tx_atten",             "tx attenuation",                   &cli_rf_tx_attenuation);
    cli_cmd_reg("rf_freq",              "RF frequence",                     &cli_rf_frequence);
#endif

#ifdef BOARD_RHUB_G1
    cli_cmd_reg("agg_weight",           "aggregator weight config",         &cli_aggregator_weight);
    cli_cmd_reg("ant_enable",           "antenna enable config",            &cli_antenna_enable);
    cli_cmd_reg("ant_remapping",        "antenna remapping config",         &cli_antenna_remapping);
    cli_cmd_reg("capture",              "antenna capture config",           &cli_antenna_capture);
    cli_cmd_reg("cpri_delay",           "show CPRI delay param",            &cli_cpri_delay_show);
    cli_cmd_reg("cpri_link",            "show CPRI link status",            &cli_cpri_link_show);
    cli_cmd_reg("time_sync",            "config time sync",                 &cli_time_sync_config);
    cli_cmd_reg("ifft_stat",            "show ifft overflow statistics",    &cli_ifft_stat);
    cli_cmd_reg("mac_cfg",              "config mac address",               &cli_mac_config);
    cli_cmd_reg("phase_comp",           "config phase compensation",        &cli_phase_compensation);
    cli_cmd_reg("pxsch_stat",           "show PUSCH & PDSCH statistics",    &cli_pxsch_stat);
    cli_cmd_reg("rx_ctrl",              "harden rx ctrl",                   &cli_harden_rx_ctrl);
    cli_cmd_reg("tx_ctrl",              "harden tx ctrl",                   &cli_harden_tx_ctrl);
    cli_cmd_reg("tx_timeout",           "show tx timeout statistics",       &cli_tx_timeout_stat);
    cli_cmd_reg("xge_stat",             "show xg_ethernet statistics",      &cli_xge_stat);
#endif

    return VOS_OK;
}

