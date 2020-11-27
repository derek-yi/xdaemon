
#ifndef _DEV_FRU_H_
#define _DEV_FRU_H_

/*************************************************************************
 * FRU_APP
 *************************************************************************/
#ifdef FRU_APP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#ifndef uint8
typedef unsigned char uint8;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef uint32
typedef unsigned int uint32;
#endif

#define VOS_OK      0
#define VOS_ERR     (-1)

#ifndef TRUE
#define TRUE        1
#endif
#ifndef FALSE
#define FALSE       0
#endif

#ifndef T_DESC
#define T_DESC(x, y)   (y)
#endif

typedef struct 
{
    int     i2c_bus;
    int     dev_id;
    int     wr_blk_size;
    int     rd_blk_size;
    int     chip_size;
}FRU_EEPROM_INFO;

extern char* read_file(const char *filename);

#endif //FRU_APP


/*************************************************************************
 * fru data structure
 *************************************************************************/
#define MAX_FRU_NUM     4

//FUHUAKE_Product_FRU_Specification_Rev_0p5_20200707.xlsx
typedef struct 
{
    uint8   fmt_version;
    uint8   internal_use_start;
    uint8   chassis_info_start;
    uint8   board_info_start;
    uint8   product_info_start;
    uint8   multi_record_start;
    uint8   pad_data;
    uint8   area_checksum;
}FRU_COMMON_HEADER;

typedef struct 
{
    uint8   fmt_version;
    uint8   area_len;
    uint8   chassis_type;
    uint8   chassis_part_num_type;
    uint8   chassis_part_num[20];
    uint8   chassis_serial_num_type;
    uint8   chassis_serial_num[20];
    uint8   chassis_info_n1_type;
    uint8   chassis_info_n1[16];
    uint8   end_marker;
    uint8   area_checksum;
}FRU_CHASSIS_INFO;

typedef struct 
{
    uint8   fmt_version;
    uint8   area_len;
    uint8   language_code;
    uint8   Manufacturing_time[3];
    uint8   manufacturer_type;
    uint8   manufacturer_name[16];
    
    uint8   board_name_type;
    uint8   board_name[16];
    uint8   board_serial_num_type;
    uint8   board_serial_num[20];
    uint8   board_part_num_type;
    uint8   board_part_num[20];
    uint8   board_fru_fileid_type;
    uint8   board_fru_fileid;

    uint8   custom_board_info_n1_type;
    uint8   custom_board_info_n1[8];
    uint8   custom_board_info_n2_type;
    uint8   custom_board_info_n2[24];
    uint8   custom_board_info_n3_type;
    uint8   custom_board_info_n3[63];
    uint8   custom_board_info_n4_type;
    uint8   custom_board_info_n4[63];
    uint8   end_marker;
    uint8   area_checksum;
}FRU_BOARD_INFO;

typedef struct 
{
    uint8   fmt_version;
    uint8   area_len;
    uint8   language_code;
    uint8   manufacturer_type;
    uint8   manufacturer_name[16];
    
    uint8   product_name_type;
    uint8   product_name[16];
    uint8   product_part_num_type;
    uint8   product_part_num[20];
    uint8   product_version_type;
    uint8   product_version[8];
    uint8   product_serial_num_type;
    uint8   product_serial_num[20];
    uint8   product_asset_tag_type;
    uint8   product_asset_tag[8];
    uint8   product_fru_fileid_type;
    uint8   product_fru_fileid;

    uint8   custom_product_info_n1_type;
    uint8   custom_product_info_n1[8];
    uint8   custom_product_info_n2_type;
    uint8   custom_product_info_n2[24];
    uint8   custom_product_info_n3_type;
    uint8   custom_product_info_n3[56];
    uint8   end_marker;
    uint8   area_checksum;
}FRU_PRODUCT_INFO;


int devm_fru_get_uuid(char *uuid_str, int max);

#endif
