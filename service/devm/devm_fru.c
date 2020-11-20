

#ifdef FRU_APP
#include "devm_fru.h" 
#include "cJSON.h"
#include "drv_i2c.h"

#else
#include "daemon_pub.h"

#include "drv_main.h"
#include "devm_main.h"
#include "drv_i2c.h"
#include "devm_fru.h" 

#endif

int dbg_mode = 0;

#ifdef FRU_APP

#define vos_print       printf
#define vos_msleep(x)   usleep((x)*1000)

int drv_get_eeprom_info(int fru_id, FRU_EEPROM_INFO *info)
{
    if (fru_id == 1) {
        info->i2c_bus       = 6;
        info->dev_id        = 0x50;
        info->wr_blk_size   = 16;
        info->rd_blk_size   = 32;
        info->chip_size     = 256;
    } else if (fru_id == 0) {
        info->i2c_bus       = 3;
        info->dev_id        = 0x54;
        info->wr_blk_size   = 16;
        info->rd_blk_size   = 32;
        info->chip_size     = 1024;
    } else {
        return VOS_ERR;
    }
    
    return VOS_OK;
}

#endif

#if T_DESC("FRU_FUNC", 1)

FRU_COMMON_HEADER   fru_common_info[MAX_FRU_NUM];
FRU_CHASSIS_INFO    fru_chassis_info[MAX_FRU_NUM];
FRU_BOARD_INFO      fru_board_info[MAX_FRU_NUM];
FRU_PRODUCT_INFO    fru_product_info[MAX_FRU_NUM];

int devm_load_area_data(int fru_id, int start, uint8 *area_data, uint32 area_len)
{
    int i, ret;
    FRU_EEPROM_INFO info;
    
    ret = drv_get_eeprom_info(fru_id, &info);
    if (ret != VOS_OK) {
        vos_print("%d: invalid fru_id %d \r\n", __LINE__, fru_id);
        return VOS_ERR;
    }

    start = start*8;
    if (start + area_len > info.chip_size)  {
        vos_print("%d: oversize %d \r\n", __LINE__, start + area_len);
        return VOS_ERR;
    }

    for(i = 0; i < area_len; i++) {
        area_data[i] = i2c_read_data(info.i2c_bus, I2C_SMBUS_BYTE_DATA, info.dev_id, start + i);
    }    

    return VOS_OK;
}

