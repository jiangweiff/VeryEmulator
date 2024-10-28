#pragma once
#include "EmulatorBase.h"
#include <string>
#include "AudioQueue.h"

class NesBus;
class NesRom;

class Nes : EmulatorBase
{
public:
    NesBus* bus;
    NesRom* rom;
    AudioQueue audioSystem;

public:
    bool Initialize();
    bool LoadGame(const std::string& filename);
    int Tick();
    void Present(Renderer * renderer);

public:
	void SetSampleFrequency(uint32_t sample_rate);

private:
	double dAudioTime = 0.0;
	double dAudioGlobalTime = 0.0;
	double dAudioTimePerNESClock = 0.0;
	double dAudioTimePerSystemSample = 0.0f;
};