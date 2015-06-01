#include "ttt3d.h"

#undef VERBOSE // no logs
//#define NDEBUG // no assertions
#include <cstdio>
#include <cassert>
#include <cinttypes>

#include <vector>
#include <functional>
#include <unordered_map>
#include <algorithm>

#ifdef __POPCNT__ // need -march=native
#include <popcntintrin.h>
#endif

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

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
    0x1002004008000, 0x8004002001000, 0x1000020000400008 };

enum Player {
    NONE, DRAW, US, THEM
};

char us_piece = 'O';

// TODO: try bitset
struct Board {
    uint64_t us = 0, them = 0;

    Board() {}
    Board(const Board& o) : us(o.us), them(o.them) {}

    bool operator==(const Board& o) const {
        return us==o.us && them==o.them;
    }

    size_t operator()(Board const& b) const {
        std::size_t h1 = std::hash<uint64_t>()(b.us);
        std::size_t h2 = std::hash<uint64_t>()(b.them);
        return h1 ^ (h2 << 1);
    }

    static inline uint64_t mask(const int x, const int y, const int z) {
        return 1UL << (x * 16 + y * 4 + z);
    }

    static inline int bitcount(uint64_t val) {
#ifdef __POPCNT__
        return _mm_popcnt_u64(val);
#else
        int i;
        for (i = 0; val; ++i)
            val &= val - 1;
        return i;
#endif
    }

    void set(Player p, int x, int y, int z) {
        if (p == US) {
            assert(get(x,y,z) == NONE);
            us |= mask(x, y, z);
        } else if (p == THEM) {
            assert(get(x,y,z) == NONE);
            them |= mask(x, y, z);
        } else if (p == NONE) {
            us &= ~mask(x, y, z);
            them &= ~mask(x, y, z);
        }
    }

    Player get(int x, int y, int z) const {
        assert(0 <= x && x <= 3 && 0 <= y && y <= 3 && 0 <= z && z <= 3);
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
        if (unlikely(getEmpty() == 0))
            return DRAW;
        for (auto w : wins) {
            if (unlikely((w & us) == w))
                return US;
            if (unlikely((w & them) == w))
                return THEM;
        }
        return NONE;
    }

    float get_weight(Player cur) const {
        const Player winner = win();
        const uint64_t empty = getEmpty();

        if (unlikely(winner == US))
            return INFINITY;
        else if (unlikely(winner == THEM))
            return -INFINITY;
        else if (unlikely(winner == DRAW))
            return 0;
        else {
#if 0
            int ways_to_win[] = {0,0,0,0};
            int them_ways_to_win[] = {0,0,0,0};

            for (auto w : wins) {
                int n = bitcount(w & empty);
                if ((w & them) == 0 && (w & us) != 0) { // if they have none of the bits
                    ways_to_win[n]++;
                } else if ((w & us) == 0 && (w & them) != 0) {
                    them_ways_to_win[n]++;
                }
            }

            if (ways_to_win[1] && cur == US)
                return INFINITY;
            if (them_ways_to_win[1] && cur == THEM)
                return -INFINITY;

            float w_us   =      ways_to_win[1] * 10 +      ways_to_win[2] * 5 +      ways_to_win[3] * 1;
            float w_them = them_ways_to_win[1] * 10 + them_ways_to_win[2] * 5 + them_ways_to_win[3] * 1;
            return  w_us * -w_them;
#else
            int score[] = { 0, 1, 4, 16, 99999999 };
            int w_us = 0, w_them = 0;
            for (auto w : wins) {
                if ((w & them) == 0) { // they have none of the bits
                    switch(bitcount(w & us)){
                        case 1: w_us += score[1]; break;
                        case 2: w_us += score[2]; break;
                        case 3: {
                            if (cur == US)
                                w_us += score[4];
                            else
                                w_us += score[3];
                            break;
                        }
                        case 4: w_us += score[4]; break;
                        default: break;
                    }
                } else if ((w & us) == 0) {
                    switch(bitcount(w & them)){
                        case 1: w_them += score[1]; break;
                        case 2: w_them += score[2]; break;
                        case 3: {
                            if (cur == US)
                                w_them += score[3];
                            else
                                w_them += score[4];
                            break;
                        }
                        case 4: w_them += score[4]; break;
                        default: break;
                    }
                }
            }
            return w_us * -w_them;
#endif
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

struct AI : public TTT3D {
    Board game_board;
    std::unordered_map<Board, float, Board> table;

    explicit AI(const duration<double> tta) : TTT3D(tta) {}

    const int max_depth = 5;

    float minimax(Board board, Player turn, int depth, float alpha, float beta) {
        if (unlikely(depth == 1 || board.win() != NONE))
            return board.get_weight(turn);
        if (table.find(board) != table.end())
            return table[board];

        float best = (turn == US) ? -INFINITY : INFINITY;

        for (int x=0;x<4;++x){ for (int y=0;y<4;++y){ for (int z=0;z<4;++z){
            if (board.get(x,y,z) == NONE) {
                board.set(turn, x, y, z);

                if (turn == US) {
                    float child = minimax(board, THEM, depth - 1, alpha, beta);
                    if (child > best) best = child;
                    if (best > alpha) alpha = best;
                } else {
                    float child = minimax(board, US, depth - 1, alpha, beta);
                    if (child < best) best = child;
                    if (best < beta) beta = best;
                }

                #ifdef VERBOSE
                for (int i=max_depth; i>depth; --i) printf("\t");
                printf("%s (%d,%d,%d), best = %g\n", turn==US?"us":"them", x,y,z, best);
                #endif

                board.set(NONE, x, y, z);

                if (beta <= alpha)
                    goto done;
            }
        }}}
done:
        table[board] = best;
        return best;
    }

    move get_best_move() {
        std::vector<move> moves;
 
        for (int x=0;x<4;++x){ for (int y=0;y<4;++y){ for (int z=0;z<4;++z){
            if (game_board.get(x,y,z) == NONE) {
                game_board.set(US, x, y, z);
                float w = minimax(game_board, THEM, max_depth, -INFINITY, INFINITY);
                game_board.set(NONE, x, y, z);
                moves.push_back((move){x, y, z, w});

                #ifdef VERBOSE
                printf("us (%d,%d,%d) = %g\n", x,y,z, moves.score);
                #endif
            }
        }}}
 
        return *std::max_element(begin(moves), end(moves), [](move a, move b){ return a.score < b.score; });
    }

    void next_move(int mv[3]) {
        if (unlikely(mv[0] == -1 && mv[1] == -1 && mv[2] == -1))
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
        if (our_move.score == -INFINITY)
            printf("i'm going to lose ;_;\n");
    }
};

}
