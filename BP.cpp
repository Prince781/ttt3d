#include "ttt3d.h"

#include <cstdio>
#include <cinttypes>
#include <cmath>
#include <vector>
#include <algorithm>

#define KRST "\x1B[0m"
#define KRED "\x1B[41m"
#define KGRN "\x1B[42m"
#define KYEL "\x1B[43m"
#define KBLU "\x1B[44m"
#define KMAG "\x1B[45m"
#define KCYN "\x1B[46m"
#define KWHT "\x1B[47m"

namespace BP {

static const uint64_t wins[] = { 0xf, 0xf0, 0xf00, 0xf000, 0xf0000, 0xf00000,
    0xf000000, 0xf0000000, 0xf00000000, 0xf000000000, 0xf0000000000,
    0xf00000000000, 0xf000000000000, 0xf0000000000000, 0xf00000000000000,
    0xf000000000000000, 0x1000100010001, 0x2000200020002, 0x4000400040004,
    0x8000800080008, 0x10001000100010, 0x20002000200020, 0x40004000400040,
    0x80008000800080, 0x100010001000100, 0x200020002000200,
    0x400040004000400, 0x800080008000800, 0x1000100010001000,
    0x2000200020002000, 0x4000400040004000, 0x8000800080008000, 0x1111,
    0x2222, 0x4444, 0x8888, 0x11110000, 0x22220000, 0x44440000, 0x88880000,
    0x111100000000, 0x222200000000, 0x444400000000, 0x888800000000,
    0x1111000000000000, 0x2222000000000000, 0x4444000000000000,
    0x8888000000000000, 0x1000010000100001, 0x2000020000200002,
    0x4000040000400004, 0x8000080000800008, 0x8000400020001,
    0x80004000200010, 0x800040002000100, 0x8000400020001000, 0x8421,
    0x84210000, 0x842100000000, 0x8421000000000000, 0x1001001001000,
    0x2002002002000, 0x4004004004000, 0x8008008008000, 0x1000200040008,
    0x10002000400080, 0x100020004000800, 0x1000200040008000, 0x1248,
    0x12480000, 0x124800000000, 0x1248000000000000, 0x8000040000200001,
    0x1002004008000, 0x8004002001000, 0x1000020000400008
};

enum Player {
    NONE, US, THEM, INVALID
};

struct Board {
    uint64_t us = 0, them = 0;

    static uint64_t mask(int x, int y, int z) {
        return 1UL << (x * 16 + y * 4 + z);
    }

    static int numbits(uint64_t val) {
        int i;
        for (i = 0; val; ++i)
            val &= val - 1;
        return i;
    }

    void set(Player p, int x, int y, int z) {
        // TODO: make sure bit is empty before set
        if (p == US)
            us |= mask(x, y, z);
        else if (p == THEM)
            them |= mask(x, y, z);
        else {
            us &= ~mask(x, y, z);
            them &= ~mask(x, y, z);
        }
    }

    Player get(int x, int y, int z) {
        if (x < 0 || x > 3 || y < 0 || y > 3 || z < 0 || z > 3)
            return INVALID;
        if (us & mask(x, y, z))
            return US;
        if (them & mask(x, y, z))
            return THEM;
        return NONE;
    }

    Player win() {
        for (int i = 0; i < 76; ++i)
            if ((wins[i] & us) == wins[i])
                return US;
        for (int i = 0; i < 76; ++i)
            if ((wins[i] & them) == wins[i])
                return THEM;
        return NONE;
    }

    uint64_t getEmpty() {
        return ~(us | them);
    }

