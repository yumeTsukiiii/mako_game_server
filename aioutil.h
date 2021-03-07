//
// Created by parallels on 2019/12/25.
//

#include <aio.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#ifndef LINUX_GAME_SERVER_AIOUTIL_H
#define LINUX_GAME_SERVER_AIOUTIL_H


int read_async(int fd, char str[], int buf_size, int sig, struct aiocb *aio);

int read_back_thread_async(int fd, char str[], int buf_size, struct aiocb *aio, void (*callback)(sigval_t sigval));

int write_async(int fd, char *str, int sig);

#endif //LINUX_GAME_SERVER_AIOUTIL_H
