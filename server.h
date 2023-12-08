#pragma once
    #define MAX_CLIENTS 1000
    #define MAP_SIZE 32


int PORT;

struct map {
    char map[MAP_SIZE][MAP_SIZE];
};

typedef struct map map_s;

struct client {
    int id;
    char is_use;
    int fd_socket;
    struct sockaddr_in address;
    void *data;
};
typedef struct client client_s;

enum PACKET_TYPE {
    MAP_PACKET = 0,
    PLAYER_PACKET = 1,
};