    float get_weight() {
        const Player winner = win();
        const uint64_t empty = getEmpty();

        /*
         * weight = (# of n-steps to victory) / n
         * n is smallest number of steps to victory
         * rationale:
         * - more ways to win > less ways to win (because other player has to defend more)
         * - smaller number of steps (n) to win > larger number of steps to win
         */
        float w;
        if (winner == US)
            w = INFINITY;   // infinite ways to win in zero steps
        else if (winner == THEM)
            w = -INFINITY;
        else {
            int us_min_n = 4;   // step length
            int us_ways = 0;
            int them_min_n = 4;
            int them_ways = 0;
            // get n for US
            for (int i = 0; i < 76; ++i) {
                if (!(wins[i] & them)) {  // wins[i] in empty or us
                    uint64_t unoccupied = wins[i] & empty;
                    int n = Board::numbits(unoccupied);
                    if (n < us_min_n) {
                        us_min_n = n;
                        us_ways = 1;
                    } else if (n == us_min_n)
                        ++us_ways;
                } else if (!(wins[i] & us)) {  // wins[i] in empty or b.them
                    uint64_t unoccupied = wins[i] & empty;
                    int n = Board::numbits(unoccupied);
                    if (n < them_min_n) {
                        them_min_n = n;
                        them_ways = 1;
                    } else if (n == them_min_n)
                        ++them_ways;
                }
            }
            float w_us = (float) us_ways / us_min_n;
            float w_them = (float) them_ways / them_min_n;
            w = w_us - w_them;
        }
        return w;
    }

    void print(FILE *stream = stdout) {
        for (int y=3; y>=0; --y) {
            for (int z=0; z<4; ++z) {
                for (int x=0; x<4; ++x) {
                    switch (get(x,y,z)) {
                        case US:   fprintf(stream, KGRN "X" KRST); break;
                        case THEM: fprintf(stream, KBLU "O" KRST); break;
                        default:   fprintf(stream, "."); break;
                    }
                }
                fprintf(stream, " ");
            }
            fprintf(stream, "\n");
        }
        for (int z=0; z<4; ++z)
            fprintf(stream, "z=%d  ", z);
        fprintf(stream, "\n");
    }
};

struct move {
    int x, y, z;
    float score;
};

// TODO: const stuff
struct AI: public TTT3D {
    explicit AI(const duration<double> tta) : TTT3D(tta) {}

#if 0
    ~AI() { thread.join(); }
#endif
    const int MAX_DEPTH = 3;

    move get_best_move(Board b, Player t, int moveX = -1, int moveY = -1, int moveZ = -1, int depth = 0) {
        if (depth == MAX_DEPTH || b.win() == US || b.win() == THEM)
            return (move){moveX, moveY, moveZ, b.get_weight()};

        /*
         * terminal node
         * check winner
         */

        std::vector<move> children;

        for (int x = 0; x < 4; ++x) {
            for (int y = 0; y < 4; ++y) {
                for (int z = 0; z < 4; ++z) {
                    if (b.get(x, y, z) == NONE) {
                        if (t == US) {
                            b.set(US, x, y, z);
                            children.push_back(get_best_move(b, THEM, x, y, z, depth + 1));
                            b.set(NONE, x, y, z);
                        } else {
                            b.set(THEM, x, y, z);
                            children.push_back(get_best_move(b, US, x, y, z, depth + 1));
                            b.set(NONE, x, y, z);
                        }
                    }
                }
            }
        }

        if (t == US)
            return *std::max_element(children.begin(), children.end(), [](move a, move b) { return a.score < b.score; });
        else
            return *std::min_element(children.begin(), children.end(), [](move a, move b) { return a.score < b.score; });
    }

    void next_move(int mv[3]) {
        if (mv[0] != -1)
            game_board.set(THEM, mv[0], mv[1], mv[2]);

        // compute move
        move move = get_best_move(game_board, US);
        printf("AI: moving to (%d, %d, %d)\n", move.x, move.y, move.z);
        game_board.set(US, move.x, move.y, move.z);
    }
    
    Board game_board;

#if 0
    void compute_game_tree() {}
    std::thread thread = std::thread(&AI::compute_game_tree, this);
#endif
};

}