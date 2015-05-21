#include "ttt3d.h"

#include <cstdio>
#include <thread>
#include <cinttypes>

namespace BP {

enum BoardItem {
    EMPTY,
    P1, // me
    P2
};

using Board = BoardItem[4][4][4];

struct AI : public TTT3D {
    explicit AI(const duration<double> tta) : TTT3D(tta) {
    }

    void next_move(int mv[3]) {
        if (mv[0] != -1)
            game_board[mv[0]][mv[1]][mv[2]] = P2;

        // compute move

        game_board[mv[0]][mv[1]][mv[2]] = P1;
    }

    Board game_board = {{{EMPTY}}};
};

}

int main() {
    auto length = minutes(3);
    auto ai = new BP::AI(length);
}
