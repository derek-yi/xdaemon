

#include "daemon_pub.h"

#include "drv_cpu.h"

#if T_DESC("devmem", 1)

#define MAP_SIZE        4096UL
#define MAP_MASK        (MAP_SIZE - 1)

int memdev_fd = -1;

int devmem_addr_valid(unsigned long mem_addr)
{
    return TRUE;
}

/*************************************************************************
 * 物理地址读函数
 * mem_addr     - io设备地址
 * access_type  - 读取数据长度类型
 * return       - 读取的值
 *************************************************************************/
unsigned long devmem_read(unsigned long mem_addr, int access_type) 
{
    void *map_base, *virt_addr;
    unsigned long read_result;
    off_t target = (off_t)mem_addr;

    if (!devmem_addr_valid(mem_addr)) {
        xlog(XLOG_ERROR, "Error at %s:%d, invalid addr 0x%lx", __FILE__, __LINE__, mem_addr);
        return 0;
    }

    if (memdev_fd < 0) {
        if ( (memdev_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1 ) {
            xlog(XLOG_ERROR, "Error at %s:%d, open failed(%s)", __FILE__, __LINE__, strerror(errno));
            return 0;
        }
    }

    /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memdev_fd, target & ~MAP_MASK);
    if (map_base == (void *) -1) {
        xlog(XLOG_ERROR, "Error at %s:%d, mmap failed(%s)", __FILE__, __LINE__, strerror(errno));
        return 0;
    }
    
    virt_addr = map_base + (target & MAP_MASK);
    switch(access_type) {
        case AT_BYTE:
            read_result = *((unsigned char *) virt_addr);
            break;
        case AT_SHORT:
            read_result = *((unsigned short *) virt_addr);
            break;
        case AT_WORD:
            read_result = *((unsigned long *) virt_addr);
            break;
        default:
            xlog(XLOG_ERROR, "Error at %s:%d, access type %d", __FILE__, __LINE__, access_type);
            return 0; 
    }

    if (munmap(map_base, MAP_SIZE) == -1) {
        xlog(XLOG_ERROR, "Error at %s:%d, munmap failed(%s)", __FILE__, __LINE__, strerror(errno));
    }

    return read_result;
}

/*************************************************************************
 * 物理地址写函数
 * mem_addr     - io设备地址
 * access_type  - 写入数据长度类型
 * writeval     - 写入的值
 *************************************************************************/
unsigned long devmem_write(unsigned long mem_addr, int access_type, unsigned long writeval) 
{
    void *map_base, *virt_addr;
    //unsigned long read_result;
    off_t target = (off_t)mem_addr;

    if (memdev_fd < 0) {
        if ( (memdev_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1 )  {
            xlog(XLOG_ERROR, "Error at %s:%d, open failed(%s)", __FILE__, __LINE__, strerror(errno));
            return 0;
        }
    }

    /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memdev_fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
        xlog(XLOG_ERROR, "Error at %s:%d, mmap failed(%s)", __FILE__, __LINE__, strerror(errno));
        return 0;
    }
    
    virt_addr = map_base + (target & MAP_MASK);
    switch (access_type) {
        case AT_BYTE:
            *((unsigned char *) virt_addr) = writeval;
            //read_result = *((unsigned char *) virt_addr);
            break;
        case AT_SHORT:
            *((unsigned short *) virt_addr) = writeval;
            //read_result = *((unsigned short *) virt_addr);
            break;
        case AT_WORD:
            *((unsigned long *) virt_addr) = writeval;
            //read_result = *((unsigned long *) virt_addr);
            break;
    }
    //printf("Written 0x%lu; readback 0x%lu\n", writeval, read_result);

    if (munmap(map_base, MAP_SIZE) == -1)  {
        xlog(XLOG_ERROR, "Error at %s:%d, munmap failed(%s)", __FILE__, __LINE__, strerror(errno));
    }

    return 0;
}


#endif


#if T_DESC("cpu_temp", 1)

