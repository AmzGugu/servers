//
// Created by badtry on 16-3-21.
//

#ifndef SERVER_UPTIME_H
#define SERVER_UPTIME_H

#include "../common/apue.h"
#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>

#define BUFLEN      128
#define MAXADDRLEN  256
#define QLEN        10

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

extern int initserver(int, const struct sockaddr*, socklen_t, int);

void server(int sockfd);

int run_uptime();

/*
 * 另一个面向连接的服务器
 */
void server2(int sockfd);

int run_uptime2();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
 * uptime服务器数据报版
 */

void server3(int sockfd);

int run_uptime3();

#endif //SERVER_UPTIME_H
