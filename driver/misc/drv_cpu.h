

#ifndef _DEV_MEM_H_
#define _DEV_MEM_H_



#define AT_BYTE     1
#define AT_SHORT    2
#define AT_WORD     4


unsigned long devmem_read(unsigned long mem_addr, int access_type);

unsigned long devmem_write(unsigned long mem_addr, int access_type, unsigned long writeval);

int drv_get_cpu_temp(int *temp);

int drv_get_cpu_usage(int *usage);

int drv_get_mem_usage(int *usage);

#endif

