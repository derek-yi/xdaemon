
#ifndef _HWMON_MSG_H_
#define _HWMON_MSG_H_

#ifndef fault_severity_t
typedef enum fault_severityT {
    NO_FAULT,
    CRITICAL = 1,
    MAJOR,
    MINOR,
    WARNING
} fault_severity_t;
#endif

typedef struct {
    char node_desc[64]; //reserved
    int  node_id;
    int  fault_state;   //fault_severity_t
}HWMON_MSG_S;

/*******************************************************************************************************
 * FAULT ID LIST
 *******************************************************************************************************/
/*
source: CPU //fault_source_t
affect: "cpu"   
text  : CRITICAL-"High cpu occupy", NO_FAULT-"null"
*/
#define NODE_ID_CPU_OCCUPY                  1101

/*
source: CPU //fault_source_t
affect: "cpu"   
text  : MINOR-"Unit temperature is high", CRITICAL-"Unit Dangerously overheating", NO_FAULT-"null"
*/
#define NODE_ID_CPU_TEMP                    1102

/*
source: CPU //fault_source_t
affect: "cpu"   
text  : CRITICAL-"High mem occupy", NO_FAULT-"null"
*/
#define NODE_ID_MEM_OCCUPY                  1103

/*
source: PA //fault_source_t
affect: "PA"   
text  : MINOR-"Unit temperature is high", CRITICAL-"Unit Dangerously overheating", NO_FAULT-"null"
*/
#define NODE_ID_BOARD_TEMP                  1110

/*
source: PA //fault_source_t
affect: "PA"
text  : CRITICAL-"PA power abnormal", NO_FAULT-"null"
*/
#define NODE_ID_POWER_CHECK                 1111

/*
source: FAN //fault_source_t
affect: "RRU" or "RHUB"
text  : CRITICAL-"Cooling fan broken", NO_FAULT-"null"
*/
#define NODE_ID_FAN_SPEED                   1112

/*
source: GPS //fault_source_t
affect: "ad9544"
text  : CRITICAL-"GPS out of lock", NO_FAULT-"null"
*/
#define NODE_ID_GNSS_LOCKED                 1113

/*
source: CPRI //fault_source_t
affect: "cpri"
text  : CRITICAL-"C/U-plane logical Connection faulty", NO_FAULT-"null"
*/
#define NODE_ID_CPRI_STATE                  1114

/*
source: AD9544 //fault_source_t
affect: "ad9544"
text  : CRITICAL-"Unexpected register value", NO_FAULT-"null"
*/
#define NODE_ID_AD9544_REG                  1120

/*
source: AD9544 //fault_source_t
affect: "ad9544"
text  : CRITICAL-"No external sync source", NO_FAULT-"null"
*/
#define NODE_ID_AD9544_PLL                  1121

/*
source: AD9528 //fault_source_t
affect: "ad9528"
text  : CRITICAL-"Unexpected register value", NO_FAULT-"null"
*/
#define NODE_ID_AD9528_REG                  1122

/*
source: AD9528 //fault_source_t
affect: "ad9528"
text  : CRITICAL-"No external sync source", NO_FAULT-"null"
*/
#define NODE_ID_AD9528_PLL                  1123

/*
source: 9FGV100 //fault_source_t
affect: "cpri"  //clk to PL eth phy
text  : CRITICAL-"Unexpected register value", NO_FAULT-"null"
*/
#define NODE_ID_9FGV100_REG                 1124

/*
source: FPGA //fault_source_t
affect: "fpga"
text  : CRITICAL-"Unexpected register value", NO_FAULT-"null"
*/
#define NODE_ID_FPGA_REG                    1130

/*
source: PLL1 //fault_source_t
affect: "ad9009-n1"
text  : CRITICAL-"Unexpected register value", NO_FAULT-"null"
*/
#define NODE_ID_AD9009A_REG                 1140

/*
source: PLL1 //fault_source_t
affect: "ad9009-n1"
text  : CRITICAL-"LO out of lock", NO_FAULT-"null"
*/
#define NODE_ID_AD9009A_PLL                 1141

/*
source: PLL2 //fault_source_t
affect: "ad9009-n2"
text  : CRITICAL-"Unexpected register value", NO_FAULT-"null"
*/
#define NODE_ID_AD9009B_REG                 1142

/*
source: PLL2 //fault_source_t
affect: "ad9009-n2"
text  : "CRITICAL-"LO out of lock", NO_FAULT-"null"
*/
#define NODE_ID_AD9009B_PLL                 1143

/*******************************************************************************************************
 * END
 *******************************************************************************************************/


#endif
