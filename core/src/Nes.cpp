#include "Nes.h"
#include "NesBus.h"
#include "NesRom.h"
#include "Renderer.h"
#include "SDL.h"

std::map<SDL_Scancode, uint8_t> keyMapper = {
    {SDL_SCANCODE_X, 0x80},
    {SDL_SCANCODE_Z, 0x40},
    {SDL_SCANCODE_A, 0x20},
    {SDL_SCANCODE_S, 0x10},

    {SDL_SCANCODE_UP, 0x08},
    {SDL_SCANCODE_DOWN, 0x04},
    {SDL_SCANCODE_LEFT, 0x02},
    {SDL_SCANCODE_RIGHT, 0x01},
};

bool Nes::Initialize()
{
    bus = new NesBus();
    return true;
}

bool Nes::LoadGame(const std::string& filename)
{
    rom = new NesRom(filename);
    bus->loadRom(rom);
    bus->reset();
    return true;
}

int Nes::Tick()
{
    bus->controller[0] = 0;
    const uint8_t* keystates = SDL_GetKeyboardState(NULL);
    for (auto &&i : keyMapper)
    {
        bus->controller[0] |= (keystates[i.first] ? i.second : 0); 
    }

    do {
        bus->clock();
    } while(!bus->ppu->frame_complete);
    bus->ppu->frame_complete = false;

    return 0;
}

void Nes::Present(Renderer * renderer)
{
    renderer->UpdateDisplayFrame(256, 240, bus->ppu->GetScreen());
    renderer->DisplayFrame();
}
