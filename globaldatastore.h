//
// Created by parallels on 2019/12/25.
//

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#ifndef LINUX_GAME_SERVER_GLOBALDATASTORE_H
#define LINUX_GAME_SERVER_GLOBALDATASTORE_H

struct msgform {
    long mtype;
    char mtext[8000];
};

int create_store_process(int auto_notify_key, int auto_notify_sig);

void update_client_data(int client_id, int px, int py, int to_px, int to_py);

void clear_client_data(int client_id);

#endif //LINUX_GAME_SERVER_GLOBALDATASTORE_H
