#include "Nes.h"
#include "NesBus.h"
#include "NesRom.h"
#include "Renderer.h"

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
