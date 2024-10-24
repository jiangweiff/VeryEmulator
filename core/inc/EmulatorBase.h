#pragma once

#include <string>

class Renderer;

class EmulatorBase
{
public:
    virtual bool Initialize() = 0;
    virtual bool LoadGame(const std::string& filename) = 0;
    virtual int Tick() = 0;
    virtual void Present(Renderer * renderer) = 0;
};