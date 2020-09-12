#ifndef _DAEMON_PUB_H_
#define _DAEMON_PUB_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>   //timer_t
#include <signal.h>

#include "vos.h"
#include "xlog.h"
#include "tiny_cli.h"



//todo: move to makefile
#define BOARD_RRU_G3
//#define BOARD_RHUB_G1



/*************************************************************************
 * board config: rru g3
 *************************************************************************/
#ifdef BOARD_RRU_G3

#define DEMO_CODE

#define JSON_TEST_CODE

#endif

/*************************************************************************
 * board config: rhub g1
 *************************************************************************/
#ifdef BOARD_RHUB_G1

//#define DEMO_CODE

#endif


#endif //_DAEMON_PUB_H_


