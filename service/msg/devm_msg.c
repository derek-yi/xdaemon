
#include "daemon_pub.h"

#include "devm_msg.h"


#define MAX_CONNECT_NUM 16

msg_func msg_func_list[CMD_TYPE_MAX] = {0};

typedef struct {
    int  sock_id;
    char *app_id;
}SOCK_INFO;

SOCK_INFO sock_list[MAX_CONNECT_NUM] = {0};


int devm_set_msg_func(int cmd_type, msg_func func)
{
    if (cmd_type >= CMD_TYPE_MAX) return VOS_ERR;

    msg_func_list[cmd_type] = func;
    
    return VOS_OK;
}

void* devm_msg_rx_task(void *param)  
{
    long int temp_val = (long int)param; //suppress warning
    int sockt_id = (int)temp_val;
    
    while(1) {
        DEVM_MSG_S rx_msg;
        DEVM_MSG_S tx_msg;
        
        int ret = recv(sockt_id, &rx_msg, sizeof(DEVM_MSG_S), 0);
        if (ret < MSG_HEAD_LEN) {
            xlog(XLOG_ERROR, "Error at %s:%d, recv failed(%s)", __FILE__, __LINE__, strerror(errno));
            break;
        }
        
        xlog(XLOG_INFO, "%s:%d: new msg %d", __FILE__, __LINE__, rx_msg.cmd_type);
        if ( (rx_msg.cmd_type < CMD_TYPE_MAX) && (msg_func_list[rx_msg.cmd_type] != NULL) ) {
            memset(&tx_msg, 0, sizeof(DEVM_MSG_S));
            msg_func_list[rx_msg.cmd_type](&rx_msg, &tx_msg);

            if (rx_msg.need_ack) {
                ret = send(sockt_id, &tx_msg, tx_msg.payload_len + MSG_HEAD_LEN, 0);
                if (ret < MSG_HEAD_LEN)  {
                    xlog(XLOG_ERROR, "Error at %s:%d, send failed(%s)", __FILE__, __LINE__, strerror(errno));
                    break;
                }
            }
        }
    }
    
    close(sockt_id);
    return NULL;
}

void* devm_msg_listen_task(void *param)  
{
    int fd,new_fd,ret;
    struct sockaddr_un un;
    
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        xlog(XLOG_ERROR, "Error at %s:%d, socket failed(%s)", __FILE__, __LINE__, strerror(errno));
        return NULL;
    }
    
    un.sun_family = AF_UNIX;
    unlink(APP_ORAN_DAEMON);
    strcpy(un.sun_path, APP_ORAN_DAEMON);

    if (bind(fd, (struct sockaddr *)&un, sizeof(un)) <0 ) {
        xlog(XLOG_ERROR, "Error at %s:%d, bind failed(%s)", __FILE__, __LINE__, strerror(errno));
        return NULL;
    }
    
    if (listen(fd, MAX_CONNECT_NUM) < 0) {
        xlog(XLOG_ERROR, "Error at %s:%d, listen failed(%s)", __FILE__, __LINE__, strerror(errno));
        return NULL;
    }
    
    while(1){
        pthread_t unused_tid;
        long int temp_val;
        
        new_fd = accept(fd, NULL, NULL);
        if (new_fd < 0) {
            xlog(XLOG_ERROR, "Error at %s:%d, accept failed(%s)", __FILE__, __LINE__, strerror(errno));
            continue;
        }

        xlog(XLOG_INFO, "%s:%d: new_fd %d", __FILE__, __LINE__, new_fd);
        temp_val = new_fd;
        ret = pthread_create(&unused_tid, NULL, devm_msg_rx_task, (void *)temp_val);  
        if (ret != 0)  {  
            xlog(XLOG_ERROR, "Error at %s:%d, pthread_create failed(%s)", __FILE__, __LINE__, strerror(errno));
            close(new_fd);
            continue;
        } 
    }
    
    close(fd);
    return NULL;
}