/*
function CPU_TEMPERATURE()
{
    tmp=`devmem 0x43ca0200`
    tmp=$(($tmp >> 4))
    tmp=`echo "scale=0; ($tmp*503.975/4096 - 273.15)/1" | bc`
    echo $tmp > /tmp/cpu_temperature.txt
}
*/
/*************************************************************************
 * 获取cpu温度
 *************************************************************************/
int drv_get_cpu_temp(int *temp)
{
    int cpu_temp;
    float float_tmp;

    if (temp == NULL) return VOS_ERR;
    
    cpu_temp = devmem_read(0x43ca0200, 4);
    float_tmp = (float)(cpu_temp >> 4);
    float_tmp = (float_tmp*503.975/4096 - 273.15);
    *temp = (int)float_tmp;
    
    return VOS_OK;
}

#endif


#if T_DESC("cpu_occupy", 1)

/* cpu_info_t结构体存放cpu相关信息 */
typedef struct _cpu_info
{
    unsigned int valid;
    char name[32];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
}cpu_info_t;

/* 从/proc/stat文件中获取cpu的相关信息 */
static void get_cpu_occupy(cpu_info_t* info)
{
    FILE* fp = NULL;
    char buf[256] = {0};

    fp = fopen("/proc/stat", "r");
    if (fp == NULL) return ;
    fgets(buf, sizeof(buf), fp);

    sscanf(buf, "%s %u %u %u %u %u %u %u", info->name, &info->user, &info->nice, 
        &info->system, &info->idle, &info->iowait, &info->irq, &info->softirq);

    fclose(fp);
    info->valid = TRUE;
}

/* 计算cpu的使用率 */
static double calc_cpu_rate(cpu_info_t* old_info, cpu_info_t* new_info)
{
    double od, nd;
    //double usr_dif, sys_dif, nice_dif;
    double user_cpu_rate;
    double kernel_cpu_rate;

    od = (double)(old_info->user + old_info->nice + old_info->system + old_info->idle + old_info->iowait + old_info->irq + old_info->softirq);
    nd = (double)(new_info->user + new_info->nice + new_info->system + new_info->idle + new_info->iowait + new_info->irq + new_info->softirq);

    if (nd - od) {
        user_cpu_rate = (new_info->user - old_info->user) / (nd - od) * 100;
        kernel_cpu_rate = (new_info->system - old_info->system) / (nd - od) * 100;

        return user_cpu_rate + kernel_cpu_rate;
    } else {
        //翻转的时候返回0
    }
    
    return 0;
}

cpu_info_t last_record = {0};

/*************************************************************************
 * 获取 cpu 占有率
 *************************************************************************/
int drv_get_cpu_usage(int *usage)
{
    cpu_info_t new_record;

    if (!usage) return VOS_ERR;
    
    get_cpu_occupy(&new_record);
    if (last_record.valid) {
        *usage = (int)calc_cpu_rate(&last_record, &new_record);   
    } else {
        *usage = 0;
    }
    
    memcpy(&last_record, &new_record, sizeof(cpu_info_t));
    return VOS_OK;
}

#endif

#if T_DESC("mem_occupy", 1)


typedef struct _mem_info_t
{
    char name[32];
    unsigned long total;
    char name2[32];
}mem_info_t;

/*************************************************************************
 * 获取 内存 占有率
 *************************************************************************/
int drv_get_mem_usage(int *usage)
{
    FILE* fp = NULL;
    char buf[256] = {0};
    mem_info_t info;
    double mem_total, mem_used_rate;

    if (!usage) return VOS_ERR;

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) return VOS_ERR;
    
    fgets(buf, sizeof(buf), fp);
    sscanf(buf, "%s %lu %s\n", &info.name[0], &info.total, &info.name2[0]);
    mem_total = info.total;
    fgets(buf, sizeof(buf), fp);
    sscanf(buf, "%s %lu %s\n", &info.name[0], &info.total, &info.name2[0]);
    mem_used_rate = (1 - info.total / mem_total) * 100;
    mem_total = mem_total / (1024 * 1024); //KB -> GB
    fclose(fp);

    //xlog(XLOG_INFO, "mem total: %.0lfG, mem usage: %.2lf\n", mem_total, mem_used_rate);
    *usage = (int)mem_used_rate;

    return VOS_OK;
}

#endif

