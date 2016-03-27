//
// Created by badtry on 16-3-21.
//

#include <sys/wait.h>
#include "uptime.h"

void server(int sockfd)
{
    int         clfd;
    FILE        *fp;
    char        buf[BUFLEN];

//    set_cloexec(sockfd);
    for(;;)
    {
        printf("waiting client connect...\n");
        if((clfd = accept(sockfd, NULL, NULL)) < 0)
        {
            printf("accept error:%d\n", errno);
            syslog(LOG_ERR, "ruptimed: accept error: %s", strerror(errno));
            exit(1);
        }

        printf("accept fd:%d\n", clfd);
        set_cloexec(clfd);
        if ((fp = fopen("/usr/bin/uptime", "r")) == NULL)
        {
            sprintf(buf, "error: %s\n", strerror(errno));
            send(clfd, buf, strlen(buf), 0);
        }
        else
        {
          /*  int k = 1999;
            k = htonl(k);
            send(clfd, &k, sizeof(k), 0);*/

            while (fgets(buf, BUFLEN, fp) != NULL)
            {
                int k = 12;
                send(clfd, buf, strlen(buf), 0);
                pclose(fp);
            }
        }
        close(clfd);
    }
}

int run_uptime()
{
    struct addrinfo         *ailist, *aip;
    struct addrinfo         hint;
    int                     sockfd, err, n;
    char                    *host;

    /*if (argc != 1)
        err_quit("usage: ruptimed");*/

    if((n = sysconf(_SC_HOST_NAME_MAX)) < 0)
        n = HOST_NAME_MAX;

    if ((host = malloc(n)) == NULL)
        err_sys("malloc error");

    if(gethostname(host, n) < 0)
        err_sys("gethostname error");

//    daemonize("ruptimed");
    memset(&hint, 0, sizeof(hint));
    hint.ai_flags = AI_CANONNAME;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;

    if((err = getaddrinfo("127.0.0.1", "ruptime", &hint, &ailist))!=0)
    {
        syslog(LOG_ERR, "ruptimed: getaddrinfo error: %s", gai_strerror(err));
        exit(1);
    }

    for (aip = ailist; aip != NULL; aip = aip->ai_next)
    {
//        printf("%s\n", aip->ai_addr->sa_data);
//        int j = 0;
//        for(j = 0; j < 14; j++)
//            printf(" %d", aip->ai_addr->sa_data[j]);
//        printf("\n");

        if((sockfd = initserver(SOCK_STREAM, aip->ai_addr, aip->ai_addrlen, QLEN)) >= 0)
        {
            server(sockfd);
            exit(0);
        }
    }
    exit(1);
}

void server2(int sockfd) {
    int         clfd, status;
    pid_t       pid;

    set_cloexec(sockfd);

    for(;;){
        if ((clfd = accept(sockfd, NULL, NULL)) < 0){
            syslog(LOG_ERR, "ruptimed: accept error: %s",strerror(errno));
            exit(1);
        }
        if ((pid = fork()) < 0){
            syslog(LOG_ERR, "ruptimed: fork error: %s", strerror(errno));
        }else if(pid == 0){
            /*
             * The parent called daemmonize (Figure 13.1), so
             * STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO
             * are already open to /dev/null. Thus, the call to
             * close doesn't need to be protected by checks that
             * clfd isn't already equal to one of these values.
             */
            if (dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO ||
                    dup2(clfd, STDERR_FILENO) != STDERR_FILENO){
                syslog(LOG_ERR, "ruptimed: unexpected error");
                exit(1);
            }
            close(clfd);
            execl("/usr/bin/uptime", "uptime", (char*)0);
            syslog(LOG_ERR, "ruptimed: unexpected return from exec: %s", strerror(errno));
        }else { /* parent */
            close(clfd);
            waitpid(pid, &status, 0);
        }
    }
}

int run_uptime2(){
    struct addrinfo     *ailist, *aip;
    struct addrinfo     hint;
    int                 sockfd, err, n;
    char*               host;

    if((n = sysconf(_SC_HOST_NAME_MAX)) < 0)
        n = HOST_NAME_MAX;
    if((host = malloc(n)) < 0)
        err_sys("malloc error");
    if(gethostname(host, n) < 0)
        err_sys("gethostname error");
    daemonize("ruptimed");
    memset(&hint, 0, sizeof(hint));
    hint.ai_flags = AI_CANONNAME;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    if ((err = getaddrinfo(/*host*/"localhost", "ruptime", &hint, &ailist)) != 0){
        syslog(LOG_ERR, "ruptimed: getaddrinfo error: %s", gai_strerror(err));
        exit(1);
    }
    for (aip = ailist; aip != NULL; aip = aip->ai_next){
        if ((sockfd = initserver(SOCK_STREAM, aip->ai_addr,aip->ai_addrlen, QLEN)) >= 0){
            server2(sockfd);
            exit(0);
        }
    }
    exit(10);
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void server3(int sockfd)
{
    int                     n;
    socklen_t               alen;
    FILE                    *fp;
    char                    buf[BUFLEN];
    char                    abuf[MAXADDRLEN];
    struct sockaddr         *addr = (struct socckaddr *)abuf;

    set_cloexec(sockfd);

    for (;;){
        alen = MAXADDRLEN;
        if((n = recvfrom(sockfd, buf, BUFLEN, 0, addr, &alen)) < 0){
            syslog(LOG_ERR, "ruptimed: recvfrom error: %s", strerror(errno));
            exit(1);
        }
        if((fp = popen("/usr/bin/uptime", "r")) == NULL){
            sprintf(buf, "error: %s\n", strerror(errno));
            sendto(sockfd, buf, strlen(buf), 0, addr, alen);
        } else {
            if(fgets(buf, BUFLEN, fp) != NULL)
                sendto(sockfd, buf, strlen(buf), 0, addr, alen);
            pclose(fp);
        }
    }
}

int run_uptime3()
{
    struct addrinfo             *ailist, *aip;
    struct addrinfo             hint;
    int                         sockfd, err, n;
    char                        *host;

    if((n = sysconf(_SC_HOST_NAME_MAX)) < 0)
        n = HOST_NAME_MAX; /* best guess */
    if((host = malloc(n)) == NULL)
        err_sys("malloc error");
    if (gethostname(host, n) < 0)
        err_sys("gethostname error");
//    daemonize("ruptimed");
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags = AI_CANONNAME;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    if((err = getaddrinfo("localhost", "ruptime", &hint, &ailist)) != 0){
        syslog(LOG_ERR, "ruptimed: getaddrinfo error: %s", gai_strerror(err));
        exit(1);
    }
    for (aip = ailist; aip != NULL; aip = aip->ai_next){
        if((sockfd = initserver(SOCK_DGRAM, aip->ai_addr, aip->ai_addrlen, 0)) >= 0){
            server3(sockfd);
            exit(0);
        }
    }
    exit(1);
}























