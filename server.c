#include "my_sockets.h"
#include "server.h"
#include "client.h"

client_s clients[MAX_CLIENTS];

void init_client_s_struct(int max_clients, client_s clients[max_clients])
{
    for (int i = 0; i < max_clients; i++) {
        memset(&clients[i], 0, sizeof(client_s));
        clients[i].id = i;
    }
}

player_s *init_player_s_struct(int id, int pos_y,
    int pos_x, char character, int color)
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

client_s *find_free_slot(int max_clients, client_s clients[max_clients])
{
    for (int i = 0; i < max_clients; ++i) {
        if (clients[i].is_use == 0) {
            return &clients[i];
        }
    }
    return NULL;
}

void *handle_client(void *data)
{
    client_s *client = data;
    char buffer[CLIENT_BUFFER] = {0};
    while (1) {
        ssize_t n = recv(client->fd_socket, buffer, CLIENT_BUFFER, 0); // -1 0
        if (n == 0) {
            printf("Client n%d ip:%s disconnected\n", client->id,
                inet_ntoa(client->address.sin_addr));
            int temp_id = client->id;
            free((client_s *) client->data);
            memset(client, 0, sizeof(client_s));
            client->id = temp_id;
            break;
        }
        if (n > 0) {
            player_s *player = (player_s *) client->data;
            memcpy(player, buffer, sizeof(player_s));
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].is_use && player->id == clients[i].id) {
                    memcpy(clients[i].data, player, sizeof(player_s));
                }
            }
        }
    }
    return NULL;
}

void *send_all_datas(void *data)
{
    char buffer[CLIENT_BUFFER] = {0};
    int player_count = 0;

    while (1) {
        player_count = 0;
        memset(buffer, 0, CLIENT_BUFFER);
        long offset = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].is_use) {
                player_count++;
                player_s *player = (player_s *) clients[i].data;
                if (player == NULL) {
                    continue;
                }
                memcpy(buffer + offset, player, sizeof(player_s));
                offset += sizeof(player_s);
            }
        }
        if (player_count > 0) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].is_use) {
                    send(clients[i].fd_socket, buffer,
                        sizeof(player_s) * player_count, 0);
                }
            }
        }
        usleep(100000);
    }
}

int handle_server()
{
    //initscr();
    //curs_set(FALSE);
    //noecho();

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
        sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //thread that perpetually send all players data to all clients
    pthread_t data_thread;
    if (pthread_create(&data_thread, NULL, send_all_datas, NULL) != 0) {
        perror("pthread_create");
    } else {
        pthread_detach(data_thread);
    }
    while (1) {
        //printf("\nWaiting for a connection...\n");
        //mvprintw(0, 0, "Waiting for a connection...\n");
        //refresh();

        //This is necessary to always give the lowest free slot to the new client
        struct sockaddr_in temp_address;
        int temp_fd_socket;
        //

        if ((temp_fd_socket = accept(server_fd,
            (struct sockaddr *) &temp_address, (socklen_t *) &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        int free_socket = find_free_slot(MAX_CLIENTS, clients)->id;

        clients[free_socket].fd_socket = temp_fd_socket;
        memcpy(&clients[free_socket].address, &temp_address,
            sizeof(struct sockaddr_in));

        client_s *current_client = &clients[free_socket];
        current_client->is_use = 1;

        srand(time(NULL));
        //select a random color for the player
        int color = rand() % 7 + 1;

        //choose a random character for the player
        char character = rand() % 93 + 33;

        current_client->data = init_player_s_struct(free_socket, 0, 0, character,
            color);

        //FIRST CONNECTION
        send(current_client->fd_socket, (player_s *) current_client->data,
            sizeof(player_s), 0);

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client,
            (void *) current_client) != 0) {
            perror("pthread_create");
        } else {
            pthread_detach(thread);
        }


        //erase();
        for (int i = 0, current = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].is_use) {
                char *client_ip = inet_ntoa(clients[i].address.sin_addr);
                //printf("New connection from %s\n", client_ip);
                //mvprintw(current + 1, 0, "Client n%d :%s", clients[i].id, client_ip);
                printf("Client n%d :%s\n", clients[i].id, client_ip);
                current++;
            }
        }
        printf("\n\n\n\n");
        //refresh();
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(84);
    }
    PORT = atoi(argv[1]);
    init_client_s_struct(MAX_CLIENTS, clients);
    handle_server();
    return 0;
}