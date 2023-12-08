#pragma once
    #define MAX_CLIENTS 10

int PORT;

struct client {
    int id;
    char is_use;
    int fd_socket;
    struct sockaddr_in address;
    void *data;
};
typedef struct client client_s;
