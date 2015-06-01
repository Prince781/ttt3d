// vim: et:ts=4:sw=4:cc=120
#include <cstdio>
#include <iostream>

#include "ttt3d.h"
#include "BP.cpp"
#include "AskUser.cpp"

enum Player {NONE, DRAW, P1, P2};

int main() {
    auto length = minutes(3);
    
    TTT3D *players[] = { new BP::AI(length), new BP::AI(length) };
    BP::Board b;
    int move[] = {-1,-1,-1};
    Player turn = P1;
    
    while (b.win() == BP::NONE) {
        players[(turn == P1 ? 0 : 1)]->sqzzl(move);
        b.set((BP::Player)turn, move[0], move[1], move[2]);
        printf(turn==P1 ? "P1:\n" : "P2:\n");
        b.print();
        turn = (turn==P1 ? P2 : P1);
        printf("------------------------------------------------\n");
    }
    
    if (b.win() == BP::DRAW)
        printf("draw\n");
    else {
        printf(b.win() == (BP::Player)P1 ? "P1" : "P2");
        printf(" won\n");
    }
    
    return 0;
}
