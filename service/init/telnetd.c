

#include "daemon_pub.h"

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <arpa/inet.h> 



void* telnet_listen_task(void *param)  
{
	struct sockaddr_in sa;
	int master_fd;
	int on = 1;

	master_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (master_fd < 0) {
		perror("socket");
		return NULL;
	}
	(void)setsockopt(master_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	/* Set it to listen to specified port */
	memset((void *)&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(TELNETD_LISTEN_PORT);

	/* Set it to listen on the specified interface */
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(master_fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("bind");
		return NULL;
	}

	if (listen(master_fd, 1) < 0) {
		perror("listen");
		return NULL;
	}

    //if (daemon(0, 1) < 0) perror("daemon");
    while (1) {
        int fd;
        socklen_t salen;

        salen = sizeof(sa); 
        if ((fd = accept(master_fd, (struct sockaddr *)&sa, &salen)) < 0) {
            perror("accept");
            continue;
        } else {
            printf("Server: connect from host %s, port %d.\n", 
                    inet_ntoa (sa.sin_addr), ntohs (sa.sin_port));        
            cli_telnet_task(fd);
            close(fd);
        }
    }

    return NULL;
}

int telnet_task_init(void)
{
    int ret;
    pthread_t unused_tid;

    ret = pthread_create(&unused_tid, NULL, telnet_listen_task, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "Error at %s:%d, pthread_create failed(%s)", __FILE__, __LINE__, strerror(errno));
        return VOS_ERR;  
    } 
    return VOS_OK;
}

