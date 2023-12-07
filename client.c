#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "pthread.h"

#define PORT 40000
#define SERVER_IP "127.0.0.1"
#define MAX_BUFFER 1024

struct player {
    int id;
    int pos_y, pos_x;
    char character;
    int color;
};


typedef struct player player_s;

struct game {
    player_s *player;
    int serv_sock;
    char data[1024];
};

typedef struct game game_s;

player_s *init_player_s_struct(int id, int pos_y, int pos_x, char character, int color)
{
    player_s *player = (player_s *) malloc(sizeof(player_s));
    if (!player) {
        perror("error init_player_s_struct");
        exit(84);
    }
    player->id = id;
    player->pos_y = pos_y;
    player->pos_x = pos_x;
    player->character = character;
    player->color = color;

    return player;
}

int etablish_connection(int *dest_sock, player_s **dest_player)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    //char buffer[MAX_BUFFER] = {0};
    //char *message = "Hello from client";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    //FIRST CONNECTION
    player_s *player = init_player_s_struct(0, 0, 0, 0, 0);

    recv(sock, (player_s *) player, sizeof(player_s), 0);

    *dest_player = player;
    *dest_sock = sock;

    return 0;
}

void *get_data_from_server(void *data)
{
    game_s *game = (game_s * ) data;

    printf("Connection etablished :\n"
           "id %d \n"
           "pos_y %d \n"
           "pos_x %d \n"
           "char %d \n"
           "color %d \n",
           game->player->id,
           game->player->pos_y,
           game->player->pos_x,
           game->player->character,
           game->player->color);


    while (recv(game->serv_sock, game->data, MAX_BUFFER, 0) != 0) {
    }

}

int main()
{
    game_s game;
    memset(&game, 0, sizeof(game_s));

    if (etablish_connection(&game.serv_sock, &game.player) != 0) {
        perror("etablish_connection failed");
        exit(84);
    }

    printf("Connection etablished :\n"
           "id %d \n"
           "pos_y %d \n"
           "pos_x %d \n"
           "char %d \n"
           "color %d \n",
           game.player->id,
           game.player->pos_y,
           game.player->pos_x,
           game.player->character,
           game.player->color);

    pthread_t thread;

        if (pthread_create(&thread, NULL, get_data_from_server, &game) != 0) {
            perror("pthread_create");
        } else {
            pthread_detach(thread);
        }

    while(1) {

    }

    //send(sock, message, strlen(message), 0);
    //printf("Message sent\n");
    //read(sock, buffer, MAX_BUFFER);
    //printf("Message from server: %s\n", buffer);

    return 0;
}