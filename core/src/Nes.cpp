#include "Nes.h"
#include "NesBus.h"
#include "NesRom.h"
#include "Renderer.h"
#include "SDL.h"
#include <chrono>
using namespace std::chrono;

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
    audioSystem.Initialize(44100, 1);
    SetSampleFrequency(44100);

    dAudioRealTime = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()).count() / 1000.0;
    return true;
}

void Nes::SetSampleFrequency(uint32_t sample_rate)
{
	dAudioTimePerSystemSample = 1.0 / (double)sample_rate;
	dAudioTimePerNESClock = 1.0 / 5369318.0; // PPU Clock Frequency
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

    AudioQueue::BatchWriter audioWriter(audioSystem);
    do {
        bus->clock();
        // Synchronising with Audio
        dAudioTime += dAudioTimePerNESClock;
        if (dAudioTime >= dAudioTimePerSystemSample)
        {
            dAudioTime -= dAudioTimePerSystemSample;
            double s = std::min(std::max(bus->apu->GetOutputSample(), -1.0), 1.0);
            int16_t sample = s * 0x7FFF;
            audioWriter.PushSample(sample);

            // calculate audio frames
            // double timeNow = duration_cast< milliseconds >(
            //     system_clock::now().time_since_epoch()).count() / 1000.0;
            // int audioRealFrames = (timeNow - dAudioRealTime)/dAudioTimePerSystemSample;
            // for(int i = 0; i < audioRealFrames; ++i) {
            //     audioWriter.PushSample(sample);
            //     dAudioRealTime = timeNow;
            // }
        }
    } while(!bus->ppu->frame_complete);
    bus->ppu->frame_complete = false;

    // dAudioRealTime = duration_cast< milliseconds >(
    //     system_clock::now().time_since_epoch()).count() / 1000.0;

    return 0;
}

void Nes::Present(Renderer * renderer)
{
    renderer->UpdateDisplayFrame(256, 240, bus->ppu->GetScreen());
    renderer->DisplayFrame();
}
