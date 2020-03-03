#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "connectfour.h"
#include <bitset>


#define ERROR_MSG "WE ENCOUNTERED AN ERROR error code"

const int n_games = 10000;
const int board_len = 29;
int game_counter = 0;

void handle_error(const char *context) {
    std::cerr << context << " failed with error:" << std::endl;
    std::cerr << strerror(errno) << std::endl;
    return;
}

int send_udp_buffer(int udp_socket, char *send_buf, uint16_t send_buf_size,
                    struct sockaddr *send_addr, socklen_t send_addr_size) {
    int ret;
    ret = sendto(udp_socket, send_buf, send_buf_size, 0, send_addr, send_addr_size);
    if (ret == -1) {
        handle_error("sendto");
    }
    return ret;
}

bool setupBoard(char board[], std::string x_pos, std::string o_pos, int len) {

    for (int i = 0; i <= len; i++) {

        if (x_pos[i] == '1' && o_pos[i] == '1') { //overlapping bits
//            std::cout << "o_pos: " << o_pos[i] << " x_pos: " << x_pos[i] << std::endl;
//            std::cout << "Y overlap with X." << std::endl;
            return false;
        }

        if (x_pos[i] == '1')
            board[i] = 'X';
        else if (o_pos[i] == '1')
            board[i] = 'O';
        else
            board[i] = '-';


    }

    // std::reverse(std::begin(board), std::end(board));
    return true;
}

void print_board(char *board) {
    int j = 0;
    for (int i = board_len; i >= 0; i--) {

        if (j++ % 6 == 0)
            std::cout << std::endl << "\t";

        std::cout << board[i] << " ";


    }
    std::cout << std::endl << std::endl;
}

void raw_to_text(int rt) {
    switch (rt) {
        case INVALID_BOARD:
            std::cout << "invalid board";
            break;
        case X_WIN:
            std::cout << "x win";
            break;
        case O_WIN:
            std::cout << "o win";
            break;
        case TIE_GAME:
            std::cout << "tie game";
            break;
        default:
            std::cout << "invalid type";
            break;
    }
}



