
#include "daemon_pub.h"

#include "drv_main.h"
#include "devm_main.h"
#include "drv_i2c.h"

#define _DEBUG

#if T_DESC("FRU_FUNC", 1)
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

FRU_COMMON_HEADER   fru_common_info[MAX_FRU_NUM];
FRU_CHASSIS_INFO    fru_chassis_info[MAX_FRU_NUM];
FRU_BOARD_INFO      fru_board_info[MAX_FRU_NUM];
FRU_PRODUCT_INFO    fru_product_info[MAX_FRU_NUM];

int devm_load_area_data(int fru_id, int start, uint8 *area_data, uint32 area_len)
{
    int ret;
    int blk_cnt, i = 0;
    FRU_EEPROM_INFO info;
    int offset;
    
    ret = drv_get_eeprom_info(fru_id, &info);
    if (ret != VOS_OK) {
        printf("%d: invalid fru_id %d \r\n", __LINE__, fru_id);
        return VOS_ERR;
    }

    start = start*8;
    if (start + area_len > info.chip_size)  {
        printf("%d: oversize %d \r\n", __LINE__, start + area_len);
        return VOS_ERR;
    }

    blk_cnt = area_len/info.rd_blk_size;
    for(i = 0; i < blk_cnt; i++) {
        offset = start + i*info.rd_blk_size;
        i2c_read_buffer(info.i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, info.dev_id, 
                        offset, area_data + i*info.rd_blk_size, info.rd_blk_size);
    }

    if (area_len%info.rd_blk_size) {
        offset = start + i*info.rd_blk_size;
        i2c_read_buffer(info.i2c_bus, I2C_SMBUS_I2C_BLOCK_DATA, info.dev_id, 
                        offset, area_data + i*info.rd_blk_size, area_len%info.rd_blk_size);
    }

    return VOS_OK;
}

int devm_store_area_data(int fru_id, int start, uint8 *area_data, uint32 area_len)
{
    int ret;
    int i;
    FRU_EEPROM_INFO info;
    
    ret = drv_get_eeprom_info(fru_id, &info);
    if (ret != VOS_OK) {
        printf("%d: invalid fru_id %d \r\n", __LINE__, fru_id);
        return VOS_ERR;
    }
    
    start = start*8;
    if (start + area_len > info.chip_size)  {
        printf("%d: oversize %d \r\n", __LINE__, start + area_len);
        return VOS_ERR;
    }

    //printf("%d: fru_id %d, i2c_bus %d, dev_id %d, start %d, len %d \r\n", __LINE__, fru_id, info.i2c_bus, info.dev_id, start, area_len);
    for(i = 0; i < area_len; i++) {
        ret |= i2c_write_buffer(info.i2c_bus, I2C_SMBUS_BYTE_DATA, info.dev_id, start + i, &area_data[i], 1); //byte mode
        vos_msleep(5);
    }
    
    if (ret != VOS_OK) {
        printf("%d: i2c_write_buffer failed \r\n", __LINE__);
        return VOS_ERR;
    }

    return VOS_OK;
}

static uint8 devm_get_area_checksum(uint8 *area_data, uint32 data_len)
{
	uint32 i;
	uint8 tmp = 0;

	for (i = 0; i < data_len; i++)
		tmp += area_data[i];

	return tmp;
}