int devm_store_area_data(int fru_id, int start, uint8 *area_data, uint32 area_len)
{
    int i, ret;
    FRU_EEPROM_INFO info;
    
    ret = drv_get_eeprom_info(fru_id, &info);
    if (ret != VOS_OK) {
        vos_print("%d: invalid fru_id %d \r\n", __LINE__, fru_id);
        return VOS_ERR;
    }
    
    start = start*8;
    if (start + area_len > info.chip_size)  {
        vos_print("%d: oversize %d \r\n", __LINE__, start + area_len);
        return VOS_ERR;
    }

    //printf("%d: fru_id %d, i2c_bus %d, dev_id %d, start %d, len %d \r\n", __LINE__, fru_id, info.i2c_bus, info.dev_id, start, area_len);
    for(i = 0; i < area_len; i++) {
        ret |= i2c_write_buffer(info.i2c_bus, I2C_SMBUS_BYTE_DATA, info.dev_id, start + i, &area_data[i], 1); //byte mode
        vos_msleep(5);
    }
    
    if (ret != VOS_OK) {
        vos_print("%d: i2c_write_buffer failed \r\n", __LINE__);
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

int devm_read_fru_info(int fru_id)
{
    int ret;
    
    if (fru_id >= MAX_FRU_NUM) {
		vos_print("invalid fru_id %d \r\n", fru_id);
        return VOS_ERR;
    }
    
    ret = devm_load_area_data(fru_id, 0, (uint8 *)&fru_common_info[fru_id], sizeof(FRU_COMMON_HEADER));
    if (ret != VOS_OK) {
		vos_print("devm_load_area_data failed, ret %d \r\n", ret);
        return VOS_ERR;
    }

    if (dbg_mode) {
        vos_print("fru_id %d \r\n", fru_id);
        vos_print("chassis_info_start %d \r\n", fru_common_info[fru_id].chassis_info_start);
        vos_print("board_info_start %d \r\n", fru_common_info[fru_id].board_info_start);
        vos_print("product_info_start %d \r\n", fru_common_info[fru_id].product_info_start);
    }

    memset(&fru_chassis_info[fru_id], 0, sizeof(FRU_CHASSIS_INFO));
    if (fru_common_info[fru_id].chassis_info_start > 0) {
        if (dbg_mode) vos_print("%d: load fru_common_info \r\n", __LINE__);
        ret = devm_load_area_data(fru_id, fru_common_info[fru_id].chassis_info_start, (uint8 *)&fru_chassis_info[fru_id], sizeof(FRU_CHASSIS_INFO));
        if (ret != VOS_OK) {
    		vos_print("%d: devm_load_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
        if ( fru_chassis_info[fru_id].area_checksum != devm_get_area_checksum((uint8 *)&fru_chassis_info[fru_id], sizeof(FRU_CHASSIS_INFO) - 1) ) {
            fru_chassis_info[fru_id].area_len = 0;  //invalid area
            vos_print("%d: fru_common_info crc error \r\n", __LINE__);
        }
    }

    memset(&fru_board_info[fru_id], 0, sizeof(FRU_BOARD_INFO));
    if (fru_common_info[fru_id].board_info_start > 0) {
        if (dbg_mode)vos_print("%d: load fru_board_info \r\n", __LINE__);
        ret = devm_load_area_data(fru_id, fru_common_info[fru_id].board_info_start, (uint8 *)&fru_board_info[fru_id], sizeof(FRU_BOARD_INFO));
        if (ret != VOS_OK) {
    		vos_print("%d: devm_load_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
        if ( fru_board_info[fru_id].area_checksum != devm_get_area_checksum((uint8 *)&fru_board_info[fru_id], sizeof(FRU_BOARD_INFO) - 1) ) {
            fru_board_info[fru_id].area_len = 0;  //invalid area
            vos_print("%d: fru_board_info crc error \r\n", __LINE__);
        }
    }

    memset(&fru_product_info[fru_id], 0, sizeof(FRU_PRODUCT_INFO));
    if (fru_common_info[fru_id].product_info_start > 0) {
        if (dbg_mode)vos_print("%d: load fru_product_info \r\n", __LINE__);
        ret = devm_load_area_data(fru_id, fru_common_info[fru_id].product_info_start, (uint8 *)&fru_product_info[fru_id], sizeof(FRU_PRODUCT_INFO));
        if (ret != VOS_OK) {
    		vos_print("%d: devm_load_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
        if ( fru_product_info[fru_id].area_checksum != devm_get_area_checksum((uint8 *)&fru_product_info[fru_id], sizeof(FRU_PRODUCT_INFO) - 1) ) {
            fru_product_info[fru_id].area_len = 0;  //invalid area
            vos_print("%d: fru_product_info crc error \r\n", __LINE__);
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
#ifndef DAEMON_RELEASE
    else if (rec_len > 0) {
        int i;
        vos_print("  RAW Data: ");
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
        devm_show_custom_record(fru_chassis_info[fru_id].chassis_info_n1_type, fru_chassis_info[fru_id].chassis_info_n1);
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
            vos_print("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
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
            vos_print("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
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
            vos_print("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
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
        vos_print("%d: error offset %d \r\n", __LINE__, offset);
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
            vos_print("%d: devm_store_area_data failed, ret %d \r\n", __LINE__, ret);
            return VOS_ERR;
        }
    }
    
    return VOS_OK;
}

#ifndef FRU_APP
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

static int _get_json_val(uint8 *dst, int max_len, cJSON* root)
{
    int sub_cnt;
    int i, j;
    int mode = 0; //invalid
    
    sub_cnt = cJSON_GetArraySize(root);
    for (i = 0; i < sub_cnt; i++) {
        cJSON* sub_node = cJSON_GetArrayItem(root, i); 

        //vos_print("%d: sub_node->string %s \r\n", __LINE__, sub_node->string);
        if ( !strcmp(sub_node->string, "Data Format") )  {
            if ( !strcmp(sub_node->valuestring, "Binary") ) mode = 1;
            else if ( !strcmp(sub_node->valuestring, "ASCII") ) mode = 2;
        }

        if ( !strcmp(sub_node->string, "value") && (mode == 1) )  {
            int p_size = cJSON_GetArraySize(sub_node);
            if (p_size > max_len) p_size = max_len;
            for (j = 0; j < p_size; j++) {
                cJSON* tmp_param = cJSON_GetArrayItem(sub_node, j);
                dst[j] = (uint8)tmp_param->valueint;
                //vos_print("%d: mode 1, value %d \r\n", __LINE__, tmp_param->valueint);
            }
        }
        else if ( !strcmp(sub_node->string, "value") && (mode == 2) )  {
            snprintf((char *)dst, max_len, "%s", sub_node->valuestring);
            //vos_print("%d: mode 2, value %s \r\n", __LINE__, sub_node->valuestring);
        }
    }

    return VOS_OK;
}

static int drv_fru_json_parse1(FRU_COMMON_HEADER* area, cJSON* root_tree)
{
    int ret = VOS_OK;
    int i, sub_cnt;
    
    sub_cnt = cJSON_GetArraySize(root_tree);
    //vos_print("%d: sub_cnt %d \r\n", __LINE__, sub_cnt);
    for (i = 0; i < sub_cnt; ++i) {
        cJSON* sub_node = cJSON_GetArrayItem(root_tree, i); //each field

        //vos_print("%d: sub_node->string %s \r\n", __LINE__, sub_node->string);
        if ( !strcmp(sub_node->string, "Common Header Format Version") ) 
            ret |= _get_json_val((uint8 *)&area->fmt_version, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Internal Use Area Starting Offset") ) 
            ret |= _get_json_val((uint8 *)&area->internal_use_start, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Chassis Info Area Starting Offset") ) 
            ret |= _get_json_val((uint8 *)&area->chassis_info_start, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board Info Area Starting Offset") ) 
            ret |= _get_json_val((uint8 *)&area->board_info_start, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product Info Area Starting Offset") ) 
            ret |= _get_json_val((uint8 *)&area->product_info_start, 1, sub_node);
        else if ( !strcmp(sub_node->string, "MultiRecord Area Starting Offset") ) 
            ret |= _get_json_val((uint8 *)&area->multi_record_start, 1, sub_node);
    }
    
    return ret;
}

static int drv_fru_json_parse2(FRU_CHASSIS_INFO* area, cJSON* root_tree)
{
    int ret = VOS_OK;
    int i, sub_cnt;
    
    sub_cnt = cJSON_GetArraySize(root_tree);
    for (i = 0; i < sub_cnt; ++i) {
        cJSON* sub_node = cJSON_GetArrayItem(root_tree, i); //each field

        //vos_print("%d: sub_node->string %s \r\n", __LINE__, sub_node->string);
        if ( !strcmp(sub_node->string, "Chassis Info Area Format Version") ) 
            ret |= _get_json_val((uint8 *)&area->fmt_version, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Chassis Info Area Length") ) 
            ret |= _get_json_val((uint8 *)&area->area_len, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Chassis Type") ) 
            ret |= _get_json_val((uint8 *)&area->chassis_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Chassis Part Number type") ) 
            ret |= _get_json_val((uint8 *)&area->chassis_part_num_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Chassis Part Number bytes") ) 
            ret |= _get_json_val((uint8 *)&area->chassis_part_num, 20, sub_node);
        else if ( !strcmp(sub_node->string, "Chassis Serial Number type") ) 
            ret |= _get_json_val((uint8 *)&area->chassis_serial_num_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Chassis Serial Number bytes") ) 
            ret |= _get_json_val((uint8 *)&area->chassis_serial_num, 20, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Chassis Info No.1 type") ) 
            ret |= _get_json_val((uint8 *)&area->chassis_info_n1_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Chassis Info No.1 bytes") ) 
            ret |= _get_json_val((uint8 *)&area->chassis_info_n1, 16, sub_node);
        else if ( !strcmp(sub_node->string, "End of Fields marker") ) 
            ret |= _get_json_val((uint8 *)&area->end_marker, 1, sub_node);
    }
    
    return ret;
}

static int drv_fru_json_parse3(FRU_BOARD_INFO* area, cJSON* root_tree)
{
    int ret = VOS_OK;
    int i, sub_cnt;
    
    sub_cnt = cJSON_GetArraySize(root_tree);
    for (i = 0; i < sub_cnt; ++i) {
        cJSON* sub_node = cJSON_GetArrayItem(root_tree, i); //each field

        //vos_print("%d: sub_node->string %s \r\n", __LINE__, sub_node->string);
        if ( !strcmp(sub_node->string, "Board Info Area Format Version") ) 
            ret |= _get_json_val((uint8 *)&area->fmt_version, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board Info Area Length") ) 
            ret |= _get_json_val((uint8 *)&area->area_len, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board Info Language Code") ) 
            ret |= _get_json_val((uint8 *)&area->language_code, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board Manufacturing Date") ) 
            ret |= _get_json_val((uint8 *)&area->Manufacturing_time, 3, sub_node);
        else if ( !strcmp(sub_node->string, "Board Manufacturer type") ) 
            ret |= _get_json_val((uint8 *)&area->manufacturer_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board Manufacturer bytes") ) 
            ret |= _get_json_val((uint8 *)&area->manufacturer_name, 16, sub_node);
        else if ( !strcmp(sub_node->string, "Board Name type") ) 
            ret |= _get_json_val((uint8 *)&area->board_name_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board Name bytes") ) 
            ret |= _get_json_val((uint8 *)&area->board_name, 16, sub_node);
        else if ( !strcmp(sub_node->string, "Board Serial Number type") ) 
            ret |= _get_json_val((uint8 *)&area->board_serial_num_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board Serial Number bytes") ) 
            ret |= _get_json_val((uint8 *)&area->board_serial_num, 20, sub_node);
        else if ( !strcmp(sub_node->string, "Board Part Number type") ) 
            ret |= _get_json_val((uint8 *)&area->board_part_num_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board Part Number bytes") ) 
            ret |= _get_json_val((uint8 *)&area->board_part_num, 20, sub_node);
        else if ( !strcmp(sub_node->string, "Board FRU File ID type") ) 
            ret |= _get_json_val((uint8 *)&area->board_fru_fileid_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board FRU File ID bytes") ) 
            ret |= _get_json_val((uint8 *)&area->board_fru_fileid, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Board Info No.1 type") ) 
            ret |= _get_json_val((uint8 *)&area->custom_board_info_n1_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Board Info No.1 bytes") ) 
            ret |= _get_json_val((uint8 *)&area->custom_board_info_n1, 8, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Board Info No.2 type") ) 
            ret |= _get_json_val((uint8 *)&area->custom_board_info_n2_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Board Info No.2 bytes") ) 
            ret |= _get_json_val((uint8 *)&area->custom_board_info_n2, 24, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Board Info No.3 type") ) 
            ret |= _get_json_val((uint8 *)&area->custom_board_info_n3_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Board Info No.3 bytes") ) 
            ret |= _get_json_val((uint8 *)&area->custom_board_info_n3, 63, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Board Info No.4 type") ) 
            ret |= _get_json_val((uint8 *)&area->custom_board_info_n4_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Board Info No.4 bytes") ) 
            ret |= _get_json_val((uint8 *)&area->custom_board_info_n4, 63, sub_node);
        else if ( !strcmp(sub_node->string, "End of Fields marker") ) 
            ret |= _get_json_val((uint8 *)&area->end_marker, 1, sub_node);
    }
    
    return ret;
}

static int drv_fru_json_parse4(FRU_PRODUCT_INFO* area, cJSON* root_tree)
{
    int ret = VOS_OK;
    int i, sub_cnt;
    
    sub_cnt = cJSON_GetArraySize(root_tree);
    for (i = 0; i < sub_cnt; ++i) {
        cJSON* sub_node = cJSON_GetArrayItem(root_tree, i); //each field

        //vos_print("%d: sub_node->string %s \r\n", __LINE__, sub_node->string);
        if ( !strcmp(sub_node->string, "Product Info Area Format Version") ) 
            ret |= _get_json_val((uint8 *)&area->fmt_version, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product Info Area Length") ) 
            ret |= _get_json_val((uint8 *)&area->area_len, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product Info Language Code") ) 
            ret |= _get_json_val((uint8 *)&area->language_code, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product Manufacturer type") ) 
            ret |= _get_json_val((uint8 *)&area->manufacturer_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Board Manufacturer bytes") ) 
            ret |= _get_json_val((uint8 *)&area->manufacturer_name, 16, sub_node);
        else if ( !strcmp(sub_node->string, "Product Name type") ) 
            ret |= _get_json_val((uint8 *)&area->product_name_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product Name bytes") ) 
            ret |= _get_json_val((uint8 *)&area->product_name, 16, sub_node);
        else if ( !strcmp(sub_node->string, "Product Part Number type") ) 
            ret |= _get_json_val((uint8 *)&area->product_part_num_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product Part Number bytes") ) 
            ret |= _get_json_val((uint8 *)&area->product_part_num, 20, sub_node);
        else if ( !strcmp(sub_node->string, "Product Version type") ) 
            ret |= _get_json_val((uint8 *)&area->product_version_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product Version bytes") ) 
            ret |= _get_json_val((uint8 *)&area->product_version, 8, sub_node);
        else if ( !strcmp(sub_node->string, "Product Serial Number type") ) 
            ret |= _get_json_val((uint8 *)&area->product_serial_num_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product Serial Number bytes") ) 
            ret |= _get_json_val((uint8 *)&area->product_serial_num, 20, sub_node);
        else if ( !strcmp(sub_node->string, "Product Asset Tag type") ) 
            ret |= _get_json_val((uint8 *)&area->product_asset_tag_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product Asset Tag bytes") ) 
            ret |= _get_json_val((uint8 *)&area->product_asset_tag, 8, sub_node);
        else if ( !strcmp(sub_node->string, "Product FRU File ID type") ) 
            ret |= _get_json_val((uint8 *)&area->product_fru_fileid_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Product FRU File ID bytes") ) 
            ret |= _get_json_val((uint8 *)&area->product_fru_fileid, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Product Info No.1 type") ) 
            ret |= _get_json_val((uint8 *)&area->custom_product_info_n1_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Product Info No.1 bytes") ) 
            ret |= _get_json_val((uint8 *)&area->custom_product_info_n1, 8, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Product Info No.2 type") ) 
            ret |= _get_json_val((uint8 *)&area->custom_product_info_n2_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Product Info No.2 bytes") ) 
            ret |= _get_json_val((uint8 *)&area->custom_product_info_n2, 24, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Product Info No.3 type") ) 
            ret |= _get_json_val((uint8 *)&area->custom_product_info_n3_type, 1, sub_node);
        else if ( !strcmp(sub_node->string, "Custom Product Info No.3 bytes") ) 
            ret |= _get_json_val((uint8 *)&area->custom_product_info_n3, 56, sub_node);
        else if ( !strcmp(sub_node->string, "End of Fields marker") ) 
            ret |= _get_json_val((uint8 *)&area->end_marker, 1, sub_node);
    }
    
    return ret;
}

int devm_fru_load_json(int fru_id, char *json_file)
{
    int ret = VOS_OK;
    char *json = NULL;
    cJSON* root_tree;
    cJSON* sub_tree;
    FRU_COMMON_HEADER   common_info;
    FRU_CHASSIS_INFO    chassis_info;
    FRU_BOARD_INFO      board_info;
    FRU_PRODUCT_INFO    product_info;

    json = read_file(json_file);
    if ((json == NULL) || (json[0] == '\0') || (json[1] == '\0')) {
        vos_print("file %s is invalid \r\n", json_file);
        return VOS_ERR;
    }
 
    root_tree = cJSON_Parse(json);
    if (root_tree == NULL) {
        vos_print("parse json file fail \r\n");
        goto FUNC_EXIT;
    }

    memset(&common_info, 0, sizeof(common_info));
    sub_tree = cJSON_GetObjectItem(root_tree, "Common Header");
    if (sub_tree == NULL) {
        vos_print("no common info \r\n");
        goto FUNC_EXIT;
    } else {
        ret = drv_fru_json_parse1(&common_info, sub_tree);
    }

    if (dbg_mode) {
        vos_print("common_info.fmt_version %d\r\n", common_info.fmt_version);
        vos_print("common_info.internal_use_start %d\r\n", common_info.internal_use_start);
        vos_print("common_info.chassis_info_start %d\r\n", common_info.chassis_info_start);
        vos_print("common_info.board_info_start %d\r\n", common_info.board_info_start);
        vos_print("common_info.product_info_start %d\r\n", common_info.product_info_start);
        vos_print("common_info.multi_record_start %d\r\n", common_info.multi_record_start);
    }

    memset(&chassis_info, 0, sizeof(chassis_info));
    sub_tree = cJSON_GetObjectItem(root_tree, "Chassis Info");
    if (sub_tree != NULL) {
        ret |= drv_fru_json_parse2(&chassis_info, sub_tree);
    }

    memset(&board_info, 0, sizeof(board_info));
    sub_tree = cJSON_GetObjectItem(root_tree, "Board Info");
    if (sub_tree != NULL) {
        ret |= drv_fru_json_parse3(&board_info, sub_tree);
    }

    memset(&product_info, 0, sizeof(product_info));
    sub_tree = cJSON_GetObjectItem(root_tree, "Product Info");
    if (sub_tree != NULL) {
        ret |= drv_fru_json_parse4(&product_info, sub_tree);
    }    

    if (ret != VOS_OK) {
        vos_print("parse json file fail \r\n");
        goto FUNC_EXIT;
    }

    if (common_info.fmt_version > 0) {
        common_info.area_checksum = devm_get_area_checksum((uint8 *)&common_info, sizeof(common_info) - 1);
        ret = devm_store_area_data(fru_id, 0, (uint8 *)&common_info, sizeof(common_info));
        vos_print("saving common info %s \r\n", (ret == VOS_OK) ? "OK":"Failed");
    }
    
    if (chassis_info.fmt_version > 0) {
        chassis_info.area_checksum = devm_get_area_checksum((uint8 *)&chassis_info, sizeof(chassis_info) - 1);
        ret = devm_store_area_data(fru_id, common_info.chassis_info_start, (uint8 *)&chassis_info, sizeof(chassis_info));
        vos_print("saving chassis info %s \r\n", (ret == VOS_OK) ? "OK":"Failed");
    }
    
    if (board_info.fmt_version > 0) {
        board_info.area_checksum = devm_get_area_checksum((uint8 *)&board_info, sizeof(board_info) - 1);
        ret = devm_store_area_data(fru_id, common_info.board_info_start, (uint8 *)&board_info, sizeof(board_info));
        vos_print("saving board info %s \r\n", (ret == VOS_OK) ? "OK":"Failed");
    }
    
    if (product_info.fmt_version > 0) {
        product_info.area_checksum = devm_get_area_checksum((uint8 *)&product_info, sizeof(product_info) - 1);
        ret = devm_store_area_data(fru_id, common_info.product_info_start, (uint8 *)&product_info, sizeof(product_info));
        vos_print("saving product info %s \r\n", (ret == VOS_OK) ? "OK":"Failed");
    }    

FUNC_EXIT:
    if (root_tree != NULL) cJSON_Delete(root_tree);
    if (json != NULL) free(json);

    return VOS_OK;
}

int fru_info_test()
{
    devm_read_fru_info(0);
    devm_read_fru_info(1);

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

int cli_fru_load_json(int argc, char **argv)
{
    int ret;
    int fru_id;
    FRU_EEPROM_INFO info;
    
    if (argc < 3) {
        vos_print("Usage: <%s> <fru-id> <jsonfile>\r\n", argv[0]);
        return VOS_OK;
    }

    fru_id = atoi(argv[1]);
    ret = drv_get_eeprom_info(fru_id, &info);
    if (ret != VOS_OK) {
        vos_print("invalid fru-id %d \r\n", fru_id);
        return VOS_ERR;
    }

    ret = devm_fru_load_json(fru_id, argv[2]);
    if (ret != VOS_OK) {
        vos_print("Failed to write fru info\r\n");
        return VOS_ERR;
    }
    
    return VOS_OK;
}  

#endif

#ifdef FRU_APP
/*
## copy file
devm_fru.c cJSON.c drv_i2c.c
devm_fru.h cJSON.h drv_i2c.h

## compile
arm-linux-gnueabihf-gcc -o fru.bin -DFRU_APP devm_fru.c cJSON.c drv_i2c.c

*/
int main(int argc, char **argv)
{
    if (argc < 2) {
        vos_print("Usage:\r\n");
        vos_print("  fru_get            -- show fru info \r\n");
        vos_print("  fru_load           -- load json fru \r\n");
        vos_print("  fru_set_mac        -- set fru mac addr \r\n");
        vos_print("  fru_set_rf_cal     -- set RF calibration param \r\n");
        vos_print("  fru_set_sku        -- set fru SKU ID \r\n");
        vos_print("  fru_set_uuid       -- set fru UUID \r\n");
        return VOS_OK;
    }

    devm_read_fru_info(0);
    devm_read_fru_info(1);

    if (!strcmp(argv[1], "fru_get")) 
        cli_show_fru_info(argc - 1, &argv[1]);
    else if (!strcmp(argv[1], "fru_load")) 
        cli_fru_load_json(argc - 1, &argv[1]);
    else if (!strcmp(argv[1], "fru_set_mac")) 
        cli_fru_set_mac(argc - 1, &argv[1]);
    else if (!strcmp(argv[1], "fru_set_rf_cal")) 
        cli_fru_set_rf_calibration(argc - 1, &argv[1]);
    else if (!strcmp(argv[1], "fru_set_sku")) 
        cli_fru_set_skuid(argc - 1, &argv[1]);
    else if (!strcmp(argv[1], "fru_set_uuid")) 
        cli_fru_set_uuid(argc - 1, &argv[1]);
    else 
        vos_print("unknown cmd \r\n");

    return VOS_OK;
}
#endif
