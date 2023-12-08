#pragma once
    #define CLIENT_BUFFER 1025 //1024
    #include "ncurses.h"
    #include <time.h>
    #define FRAME_TIME_MS (1000 / FPS)

char *SERVER_IP;
struct timespec g_start_time;
int FPS;

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
    char map[32][32];
};

typedef struct game game_s;