int handle_packet_data(char *buf, int size, Game *games, int udp_socket, sockaddr_in dest_addr) {
//    struct BaseHeader *hdr;
//    struct NumberData *numb;
//    struct TextData *text;

    struct CFMessage *cf_message;
    struct GetGameMessage *game_message;
    struct GameResultMessage *game_result_message;

    if (size < sizeof(struct CFMessage)) {
        std::cout << "Invalid request." << std::endl;
        struct CFMessage cf_response;
        cf_response.type = htons(ServerInvalidRequestReply);
        cf_response.len = htons(sizeof(struct CFMessage));
        CFMessage *send_buffer = (CFMessage *) malloc(sizeof(struct CFMessage));


        memcpy(send_buffer, &cf_response, sizeof(struct CFMessage));
        memcpy(&send_buffer[sizeof(struct GameSummaryMessage)], &cf_response,
               sizeof(cf_response));

        int ret = sendto(udp_socket, send_buffer, sizeof(cf_response), 0, (struct sockaddr *) &dest_addr,
                         sizeof(struct sockaddr_in));;
        return -1;
    }

    cf_message = (struct CFMessage *) buf;
    cf_message->type = ntohs(cf_message->type);
    cf_message->len = ntohs(cf_message->len);


    switch (cf_message->type) {
        case ClientGetGame:
            if ((size >= sizeof(struct GetGameMessage)) && (cf_message->len >= sizeof(struct GetGameMessage))) {
                struct GameSummaryMessage game_reply_message;

                game_message = (struct GetGameMessage *) buf;
                game_message->client_id = ntohs(game_message->client_id);
//
                std::cout << "\n\n\tConnect Four\n\n"
                          << "Player 1(X) - Player 2(O)\n\n";
                char board[board_len];
                setupBoard(board, std::bitset<30>(games[game_counter].x_positions).to_string(),
                           std::bitset<30>(games[game_counter].o_positions).to_string(), board_len);
                print_board(board);

                //maybe check if id is empty
                game_reply_message.hdr.type = ServerGameReply;
                game_reply_message.hdr.len = htons(sizeof(struct GameSummaryMessage));

                game_reply_message.client_id = htons(game_message->client_id);
                game_reply_message.game_id = htons(games[game_counter].game_id);
                game_reply_message.x_positions = htonl(games[game_counter].x_positions);
                game_reply_message.o_positions = htonl(games[game_counter].o_positions);
                if (++game_counter >= n_games)
                    game_counter = 0;


                GameSummaryMessage *send_buffer = (GameSummaryMessage *) malloc(sizeof(struct GameSummaryMessage));


                memcpy(send_buffer, &game_reply_message, sizeof(struct GameSummaryMessage));
                memcpy(&send_buffer[sizeof(struct GameSummaryMessage)], &game_reply_message,
                       sizeof(game_reply_message));

                int ret = sendto(udp_socket, send_buffer, sizeof(game_reply_message), 0, (struct sockaddr *) &dest_addr,
                                 sizeof(struct sockaddr_in));;

                if (ret == -1) {
                    handle_error("sendto");
                }

                break;

            }
        case ClientResult:
            if ((size >= sizeof(struct GameResultMessage) && (cf_message->len >= sizeof(struct GetGameMessage)))) {
                game_result_message = (struct GameResultMessage *) buf;
                game_result_message->game_id = ntohs(game_result_message->game_id);
                game_result_message->hdr.type = ntohs(game_result_message->hdr.type);
                game_result_message->hdr.len = ntohs(game_result_message->hdr.len);
                game_result_message->result = ntohs(game_result_message->result);

                std::cout << "Client sent raw result '" << game_result_message->result << "', text result ";
                raw_to_text(game_result_message->result);


                std::cout << " for game id " << game_result_message->game_id << "." << std::endl;
                std::cout << "Expected result: ";
                raw_to_text(games[game_result_message->game_id - 1].winner);
                std::cout << std::endl;
                struct CFMessage cf_response;
                cf_response.len = htons(sizeof(struct CFMessage));
                cf_response.type = htons(ServerInvalidRequestReply);


                //check game id length
                if(game_result_message->game_id - 1 < 0 || game_result_message->game_id - 1 >= n_games){
                    std::cout << "Invalid game ID" << std::endl;
                }
                else if (games[game_result_message->game_id - 1].winner == game_result_message->result) {
                    std::cout << "Sending CORRECT response to client!" << std::endl;
                    cf_response.type = htons(ServerClientResultCorrect);
                } else {
                    std::cout << "Sending INCORRECT response to client!" << std::endl;
                    cf_response.type = htons(ServerClientResultIncorrect);
                }

                CFMessage *send_buffer = (CFMessage *) malloc(sizeof(struct CFMessage));


                memcpy(send_buffer, &cf_response, sizeof(struct CFMessage));
                memcpy(&send_buffer[sizeof(struct GameSummaryMessage)], &cf_response,
                       sizeof(cf_response));

                int ret = sendto(udp_socket, send_buffer, sizeof(cf_response), 0, (struct sockaddr *) &dest_addr,
                                 sizeof(struct sockaddr_in));;
                break;

            }

        default:

            struct CFMessage cf_response;
            cf_response.type = htons(ServerInvalidRequestReply);
            cf_response.len = htons(sizeof(struct CFMessage));
            CFMessage *send_buffer = (CFMessage *) malloc(sizeof(struct CFMessage));


            memcpy(send_buffer, &cf_response, sizeof(struct CFMessage));
            memcpy(&send_buffer[sizeof(struct GameSummaryMessage)], &cf_response,
                   sizeof(cf_response));

            int ret = sendto(udp_socket, send_buffer, sizeof(cf_response), 0, (struct sockaddr *) &dest_addr,
                             sizeof(struct sockaddr_in));;

            break;
    }


    return 0;
}


