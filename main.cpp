// vim: et:ts=4:sw=4:cc=120
#include <cstdio>
#include <iostream>

#include "ttt3d.h"
#include "BP.cpp"

int main() {
    auto length = minutes(3);
    BP::AI ai(length);
    while (ai.game_board.win() == BP::NONE && ai.game_board.getEmpty()) {
        int move[] = { -1, -1, -1 };
        do {
            printf("Enter move (x y z): ");
            std::cin >> move[0] >> move[1] >> move[2];
        } while (ai.game_board.get(move[0], move[1], move[2]) != BP::NONE);
        printf("Player: moving to (%d, %d, %d)\n", move[0], move[1], move[2]);
        ai.sqzzl(move);
        ai.game_board.print();
    }
    switch (ai.game_board.win()) {
        case BP::US:   printf("Game over: AI has won\n"); break;
        case BP::THEM: printf("Game over: Player has won\n"); break;
        default:       printf("Game over: Draw\n"); break;
    }
    return 0;
}