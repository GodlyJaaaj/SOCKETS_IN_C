#pragma once
    #define CLIENT_BUFFER 1024
    #include "ncurses.h"

char *SERVER_IP;

struct player {
    int id;
    volatile int pos_y, pos_x;
    char character;
    int color;
};
typedef struct player player_s;

struct game {
    player_s *player;
    int serv_sock;
    int nb_players;
    char is_connected;
    char data[1024];
};

typedef struct game game_s;
