// vim: et:ts=4:sw=4:cc=120
#include <cstdio>
#include <iostream>

#include "ttt3d.h"
#include "BP.cpp"
#include "AskUser.cpp"

enum Player {NONE, P1, P2, DRAW};
struct Board {
    uint64_t p1 = 0, p2 = 0;

    static inline uint64_t mask(const int x, const int y, const int z) {
        return 1UL << (x * 16 + y * 4 + z);
    }

    static inline unsigned numbits(uint64_t val) {
        unsigned i;
        for (i = 0; val; ++i)
            val &= val - 1;
        return i;
    }

    void set(Player p, int x, int y, int z) {
        if (p == P1 || p == P2)
            assert(get(x,y,z) == NONE);

        if (p == P1) {
            p1 |= mask(x, y, z);
        } else if (p == P2) {
            p2 |= mask(x, y, z);
        } else {
            p1 &= ~mask(x, y, z);
            p2 &= ~mask(x, y, z);
        }
    }

    Player get(int x, int y, int z) const {
        assert(0 <= x && x <= 3 && 0 <= y && y <= 3 && 0 <= z && z <= 3);
        if (p1 & mask(x, y, z))
            return P1;
        if (p2 & mask(x, y, z))
            return P2;
        return NONE;
    }

    inline uint64_t getEmpty() const {
        return ~(p1 | p2);
    }

    Player win() const {
        if (getEmpty() == 0)
            return DRAW;
        for (auto w : BP::wins) {
            if ((w & p1) == w)
                return P1;
            if ((w & p2) == w)
                return P2;
        }
        return NONE;
    }
    
    void print() const {
        for (int y=3; y>=0; --y) {
            for (int z=0; z<4; ++z) {
                for (int x=0; x<4; ++x) {
                    switch (get(x,y,z)) {
                        case P1: printf(KGRN "X" KRST); break;
                        case P2: printf(KBLU "O" KRST); break;
                        default: printf("."); break;
                    }
                }
                printf("    ");
            }
            printf("\n");
        }
        for (int z=0; z<4; ++z)
            printf("z=%d     ", z);
        printf("\n");
    }
};

int main() {
    auto length = minutes(3);
    
    TTT3D *players[] = { new AskUser(length), new BP::AI(length) };
    Board b;
    int move[] = {-1,-1,-1};
    Player turn = P1;
    
    while (b.win() == NONE) {
        players[(turn == P1 ? 0 : 1)]->sqzzl(move);
        b.set(turn, move[0], move[1], move[2]);
        printf(turn==P1 ? "P1:\n" : "P2:\n");
        b.print();
        turn = (turn==P1 ? P2 : P1);
        printf("------------------------------------------------\n");
    }
    
    if (b.win() == DRAW)
        printf("draw\n");
    else {
        printf(b.win() == P1 ? "P1" : "P2");
        printf(" won\n");
    }
    
    return 0;
}
