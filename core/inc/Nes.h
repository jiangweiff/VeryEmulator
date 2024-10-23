#pragma once
#include "EmulatorBase.h"

class Nes : EmulatorBase
{
public:
    bool Initialize();
    bool LoadGame(const char* filename);
};