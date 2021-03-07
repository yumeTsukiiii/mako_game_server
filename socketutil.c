//
// Created by parallels on 2019/12/25.
//

#include "socketutil.h"

/**
 * 创建一个基于Ipv4协议的socket
 * */
int createTcpServer(int *socket_fd) {

    signal(SIGSEGV, SIG_IGN);

    //服务端fd(socket)
    int server_fd;

    //创建基于ipv4的socket
    server_fd = *socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    return server_fd;

}

/**
 * 将socket绑定到指定端口号上，可以设置循环接收器。
 * */
void listenTcpServer(int *result, int socket_fd, int port, void (*on_accept)(int client_fd, struct sockaddr_in * __restrict, socklen_t * __restrict)) {

    //服务器用来配置socket的地址
    struct sockaddr_in server_addr, client_addr;

    socklen_t client_socklen_t;

    int client_fd;

    //初始化socket相关参数
    server_addr.sin_family = AF_INET;//ipv4协议
    server_addr.sin_port = htons(port); //端口号
    server_addr.sin_addr.s_addr = INADDR_ANY; //ip过滤，这里允许任何地址

    //可以通过setsockport函数做一些设置搞优化，这里忽略

    //通过bind函数绑定socket相关信息
    if ((*result = bind(socket_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr))) == -1) {
        perror("server bind: ");
        return;
    }

    //第二个参数为所谓的最大连接数，不是真的只能连那么多，很神奇的参数
    if ((*result = listen(socket_fd, 10)) == -1) {
        perror("server listen: ");
        return;
    }

    printf("server listen in %d\n", port);

    while (1) {
        if (on_accept == NULL) break;
        if ((client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_socklen_t)) == -1){
            if (errno == EINTR)
                continue;
            perror("server accept: ");
        }
        on_accept(client_fd, &client_addr, &client_socklen_t);
    }
}
