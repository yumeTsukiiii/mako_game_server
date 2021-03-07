////
//// Created by parallels on 2019/12/25.
////

#include "multiclienthandler.h"
#include "globaldatastore.h"
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <fcntl.h>

#define EACH_POLL_MAX_CLIENT 10000

#define EPOLL_CLIENT_ID_TYPE 10

int sv[2];

int epoll_fd;

int main_pid;

int recv_fd_control = 0;

struct epoll_event *ep_events;

#define CONTROLLEN CMSG_LEN(sizeof(int))

int send_fd(int fd, int fd_to_send) {
    struct iovec    iov[1];
    struct msghdr   msg;
    char        buf[1];
    struct cmsghdr  *cmptr = NULL;

    iov[0].iov_base = buf;
    iov[0].iov_len  = 1;
    msg.msg_iov = iov;
    msg.msg_iovlen  = 1;
    msg.msg_name    = NULL;
    msg.msg_namelen = 0;

    cmptr = malloc(CONTROLLEN);
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type  = SCM_RIGHTS;
    cmptr->cmsg_len    = CONTROLLEN;
    msg.msg_control   = cmptr;
    msg.msg_controllen= CONTROLLEN;
    *(int*)CMSG_DATA(cmptr) = fd_to_send;
    buf[0] = 0;

    if (sendmsg(fd, &msg, 0) != 1) {
        return -1;
    }
    return 0;
}

int recv_fd(int fd, int *fd_to_recv) {
    int         nr;
    char        buf[1];
    struct iovec    iov[1];
    struct msghdr   msg;
    struct cmsghdr  *cmptr = NULL;

    iov[0].iov_base = buf;
    iov[0].iov_len  = 1;
    msg.msg_iov = iov;
    msg.msg_iovlen  = 1;
    msg.msg_name    = NULL;
    msg.msg_namelen = 0;

    cmptr = malloc(CONTROLLEN);
    msg.msg_control = cmptr;
    msg.msg_controllen = CONTROLLEN;

    if(recvmsg(fd, &msg, 0) < 0) {
        printf("recvmsg error\n");
        return -1;
    }

    if(msg.msg_controllen < CONTROLLEN) {
        printf("recv_fd get invalid fd\n");
        return -1;
    }

    *fd_to_recv = *(int*)CMSG_DATA(cmptr);
    return 0;
}

void on_client_add(int signum,siginfo_t *info,void *myact) {

    int client_fd_num;

    printf("dispatcher will receive client... \n");

    recv_fd(sv[0], &client_fd_num);

    struct epoll_event event;

    event.events = EPOLLIN|EPOLLRDHUP;

    event.data.fd = client_fd_num;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd_num, &event) < 0) {
        perror("dispatch client to epoll process error");
    } else {
        printf("dispatch client %d to epoll %d process\n", client_fd_num, getpid());

        char connect_data[1000];
        sprintf(connect_data, "connect %d\n", client_fd_num);
        write(client_fd_num, connect_data, strlen(connect_data));

        update_client_data(client_fd_num, 0, 0, 0, 0);
        union sigval val;
        sigqueue(main_pid, SIGRTMIN + 5, val);
    }
}

int create_multi_client_handler(void (*on_read)(int sig, siginfo_t *siginfo, void *r)) {

    int pid;

    main_pid = getpid();

    epoll_fd = epoll_create(EACH_POLL_MAX_CLIENT);

    while ((pid = fork()) == -1);

    if (pid == 0) {

        //epoll
        int epoll_rev;

        struct aiocb aio;

        register_signal_info(SIGRTMIN, on_client_add);

        register_signal_info(SIGRTMIN + 1, on_read);

        ep_events=malloc(sizeof(struct epoll_event)*100);

        printf("epoll start in process: %d\n", getpid());

        while (1) {
            epoll_rev = epoll_wait(epoll_fd, ep_events, 100, 30 * 1000);
            for (int i = 0; i < epoll_rev; ++i) {
                if (aio_error(&aio) == EINPROGRESS) {
                    continue;
                }
                if (ep_events[i].events & EPOLLRDHUP) {
                    printf("client %d close\n", ep_events[i].data.fd);
                    clear_client_data(ep_events[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    continue;
                }

                char buf[1000] = {0};
                if (read_async(ep_events[i].data.fd, buf, sizeof(buf), SIGRTMIN + 1, &aio) < 0) {
                    perror("aio read: ");
                    exit(1);
                }
            }
        }
    }

    return pid;

}

void dispatch_client(int pid, int client_fd) {
    union sigval val;
    char *fd = malloc(10);
    sprintf(fd, "%d", client_fd);
    val.sival_ptr = fd;
    sigqueue(pid, SIGRTMIN, val);

    send_fd(sv[1], client_fd);
}

void create_client_fd_sender_pipe() {
    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
}