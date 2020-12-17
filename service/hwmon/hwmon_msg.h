
#ifndef _HWMON_MSG_H_
#define _HWMON_MSG_H_

#ifndef fault_source_t
typedef enum fault_sourceT {
	CPU = 1,
	PA,
	Fan,
	GPS,
	CPRI,
	AD9544,
	AD9528,
	_9FGV100,
	FPGA,
	PLL1,
	PLL2,
	CONF_FILE,
	EEPROM	
} fault_source_t;
#endif

#ifndef fault_severity_t
typedef enum fault_severityT {
    NO_FAULT,
    CRITICAL = 1,
    MAJOR,
    MINOR,
    WARNING
} fault_severity_t;
#endif

#define DESC_MAX_LEN    64

typedef struct {
    char node_desc[DESC_MAX_LEN]; //reserved
    int  node_id;
    int  fault_state;   //fault_severity_t
    int  fault_src;     //fault_source_t
}HWMON_MSG_S;

/*******************************************************************************************************
 * FAULT ID LIST
 *******************************************************************************************************/
#define NODE_ID_HIGH_TEMP                   1       //src: CPU PA
#define NODE_ID_OVER_HEATING                2       //src: CPU PA
#define NODE_ID_FAN_BROKEN                  5       //src: Fan
#define NODE_ID_UNIT_OUT_OF_ORDER           15      //src: FPGA _9FGV100 AD9528 AD9544 PLL1 PLL2
#define NODE_ID_NO_SYNC_SRC                 17      //src: GPS
#define NODE_ID_OUT_OF_SYNC                 18      //src: AD9528 AD9544
#define NODE_ID_INPUT_PWR_FAULT             26      //src: PA
#define NODE_ID_PWR_AMP_FAULT               27      //src: 
#define NODE_ID_CU_LINK_FAULT               28      //src: CPRI

#define NODE_ID_PROCESS_OVERLOAD            1001    //src: CPU
#define NODE_ID_APP_LOAD_FAILED             1002    //src: 
#define NODE_ID_LO_UNLOCK                   1003    //src: PLL1 PLL2
#define NODE_ID_MEM_OVERLOAD                1004    //src: CPU



/*******************************************************************************************************
 * END
 *******************************************************************************************************/


#endif
