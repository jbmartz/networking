#include <stdint.h>

#define BOARD_SIZE 30
#define GAME_NAME "Connect Four"
#define BOARD_COLUMNS 6
#define BOARD_ROWS 5
typedef uint32_t uint_cf_marks;

/**
 * The message types possible for each message.
 * This enum type goes into the 'len' field for every
 * CFMessage header.
 *
 */
enum MessageType {
    ClientGetGame = 200,
    ServerGameReply,
    ServerInvalidRequestReply,
    ClientResult,
    ServerClientResultCorrect,
    ServerClientResultIncorrect
};

/**
 * The types of board results the client can report to
 * the server. When sending GameResultMessage the value
 * of the 'result' member variable must be one of these
 * types.
 */
enum ResultType {
    X_WIN = 1001,
    O_WIN,
    TIE_GAME,
    INVALID_BOARD
};

/**
 * This struct defines the 'header' for all messages
 * passed between the connectfour clients and connectfour
 * servers. The type must match one of the MessageType's
 * and the len must be the length of the entire message
 * including the header itself.
 *
 * ALL messages have this header as part of the message,
 * but only message 4 will _only_ be this type. If you
 * receive a message the size of a CFMessage (other than
 * as the 4'th message) this indicates your previous
 * message probably has an error in it (that's a HINT that
 * you should check the size of every message you receive
 * as part of checking its validity).
 */

struct Game {
    uint16_t game_id;
    uint_cf_marks x_positions;
    uint_cf_marks o_positions;
    ResultType winner;
};
struct CFMessage {
    uint16_t type;
    uint16_t len;
} __attribute__((packed));

/**
 * This struct defines the GetGameMessage that the client
 * will send to the server to request a game. The client_id
 * should be a randomly chosen number on the client,
 * which will be verified when the server sends it back in
 * a GameSummaryMessage.
 *
 * Message 1 of 4 in diagram.
 */
struct GetGameMessage {
    struct CFMessage hdr;
    uint16_t client_id;
} __attribute__((packed));

struct Games {
    uint_cf_marks x_pos;
    uint_cf_marks o_pos;
    ResultType result;
    char board[9];
};

/**
 * This struct defines the GameSummaryMessage that the
 * server will send to the client after receiving a valid
 * GetGameMessage.
 *
 * hdr.type = ServerGameReply
 * hdr.len = sizeof(GameSummaryMessage)
 * client_id should match the client id sent in the GetGameMessage
 * game_id will be a game_id to be used when the client sends a GameResultMessage
 * x_positions contains the positions of the 'X' discs on the connect four board.
 * 						 9 bit positions are used to indicate whether a position is set.
 * 						 i.e., if the i'th bit position is set to 1, it indicates that an
 * 						 'X' is present in the i position of the board.
 * o_positions contains the positions of the 'O' discs on the connect four board.
 * 						 i.e., if the i'th bit position is set to 1, it indicates that an
 * 						 'O' is present in the i position of the board.
 *
 * Message 2 of 4 in diagram.
 */
struct GameSummaryMessage {
    struct CFMessage hdr;
    uint16_t client_id;
    uint16_t game_id;
    uint_cf_marks x_positions;
    uint_cf_marks o_positions;
} __attribute__((packed));

/**
 * This struct defines the message format that the client
 * sends to the server after receiving a GameSummaryMessage.
 * The purpose of this message is to inform the server whether
 * the game sent by the server had a result of 'X' winning, 'O'
 * winning, neither 'X' nor 'O' winning (cats game), or an
 * invalid board. Note there are two invalid board possibilities:
 *   1. If X and O both claim to have a position marked
 *   2. If X and O would both win based on the board
 *
 * game_id must match the game id sent by the server in the GameSummaryMessage
 * result must be one of the values in the ResultType enum.
 *
 * Message 3 of 4 in diagram.
 */
struct GameResultMessage {
    struct CFMessage hdr;
    uint16_t game_id;
    uint16_t result;
} __attribute__((packed));


/***
 *	Example Connect Four board, with number positions.

      	Connect Four (6x5 grid)

      	|  0  |  1  |  2  |  3  |  4  |  5  |
      	|  6  |  7  |  8  |  9  |  10 |  11 |
      	|  12 |  13 |  14 |  15 |  16 |  17 |
      	|  18 |  19 |  20 |  21 |  22 |  23 |
      	|  24 |  25 |  26 |  27 |  28 |  29 |

 */
