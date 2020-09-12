#ifndef _VOS_H_
#define _VOS_H_


#ifndef uint8
typedef unsigned char uint8;
#endif

#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef uint32
typedef unsigned int uint32;
#endif

#define VOS_OK      0
#define VOS_ERR     1

#ifndef TRUE
#define TRUE        1
#endif
#ifndef FALSE
#define FALSE       0
#endif


typedef void (* timer_callback)(union sigval);

int vos_create_timer(timer_t *ret_tid, int interval, timer_callback callback, void *param);

#endif


