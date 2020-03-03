#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <bitset>
#include <climits>
#include "connectfour.h"

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


char checkBoard(char board[], int board_len, std::string str) {
//at index 5, 11, 17, 23, 29

    //check horizontal wins
    for(int i = 0; i < board_len; i++) {
        std::bitset<4> check_bits(str.substr(i, 4));

        if(i == 3 || i == 9 || i == 15 || i == 21) {
            i = i + 2;
            continue;
        }

        if(check_bits.all()) {
            return board[i];
        }

    }


    //check vertical wins
    for(int i = 0; i <= 12; i++) {
        char c_str[] = {str[i], str[i+6], str[i+12], str[i+18], '\0'};
        std::bitset<4> check_bits(c_str);

        if(check_bits.all())
            return board[i];
    }


    return 'T';
}


/**
 *
 * Dead simple UDP client example. Reads in IP PORT DATA
 * from the command line, and sends DATA via UDP to IP:PORT.
 *
 * e.g., ./udpclient 127.0.0.1 8888 this_is_some_data_to_send
 *
 * @param argc count of arguments on the command line
 * @param argv array of command line arguments
 * @return 0 on success, non-zero if an error occurred
 */


int main(int argc, char *argv[]) {
    // Alias for argv[1] for convenience
    char *ip_string;
    // Alias for argv[2] for convenience
    char *port_string;
    // Port to send UDP data to. Need to convert from command line string to a number
    unsigned int port;
    // The socket used to send UDP data on
    int udp_socket;
    // Variable used to check return codes from various functions
    int ret;




    // IPv4 structure representing and IP address and port of the destination
    struct sockaddr_in dest_addr;

    // Set dest_addr to all zeroes, just to make sure it's not filled with junk
    // Note we could also make it a static variable, which will be zeroed before execution
    memset(&dest_addr, 0, sizeof(struct sockaddr_in));

    // Note: this needs to be 4, because the program name counts as an argument!
    if (argc < 3) {
        std::cerr << "Please specify IP PORT as first two arguments." << std::endl;
        return 1;
    }
    // Set up variables "aliases"
    ip_string = argv[1];
    port_string = argv[2];

    // Create the UDP socket.
    // AF_INET is the address family used for IPv4 addresses
    // SOCK_DGRAM indicates creation of a UDP socket
    // ADD ME
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Make sure socket was created successfully, or exit.
    if (udp_socket == -1) {
        std::cerr << "Failed to create udp socket!" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        return 1;
    }

    // inet_pton converts an ip address string (e.g., 1.2.3.4) into the 4 byte
    // equivalent required for using the address in code.
    // Note that because dest_addr is a sockaddr_in (again, IPv4) the 'sin_addr'
    // member of the struct is used for the IP
    // ADD ME: Convert IP string into a binary IP address

    ret = inet_pton(AF_INET, ip_string, &dest_addr.sin_addr);

    // Check whether the specified IP was parsed properly. If not, exit.
    if (ret == -1) {
        std::cerr << "Failed to parse IPv4 address!" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        close(udp_socket);
        return 1;
    }

    // Convert the port string into an unsigned integer.
    ret = sscanf(port_string, "%u", &port);

    // sscanf is called with one argument to convert, so the result should be 1
    // If not, exit.
    if (ret != 1) {
        std::cerr << "Failed to parse port!" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        close(udp_socket);
        return 1;
    }

    // Set the address family to AF_INET (IPv4)
    // ADD ME
    dest_addr.sin_family = AF_INET;
    // Set the destination port. Use htons (host to network short)
    // to ensure that the port is in big endian format
    dest_addr.sin_port = htons(port);

    struct GetGameMessage game_message;
    // struct CFMessage cf_message;

    /* initialize random seed: */
    srand(time(NULL));


    uint16_t client_id = rand() % USHRT_MAX + 1;

    //setup new game header
    game_message.client_id = client_id;
    game_message.hdr.type = ClientGetGame;
    game_message.hdr.len = sizeof(struct GetGameMessage);


    GetGameMessage *send_buffer = (GetGameMessage *) malloc(game_message.hdr.len);
    game_message.hdr.type = htons(game_message.hdr.type);
    game_message.hdr.len = htons(game_message.hdr.len);
    game_message.client_id = htons(game_message.client_id);


    memcpy(send_buffer, &game_message, sizeof(struct GetGameMessage));
    memcpy(&send_buffer[sizeof(struct GetGameMessage)], &game_message, sizeof(game_message));

    ret = sendto(udp_socket, send_buffer, sizeof(game_message), 0, (struct sockaddr *) &dest_addr,
                 sizeof(struct sockaddr_in));

    // Check if send worked, clean up and exit if not.
    if (ret == -1) {
        std::cerr << "Failed to send data!" << std::endl;
        std::cerr << strerror(errno) << std::endl;
        close(udp_socket);
        return 1;
    }

    std::cout << "Sent " << ret << " bytes via UDP." << std::endl;


    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);

    bind(udp_socket, (struct sockaddr *) &dest, sizeof(struct sockaddr_in));
    static char recv_buf[2048];
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_size = sizeof(struct sockaddr_in);
    ret = recvfrom(udp_socket, recv_buf, 2047, 0,
                   (struct sockaddr *) &recv_addr, &recv_addr_size);


    std::cout << "Received " << ret << " bytes via UDP.\nReceived " << ret << " bytes from " << ip_string << std::endl;

    if (recv_addr_size < sizeof(struct CFMessage)) {
        std::cout << "Bad data" << std::endl;
        return 1;
    }

    struct GameSummaryMessage *game_reply_message;

    int board_len = 29;
    char board[board_len];

    if (recv_addr_size >= sizeof(struct GameSummaryMessage)) {
        game_reply_message = (struct GameSummaryMessage *) recv_buf;
        game_reply_message->hdr.type = ntohs(game_reply_message->hdr.type);
        if (game_reply_message->hdr.type == ServerInvalidRequestReply) {
            std::cout << "ServerInvalidRequestReply" << std::endl;
            return 1;
        }

        game_reply_message->hdr.len = ntohs(game_reply_message->hdr.len);
        game_reply_message->client_id = ntohs(game_reply_message->client_id);

        if(game_reply_message->client_id != client_id) {
            std::cout << "Client ID does not match" << std::endl;
            return 1;
        }

        game_reply_message->game_id = ntohs(game_reply_message->game_id);
        game_reply_message->x_positions = ntohl(game_reply_message->x_positions);
        game_reply_message->o_positions = ntohl(game_reply_message->o_positions);

        std::cout << "Received game result length " << game_reply_message->hdr.len << " client ID "
                  << game_reply_message->client_id << " game ID " << game_reply_message->game_id
                  << std::endl;

        std::cout << "X positions: " << game_reply_message->x_positions << ", O positions: "
                  << game_reply_message->o_positions << "\n\n\tConnect Four\n\n" << "Player 1(X) - Player 2(O)\n\n";
        std::string x_pos = std::bitset<30>(game_reply_message->x_positions).to_string();
        std::string o_pos = std::bitset<30>(game_reply_message->o_positions).to_string();

        if (setupBoard(board, x_pos, o_pos, board_len)) { //no bits overlap so do other checks

            int j = 0;
            for (int i = board_len; i >= 0; i--) {

                if (j++ % 6 == 0)
                    std::cout << std::endl << "\t";

                std::cout << board[i] << " ";


            }
            std::cout << std::endl << std::endl;
            char x_win = checkBoard(board, board_len, x_pos);
            char o_win = checkBoard(board, board_len, o_pos);

            char winner = '\0';
            if(x_win == 'T' && o_win == 'T')
                winner = 'T';
            else if (x_win == 'X' && o_win == 'O')
                winner = '\0';
            else if(x_win == 'X')
                winner = 'X';
            else if(o_win == 'O')
                winner = 'O';

          if(!winner)
              std::cout << "Invalid board." << std::endl;
          else if(winner == 'T')
              std::cout << "Tie game." << std::endl;
          else
            std::cout << winner << " is a winner!" << std::endl;



            struct GameResultMessage game_result_message;
            game_result_message.hdr.type = htons(ClientResult);
            game_result_message.hdr.len = htons(sizeof(struct GameResultMessage));
            game_result_message.game_id = htons(game_reply_message->game_id);

            switch (winner) {
                case 'O':
                    game_result_message.result = htons(O_WIN);
                    break;
                case 'X':
                    game_result_message.result = htons(X_WIN);
                    break;
                case 'T':
                    game_result_message.result = htons(TIE_GAME);
                    break;
                default:
                    game_result_message.result = htons(INVALID_BOARD);
                    break;


            }
            GameResultMessage *snd_buffer = (GameResultMessage *) malloc(game_message.hdr.len);
            memcpy(snd_buffer, &game_result_message, sizeof(struct GameResultMessage));
            memcpy(&snd_buffer[sizeof(struct GameResultMessage)], &game_result_message, sizeof(game_result_message));

            ret = sendto(udp_socket, snd_buffer, sizeof(game_result_message), 0, (struct sockaddr *) &dest_addr,
                         sizeof(struct sockaddr_in));

            if (ret == -1) {
                std::cerr << "Failed to send data!" << std::endl;
                std::cerr << strerror(errno) << std::endl;
                close(udp_socket);
                return 1;
            }

            std::cout << "Sent " << ret << " bytes via UDP." << std::endl;

            ret = recvfrom(udp_socket, recv_buf, 2047, 0,
                           (struct sockaddr *) &recv_addr, &recv_addr_size);


            std::cout << "Received " << ret << " bytes from " << ip_string << std::endl;

            if (recv_addr_size >= sizeof(struct CFMessage)) {
                struct CFMessage *cf_message = (struct CFMessage *) recv_buf;

                cf_message->type = ntohs(cf_message->type);
                if (cf_message->type == ServerInvalidRequestReply) {
                    std::cout << "ServerInvalidRequestReply" << std::endl;
                    return 1;
                }

                cf_message->len = ntohs(cf_message->len);


                std::string result;
                if(cf_message->type == ServerClientResultCorrect)
                    result = "CORRECT";
                else if(cf_message->type == ServerClientResultIncorrect)
                    result = "INCORRECT";

                std::cout << "Got " << result << " result from server!" << std::endl;
            }


        } else { //one or more bits overlap so board is invalid
            struct GameResultMessage game_result_message;
            game_result_message.hdr.type = htons(ClientResult);
            game_result_message.hdr.len = htons(sizeof(struct GameResultMessage));
            game_result_message.game_id = htons(game_reply_message->game_id);
            game_result_message.result = htons(INVALID_BOARD);


            GameResultMessage *snd_buffer = (GameResultMessage *) malloc(game_message.hdr.len);
            memcpy(snd_buffer, &game_result_message, sizeof(struct GameResultMessage));
            memcpy(&snd_buffer[sizeof(struct GameResultMessage)], &game_result_message, sizeof(game_result_message));

            ret = sendto(udp_socket, snd_buffer, sizeof(game_result_message), 0, (struct sockaddr *) &dest_addr,
                         sizeof(struct sockaddr_in));

            if (ret == -1) {
                std::cerr << "Failed to send data!" << std::endl;
                std::cerr << strerror(errno) << std::endl;
                close(udp_socket);
                return 1;
            }

            std::cout << "Sent " << ret << " bytes via UDP." << std::endl;

            ret = recvfrom(udp_socket, recv_buf, 2047, 0,
                           (struct sockaddr *) &recv_addr, &recv_addr_size);


            std::cout << "Received " << ret << " bytes from " << ip_string << std::endl;

            if (recv_addr_size >= sizeof(struct CFMessage)) {
                struct CFMessage *cf_message = (struct CFMessage *) recv_buf;

                cf_message->type = ntohs(cf_message->type);
                if (cf_message->type == ServerInvalidRequestReply) {
                    std::cout << "ServerInvalidRequestReply" << std::endl;
                    return 1;
                }

                cf_message->len = ntohs(cf_message->len);


                std::string result;
                if(cf_message->type == ServerClientResultCorrect)
                    result = "CORRECT";
                else if(cf_message->type == ServerClientResultIncorrect)
                    result = "INCORRECT";

                std::cout << "Got " << result << " result from server!" << std::endl;
            }
        }


    }





    close(udp_socket);
    return 0;
}