int devm_rebuild_socket(char *app_id)
{
    int i;

    for (i = 0; i < MAX_CONNECT_NUM; i++) {
        if (!strcmp(app_id, sock_list[i].app_id)) {
            break;
        }
    }

    if (i == MAX_CONNECT_NUM) { //not found
        xlog(XLOG_ERROR, "Error at %s:%d", __FILE__, __LINE__);
        return VOS_ERR;
    }
    
    struct sockaddr_un un;
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, app_id);
    int socket_id = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_id < 0) {
        xlog(XLOG_ERROR, "Error at %s:%d, socket failed(%s)", __FILE__, __LINE__, strerror(errno));
        return VOS_ERR;
    }

    if (connect(socket_id, (struct sockaddr *)&un, sizeof(un)) < 0) {
        xlog(XLOG_ERROR, "Error at %s:%d, connect failed(%s)", __FILE__, __LINE__, strerror(errno));
        close(socket_id);
        return VOS_ERR;
    }

    close(sock_list[i].sock_id); //close old fd
    sock_list[i].sock_id = socket_id;
    return VOS_OK;
}


int devm_get_socket(char *app_id)
{
    int i, j = -1;

    for (i = 0; i < MAX_CONNECT_NUM; i++) {
        if (sock_list[i].app_id == NULL) {
            if (j < 0) j = i;
        }
        else if (!strcmp(app_id, sock_list[i].app_id)) {
            return sock_list[i].sock_id;
        }
    }

    if (j < 0) { //full
        xlog(XLOG_ERROR, "Error at %s:%d", __FILE__, __LINE__);
        return VOS_ERR;
    }
    
    struct sockaddr_un un;
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, app_id);
    int socket_id = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_id < 0) {
        xlog(XLOG_ERROR, "Error at %s:%d, socket failed(%s)", __FILE__, __LINE__, strerror(errno));
        return VOS_ERR;
    }

    if (connect(socket_id, (struct sockaddr *)&un, sizeof(un)) < 0) {
        xlog(XLOG_ERROR, "Error at %s:%d, connect failed(%s)", __FILE__, __LINE__, strerror(errno));
        close(socket_id);
        return VOS_ERR;
    }

    sock_list[j].sock_id = socket_id;
    sock_list[j].app_id = strdup(app_id);
    vos_msleep(300); //wait server gets ready
    return socket_id;
}

int devm_send_msg(char *app_id, DEVM_MSG_S *tx_msg, DEVM_MSG_S *rx_msg)
{
    static int serial_num = 0;

    if (tx_msg == NULL) {
        xlog(XLOG_ERROR, "Error at %s:%d", __FILE__, __LINE__);
        return VOS_ERR;
    }

    int tx_socket = devm_get_socket(app_id);
    if (tx_socket < 0) {
        xlog(XLOG_ERROR, "Error at %s:%d, socket failed(%s)", __FILE__, __LINE__, strerror(errno));
        return VOS_ERR;
    }

    tx_msg->serial_num = serial_num++;
    if ( send(tx_socket, tx_msg, tx_msg->payload_len + MSG_HEAD_LEN, 0) < MSG_HEAD_LEN ) {
        xlog(XLOG_ERROR, "Error at %s:%d, send failed(%s)", __FILE__, __LINE__, strerror(errno));
        devm_rebuild_socket(app_id);
        return VOS_ERR;
    }

    if (tx_msg->need_ack && rx_msg) {
        int ret = recv(tx_socket, rx_msg, sizeof(DEVM_MSG_S), 0);
        if ( (ret < MSG_HEAD_LEN) || (rx_msg->serial_num != tx_msg->serial_num) ) {
            xlog(XLOG_ERROR, "Error at %s:%d, recv failed %d", __FILE__, __LINE__, ret);
            return VOS_ERR;
        }
    }

    return VOS_OK;
}

int devm_msg_init(char *cfg_file)
{
    int ret;
    pthread_t unused_tid;
    
    ret = pthread_create(&unused_tid, NULL, devm_msg_listen_task, NULL);  
    if (ret != 0)  {  
        xlog(XLOG_ERROR, "Error at %s:%d, pthread_create failed(%s)", __FILE__, __LINE__, strerror(errno));
        return VOS_ERR;  
    } 

    return VOS_OK;
}