bool is_winner(std::string str) {
//at index 5, 11, 17, 23, 29

    //check horizontal wins
    for (int i = 0; i < str.size(); i++) {
        std::bitset<4> check_bits(str.substr(i, 4));

        if (i == 3 || i == 9 || i == 15 || i == 21) {
            i = i + 2;
            continue;
        }

        if (check_bits.all()) {
            return true;
        }

    }


    //check vertical wins
    for (int i = 0; i <= 12; i++) {
        char c_str[] = {str[i], str[i + 6], str[i + 12], str[i + 18], '\0'};
        std::bitset<4> check_bits(c_str);

        if (check_bits.all())
            return true;
    }


    return false;
}

bool bit_is_set(std::bitset<30> bits, int pos) {
    return bits.test(pos);
}

//duplicate games solve later maybe hash table?
void generate_games(Game *games, int n_games) {
    srand(time(NULL));

    for (int i = 0; i < n_games; i++) {
        games[i].game_id = i + 1;


        std::bitset<30> x_pos, o_pos;
        for (int j = 0; j < 15; j++) {
            int r = rand() % 30;

            while (bit_is_set(x_pos, r) || bit_is_set(o_pos, r)) {
                r = rand() % 30;
            }
            x_pos.set(r, true);

            r = rand() % 30;
            while (bit_is_set(x_pos, r) || bit_is_set(o_pos, r)) {
                r = rand() % 30;
            }
            o_pos.set(r, true);


        }
        //    std::cout << "x: " << x_pos.count() << " o: " << o_pos.count() << std::endl;

        games[i].x_positions = x_pos.to_ulong();
        games[i].o_positions = o_pos.to_ulong();

        bool x_win = is_winner(x_pos.to_string());
        bool o_win = is_winner(o_pos.to_string());

        if (!x_win && !o_win)
            games[i].winner = TIE_GAME;
        else if (x_win && o_win)
            games[i].winner = INVALID_BOARD;
        else if (x_win)
            games[i].winner = X_WIN;
        else if (o_win)
            games[i].winner = O_WIN;


    }
}

int main(int argc, char *argv[]) {
    char *ip_str;
    char *port_str;

    int udp_socket;
    unsigned int port;
    /* Dest is where we are listening */
    struct sockaddr_in dest;

    static char recv_buf[2048];
    /* recv_addr is the client who is talking to us */
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_size;

    static char send_buf[2048];
    int random_number;
    int ret;
    int bytes_printed;

    if (argc < 3) {
        std::cerr << "Provide IP PORT as first two arguments." << std::endl;
        return 1;
    }
    ip_str = argv[1];
    port_str = argv[2];

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (udp_socket == -1) {
        std::cerr << "Failed to create socket with error:" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        return 1;
    }

    ret = inet_pton(AF_INET, ip_str, &dest.sin_addr);

    if (ret == -1) {
        handle_error("inet_pton");
        return 1;
    }

    ret = sscanf(port_str, "%u", &port);

    if (ret != 1) {
        handle_error("sscanf");
    }

    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);

    bind(udp_socket, (struct sockaddr *) &dest, sizeof(struct sockaddr_in));

    Game games[n_games];

    generate_games(games, n_games);
    while (1) {
        recv_addr_size = sizeof(struct sockaddr_in);
        ret = recvfrom(udp_socket, recv_buf, 2047, 0,
                       (struct sockaddr *) &recv_addr, &recv_addr_size);

        if (ret == -1) {
            handle_error("recvfrom");
            close(udp_socket);
            return 1;
        }

        std::cout << "Received " << ret << " bytes from address size " << recv_addr_size << std::endl;
        recv_buf[ret] = '\0';

        ret = handle_packet_data(recv_buf, ret, games, udp_socket, recv_addr);
    }
    close(udp_socket);
    return 0;
}
