
#include "daemon_pub.h"
#include "drv_cpu.h"
#include "drv_fpga.h"


uint32 fpga_read(uint32 addr)
{
    return (uint32)devmem_read((unsigned long)addr, AT_WORD);
}

uint32 fpga_write(uint32 addr, uint32 value)
{
    return (uint32)devmem_write((unsigned long)addr, AT_WORD, (unsigned long)value);
}

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
        uint32 value = fpga_read(CPRI_REG_BASE + i*0x10000);
        if (value & 0xF) cnt++;
    }

    *link_cnt = cnt;
    return VOS_OK;
}

//getversion
int drv_board_type(void)
{
    uint32 value = fpga_read(FPGA_VER_ADDRESS);
    if ( (value & 0x01000000) == 0x01000000)
        return BOARD_TYPE_RHUB;
    return BOARD_TYPE_RRU;
}

#ifdef BOARD_RHUB_G1

#define SMI_REG_MOD(base)   ( (base) + 0x500 )
#define SMI_REG_CSR(base)   ( (base) + 0x504 )
#define SMI_REG_TXD(base)   ( (base) + 0x508 )
#define SMI_REG_RXD(base)   ( (base) + 0x50c )

// refer from /root/fw/xge-xlnx-smi.sh
uint32 xlnx_smi_r(uint32 mac_base, uint32 phy_addr, uint32 reg_addr) 
{
	uint32 prt = 0;
    uint32 value;

	//# 0> configure the mode of the SMI controller
	//devmem $SMI_REG_MOD 32 0x68
	fpga_write(SMI_REG_MOD(mac_base), 0x68);

	//# 1> issue the address transaction, then poll CSR.bit7 until TRUE
	//devmem $SMI_REG_TXD 32 $2
	fpga_write(SMI_REG_TXD(mac_base), reg_addr);
	
	//cmd=$(( ($prt << 24) | ($1 << 16) | (0 << 14) | (1 << 11) ))
	//#printf "cmd-a is 0x%08x\n" $cmd
    uint32 cmd = (prt << 24) | (phy_addr < 16) | (0 << 14) | (1 << 11);
    
	//devmem $SMI_REG_CSR 32 $cmd
	fpga_write(SMI_REG_CSR(mac_base), cmd);
	
	//bits_set_poll $SMI_REG_CSR 0x80 "issue ADDR-transaction"
	//[ $? -eq 0 ] || exit 1
	vos_msleep(100);
    value = fpga_read(SMI_REG_CSR(mac_base)); 
    if (!(value & 0x80)) {
        return 0x0; //error
    }

	//# 2> issue the read transaction, then poll CSR.bit7 until TRUE
	//cmd=$(( $cmd | (3 << 14) ))
	//#printf "cmd-r is 0x%08x\n" $cmd
	cmd = cmd | (3 << 14);
    
	//devmem $SMI_REG_CSR 32 $cmd
	fpga_write(SMI_REG_CSR(mac_base), cmd);

	//bits_set_poll $SMI_REG_CSR 0x80 "issue READ-transaction"
	//[ $? -eq 0 ] || exit 2
	vos_msleep(100);
    value = fpga_read(SMI_REG_CSR(mac_base)); 
    if (!(value & 0x80)) {
        return 0x0; //error
    }

	//# 3> get the value latched on the smi bus
	//devmem $SMI_REG_RXD 16 # 32->16
	value = fpga_read(SMI_REG_RXD(mac_base)); 
    value = value & 0xFFFF;
    
	//exit 0
	return value;
}

int xlnx_smi_w(uint32 mac_base, uint32 phy_addr, uint32 reg_addr, uint32 wr_val) 
{
	uint32 prt = 0;
    uint32 value;

	//# 0> configure the mode of the SMI controller
	//devmem $SMI_REG_MOD 32 0x68
	fpga_write(SMI_REG_MOD(mac_base), 0x68);

	//# 1> issue the address transaction, then poll CSR.bit7 until TRUE
	//devmem $SMI_REG_TXD 32 $2
	fpga_write(SMI_REG_TXD(mac_base), reg_addr);
	
	//cmd=$(( ($prt << 24) | ($1 << 16) | (0 << 14) | (1 << 11) ))
	//#printf "cmd-a is 0x%08x\n" $cmd
    uint32 cmd = (prt << 24) | (phy_addr < 16) | (0 << 14) | (1 << 11);

	//devmem $SMI_REG_CSR 32 $cmd
    fpga_write(SMI_REG_CSR(mac_base), cmd);
    
	//bits_set_poll $SMI_REG_CSR 0x80 "issue ADDR-transaction"
	//[ $? -eq 0 ] || exit 1
	vos_msleep(100);
    value = fpga_read(SMI_REG_CSR(mac_base)); 
    if (!(value & 0x80)) {
        return -1; //error
    }

	//# 2> issue the write transaction, then poll CSR.bit7 until TRUE
	//devmem $SMI_REG_RXD 32 $3
    fpga_write(SMI_REG_RXD(mac_base), wr_val);
	
	//cmd=$(( $cmd | (1 << 14) ))
	//#printf "cmd-w is 0x%08x\n" $cmd
	cmd = cmd | (1 << 14);
    
	//devmem $SMI_REG_CSR 32 $cmd
	fpga_write(SMI_REG_CSR(mac_base), cmd);
	
	//bits_set_poll $SMI_REG_CSR 0x80 "issue WRIT-transaction"
	//[ $? -eq 0 ] || exit 2
	vos_msleep(100);
    value = fpga_read(SMI_REG_CSR(mac_base)); 
    if (!(value & 0x80)) {
        return -1; //error
    }

	//exit 0
	return 0;
}

#endif
