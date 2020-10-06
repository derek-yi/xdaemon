
#include "daemon_pub.h"



char* read_file(const char *filename) 
{
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';


cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}

int pipe_read(char *cmd_str, char *buff, int buf_len)
{
	FILE *fp;

    if (!cmd_str) return VOS_ERR;
    if (!buff) return VOS_ERR;
    
	fp = popen(cmd_str, "r");
    if (fp == NULL) {
        fprintf(stderr, "popen failed(%s), cmd(%s)\n", strerror(errno), cmd_str);
        return VOS_ERR;
    }
    
    memset(buff, 0, buf_len);
	fgets(buff, buf_len, fp);
	pclose(fp);
    return VOS_OK;
}

int sys_node_readstr(char *node_str, char *rd_buf, int buf_len)
{
	FILE *fp;
    char cmd_buf[256];

    if (node_str == NULL) return VOS_ERR;
    if (rd_buf == NULL) return VOS_ERR;
    
    snprintf(cmd_buf, sizeof(cmd_buf), "cat %s", node_str);
	fp = popen(cmd_buf, "r");
    if (fp == NULL) {
        fprintf(stderr, "popen failed(%s), cmd(%s)\n", strerror(errno), cmd_buf);
        return VOS_ERR;
    }
    
    memset(rd_buf, 0, buf_len);
	fgets(rd_buf, buf_len, fp);
	pclose(fp);

    return VOS_OK;
}

int sys_node_read(char *node_str, int *value)
{
	FILE *fp;
    char cmd_buf[256];
    char rd_buf[32];

    if (node_str == NULL) return VOS_ERR;
    
    snprintf(cmd_buf, sizeof(cmd_buf), "cat %s", node_str);
	fp = popen(cmd_buf, "r");
    if (fp == NULL) {
        fprintf(stderr, "popen failed(%s), cmd(%s)\n", strerror(errno), cmd_buf);
        return VOS_ERR;
    }
    
	fgets(rd_buf, 30, fp);
	pclose(fp);

    if (value) {
        *value = (int)strtoul(rd_buf, 0, 0);
    }
    
    return VOS_OK;
}

int sys_node_write(char *node_str, int value)
{
    FILE *fp;
    char cmd_buf[256];

    if (node_str == NULL) return VOS_ERR;

    snprintf(cmd_buf, sizeof(cmd_buf), "echo 0x%x > %s", value, node_str);
    fp = popen(cmd_buf, "r");
    if (fp == NULL) {
        fprintf(stderr, "popen failed(%s), cmd(%s)\n", strerror(errno), cmd_buf);
        return VOS_ERR;
    }
    pclose(fp);
    
    return VOS_OK;
}


/*
union sigval {
    int sival_int;
    void *sival_ptr;
};
*/
int vos_create_timer(timer_t *ret_tid, int interval, timer_callback callback, void *param)
{
	timer_t timerid;
	struct sigevent evp;

	memset(&evp, 0, sizeof(struct sigevent));
	evp.sigev_value.sival_ptr = param; 
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = callback;
	if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1) {
        fprintf(stderr, "timer_create failed(%s)\n", strerror(errno));
		return 1;
	}
	
	//第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长
	struct itimerspec it;
	it.it_interval.tv_sec = interval;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = interval;
	it.it_value.tv_nsec = 0;
	if (timer_settime(timerid, 0, &it, NULL) == -1) {
        fprintf(stderr, "timer_settime failed(%s)\n", strerror(errno));
        timer_delete(timerid);
		return 1;
	}

    if (ret_tid) {
        *ret_tid = timerid;
    }

    return 0;
}

void vos_msleep(uint32 milliseconds) 
{
    struct timespec ts = {
        milliseconds / 1000,
        (milliseconds % 1000) * 1000000
    };
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}


//https://blog.csdn.net/Primeprime/article/details/60954203
int shell_run_cmd(char *cmd_str)
{
    int status;
    
    if (NULL == cmd_str)
    {
        return VOS_ERR;
    }
    
    status = system(cmd_str);
    if (status < 0)
    {
        fprintf(stderr, "cmd: %s, error: %s", cmd_str, strerror(errno));
        return status;
    }
     
    if (WIFEXITED(status))
    {
        //printf("normal termination, exit status = %d\n", WEXITSTATUS(status)); //取得cmdstring执行结果
        return WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        //printf("abnormal termination,signal number =%d\n", WTERMSIG(status)); //如果cmdstring被信号中断，取得信号值
        return VOS_ERR;
    }
    else if (WIFSTOPPED(status))
    {
        //printf("process stopped, signal number =%d\n", WSTOPSIG(status)); //如果cmdstring被信号暂停执行，取得信号值
        return VOS_ERR;
    }

    return VOS_OK;
}

int shell_run_file(char *file_name)
{
#if XXX // todo
    printf("current path: %s \n", getcwd(NULL, NULL));
    
    if ( access(file_name, F_OK)  < 0 ) {
        fprintf(stderr, "file %s not exist\n", file_name);
        return VOS_ERR;
    }
    
    if ( access(file_name, X_OK)  < 0 ) {
        fprintf(stderr, "file %s not executable\n", file_name);
        return VOS_ERR;
    }
#endif 

    return shell_run_cmd(file_name);
}

