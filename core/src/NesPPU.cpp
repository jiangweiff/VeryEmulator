#include "NesPPU.h"

void NesPPU::WritePAM(uint32_t offset, uint8_t value)
{
    auto o1 = offset/4;
    auto o2 = offset%4;
    switch(o2) {
        case 0: OAM[o1].y = value; break;
        case 1: OAM[o1].id = value; break;
        case 2: OAM[o1].attribute = value; break;
        case 3: OAM[o1].x = value; break;
    }
}