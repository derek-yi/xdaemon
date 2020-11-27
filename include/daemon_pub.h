#ifndef _DAEMON_PUB_H_
#define _DAEMON_PUB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>   //timer_t
#include <signal.h>
#include <sys/time.h>

#include "vos.h"
#include "xlog.h"
#include "tiny_cli.h"
#include "cJSON.h"


//todo: move to makefile
#define BOARD_RRU_G3
//#define BOARD_RHUB_G1

//todo: move to makefile
//define DAEMON_RELEASE

typedef struct _DYN_CFG{
    struct _DYN_CFG *next;
    char    *cfg_str;
    char    *cfg_val;
}DYN_CFG_S;

typedef struct _SYS_CONF_PARAM
{
//"fix.config"
    char    *drv_conf;
    char    *hwmon_conf;
    char    *devm_conf;
    char    *upcfg_conf;
    int     MAX_FRUID;
    int     MAX_RF_CHIP;
    char    *uds_file;
    char    *xlog_path;
    char    *customer_name;
    int     customer_id;

//"dyn.config"
    DYN_CFG_S *dyn_cfg;
}SYS_CONF_PARAM;

extern SYS_CONF_PARAM sys_conf;

/*************************************************************************
 * global config
 *************************************************************************/
#define DAEMON_VERSION          0x100

//#define INCLUDE_CONSOLE
#define INCLUDE_TELNETD

/*************************************************************************
 * board config: rru g3
 *************************************************************************/
#ifdef BOARD_RRU_G3


#define INCLUDE_ADRV9009

#endif

/*************************************************************************
 * board config: rhub g1
 *************************************************************************/
#ifdef BOARD_RHUB_G1


#define INCLUDE_UBLOX_GNSS


#endif


#endif //_DAEMON_PUB_H_


