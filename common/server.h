//
// Created by badtry on 16-3-20.
//

#ifndef SECONDPRJ_SERVER_H
#define SECONDPRJ_SERVER_H

#include "../common/apue.h"
#include <errno.h>
#include <sys/socket.h>

int initserver(int type, const struct sockaddr *addr, socklen_t alen, int qlen);

#endif //SECONDPRJ_SERVER_H
