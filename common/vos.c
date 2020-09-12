
#include "daemon_pub.h"


/*
union sigval {
    int sival_int;
    void *sival_ptr;
};
*/
int vos_create_timer(timer_t *ret_tid, int interval, timer_callback callback, void *param)
{
    int ret;
	timer_t timerid;
	struct sigevent evp;

	memset(&evp, 0, sizeof(struct sigevent));
	evp.sigev_value.sival_ptr = param; 
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = callback;
	if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1) {
		perror("fail to timer_create");
		return 1;
	}
	
	//第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长
	struct itimerspec it;
	it.it_interval.tv_sec = interval;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = interval;
	it.it_value.tv_nsec = 0;
	if (timer_settime(timerid, 0, &it, NULL) == -1) {
		perror("fail to timer_settime");
        timer_delete(timerid);
		return 1;
	}

    if (ret_tid) {
        *ret_tid = timerid;
    }

    return 0;
}

