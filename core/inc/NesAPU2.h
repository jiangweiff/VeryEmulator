#pragma once

#include <cstdint>

enum SequencerMode{
    FourStep,
    FiveStep
};

class NesAPU2
{
public:
    NesAPU2();
    ~NesAPU2();

public:
	void cpuWrite(uint16_t addr, uint8_t data);
	uint8_t cpuRead(uint16_t addr);
	void clock();
	void reset();

	double GetOutputSample();

private:
	static uint8_t length_table[32];
	static uint8_t duty_table[4][8];

    struct SquareWave
    {
        bool enabled;
        uint8_t channel;

        bool length_enabled;
        uint8_t length_value;

        uint16_t timer_period;
        uint16_t timer_value;
        uint8_t duty_mode;
        uint8_t duty_value;

        bool sweep_enabled;
        bool sweep_negate;
        bool sweep_reload;
        uint8_t sweep_period;
        uint8_t sweep_shift;
        uint8_t sweep_value;

        bool envelope_enabled;
        bool envelope_start;
        bool envelope_loop;
        uint8_t envelope_volume;
        uint8_t envelope_period;
        uint8_t envelope_value;
        uint8_t constant_volume;

    public:
        void reset();
        uint8_t signal();
        void step_envelope();
        void sweep();
        void step_sweep();
        void step_length();
        void step_timer();
        void write_control(uint8_t data);
        void write_sweep(uint8_t data);
        void write_timer_low(uint8_t data);
        void write_timer_high(uint8_t data);
    };

    SquareWave square1;
    SquareWave square2;

    struct PassFilter
    {
        double b0;
        double b1;
        double a1;
        double prev_x = 0;
        double prev_y = 0;

    public:
        double process(double signal)
        {
            auto y = b0 * signal + b1 * prev_x - a1 * prev_y;
            prev_y = y;
            prev_x = signal;
            return y;
        }
    };
    

    struct LowPassFilter : PassFilter
    {
    public:
        LowPassFilter(double samplerate, double cutoff)
        {
            auto c = samplerate / 3.14159 / cutoff;
            auto a0i = 1.0 / (1.0 + c);
            b0 = b1 = a0i;
            a1 = (1.0 - c) * a0i;
        }
    };

    struct HighPassFilter : PassFilter
    {
    public:
        HighPassFilter(double samplerate, double cutoff)
        {
            auto c = samplerate / 3.14159 / cutoff;
            auto a0i = 1.0 / (1.0 + c);
            b0 = c * a0i;
            b1 = -c * a0i;
            a1 = (1.0 - c) * a0i;
        }
    };

    uint64_t cycles = 0;
    SequencerMode sequencer_mode;
    uint8_t sequencer_value = 0;
    bool irq;
    bool frame_irq;
    PassFilter passFilters[3] = {HighPassFilter(44100, 90), HighPassFilter(44100, 440), LowPassFilter(44100, 14000)};

    void write_frame_counter(uint8_t data);
    void step_envelopes();
    void step_sweeps();
    void step_lengths();
    void step_timers();
    void step_sequencer();
};