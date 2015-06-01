#include "ttt3d.h"

#include <cstdio>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <vector>
#include <algorithm>

#ifdef __POPCNT__ // need -march=native
#include <popcntintrin.h>
#endif

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
    NONE, DRAW, US, THEM
};

char us_piece = 'O';

struct Board {
    uint64_t us = 0, them = 0;

    static inline uint64_t mask(const int x, const int y, const int z) {
        return 1UL << (x * 16 + y * 4 + z);
    }

    static inline unsigned bitcount(uint64_t val) {
#ifdef __POPCNT__
        return _mm_popcnt_u64(val);
#else
        unsigned i;
        for (i = 0; val; ++i)
            val &= val - 1;
        return i;
#endif
    }

    void set(Player p, int x, int y, int z) {
        if (p == US) {
            us |= mask(x, y, z);
        } else if (p == THEM) {
            them |= mask(x, y, z);
        } else {
            us &= ~mask(x, y, z);
            them &= ~mask(x, y, z);
        }
    }

    Player get(int x, int y, int z) const {
        if (us & mask(x, y, z))
            return US;
        if (them & mask(x, y, z))
            return THEM;
        return NONE;
    }

    inline uint64_t getEmpty() const {
        return ~(us | them);
    }

    Player win() const {
        if (getEmpty() == 0)
            return DRAW;
        for (auto w : wins) {
            if ((w & us) == w)
                return US;
            if ((w & them) == w)
                return THEM;
        }
        return NONE;
    }

    bool canWinInOneMove(Player p) const {
        if (p == US) {
            for (auto w : wins) {
                // if you have 3 of 4 bits, and they have none, you can win in 1 move
                if (bitcount(us & w) == 3 && (them & w) == 0)
                    return true;
            }
        } else if (p == THEM) {
            for (auto w : wins) {
                if (bitcount(them & w) == 3 && (us & w) == 0)
                    return true;
            }
        }
        return false;
    }

    float get_weight(Player cur) const {
        const Player winner = win();
        const uint64_t empty = getEmpty();

        /*
         * weight = (# of n-steps to victory) / n
         * n is smallest number of steps to victory
         * rationale:
         * - more ways to win > less ways to win (because other player has to defend more)
         * - smaller number of steps (n) to win > larger number of steps to win
         */
        if (winner == US)
            return INFINITY;
        else if (winner == THEM)
            return -INFINITY;
        else if (winner == DRAW)
            return 0;
        else if (canWinInOneMove(cur))
            return (cur == US) ? INFINITY : -INFINITY;
        else {
            int us_min_n = 4;   // step length
            int us_ways = 0;
            int them_min_n = 4;
            int them_ways = 0;
            // get n for US
            for (auto w : wins) {
                int us_needed = w & ~us;               // if the bits we don't have of the win
                if ((us_needed & empty) == us_needed){ // are available
                    int n_us_needed = bitcount(us_needed);
                    if (n_us_needed < us_min_n) {
                        us_min_n = n_us_needed;
                        us_ways = 1;
                    } else if (n_us_needed == us_min_n) {
                        ++us_ways;
                    }
                }

                int them_needed = w & ~them;               // if the bits they don't have of the win
                if ((them_needed & empty) == them_needed){     // are available
                    int n_them_needed = bitcount(them_needed);
                    if (n_them_needed < them_min_n) {
                        them_min_n = n_them_needed;
                        them_ways = 1;
                    } else if (n_them_needed == them_min_n) {
                        ++them_ways;
                    }
                }
               /*
                if (!(w & them)) {  // wins[i] in empty or us
                    uint64_t unoccupied = w & empty;
                    int n = bitcount(unoccupied);
                    if (n < us_min_n) {
                        us_min_n = n;
                        us_ways = 1;
                    } else if (n == us_min_n)
                        ++us_ways;
                } else if (!(w & us)) {  // wins[i] in empty or them
                    uint64_t unoccupied = w & empty;
                    int n = bitcount(unoccupied);
                    if (n < them_min_n) {
                        them_min_n = n;
                        them_ways = 1;
                    } else if (n == them_min_n)
                        ++them_ways;
                }
                */
            }
            float w_us = (float) us_ways / us_min_n;
            float w_them = (float) them_ways / them_min_n;
            return w_us / w_them;
        }
    }

    void print(FILE *stream = stdout) const {
        for (int y=3; y>=0; --y) {
            for (int z=0; z<4; ++z) {
                for (int x=0; x<4; ++x) {
                    switch (get(x,y,z)) {
                        case US:   fprintf(stream, KGRN "%c" KRST, us_piece); break;
                        case THEM: fprintf(stream, KBLU "%c" KRST, (us_piece == 'X') ? 'O' : 'X'); break;
                        default:   fprintf(stream, "."); break;
                    }
                }
                fprintf(stream, "    ");
            }
            fprintf(stream, "\n");
        }
        for (int z=0; z<4; ++z)
            fprintf(stream, "z=%d     ", z);
        fprintf(stream, "\n");
    }
};

struct move {
    int x, y, z;
    float score;
};

struct AI: public TTT3D {
    explicit AI(const duration<double> tta) : TTT3D(tta) {}

#if 0
    ~AI() { thread.join(); }
#endif

    float minimax(Board board, Player turn, int depth, float alpha, float beta) {
        if (depth == 1 || board.win() != NONE)
            return board.get_weight(turn);

        float best = (turn == US) ? -INFINITY : INFINITY;

#define FOREMPTY for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) for (int z = 0; z < 4; ++z) if (board.get(x, y, z) == NONE)
        if (turn == US){
            FOREMPTY {
                board.set(US, x, y, z);
                best = fmax(best, minimax(board, THEM, depth - 1, alpha, beta));
                alpha = fmax(alpha, best);
                board.set(NONE, x, y, z);
                if (beta <= alpha)
                    return best;
            }
        } else if (turn == THEM) {
            FOREMPTY {
                board.set(THEM, x, y, z);
                best = fmin(best, minimax(board, US, depth - 1, alpha, beta));
                beta = fmin(beta, best);
                board.set(NONE, x, y, z);
                if (beta <= alpha)
                    return best;
            }
        }
#undef FOREMPTY
        return best;
    }

    move get_best_move() {
        std::vector<move> moves;
 
#define FOREMPTY for (int x = 0; x < 4; ++x) for (int y = 0; y < 4; ++y) for (int z = 0; z < 4; ++z) if (game_board.get(x, y, z) == NONE)
        FOREMPTY {
            game_board.set(US, x, y, z);
            moves.push_back((move){x, y, z, minimax(game_board, THEM, 4, -INFINITY, INFINITY)}); // max depth here
            game_board.set(NONE, x, y, z);
        }
#undef FOREMPTY

        return *std::max_element(begin(moves), end(moves), [](move a, move b){ return a.score < b.score; });
    }

    void next_move(int mv[3]) {
        if (mv[0] == -1 && mv[1] == -1 && mv[2] == -1)
            us_piece = 'X'; // we are first
        else
            game_board.set(THEM, mv[0], mv[1], mv[2]);

        // compute move
        move our_move = get_best_move();
        game_board.set(US, our_move.x, our_move.y, our_move.z);
        mv[0] = our_move.x;
        mv[1] = our_move.y;
        mv[2] = our_move.z;

        printf("AI: moving to (%d, %d) on board %d\n", our_move.x, our_move.y, our_move.z);
    }
    
    Board game_board;

#if 0
    void compute_game_tree() {}
    std::thread thread = std::thread(&AI::compute_game_tree, this);
#endif
};

}
