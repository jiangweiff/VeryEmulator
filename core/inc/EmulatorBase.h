#pragma once

class EmulatorBase
{
public:
    virtual bool Initialize() = 0;
    virtual bool LoadGame(const char* filename) = 0;
};