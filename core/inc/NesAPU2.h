#pragma once

#include <cstdint>

class NesCPU;

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
	void clock(NesCPU *cpu);
	void reset();

	double GetOutputSample();

private:
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
    
    struct Noise {
        bool enabled;      
        bool length_enabled;
        uint8_t length_value;

        bool envelope_enabled;
        bool envelope_start;
        bool envelope_loop;
        uint8_t envelope_volume;
        uint8_t envelope_period;
        uint8_t envelope_value;
        uint8_t constant_volume;

        uint16_t timer_period;
        uint16_t timer_value;
        
        uint8_t shift_mode;
        uint16_t shift_register;
    public:
        void reset();
        uint8_t signal();
        void step_envelope();
        void step_length();
        void step_timer();
        void write_control(uint8_t data);
        void write_mode(uint8_t data);
        void write_length_index(uint8_t data);
    };
    Noise noise;

    struct TriangleWave {
        bool enabled;

        bool length_enabled;
        uint8_t length_value;

        bool counter_reload;
        uint8_t counter_period;
        uint8_t counter_value;

        uint16_t timer_value;
        uint16_t timer_period;
        uint8_t duty_value;
    public:
        void reset();
        uint8_t signal();
        void step_length();
        void step_timer();
        void step_counter();
        void write_control(uint8_t data);
        void write_timer_low(uint8_t data);
        void write_timer_high(uint8_t data);        
    };
    TriangleWave triangle;

    struct DMC {
        bool enabled;

        uint8_t buffer;

        bool irq_enabled;
        bool irq_flag;

        bool dmc_loop;
        uint8_t bit_count;
        uint8_t shift_register;

        uint16_t sample_address;
        uint16_t current_address;
        uint16_t sample_length;
        uint16_t current_length;

        uint16_t timer_period;
        uint16_t timer_value;
    public:
        void reset();
        uint8_t signal();
        void write_control(uint8_t val);
        void write_dac(uint8_t val);
        void write_address(uint8_t val);
        void write_length(uint8_t val);
        void step_reader(NesCPU *cpu);
        void step_shifter();
        void step_timer(NesCPU *cpu);
        bool get_irq_flag() { return irq_flag; }
        void clear_irq_flag() { irq_flag = false; }
    };
    DMC dmc;

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
    void step_timers(NesCPU *cpu);
    void step_sequencer();
};