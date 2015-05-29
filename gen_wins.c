#include <stdio.h>
#include <stdint.h>

#define LOOP(id) for(int id=0; id<4; id++)

uint64_t mask(int x, int y, int z){
    return 1UL << (x*16 + y*4 + z);
}

#define print(d) printf("%#18lx,\n", d)

int main() {
    LOOP(x) LOOP(y) {
        uint64_t a = 0;
        LOOP(z) a |= mask(x, y, z);
        print(a);
    }
    LOOP(y) LOOP(z) {
        uint64_t a = 0;
        LOOP(x) a |= mask(x, y, z);
        print(a);
    }
    LOOP(x) LOOP(z) {
        uint64_t a = 0;
        LOOP(y) a |= mask(x, y, z);
        print(a);
    }


    // diag
    
    LOOP(z) {
        uint64_t a = 0;
        LOOP(xy) a |= mask(xy, xy, z);
        print(a);
    }
    LOOP(y) {
        uint64_t a = 0;
        LOOP(xz) a |= mask(xz, y, xz);
        print(a);
    }
    LOOP(x) {
        uint64_t a = 0;
        LOOP(yz) a |= mask(x, yz, yz);
        print(a);
    }


    LOOP(z) {
        uint64_t a = 0;
        LOOP(xy) a |= mask(xy, 3-xy, z);
        print(a);
    }
    LOOP(y) {
        uint64_t a = 0;
        LOOP(xz) a |= mask(3-xz, y, xz);
        print(a);
    }
    LOOP(x) {
        uint64_t a = 0;
        LOOP(yz) a |= mask(x, 3-yz, yz);
        print(a);
    }

    { 
        uint64_t a = 0;
        LOOP(xyz) a |= mask(xyz, xyz, xyz);
        print(a);
    }
    { 
        uint64_t a = 0;
        LOOP(xyz) a |= mask(3-xyz, xyz, xyz);
        print(a);
    }
    { 
        uint64_t a = 0;
        LOOP(xyz) a |= mask(xyz, 3-xyz, xyz);
        print(a);
    }
    { 
        uint64_t a = 0;
        LOOP(xyz) a |= mask(xyz, xyz, 3-xyz);
        print(a);
    }
}
