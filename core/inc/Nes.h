#pragma once
#include "EmulatorBase.h"
#include <string>

class NesBus;
class NesRom;

class Nes : EmulatorBase
{
public:
    NesBus* bus;
    NesRom* rom;

public:
    bool Initialize();
    bool LoadGame(const std::string& filename);
    int Tick();
    void Present(Renderer * renderer);
};