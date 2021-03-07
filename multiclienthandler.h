////
//// Created by parallels on 2019/12/25.
////
//
#ifndef LINUX_GAME_SERVER_MULTICLIENTHANDLER_H
#define LINUX_GAME_SERVER_MULTICLIENTHANDLER_H

#include <aio.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include "aioutil.h"
#include "socketutil.h"
#include "signalutil.h"

int create_multi_client_handler(void (*on_read)(int sig, siginfo_t *siginfo, void *r));

void create_client_fd_sender_pipe();

void dispatch_client(int pid, int client_fd);

#endif //LINUX_GAME_SERVER_MULTICLIENTHANDLER_H
