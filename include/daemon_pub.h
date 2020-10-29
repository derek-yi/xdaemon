#ifndef _DAEMON_PUB_H_
#define _DAEMON_PUB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>   //timer_t
#include <signal.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/file.h>

#include "vos.h"
#include "xlog.h"
#include "tiny_cli.h"
#include "cJSON.h"


//todo: move to makefile
#define BOARD_RRU_G3

//todo: move to makefile
//define DAEMON_RELEASE

/*************************************************************************
 * global config
 *************************************************************************/
#define DAEMON_VERSION          0x100

//#define INCLUDE_CONSOLE

#define INCLUDE_TELNETD
#define TELNETD_LISTEN_PORT     2300

/*************************************************************************
 * board config: rru g3
 *************************************************************************/
#ifdef BOARD_RRU_G3

//#define DEMO_THREAD_CODE

#define HWMON_MSG_DEMO

#define DRV_CFG_FILE            "configs/drv_cfg_rru.json"
#define HWMON_CFG_FILE          "configs/hwmon_cfg_rru.json"
#define DEVM_CFG_FILE           "configs/devm_cfg_rru.json"

#define INCLUDE_ADRV9009

#endif

/*************************************************************************
 * board config: rhub g1
 *************************************************************************/
#ifdef BOARD_RHUB_G1

//#define DEMO_THREAD_CODE

#define INCLUDE_UBLOX_GNSS


#endif


#endif //_DAEMON_PUB_H_


