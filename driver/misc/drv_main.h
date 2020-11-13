
#ifndef _DRV_MAIN_H_
#define _DRV_MAIN_H_

#define DFAULT_VALUE    0

#define UT_CHECK_VALUE(ret, exp)  { if(ret != exp) vos_print("%d: failed, ret 0x%x, exp 0x%x \n", __LINE__, ret, exp); }

typedef struct 
{
    int     i2c_bus;
    int     dev_id;
    int     wr_blk_size;
    int     rd_blk_size;
    int     chip_size;
}FRU_EEPROM_INFO;

int drv_module_init(char *cfg_file);

int drv_module_exit();

int drv_get_board_temp(int temp_id, int * temp_val);

int drv_get_eeprom_info(int fru_id, FRU_EEPROM_INFO *info);

int clk_ad9544_reg_read(uint32 chip_id, uint32 reg_addr);

int clk_ad9544_reg_write(uint32 chip_id, uint32 reg_addr, uint32 value);

int drv_ad9544_pll_locked  (uint32 chip_id);

int clk_ad9528_reg_read(uint32 chip_id, uint32 reg_addr);

int clk_ad9528_reg_write(uint32 chip_id, uint32 reg_addr, uint32 value);

int drv_ad9528_pll_locked  (uint32 chip_id);

int clk_9FGV100X_reg_read(uint32 chip_id, uint32 reg_addr);

int adrv9009_reg_read(uint32 chip_id, uint32 reg_addr);

int adrv9009_pll_locked  (uint32 chip_id);

int drv_fan_get_speed(int fan_id, int *speed);

int drv_power_sensor_get(int chip, int type, int *value);

int drv_gnss_is_locked  ();

#endif

