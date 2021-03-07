//
// Created by parallels on 2019/12/25.
//

#include "signalutil.h"

void register_signal_info(int sig, void (*action)(int sig, siginfo_t *siginfo, void *r)) {
    struct sigaction act; //定义信号
    memset(&act, 0, sizeof(act)); //内存初始化

    act.sa_sigaction = action; //设置信号handler
    act.sa_flags = SA_SIGINFO; //表示信号可以携带信息

    //这里应当使用可靠信号SIGRTMIN，不过这里没有，暂时用SIGUSR1处理
    //第三个参数是老的act，因为不关心这个参数所以忽略

    sigaction(sig, &act, NULL);
}

void register_signal_info_with_flag(int sig, void (*action)(int sig, siginfo_t *siginfo, void *r), int flag) {
    struct sigaction act; //定义信号
    memset(&act, 0, sizeof(act)); //内存初始化

    act.sa_sigaction = action; //设置信号handler
    act.sa_flags = SA_SIGINFO|flag; //表示信号可以携带信息

    //这里应当使用可靠信号SIGRTMIN，不过这里没有，暂时用SIGUSR1处理
    //第三个参数是老的act，因为不关心这个参数所以忽略

    sigaction(sig, &act, NULL);
}