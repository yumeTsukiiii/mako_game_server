#include <aio.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "aioutil.h"
#include "socketutil.h"
#include "signalutil.h"
#include "multiclienthandler.h"
#include "globaldatastore.h"
#include <netinet/tcp.h>


#define EACH_POLL_MAX_CLIENT 10000

#define NOTIFY_QUEUE_KEY 13

int client_epoll;

int client_epolls[5];

int clients[1000];

int store_pid;

int main_pid;

void notify_client(int sig, siginfo_t *siginfo, void *r) {
    int qid = msgget(NOTIFY_QUEUE_KEY, 0777|IPC_CREAT);
    struct msgform data;
    if (msgrcv(qid, &data, 8000, 0, 0) < 0) {
        perror("receive notify error: ");
    }
    for (int i = 0; i < 1000; ++i) {
        if (i == clients[i] && clients[i] != 0) {
            struct tcp_info info;
            int len=sizeof(info);
            getsockopt(i, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
            if(info.tcpi_state==TCP_ESTABLISHED) {
                // write_async(i, data.mtext, -1);
                write(i, data.mtext, strlen(data.mtext));
            } else {
                printf("main client %d closed: \n", i);
                clients[i] = 0;
            }
        }
    }
}

void respond_client(int sig, siginfo_t *siginfo, void *r) {
    struct aiocb *aio;
    aio = siginfo->si_value.sival_ptr;

    char command[10];
    int px = -1,py = -1,to_px = -1,to_py = -1,client_id = 0;

    int receive_message_length = aio_return(aio);

    char receive_message[receive_message_length];

    char *copy_origin_message = (char *)aio->aio_buf;

    copy_origin_message[receive_message_length] = '\0';

    strcpy(receive_message, copy_origin_message);

    sscanf(receive_message, "%s %d %d %d %d %d", command, &px,&py,&to_px,&to_py, &client_id);

    if (strcmp(command, "update") == 0) {
        printf("%s %d %d %d %d %d\n", command, px,py,to_px,to_py,client_id);
        if (px < 0||px > 5||py < 0||py > 5||to_px < 0||to_px > 5||to_py < 0||to_py > 5||client_id < 0||client_id > 9999||(to_px == 0&&to_py == 0)) return;
        update_client_data(client_id, px, py, to_px, to_py);
        union sigval val;
        val.sival_ptr = receive_message;
        sigqueue(main_pid, SIGRTMIN + 5, val);
    }

}

void on_server_accept(int client_fd, struct sockaddr_in * __restrict sockaddr, socklen_t * __restrict socklen) {

    int client_p;

    dispatch_client(client_epolls[client_fd % 5], client_fd);

    clients[client_fd] = client_fd;
//    while ((client_p = fork()) == -1);
//
//    if (client_p == 0) {
//
//        printf("child process ok\n");
//
//        register_signal_info(SIGUSR1, respond_client);
//
//        printf("sigaction ok\n");
//
//        struct aiocb aio;
//
//        //epoll
//        struct epoll_event event;
//
//        struct epoll_event *ep_events;
//
//        int epoll_fd = epoll_create(100);
//
//        printf("epoll_fd: %d; client_d: %d", epoll_fd, client_fd);
//
//        int epoll_rev;
//
//        ep_events=malloc(sizeof(struct epoll_event)*100);
//
//        //listen read event
//        event.events = EPOLLIN;
//
//        event.data.fd = client_fd;
//
//        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
//
//        printf("epoll start\n");
//
//        while (1) {
//            epoll_rev = epoll_wait(epoll_fd, ep_events, 100, 30);
//            for (int i = 0; i < epoll_rev; ++i) {
//                if (aio_error(&aio) == EINPROGRESS) {
//                    continue;
//                }
//                char buf[1000] = {0};
//                if (read_async(ep_events[i].data.fd, buf, sizeof(buf), SIGUSR1, &aio) < 0) {
//                    perror("aio read: ");
//                    exit(1);
//                }
//            }
//        }
//
//    }
}

int main() {

    int socket_fd, listen_result;

    main_pid = getpid();

    signal(SIGCHLD, SIG_IGN);

    register_signal_info(SIGRTMIN + 5, notify_client);

    createTcpServer(&socket_fd);

    create_client_fd_sender_pipe();

    store_pid = create_store_process(NOTIFY_QUEUE_KEY, SIGRTMIN + 5);

    for (int i = 0; i < 5; ++i) {
        client_epolls[i] = create_multi_client_handler(respond_client);
    }

    listenTcpServer(&listen_result, socket_fd, 8081, on_server_accept);
    if (listen_result < 0) {
        kill(client_epoll, SIGINT);
    }
    return 0;
}