int devm_load_fru_info(int fru_id)
{
    int ret;
    
    if (fru_id >= MAX_FRU_NUM) {
		fprintf(stderr, "invalid fru_id %d \r\n", fru_id);
        return VOS_ERR;
    }
    
    ret = devm_load_area_data(fru_id, 0, (uint8 *)&fru_common_info[fru_id], sizeof(FRU_COMMON_HEADER));
    if (ret != VOS_OK) {
		fprintf(stderr, "devm_load_area_data failed, ret %d \r\n", ret);
        return VOS_ERR;
    }

    xlog(XLOG_DEVM, "fru_id %d ", fru_id);
    xlog(XLOG_DEVM, "chassis_info_start %d ", fru_common_info[fru_id].chassis_info_start);
    xlog(XLOG_DEVM, "board_info_start %d ", fru_common_info[fru_id].board_info_start);
    xlog(XLOG_DEVM, "product_info_start %d ", fru_common_info[fru_id].product_info_start);

    memset(&fru_chassis_info[fru_id], 0, sizeof(FRU_CHASSIS_INFO));
    if (fru_common_info[fru_id].chassis_info_start > 0) {
        ret = devm_load_area_data(fru_id, fru_common_info[fru_id].chassis_info_start, (uint8 *)&fru_chassis_info[fru_id], sizeof(FRU_CHASSIS_INFO));
        if (ret != VOS_OK) {
    		printf("%d: devm_load_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
        if ( fru_chassis_info[fru_id].area_checksum != devm_get_area_checksum((uint8 *)&fru_chassis_info[fru_id], sizeof(FRU_CHASSIS_INFO) - 1) ) {
            fru_chassis_info[fru_id].area_len = 0;  //invalid area
        }
    }

    memset(&fru_board_info[fru_id], 0, sizeof(FRU_BOARD_INFO));
    if (fru_common_info[fru_id].board_info_start > 0) {
        ret = devm_load_area_data(fru_id, fru_common_info[fru_id].board_info_start, (uint8 *)&fru_board_info[fru_id], sizeof(FRU_BOARD_INFO));
        if (ret != VOS_OK) {
    		printf("%d: devm_load_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
        if ( fru_board_info[fru_id].area_checksum != devm_get_area_checksum((uint8 *)&fru_board_info[fru_id], sizeof(FRU_BOARD_INFO) - 1) ) {
            fru_board_info[fru_id].area_len = 0;  //invalid area
        }
    }

    memset(&fru_product_info[fru_id], 0, sizeof(FRU_PRODUCT_INFO));
    if (fru_common_info[fru_id].product_info_start > 0) {
        ret = devm_load_area_data(fru_id, fru_common_info[fru_id].product_info_start, (uint8 *)&fru_product_info[fru_id], sizeof(FRU_PRODUCT_INFO));
        if (ret != VOS_OK) {
    		printf("%d: devm_load_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
        if ( fru_product_info[fru_id].area_checksum != devm_get_area_checksum((uint8 *)&fru_product_info[fru_id], sizeof(FRU_PRODUCT_INFO) - 1) ) {
            fru_product_info[fru_id].area_len = 0;  //invalid area
        }
    }
    
    return VOS_OK;
}

#define FOMRT_FIELD_STR(buffer, field_data, len)    \
    do {    \
        memcpy(buffer, field_data, len);    \
        buffer[len] = 0;    \
    } while(0)

int devm_show_custom_record(int rec_len, uint8 *record)
{
    if (!record) return VOS_ERR;
    
    if (record[0] == 0x1) {
        vos_print("  Format Version: %d\r\n", record[1]);
        vos_print("  Mac Address   : %02x%02x-%02x%02x-%02x%02x\r\n", 
                    record[2], record[3], record[4], record[5], record[6], record[7]);
    }
    else if (record[0] == 0x2) { //2a5d7f1f-4817-4dea-9a70-b8ff7ae4710a
        vos_print("  Format Version: %d\r\n", record[1]);
        vos_print("  UUID          : %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\r\n", 
                    record[2], record[3], record[4], record[5], record[6], record[7], record[8], record[9], 
                    record[10], record[11], record[12], record[13], record[14], record[15], record[16], record[17]);
    }
    else if (record[0] == 0x20) {
        vos_print("  Format Version: %d\r\n", record[1]);
        vos_print("  SKU ID        : 0x%02x\r\n", record[2]);
    }
    else if (record[0] == 0x40) {
        vos_print("  Format Version    : %d\r\n", record[1]);
        vos_print("  Channel Quantity  : %d\r\n", record[2]);
        vos_print("  Channel.1 Tx Power: %d\r\n", record[3]);
        vos_print("  Channel.2 Tx Power: %d\r\n", record[4]);
    }
#ifdef _DEBUG
    else if (rec_len > 0) {
        int i;
        vos_print("unknown record: ");
        for(i = 0; i < rec_len; i++ ) vos_print("0x%02x ", record[i]);
        vos_print("\r\n");    
    }
#endif

    return VOS_OK;
}

int devm_show_fru_info(int fru_id)
{
    uint8 fmt_str[64];

    if (fru_chassis_info[fru_id].area_len > 0) {
        vos_print("Chassis Area Format Type     : 0x%02x \r\n", fru_chassis_info[fru_id].fmt_version);
        vos_print("Chassis Area Length          : 0x%02x \r\n", fru_chassis_info[fru_id].area_len);
        vos_print("Chassis Type                 : 0x%02x \r\n", fru_chassis_info[fru_id].chassis_type);
        vos_print("Chassis Part Num Type        : 0x%02x \r\n", fru_chassis_info[fru_id].chassis_part_num_type);
        FOMRT_FIELD_STR(fmt_str, fru_chassis_info[fru_id].chassis_part_num, 20);
        vos_print("Chassis Part Num             : %s \r\n", fmt_str);
        vos_print("Chassis Serial Num Type      : 0x%02x \r\n", fru_chassis_info[fru_id].chassis_serial_num_type);
        FOMRT_FIELD_STR(fmt_str, fru_chassis_info[fru_id].chassis_serial_num, 20);
        vos_print("Chassis Serial Num           : %s \r\n", fmt_str);
        vos_print("Custom Chassis Info No.1 Type: 0x%02x \r\n", fru_chassis_info[fru_id].chassis_info_n1_type);
        FOMRT_FIELD_STR(fmt_str, fru_chassis_info[fru_id].chassis_info_n1, 16);
        vos_print("Custom Chassis Info No.1     : %s \r\n", fmt_str);
        vos_print(" \r\n");
    }
    
    if (fru_board_info[fru_id].area_len > 0) {
        vos_print("Board Info Area Format Type  : 0x%02x \r\n", fru_board_info[fru_id].fmt_version);
        vos_print("Board Info Area Length       : 0x%02x \r\n", fru_board_info[fru_id].area_len);
        vos_print("Board Info Language Code     : 0x%02x \r\n", fru_board_info[fru_id].language_code);
        vos_print("Board Manufacturing Time     : 0x%02x 0x%02x 0x%02x\r\n", fru_board_info[fru_id].Manufacturing_time[0],
                fru_board_info[fru_id].Manufacturing_time[1], fru_board_info[fru_id].Manufacturing_time[2]);
        FOMRT_FIELD_STR(fmt_str, fru_board_info[fru_id].manufacturer_name, 16);
        vos_print("Board Manufacturer Name      : %s \r\n", fmt_str);
        vos_print("Board Name type              : 0x%02x \r\n", fru_board_info[fru_id].board_name_type);
        FOMRT_FIELD_STR(fmt_str, fru_board_info[fru_id].board_name, 16);
        vos_print("Board Name                   : %s \r\n", fmt_str);
        vos_print("Board Serial Num Type        : 0x%02x \r\n", fru_board_info[fru_id].board_serial_num_type);
        FOMRT_FIELD_STR(fmt_str, fru_board_info[fru_id].board_serial_num, 20);
        vos_print("Board Serial Num             : %s \r\n", fmt_str);
        vos_print("Board Part Num Type          : 0x%02x \r\n", fru_board_info[fru_id].board_part_num_type);
        FOMRT_FIELD_STR(fmt_str, fru_board_info[fru_id].board_part_num, 20);
        vos_print("Board Part Num               : %s \r\n", fmt_str);
        vos_print("Board FRU File ID type       : 0x%02x \r\n", fru_board_info[fru_id].board_fru_fileid_type);
        vos_print("Board FRU File ID type       : 0x%02x \r\n", fru_board_info[fru_id].board_fru_fileid);
        vos_print("Custom Board Info No.1 Type  : 0x%02x \r\n", fru_board_info[fru_id].custom_board_info_n1_type);
        devm_show_custom_record(fru_board_info[fru_id].custom_board_info_n1_type, fru_board_info[fru_id].custom_board_info_n1);
        vos_print("Custom Board Info No.2 Type  : 0x%02x \r\n", fru_board_info[fru_id].custom_board_info_n2_type);
        devm_show_custom_record(fru_board_info[fru_id].custom_board_info_n2_type, fru_board_info[fru_id].custom_board_info_n2);
        vos_print("Custom Board Info No.3 Type  : 0x%02x \r\n", fru_board_info[fru_id].custom_board_info_n3_type);
        devm_show_custom_record(fru_board_info[fru_id].custom_board_info_n3_type, fru_board_info[fru_id].custom_board_info_n3);
        vos_print("Custom Board Info No.4 Type  : 0x%02x \r\n", fru_board_info[fru_id].custom_board_info_n4_type);
        devm_show_custom_record(fru_board_info[fru_id].custom_board_info_n4_type, fru_board_info[fru_id].custom_board_info_n4);
        vos_print(" \r\n");
    }

    if (fru_product_info[fru_id].area_len > 0) {
        vos_print("Product Info Area Format Type: 0x%02x \r\n", fru_product_info[fru_id].fmt_version);
        vos_print("Product Info Area Length     : 0x%02x \r\n", fru_product_info[fru_id].area_len);
        vos_print("Product Info Language Code   : 0x%02x \r\n", fru_product_info[fru_id].language_code);
        vos_print("Product Manufacturer type    : 0x%02x \r\n", fru_product_info[fru_id].manufacturer_type);
        FOMRT_FIELD_STR(fmt_str, fru_product_info[fru_id].manufacturer_name, 16);
        vos_print("Product Manufacturer Name    : %s \r\n", fmt_str);
        vos_print("Product Name type            : 0x%02x \r\n", fru_product_info[fru_id].product_name_type);
        FOMRT_FIELD_STR(fmt_str, fru_product_info[fru_id].product_name, 16);
        vos_print("Product Name                 : %s \r\n", fmt_str);
        vos_print("Product Version type         : 0x%02x \r\n", fru_product_info[fru_id].product_version_type);
        FOMRT_FIELD_STR(fmt_str, fru_product_info[fru_id].product_version, 8);
        vos_print("Product Version              : %s \r\n", fmt_str);
        vos_print("Product Serial Num Type      : 0x%02x \r\n", fru_product_info[fru_id].product_serial_num_type);
        FOMRT_FIELD_STR(fmt_str, fru_product_info[fru_id].product_serial_num, 20);
        vos_print("Product Serial Num           : %s \r\n", fmt_str);
        vos_print("Product Asset Tag type       : 0x%02x \r\n", fru_product_info[fru_id].product_asset_tag_type);
        FOMRT_FIELD_STR(fmt_str, fru_product_info[fru_id].product_asset_tag, 8);
        vos_print("Product Asset Tag            : %s \r\n", fmt_str);
        vos_print("Product FRU File ID type     : 0x%02x \r\n", fru_product_info[fru_id].product_fru_fileid_type);
        vos_print("Product FRU File ID type     : 0x%02x \r\n", fru_product_info[fru_id].product_fru_fileid);
        vos_print("Custom Product Info No.1 Type: 0x%02x \r\n", fru_product_info[fru_id].custom_product_info_n1_type);
        devm_show_custom_record(fru_product_info[fru_id].custom_product_info_n1_type, fru_product_info[fru_id].custom_product_info_n1);
        vos_print("Custom Product Info No.2 Type: 0x%02x \r\n", fru_product_info[fru_id].custom_product_info_n2_type);
        devm_show_custom_record(fru_product_info[fru_id].custom_product_info_n2_type, fru_product_info[fru_id].custom_product_info_n2);
        vos_print("Custom Product Info No.3 Type: 0x%02x \r\n", fru_product_info[fru_id].custom_product_info_n3_type);
        devm_show_custom_record(fru_product_info[fru_id].custom_product_info_n3_type, fru_product_info[fru_id].custom_product_info_n3);
        vos_print(" \r\n");
    }
    
    return VOS_OK;
}

int devm_fru_set_mac(int store, uint8 mac[6])
{
    int ret;
    FRU_BOARD_INFO *pInfo;

    pInfo = &fru_board_info[0]; //todo
    
    pInfo->custom_board_info_n1_type = 0x8;
    pInfo->custom_board_info_n1[0] = 0x1;
    pInfo->custom_board_info_n1[1] = 0x1;
    memcpy(&pInfo->custom_board_info_n1[2], mac, 6);
    pInfo->area_checksum = devm_get_area_checksum((uint8 *)pInfo, sizeof(FRU_BOARD_INFO) - 1);

    if (store) {
        ret = devm_store_area_data(0, fru_common_info[0].board_info_start, (uint8 *)pInfo, sizeof(FRU_BOARD_INFO));
        if (ret != VOS_OK) {
            printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
    }
    
    return VOS_OK;
}

int devm_fru_set_uuid(int store, uint8 uuid[16])
{
    int ret;
    FRU_BOARD_INFO *pInfo;

    pInfo = &fru_board_info[0]; //todo
    
    pInfo->custom_board_info_n2_type = 0x18;
    pInfo->custom_board_info_n2[0] = 0x2;
    pInfo->custom_board_info_n2[1] = 0x1;
    memcpy(&pInfo->custom_board_info_n2[2], uuid, 16);
    pInfo->area_checksum = devm_get_area_checksum((uint8 *)pInfo, sizeof(FRU_BOARD_INFO) - 1);

    if (store) {
        ret = devm_store_area_data(0, fru_common_info[0].board_info_start, (uint8 *)pInfo, sizeof(FRU_BOARD_INFO));
        if (ret != VOS_OK) {
            printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
    }
    
    return VOS_OK;
}

int devm_fru_set_skuid(int store, uint8 skuid)
{
    int ret;
    FRU_PRODUCT_INFO *pInfo;

    pInfo = &fru_product_info[0]; //todo
    
    pInfo->custom_product_info_n1_type = 0x8;
    pInfo->custom_product_info_n1[0] = 0x20;
    pInfo->custom_product_info_n1[1] = 0x1;
    pInfo->custom_product_info_n1[2] = skuid;
    pInfo->area_checksum = devm_get_area_checksum((uint8 *)pInfo, sizeof(FRU_PRODUCT_INFO) - 1);

    if (store) {
        ret = devm_store_area_data(0, fru_common_info[0].board_info_start, (uint8 *)pInfo, sizeof(FRU_PRODUCT_INFO));
        if (ret != VOS_OK) {
            printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
    }
    
    return VOS_OK;
}

int devm_fru_set_rf_calibration(int store, uint8 offset, uint8 value)
{
    int ret;
    FRU_BOARD_INFO *pInfo;

    if (offset >= 63) {
        printf("%d: error offset %d \r\n", __LINE__, offset);
        return VOS_ERR;
    }
    
    pInfo = &fru_board_info[1]; //todo
    
    pInfo->custom_board_info_n3_type = 0x3F;
    pInfo->custom_board_info_n3[0] = 0x40;
    pInfo->custom_board_info_n3[1] = 0x1;
    pInfo->custom_board_info_n3[offset] = value;
    pInfo->area_checksum = devm_get_area_checksum((uint8 *)pInfo, sizeof(FRU_BOARD_INFO) - 1);

    if (store) {
        ret = devm_store_area_data(1, fru_common_info[1].board_info_start, (uint8 *)pInfo, sizeof(FRU_BOARD_INFO));
        if (ret != VOS_OK) {
            printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
    }
    
    return VOS_OK;
}

//refer to cpri-r21-mul-slave.sh
int devm_import_fru_info(void)
{
    char temp_data[64];

    //将sn写入文件/tmp/sn.txt
    if (fru_common_info[0].product_info_start > 0 && fru_product_info[0].product_serial_num_type > 0) {
        memcpy(temp_data, fru_product_info[0].product_serial_num, 20);
        temp_data[20] = 0;
    } else {
        sprintf(temp_data, "SZ20201111");
    }
    unlink("/tmp/sn.txt");
    xlog(XLOG_DEVM, "Product SN: %s", temp_data);
    sys_node_writestr("/tmp/sn.txt", temp_data);

    //mac address
    if (fru_common_info[0].board_info_start > 0 && fru_board_info[0].custom_board_info_n1_type > 0) {
        char mac_addr[32];
        uint8 *ptr = fru_board_info[0].custom_board_info_n1;
        sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);
        xlog(XLOG_DEVM, "Board Mac: %s", mac_addr);
        sprintf(temp_data, "ifconfig %s hw ether %s", "eth1", mac_addr);
        shell_run_cmd(temp_data);
    } 
    
    //将UUID写入文件/tmp/uuid.txt
    if (fru_common_info[0].board_info_start > 0 && fru_board_info[0].custom_board_info_n2_type > 0) {
        uint8 *ptr = fru_board_info[0].custom_board_info_n2;
        sprintf(temp_data, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
                    ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ptr[8], ptr[9], 
                    ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15], ptr[16], ptr[17]);
    } else {
        sys_node_readstr("/proc/sys/kernel/random/uuid", temp_data, sizeof(temp_data));
    }
    unlink("/tmp/uuid.txt");
    xlog(XLOG_DEVM, "Product UUID: %s", temp_data);
    sys_node_writestr("/tmp/uuid.txt", temp_data);


    //RF calibration param
    
    return VOS_OK;
}

#endif

#ifndef DAEMON_RELEASE

int devm_store_fru_info(int force) 
{
    int ret;
    FRU_COMMON_HEADER   common_info;
    FRU_CHASSIS_INFO    chassis_info;
    FRU_BOARD_INFO      board_info;
    FRU_PRODUCT_INFO    product_info;
    uint8 hex_string[6] = {0x2, 0xd1, 0x2, 0x3a, 0x2, 0xa0};

    printf("sizeof(FRU_COMMON_HEADER) %d \r\n", sizeof(FRU_COMMON_HEADER));
    printf("sizeof(FRU_CHASSIS_INFO) %d \r\n", sizeof(FRU_CHASSIS_INFO));
    printf("sizeof(FRU_BOARD_INFO) %d \r\n", sizeof(FRU_BOARD_INFO));
    printf("sizeof(FRU_PRODUCT_INFO) %d \r\n", sizeof(FRU_PRODUCT_INFO));

    ret = devm_load_area_data(0, 0, (uint8 *)&common_info, sizeof(common_info));
    if (ret == VOS_OK) {
        if ( (common_info.area_checksum == devm_get_area_checksum((uint8 *)&common_info, sizeof(common_info) - 1))
            && (force != TRUE) ) {
            printf("%d: fru data exist \r\n", __LINE__);
            return VOS_OK;
        }
    }

#if 1 //fru_id 0
    memset(&common_info, 0x0, sizeof(common_info));
    common_info.fmt_version          = 0x1;     //start 0, size 8
    common_info.chassis_info_start   = 0x1;     //8/8, size 64
    common_info.board_info_start     = 0x9;     //(8+64)/8, size 248
    common_info.product_info_start   = 0x28;    //(8+64+248)/8, size 192
    common_info.area_checksum = devm_get_area_checksum((uint8 *)&common_info, sizeof(common_info) - 1);
    ret = devm_store_area_data(0, 0, (uint8 *)&common_info, sizeof(common_info));
    if (ret != VOS_OK) {
        printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
        return VOS_ERR;
    }

    memset(&chassis_info, 0x0, sizeof(chassis_info));
    chassis_info.fmt_version = 0x1;
    chassis_info.area_len = 0x8;
    chassis_info.chassis_type = 0x1;
    chassis_info.chassis_part_num_type = 0xD4;
    sprintf((char *)chassis_info.chassis_part_num, "%s", "440.00556.005");
    chassis_info.chassis_serial_num_type = 0xD4;
    sprintf((char *)chassis_info.chassis_serial_num, "%s", "chassis_serial");
    chassis_info.area_checksum = devm_get_area_checksum((uint8 *)&chassis_info, sizeof(chassis_info) - 1);
    ret = devm_store_area_data(0, common_info.chassis_info_start, (uint8 *)&chassis_info, sizeof(chassis_info));
    if (ret != VOS_OK) {
        printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
        return VOS_ERR;
    }

    memset(&board_info, 0x0, sizeof(FRU_BOARD_INFO));
    board_info.fmt_version = 0x1;
    board_info.area_len = 0x1F;
    board_info.language_code = 0x0;
    board_info.manufacturer_type = 0xD0;
    sprintf((char *)board_info.manufacturer_name, "%s", "FUHUAKE");
    board_info.board_name_type = 0xD0;
    sprintf((char *)board_info.board_name, "%s", "DF2010A");
    board_info.custom_board_info_n1_type = 0x8; //mac address
    board_info.custom_board_info_n1[0] = 0x1;
    board_info.custom_board_info_n1[1] = 0x1;
    memcpy(&board_info.custom_board_info_n1[2], hex_string, 6);
    board_info.area_checksum = devm_get_area_checksum((uint8 *)&board_info, sizeof(board_info) - 1);
    ret = devm_store_area_data(0, common_info.board_info_start, (uint8 *)&board_info, sizeof(board_info));
    if (ret != VOS_OK) {
        printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
        return VOS_ERR;
    }

    memset(&product_info, 0x0, sizeof(product_info));
    product_info.fmt_version = 0x1;
    product_info.area_len = 0x18;
    product_info.language_code = 0x0;
    product_info.manufacturer_type = 0xD0;
    sprintf((char *)product_info.manufacturer_name, "%s", "FHK111");
    product_info.product_name_type = 0xD0;
    sprintf((char *)product_info.product_name, "%s", "RR4780B");
    product_info.product_serial_num_type = 0xD4;
    sprintf((char *)product_info.product_serial_num, "%s", "SZ20201111");
    product_info.custom_product_info_n1_type = 0x8; //sku
    product_info.custom_product_info_n1[0] = 0x20;
    product_info.custom_product_info_n1[1] = 0x1;
    product_info.custom_product_info_n1[2] = 0xa1;
    product_info.area_checksum = devm_get_area_checksum((uint8 *)&product_info, sizeof(product_info) - 1);
    ret = devm_store_area_data(0, common_info.product_info_start, (uint8 *)&product_info, sizeof(product_info));
    if (ret != VOS_OK) {
        printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
        return VOS_ERR;
    }
#endif    

#if 1 //fru_id 1
    memset(&common_info, 0x0, sizeof(common_info));
    common_info.fmt_version          = 0x1;
    common_info.internal_use_start   = 0;     
    common_info.board_info_start     = 1;     
    common_info.area_checksum = devm_get_area_checksum((uint8 *)&common_info, sizeof(common_info) - 1);
    ret = devm_store_area_data(1, 0, (uint8 *)&common_info, sizeof(common_info));
    if (ret != VOS_OK) {
        printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
        return VOS_ERR;
    }

    memset(&board_info, 0x0, sizeof(board_info));
    board_info.fmt_version = 0x1;
    board_info.area_len = 0x1F;
    board_info.language_code = 0x0;
    board_info.manufacturer_type = 0xD0;
    sprintf((char *)board_info.manufacturer_name, "%s", "FUHUAKE");
    board_info.board_name_type = 0xD0;
    sprintf((char *)board_info.board_name, "%s", "RF4780B");
    board_info.area_checksum = devm_get_area_checksum((uint8 *)&board_info, sizeof(board_info) - 1);
    ret = devm_store_area_data(1, common_info.board_info_start, (uint8 *)&board_info, sizeof(board_info));
    if (ret != VOS_OK) {
        printf("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
        return VOS_ERR;
    }
#endif
    
    return VOS_OK;
}

int fru_info_test()
{
    devm_store_fru_info(1);

    devm_load_fru_info(0);
    devm_load_fru_info(1);

    devm_show_fru_info(0);
    devm_show_fru_info(1);
    return 0;
}


#endif

#if T_DESC("CLI", 1)

int cli_show_fru_info(int argc, char **argv)
{
    int ret;
    FRU_EEPROM_INFO info;
    int fru_id;

    if (argc < 2) {
        vos_print("Usage: <%s> <fru-id> \r\n", argv[0]);
        return VOS_OK;
    }

    fru_id = atoi(argv[1]);
    ret = drv_get_eeprom_info(fru_id, &info);
    if (ret != VOS_OK) {
        vos_print("invalid FRU-ID %d \r\n", fru_id);
        return VOS_ERR;
    }

    devm_show_fru_info(fru_id);
    return VOS_OK;
}    

static inline int hex2num(char c)
{
	if (c>='0' && c<='9') return c - '0';
	if (c>='a' && c<='z') return c - 'a' + 10;//这里+10的原因是:比如16进制的a值为10
	if (c>='A' && c<='Z') return c - 'A' + 10;
	return -1;
}
 
int parse_hex_string(char *hexStr, int max, uint8 *pData)  
{  
    int value;
    int i, j, k;
    
    for (i = 0, j = 0, k = 0; i < strlen(hexStr); i++)  
    {  
        value = hex2num(hexStr[i]);
        if (value < 0) continue;
        
        if (k == 0) {
            pData[j] = value;  
            k++;
        } else {
            pData[j] = pData[j]*16 + value;
            k = 0; j++;
        }

        if (j == max) break;
    }     

    if (j < max) return -1;
    
    return 0;
} 

int cli_fru_set_mac(int argc, char **argv)
{
    uint8 mac_addr[6];

    if (argc < 2) {
        vos_print("Usage: <%s> <mac-addr> \r\n", argv[0]);
        vos_print("       <mac-addr> mac in hex, such as 000c-293e-ce9d \r\n");
        return VOS_OK;
    }

    if ( parse_hex_string(argv[1], 6, mac_addr) != VOS_OK) {
        vos_print("Invalid Mac Address \r\n");
        return VOS_OK;
    }
    
    devm_fru_set_mac(TRUE, mac_addr);
    return VOS_OK;
}    

int cli_fru_set_uuid(int argc, char **argv)
{
    uint8 hex_uuid[16];

    if (argc < 2) {
        vos_print("Usage: <%s> <uuid> \r\n", argv[0]);
        vos_print("       <uuid> uuid in hex, such as 2a5d7f1f-4817-4dea-9a70-b8ff7ae4710a \r\n");
        return VOS_OK;
    }

    if ( parse_hex_string(argv[1], 16, hex_uuid) != VOS_OK) {
        vos_print("Invalid UUID \r\n");
        return VOS_OK;
    }
    
    devm_fru_set_uuid(TRUE, hex_uuid);
    return VOS_OK;
}    

int cli_fru_set_skuid(int argc, char **argv)
{
    uint8 sku_id;

    if (argc < 2) {
        vos_print("Usage: <%s> <skuid> \r\n", argv[0]);
        return VOS_OK;
    }

    sku_id = strtoul(argv[1], 0, 0);             
    devm_fru_set_skuid(TRUE, sku_id);
    return VOS_OK;
}  

int cli_fru_set_rf_calibration(int argc, char **argv)
{
    uint8 offset, value;
    uint8 store = FALSE;

    if (argc < 3) {
        vos_print("Usage: <%s> <offset> <value> [<store>]\r\n", argv[0]);
        return VOS_OK;
    }

    offset = strtoul(argv[1], 0, 0);             
    value  = strtoul(argv[2], 0, 0);  
    if (argc > 3) store = strtoul(argv[3], 0, 0);  
   
    devm_fru_set_rf_calibration(store, offset, value);
    return VOS_OK;
}  
#endif
