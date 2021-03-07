//
// Created by parallels on 2019/12/25.
//
#include <aio.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#ifndef LINUX_GAME_SERVER_SIGNALUTIL_H
#define LINUX_GAME_SERVER_SIGNALUTIL_H

void register_signal_info(int sig, void (*action)(int sig, siginfo_t *siginfo, void *r));

void register_signal_info_with_flag(int sig, void (*action)(int sig, siginfo_t *siginfo, void *r), int flag);

#endif //LINUX_GAME_SERVER_SIGNALUTIL_H
