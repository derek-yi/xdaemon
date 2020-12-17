
#ifndef _DRV_FPGA_H_
#define _DRV_FPGA_H_


/*******************************************************************************************************
 * ALL BOARD
 *******************************************************************************************************/
#define FPGA_VER_ADDRESS                    0x43c30000

#define BOARD_TYPE_NONE                     0
#define BOARD_TYPE_RRU                      1
#define BOARD_TYPE_RHUB                     2

/*******************************************************************************************************
 * BOARD_RRU_G3
 *******************************************************************************************************/
#ifdef BOARD_RRU_G3

#define CPRI_REG_BASE                       0x43C40000

#define MAX_CPRI_CNT                        1

#endif

/*******************************************************************************************************
 * BOARD_RHUB_G1
 *******************************************************************************************************/
#ifdef BOARD_RHUB_G1

#define CPRI_REG_BASE                       0x80000000

#define MAX_CPRI_CNT                        8

#endif

uint32 fpga_read(uint32 addr);

uint32 fpga_write(uint32 addr, uint32 value);

int drv_get_cpri_links(int *link_cnt);

int drv_board_type(void);

uint32 xlnx_smi_r(uint32 mac_base, uint32 phy_addr, uint32 reg_addr);

int xlnx_smi_w(uint32 mac_base, uint32 phy_addr, uint32 reg_addr, uint32 wr_val);

int cpri_link_monitor(void *param);


#endif


