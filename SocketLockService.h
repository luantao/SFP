/* 
 * File:   SocketLockService.h
 * Author: 栾涛 <286976625@qq.com>
 *
 * Created on 2015年4月3日, 上午10:42
 */

#ifndef SOCKETLOCKSERVICE_H
#define	SOCKETLOCKSERVICE_H

#ifdef	__cplusplus
extern "C" {
#endif
#define CONNECT_SIGNAL "ok"
#define QUEUE_HEADER "_queue_header"
#define KEY_MAX_LEN 100
#define MAX_NUM FD_SETSIZE
#define PORT 8888

#define UNIT_NEW (queue_unit *)malloc(sizeof(queue_unit))

    typedef struct _queue_unit {
        int socket_num;
        char key[KEY_MAX_LEN];
        struct _queue_unit *next;
        struct _queue_unit *pre;
    } queue_unit;

    int sfp,client_fd[MAX_NUM] = {0};
#ifdef	__cplusplus
}
#endif

#endif	/* SOCKETLOCKSERVICE_H */

