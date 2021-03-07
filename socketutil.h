//
// Created by parallels on 2019/12/25.
//

#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>

#ifndef LINUX_GAME_SERVER_SOCKETUTIL_H
#define LINUX_GAME_SERVER_SOCKETUTIL_H

/**
 * 创建一个基于Ipv4协议的socket
 * */
int createTcpServer(int *socket_fd);

/**
 * 将socket绑定到指定端口号上，可以设置循环接收器。
 * */
void listenTcpServer(int *result, int socket_fd, int port, void (*on_accept)(int client_fd, struct sockaddr_in * __restrict, socklen_t * __restrict));

#endif //LINUX_GAME_SERVER_SOCKETUTIL_H
