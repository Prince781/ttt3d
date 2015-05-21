#include "ttt3d.h"

#include <cstdio>
#include <cinttypes>
#include <thread>

namespace BenAndPrinceton {

enum BoardItem {
    EMPTY,
    US, // me
    THEM
};

using Board = BoardItem[4][4][4];

class AI : public TTT3D {
public:
    explicit AI(const duration<double> tta) : TTT3D(tta) {
    }

    ~AI() {
        thread.join();
    }

    void next_move(int mv[3]) {
        if (mv[0] != -1)
            game_board[mv[0]][mv[1]][mv[2]] = THEM;

        // compute move

        game_board[mv[0]][mv[1]][mv[2]] = US;
    }

private:
    void compute_game_tree() {
    }

    std::thread thread = std::thread(&AI::compute_game_tree, this);

    Board game_board;
};

}

int main() {
    auto length = minutes(3);
    BenAndPrinceton::AI ai(length);
    ai.sqzzl((int[]){0,0,0});
}
