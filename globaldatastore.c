//
// Created by parallels on 2019/12/25.
//

#include "globaldatastore.h"
#include "multiclienthandler.h"

#define MSGKEY 11
#define UPDATE_DATA_TYPE 10
#define CLEAR_CLIENT_DATA_TYPE 12

struct update_data_msg {
    long mtype;
    int client_id;
    int px;
    int py;
    int to_px;
    int to_py;
};

struct client_data {
    int id;
    int score;
};

struct map_data {
    int score;
    int type;
};

struct client_data *clients_data[6][6];

struct client_data *no_position_clients_data[1000];

struct map_data *maps_data[6][6];

int times = 0;

int create_store_process(int auto_notify_key, int auto_notify_sig) {

    for (int k = 0; k < 6; ++k) {
        for (int i = 0; i < 6; ++i) {
            clients_data[k][i] = NULL;
            maps_data[k][i] = NULL;
        }
    }

    for (int k = 0; k < 1000; ++k) {
        no_position_clients_data[k] = NULL;
    }

    int pid;
    int receiver_id;
    receiver_id = msgget(MSGKEY, 0777 | IPC_CREAT);
    while ((pid = fork()) == -1);
    if (pid == 0) {
        struct update_data_msg queue_receiver;
        while (1) {
            if (msgrcv(receiver_id, &queue_receiver, 2060, 0, 0) < 0) {
                perror("global data store error");
                exit(1);
            }
            printf("global data store: received message from client %d\n", queue_receiver.client_id);
            if (queue_receiver.mtype == UPDATE_DATA_TYPE) {

                struct client_data *data = malloc(sizeof(struct client_data));

                struct client_data *client = NULL;
                int flag = 0;
                for (int i = 0; i < 6; ++i) {
                    for (int j = 0; j < 6; ++j) {
                        if (clients_data[i][j] != NULL && clients_data[i][j]->id == queue_receiver.client_id && no_position_clients_data[clients_data[i][j]->id] == NULL) {
                            flag = 1;
                            client = clients_data[i][j];
                            break;
                        }
                    }
                    if (flag) break;
                }

                if (no_position_clients_data[queue_receiver.client_id] != NULL && !flag) {
                    flag = 1;
                    client = no_position_clients_data[queue_receiver.client_id];
                }

                if (!flag) {
                    data->id = queue_receiver.client_id;
                    data->score = 0;
                    no_position_clients_data[queue_receiver.client_id] = data;
                } else {
                    if (client->id != queue_receiver.client_id) {
                        continue;
                    }
                    if (clients_data[queue_receiver.to_px][queue_receiver.to_py] == NULL) {
                        if (queue_receiver.px == 0 && queue_receiver.py == 0) {
                            no_position_clients_data[queue_receiver.client_id] = NULL;
                        }
                        data->id = queue_receiver.client_id;
                        data->score = client->score;
                        free(clients_data[queue_receiver.px][queue_receiver.py]);
                        clients_data[queue_receiver.px][queue_receiver.py] = NULL;
                        if (maps_data[queue_receiver.to_px][queue_receiver.to_py] != NULL) {
                            data->score += maps_data[queue_receiver.to_px][queue_receiver.to_py]->score;
                            free(maps_data[queue_receiver.to_px][queue_receiver.to_py]);
                            maps_data[queue_receiver.to_px][queue_receiver.to_py] = NULL;
                        }
                        printf("update client %d score to %d\n", data->id, data->score);
                        clients_data[queue_receiver.to_px][queue_receiver.to_py] = data;
                    } else {
                        free(data);
                        continue;
                    }
                }

                if (times == 5) {
                    times = 0;
                    for (int i = 0; i < 5; ++i) {
                        int score = (rand() % 5) + 1;
                        int px_r = rand();
                        int py_r = rand();

                        struct map_data *md = malloc(sizeof(struct map_data));

                        md->type = 2;
                        md->score = score;
                        int px = (px_r % 5) + 1;
                        int py = (py_r % 5) + 1;
                        if (maps_data[px][py] != NULL || clients_data[px][py] != NULL) {
                            free(md);
                            continue;
                        }
                        maps_data[px][py] = md;
                        printf("data: %d, %d, %d, %d\n", px, py, maps_data[px][py]->score, maps_data[px][py]->type);
                    }
                } else {
                    times++;
                    printf("times++: %d\n", times);
                }

                char mapdata[8000];

                memset(mapdata, 0, sizeof(mapdata));

                for (int i = 0; i < 6; ++i) {
                    for (int j = 0; j < 6; ++j) {
                        char client_data_info[1000];
                        char map_data_info[1000];

                        char px[3];
                        char py[3];
                        char score[10];
                        char client_id[10];

                        char map_score[10];
                        char map_type[3];

                        sprintf(px, "%d", i);
                        sprintf(py, "%d", j);
                        if (clients_data[i][j] != NULL) {
                            sprintf(score, "%d", clients_data[i][j]->score);
                            sprintf(client_id, "%d", clients_data[i][j]->id);
                            sprintf(client_data_info, "%s %s %s %s %s", "client", px, py, score, client_id);
                            strcat(mapdata, client_data_info);
                            strcat(mapdata, "#");
                        }

                        if (maps_data[i][j] != NULL) {
                            printf("will send data: %d %d %d %d\n", i, j, maps_data[i][j]->score, maps_data[i][j]->type);
                            sprintf(map_score, "%d", maps_data[i][j]->score);
                            sprintf(map_type, "%d", maps_data[i][j]->type);
                            sprintf(map_data_info, "%s %s %s %s %s","map", px, py, map_score, map_type);
                            strcat(mapdata, map_data_info);
                            strcat(mapdata, "#");
                        }
                    }
                }

                int msgqid;
                struct msgform sender;
                memset(&sender, 0, sizeof(sender));
                while ((msgqid = msgget(auto_notify_key, 0777)) < 0);
                sender.mtype = 1;
                strcpy(sender.mtext, mapdata);
                for (int l = 0; l < 1000; ++l) {
                    if (no_position_clients_data[l] != NULL) {
                        char c[1000];
                        sprintf(c, "client %d %d %d %d#", 0, 0, 0, no_position_clients_data[l]->id);
                        strcat(sender.mtext, c);
                    }
                }
                strcat(sender.mtext, "\n");
                printf("send queue data: %s\n", sender.mtext);
                if (msgsnd(msgqid, &sender, 8000, 0) < 0){
                    perror("global store data notify error");
                }

            }
            if (queue_receiver.mtype == CLEAR_CLIENT_DATA_TYPE) {
                if (no_position_clients_data[queue_receiver.client_id] != NULL) {
                    free(no_position_clients_data[queue_receiver.client_id]);
                    no_position_clients_data[queue_receiver.client_id] = NULL;
                    printf("clear client %d data\n", queue_receiver.client_id);
                } else {
                    int flag = 0;
                    for (int i = 0; i < 6; ++i) {
                        for (int j = 0; j < 6; ++j) {
                            if (clients_data[i][j] == NULL) continue;
                            if (clients_data[i][j]->id == queue_receiver.client_id) {
                                printf("client will close %d\n", clients_data[i][j]->id);
                                free(clients_data[i][j]);
                                clients_data[i][j] = NULL;
                                flag = 1;
                                printf("clear client %d data\n", queue_receiver.client_id);
                                break;
                            }
                        }
                        if (flag) break;
                    }
                }
            }
        }
    }
    printf("global data store created in process %d\n", pid);
    return pid;
}

void update_client_data(int client_id, int px, int py, int to_px, int to_py) {
    int msgqid;
    struct update_data_msg sender;

    msgqid = msgget(MSGKEY, 0777);

    sender.mtype = UPDATE_DATA_TYPE;

    sender.px = px;
    sender.py = py;
    sender.to_px = to_px;
    sender.to_py = to_py;
    sender.client_id = client_id;

    msgsnd(msgqid, &sender, 1000, 0);
}

void clear_client_data(int client_id) {
    int msgqid;
    struct update_data_msg sender;

    msgqid = msgget(MSGKEY, 0777);

    sender.mtype = CLEAR_CLIENT_DATA_TYPE;

    sender.px = 0;
    sender.py = 0;
    sender.to_px = 0;
    sender.to_py = 0;
    sender.client_id = client_id;

    msgsnd(msgqid, &sender, 1000, 0);
}