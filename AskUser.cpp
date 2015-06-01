#include <cstdio>
#include <iostream>

#include "ttt3d.h"

struct AskUser : public TTT3D {
    explicit AskUser(const duration<double> tta) : TTT3D(tta) {}
 
    enum Player { NONE, US, THEM };
    Player board[4][4][4] = {{{NONE}}};
    
    Player get(int mv[3]) { return board[mv[0]][mv[1]][mv[2]]; }
    void set(Player p, int mv[3]) { board[mv[0]][mv[1]][mv[2]] = p; }
 
    void next_move(int mv[3]) {
        if (!(mv[0] == -1 && mv[1] == -1 && mv[2] == -1)){
            set(THEM, mv);
        }
        do {
            printf("Enter move (x y z): ");
            if (feof(stdin))
                exit(1);
            std::cin >> mv[0] >> mv[1] >> mv[2];
        } while (!(0 <= mv[0] && mv[0] <= 3 && 0 <= mv[1] && mv[1] <= 3 && 0 <= mv[2] && mv[2] <= 3 && get(mv) == NONE));
        set(US, mv);
    }
};