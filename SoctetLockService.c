/* 
 * File:   main.c
 * Author: 栾涛 <286976625@qq.com>
 *
 * Created on 2015年3月26日, 下午4:11
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#define CONNECT_SIGNAL "ok"
#define QUEUE_HEADER "_queue_header"
#define KEY_MAX_LEN 100
#define PORT 8888

typedef struct _queue_unit {
    int socket_num;
    char key[KEY_MAX_LEN];
    struct _queue_unit *next;
    struct _queue_unit *pre;
} queue_unit;

#define UNIT_NEW (queue_unit *)malloc(sizeof(queue_unit))

queue_unit init_queue(queue_unit *init_unit) {
    init_unit->socket_num = 0;
    strcpy(init_unit->key, QUEUE_HEADER);
    init_unit->next = NULL;
    init_unit->pre = NULL;
}

queue_unit* find_in_queue(queue_unit *unit, char *key) {
    if (unit == NULL || strcmp(unit->key, key) == 0) {
        return unit;
    } else {
        return find_in_queue(unit->next, key);
    }
}

queue_unit* add_queue(queue_unit *unit, int cfp, char *key) {
    if (unit->next == NULL) {
        unit->next = UNIT_NEW;
        unit->next->pre = unit;
        unit->next->socket_num = cfp;
        unit->next->next = NULL;
        strcpy(unit->next->key, key);
        return unit->next;
    } else {
        return add_queue(unit->next, cfp, key);
    }
}

char* del_queue(queue_unit *unit, int cfp, char *buf) {
    if (unit != NULL) {
        if (unit->socket_num == cfp) {
            strcpy(buf, unit->key);
            unit->pre->next = unit->next;
            if (unit->next != NULL) {
                unit->next->pre = unit->pre;
            }
            free(unit);
        } else {
            return del_queue(unit->next, cfp, buf);
        }
    }
}

void echo_queue(queue_unit *unit) {
    if (unit != NULL) {
        printf("%d \t %s \n", unit->socket_num, unit->key);
        echo_queue(unit->next);
    }
}

int main(int argc, char** argv) {
    queue_unit use_list, wait_list, *use_p, *wait_p;
    init_queue(&use_list);
    init_queue(&wait_list);
    int opt, sfp, cfp, len, sel, i, client_fd[FD_SETSIZE] = {0}, temp, port = PORT;
    struct sockaddr_in client_sockaddr, service_sockaddr;
    char buf[1024];
    while ((opt = getopt(argc, argv, "hp:")) != -1) {
        switch (opt) {
            case 'p':port = atoi(optarg);
                break;
            case 'h':
            default:
                printf("Welcome to use SocketLock For PHP\n"
                        "-h\t\t\t help \n"
                        "-p\t\t\t listen port\n");
        }
    }
    sfp = socket(AF_INET, SOCK_STREAM, 0);
    service_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    service_sockaddr.sin_family = AF_INET;
    service_sockaddr.sin_port = htons(port);
    if (-1 == bind(sfp, (struct sockaddr *) &service_sockaddr, sizeof (struct sockaddr))) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }
    printf("bind success!\n");
    if (-1 == listen(sfp, FD_SETSIZE)) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }
    printf("listen success!\n");
    socklen_t sl = sizeof (struct sockaddr_in);
    fd_set readset, bufset;
    FD_ZERO(&readset);
    FD_ZERO(&bufset);
    FD_SET(sfp, &readset);
    struct timeval tm;
    tm.tv_sec = 5;
    tm.tv_usec = 0;
    while (1) {
        memcpy(&bufset, &readset, sizeof (fd_set));
        sel = select(FD_SETSIZE + 1, &bufset, NULL, NULL, &tm);
        if (sel < 0) {
            perror("select error");
            exit(EXIT_FAILURE);
        }
        if (sel == 0) {
            printf("timeout!\n");
            printf("----use Queue!----\n");
            echo_queue(&use_list);
            printf("----wait Queue!----\n");
            echo_queue(&wait_list);
            continue;
        }
        if (FD_ISSET(sfp, &bufset)) {
            cfp = accept(sfp, (struct sockaddr *) &client_sockaddr, &sl);
            if (cfp == -1) {
                perror("accept error");
                exit(EXIT_FAILURE);
            }
            write(cfp, CONNECT_SIGNAL, strlen(CONNECT_SIGNAL));
            FD_SET(cfp, &readset);
            client_fd[cfp] = 1;
            printf("client success %d \n", cfp);
        }

        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &bufset)) {
                if (client_fd[i] == 1) {
                    len = read(i, buf, 1024);
                    if (len <= 0) {
                        client_fd[i] = 0;
                        FD_CLR(i, &readset);
                        close(i);
                        del_queue(&use_list, i, buf); //删除在用队列里的数据
                        printf("close: %d %s\n", i, buf);

                        use_p = find_in_queue(&wait_list, buf); //查找等待队列中是否有数据
                        if (use_p != NULL) {//如果有等待
                            temp = use_p->socket_num;
                            del_queue(&wait_list, temp, buf); //从等待队列中去除
                            printf("client: %d %s using\n", temp, buf);
                            add_queue(&use_list, temp, buf); //加入在用队列
                            write(cfp, CONNECT_SIGNAL, strlen(CONNECT_SIGNAL)); //发送通过信号
                        }
                    } else {
                        buf[len] = '\0';
                        use_p = find_in_queue(&use_list, buf);
                        if (use_p == NULL) {
                            printf("client: %d %s using\n", i, buf);
                            add_queue(&use_list, i, buf);
                            write(cfp, CONNECT_SIGNAL, strlen(CONNECT_SIGNAL)); //发送通过信号
                        } else {
                            add_queue(&wait_list, i, buf); //不许通过进入等待队列
                        }
                    }
                }

            }
        }
    }
    close(sfp);
    return (EXIT_SUCCESS);
}
