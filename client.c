#include "my_sockets.h"
#include "client.h"
#include "server.h"


void send_data_to_server(int sock, void *data, size_t size)
{
    send(sock, data, size, 0);
}

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

void *get_data_from_server(void *data)
{
    game_s *game = data;
    size_t n;

    while (1) {
        n = recv(game->serv_sock, game->data, CLIENT_BUFFER, 0);
        if (n <= 0) {
            if (n == -1) {
                perror("recv");
            }
            break;
        }
        game->data[n] = 0;
        game->nb_players = n / sizeof(player_s);
    }
    game->is_connected = 0;
}

void move_player(player_s *player, int key)
{
    switch (key) {
        case 65:
            player->pos_y--;
            break;
        case 66:
            player->pos_y++;
            break;
        case 68:
            player->pos_x--;
            break;
        case 67:
            player->pos_x++;
            break;
        default:
            break;
    }
}

void print_player(player_s *player)
{
    attron(COLOR_PAIR(player->color));
    mvprintw(player->pos_y, player->pos_x, "%c", player->character);
    attroff(COLOR_PAIR(player->color));
}

void print_all_players(game_s *game)
{
    for (int i = 0; i < game->nb_players; ++i) {
        player_s *player = (player_s *) &game->data[i * sizeof(player_s)];
        if (player->id == game->player->id) {
            continue;
        }
        print_player(player);
    }
}

void init_pairs()
{
    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(4, COLOR_BLUE, -1);
    init_pair(5, COLOR_MAGENTA, -1);
    init_pair(6, COLOR_CYAN, -1);
    init_pair(7, COLOR_WHITE, -1);
    init_pair(8, COLOR_BLACK, -1);
}

void *error_corrector(void *data)
{
    game_s *game = data;
    while (1) {
        send_data_to_server(game->serv_sock, game->player, sizeof(player_s));
        usleep(10000);
    }
}

int etablish_connection(int *dest_sock, player_s **dest_player)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    //char buffer[CLIENT_BUFFER] = {0};
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

void init_ncurses(void)
{
    initscr();

    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    curs_set(FALSE);
    noecho();
    nodelay(stdscr, TRUE);
    start_color();
    use_default_colors();
    init_pairs();
    wbkgd(stdscr, COLOR_PAIR(1));
}

int client_loop(void)
{

    game_s game;
    memset(&game, 0, sizeof(game_s));

    if (etablish_connection(&game.serv_sock, &game.player) != 0) {
        perror("etablish_connection failed");
        exit(84);
    }

    game.is_connected = 1;

    //FETCH DATA FROM SERVER
    pthread_t thread;
    if (pthread_create(&thread, NULL, get_data_from_server, &game) != 0) {
        perror("pthread_create");
        exit(84);
    }

    //SEND YOUR POSITION PERIODICALLY TO CORRECT THREAD ERRORS
    pthread_t error_thread;
    if (pthread_create(&error_thread, NULL, error_corrector, &game) != 0) {
        perror("pthread_create");
        exit(84);
    }

    init_ncurses();

    int key = ERR;
    while(game.is_connected) {
        key = getch();
        if (key != ERR) {
            move_player(game.player, key);
            send_data_to_server(game.serv_sock, game.player, sizeof(player_s));
        }
        erase();
        print_all_players(&game);
        print_player(game.player);
        refresh();

    }
    perror("Connection lost or server closed");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(84);
    }
    SERVER_IP = argv[1];
    PORT = atoi(argv[2]);
    return client_loop();
}