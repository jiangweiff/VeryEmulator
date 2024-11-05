#include "NesAPU2.h"
#include "NesCPU.h"

const uint8_t LENGTH_TABLE[] = {  10, 254, 20,  2, 40,  4, 80,  6,
							        160,   8, 60, 10, 14, 12, 26, 14,
							         12,  16, 24, 18, 48, 20, 96, 22,
							        192,  24, 72, 26, 16, 28, 32, 30 };

// Duty cycle values. There are 4 modes, which silence different percentages of
// the square wave.
const uint8_t DUTY_TABLE[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0},  // 12.5%
    {0, 1, 1, 0, 0, 0, 0, 0},  // 25%
    {0, 1, 1, 1, 1, 0, 0, 0},  // 50%
    {1, 0, 0, 1, 1, 1, 1, 1},  // 75%
};

const uint16_t NOISE_TIMER_TABLE[16] = {
    0x004, 0x008, 0x010, 0x020,
    0x040, 0x060, 0x080, 0x0a0,
    0x0ca, 0x0fe, 0x17c, 0x1fc,
    0x2fa, 0x3f8, 0x7f2, 0xfe4,
};

const uint16_t DMC_TIMER_TABLE[16] = {
    0x01AC, 0x017C, 0x0154, 0x0140,
    0x011E, 0x00FE, 0x00E2, 0x00D6,
    0x00BE, 0x00A0, 0x008E, 0x0080,
    0x006A, 0x0054, 0x0048, 0x0036
};

// These volume values form the triangle shape
const uint8_t TRIANGLE_WAVEFORM[32] = {
    15, 14, 13, 12, 11, 10, 9,  8,
    7,  6,  5,  4,  3,  2,  1,  0,
    0,  1,  2,  3,  4,  5,  6,  7,
    8,  9,  10, 11, 12, 13, 14, 15,
};

void NesAPU2::SquareWave::reset()
{
    enabled = false;
    length_enabled = false;
    length_value = 0;
    timer_period = 0;
    timer_value = 0;
    duty_mode = 0;
    duty_value = 0;
    sweep_enabled = false;
    sweep_negate = false;
    sweep_reload = false;
    sweep_period = 0;
    sweep_shift = 0;
    sweep_value = 0;
    envelope_enabled = false;
    envelope_start = false;
    envelope_loop = false;
    envelope_volume = 0;
    envelope_period = 0;
    envelope_value = 0;
    constant_volume = 0;
}

uint8_t NesAPU2::SquareWave::signal()
{
    // The mixer receives the current envelope volume except when

    // The sequencer output is zero, or
    if (!enabled) {
        return 0;
    }

    // overflow from the sweep unit's adder is silencing the channel, or
    if (timer_period > 0x7ff) {
        return 0;
    }

    // the length counter is zero, or
    if (length_value == 0) {
        return 0;
    }

    // the timer has a value less than eight.
    if (timer_period < 8) {
        return 0;
    }

    if (DUTY_TABLE[duty_mode][duty_value] == 0) {
        return 0;
    }

    if (envelope_enabled) {
        return envelope_volume;
    } else {
        return constant_volume;
    }
}

void NesAPU2::SquareWave::step_envelope()
{
    if (envelope_start) {
        envelope_volume = 15;
        // The divider's period is set to n + 1.
        envelope_value = envelope_period + 1;
        envelope_start = false;
    } else if (envelope_value > 0) {
        envelope_value -= 1;
    } else {
        if (envelope_volume > 0) {
            envelope_volume -= 1;
        } else if (envelope_loop) {
            envelope_volume = 15;
        }
        envelope_value = envelope_period + 1;
    }
}

void NesAPU2::SquareWave::sweep()
{
    auto delta = timer_period >> sweep_shift;

    if (sweep_negate) {
        timer_period -= delta;

        if (channel == 1) {
            timer_period -= 1;
        }
    } else {
        timer_period += delta;
    }
}

void NesAPU2::SquareWave::step_sweep()
{
    if (sweep_reload) {
        if (sweep_enabled && sweep_value == 0) {
            sweep();
        }

        sweep_value = sweep_period + 1;
        sweep_reload = false;
    } else if (sweep_value > 0) {
        sweep_value -= 1;
    } else {
        if (sweep_enabled) {
            sweep();
        }
        sweep_value = sweep_period + 1;
    }
}

void NesAPU2::SquareWave::step_length()
{
    if (length_enabled && length_value > 0) {
        length_value -= 1;
    }
}

void NesAPU2::SquareWave::step_timer()
{
    if (timer_value == 0) {
        timer_value = timer_period + 1;
        duty_value = (duty_value + 1) % 8;
    } else {
        timer_value -= 1;
    }
}

// $4000/$4004
//
// A channel's first register controls the envelope:
//
//     ddle nnnn   duty, loop env/disable length, env disable, vol/env period
//
// Note that the bit position for the loop flag is also mapped to a flag in
// the Length Counter.
//
// The divider's period is set to n + 1.
//
// In addition to the envelope, the first register controls the duty cycle of the
// square wave, without resetting the position of the sequencer:
void NesAPU2::SquareWave::write_control(uint8_t data)
{
    duty_mode        = (data & 0b11000000) >> 6;
    envelope_loop    = (data & 0b00100000) != 0;
    length_enabled   = (data & 0b00100000) == 0;
    envelope_enabled = (data & 0b00010000) == 0;
    envelope_period  =  data & 0b00001111;
    constant_volume  =  data & 0b00001111;
    envelope_start   = true;
}

// $4001/$4005
//
// A channel's second register configures the sweep unit:
//
//     eppp nsss   enable sweep, period, negative, shift
//
// The divider's period is set to p + 1.
void NesAPU2::SquareWave::write_sweep(uint8_t data)
{
    sweep_enabled = (data & 0b10000000) != 0;
    sweep_period  = (data & 0b01110000) >> 4;
    sweep_negate  = (data & 0b00001000) != 0;
    sweep_shift   =  data & 0b00000111;
    sweep_reload = true;
}

// $4002/$4006
//
// For the square and triangle channels, the third and fourth registers form
// an 11-bit value and the divider's period is set to this value *plus one*.
//
// We add this *plus one* to the period in the step function.
void NesAPU2::SquareWave::write_timer_low(uint8_t data)
{
    // pppp pppp   period low
    timer_period = (timer_period & 0xff00) | (uint16_t)data;
}

// $4003/$4007
void NesAPU2::SquareWave::write_timer_high(uint8_t data)
{
    // llll lppp   length index, period high
    auto length_index = (data & 0b11111000) >> 3;
    uint16_t period_high  = (data & 0b00000111);

    length_value = LENGTH_TABLE[length_index];
    timer_period = (timer_period & 0x00ff) | (period_high << 8);
    envelope_start = true;
    duty_value = 0;
}

uint8_t NesAPU2::Noise::signal()
{
    if (!enabled) {
        return 0;
    }

    if (length_value == 0) {
        return 0;
    }

    // When bit 0 of the shift register is set, the DAC receives 0.
    if (shift_register & 1) {
        return 0;
    }

    if (envelope_enabled) {
        return envelope_volume;
    } else {
        return constant_volume;
    }
}

void NesAPU2::Noise::reset()
{
    enabled = false;
    length_enabled = false;
    length_value = 0;
    envelope_enabled = false;
    envelope_start = false;
    envelope_loop = false;
    envelope_volume = 0;
    envelope_period = 0;
    envelope_value = 0;
    constant_volume = 0;
    timer_period = 0;
    timer_value = 0;
    shift_mode = 1;
    shift_register = 1;
}

void NesAPU2::Noise::step_envelope()
{
    if (envelope_start) {
        envelope_volume = 15;
        // The divider's period is set to n + 1.
        envelope_value = envelope_period + 1;
        envelope_start = false;
    } else if (envelope_value) {
        envelope_value -= 1;
    } else {
        if (envelope_volume) {
            envelope_volume -= 1;
        } else if (envelope_loop) {
            envelope_volume = 15;
        }
        envelope_value = envelope_period + 1;
    }
}

void NesAPU2::Noise::step_length()
{
    if (length_enabled && length_value) {
        length_value -= 1;
    }
}

void NesAPU2::Noise::step_timer()
{
    if (timer_value == 0) {
        timer_value = timer_period;

        // When the timer clocks the shift register, the following actions
        // occur in order:
        //
        // 1. Feedback is calculated as the exclusive-OR of bit 0 and one
        //    other bit: bit 6 if Mode flag is set, otherwise bit 1.
        // 2. The shift register is shifted right by one bit.
        // 3. Bit 14, the leftmost bit, is set to the feedback calculated
        //    earlier.
        auto feedback = (shift_register & 1) ^ ((shift_register >> shift_mode) & 1);
        shift_register >>= 1;
        shift_register |= feedback << 14;
    } else {
        timer_value -= 1;
    }
}

// $400c
void NesAPU2::Noise::write_control(uint8_t val)
{
    // --le nnnn   loop env/disable length, env disable, vol/env period
    envelope_loop    = (val & 0b00100000) != 0;
    length_enabled   = (val & 0b00100000) == 0;
    envelope_enabled = (val & 0b00010000) == 0;
    envelope_period  =  val & 0b00001111;
    constant_volume  =  val & 0b00001111;
    envelope_start   = true;
}

// $400e
//
// The noise channel and DMC use lookup tables to set the timer's period.
void NesAPU2::Noise::write_mode(uint8_t val)
{
    // s--- pppp   short mode, period index
    val & 0x80 ? shift_mode = 1 : shift_mode = 6;

    auto period_index = val & 0b00001111;
    timer_period = NOISE_TIMER_TABLE[period_index];
}

// $400f
void NesAPU2::Noise::write_length_index(uint8_t val) {
    // llll l---   length index
    auto length_index = val >> 3;
    length_value = LENGTH_TABLE[length_index];
}

uint8_t NesAPU2::TriangleWave::signal()
{
    if (!enabled) {
        return 0;
    }

    if (length_value == 0) {
        return 0;
    }

    if (counter_value == 0) {
        return 0;
    }

    return TRIANGLE_WAVEFORM[duty_value];
}

void NesAPU2::TriangleWave::reset()
{
    enabled = false;
    length_enabled = false;
    length_value = 0;
    counter_reload = false;
    counter_period = 0;
    counter_value = 0;
    timer_value = 0;
    timer_period = 0;
    duty_value = 0;
}

void NesAPU2::TriangleWave::step_length()
{
    if (length_enabled && length_value > 0) {
        length_value -= 1;
    }
}

void NesAPU2::TriangleWave::step_timer()
{
    if (timer_value == 0) {
        timer_value = timer_period + 1;

        if (length_value > 0 && counter_value > 0) {
            duty_value = (duty_value + 1) % 32;
        }
    } else {
        timer_value -= 1;
    }
}

void NesAPU2::TriangleWave::step_counter()
{
    if (counter_reload) {
        counter_value = counter_period;
    } else if (counter_value > 0) {
        counter_value -= 1;
    }

    if (length_enabled) {
        counter_reload = false;
    }
}

// $4008
void NesAPU2::TriangleWave::write_control(uint8_t val)
{
    // clll llll   control, linear counter load
    length_enabled = (val & 0b10000000) == 0;
    counter_period =  val & 0b01111111;
}

// $400a
//
// For the square and triangle channels, the third and fourth registers form
// an 11-bit value and the divider's period is set to this value *plus one*.
void NesAPU2::TriangleWave::write_timer_low(uint8_t val)
{
    // pppp pppp   period low
    timer_period = (timer_period & 0xff00) | (uint16_t)val;
}

// $400b
void NesAPU2::TriangleWave::write_timer_high(uint8_t val) {
    // llll lppp   length index, period high
    auto length_index = (val & 0b11111000) >> 3;
    uint16_t period_high  = (val & 0b00000111);

    length_value = LENGTH_TABLE[length_index];
    timer_period = (timer_period & 0x00ff) | (period_high << 8);
    timer_value = timer_period + 1;
    counter_reload = true;
}

uint8_t NesAPU2::DMC::signal()
{
    if (!enabled) {
        return 0;
    }

    return buffer;
}

void NesAPU2::DMC::reset()
{
    enabled = false;
    buffer = 0;
    irq_enabled = false;
    irq_flag = false;
    dmc_loop = false;
    bit_count = 0;
    shift_register = 0;

    sample_address = 0;
    current_address = 0;
    sample_length = 0;
    current_length = 0;

    timer_period = 0;
    timer_value = 0;

    current_address = sample_address = 0;
    current_length = sample_length = 0;
}

// Register $4010 sets the interrupt enable, loop, and timer period. If the
// new interrupt enabled status is clear, the interrupt flag is cleared.
//
//     il-- ffff       interrupt enabled, loop, frequency index
void NesAPU2::DMC::write_control(uint8_t val) {
    auto irq      = (val & 0b10000000) != 0;
    auto dmc_loop = (val & 0b01000000) != 0;
    auto f_index  =  val & 0b00001111;

    irq_enabled = irq;

    if (!irq) {
        irq_flag = false;
    }

    dmc_loop = dmc_loop;
    timer_period = DMC_TIMER_TABLE[f_index];
}

// A write to $4011 sets the counter and DAC to a new value:
//
//     -ddd dddd       new DAC value
void NesAPU2::DMC::write_dac(uint8_t val)
{
    buffer = val & 0b01111111;
}

void NesAPU2::DMC::write_address(uint8_t val) {
    // Sample address = %11AAAAAA.AA000000
    sample_address = 0xC000 | (((uint16_t)val) << 6);
}

void NesAPU2::DMC::write_length(uint8_t val) {
    // Sample length = %LLLL.LLLL0001
    sample_length = (((uint16_t)val) << 4) + 1;
}

void NesAPU2::DMC::step_reader(NesCPU *cpu)
{
    // When the sample buffer is in an empty state and the bytes counter is non-zero,
    // the following occur: The sample buffer is filled with the next sample byte read
    // from memory at the current address, subject to whatever mapping hardware is
    // present (the same as CPU memory accesses). The address is incremented; if it
    // exceeds $FFFF, it is wrapped around to $8000. The bytes counter is decremented;
    // if it becomes zero and the loop flag is set, the sample is restarted (see
    // above), otherwise if the bytes counter becomes zero and the interrupt enabled
    // flag is set, the interrupt flag is set.

    if (current_length == 0 || bit_count != 0) {
        return;
    }

    if (cpu) {
        // TODO this is up to 4 extra cycles, but could be fewer
        cpu->cycles += 4;

        shift_register = cpu->read(current_address);
        // debug!("shift_register={:02X}", shift_register);
    } else {
        // error!("No CPU configured. This breaks the DMC.");
        return;
    }

    bit_count = 8;

    // The address is incremented; if it exceeds $FFFF, it is wrapped around to $8000.
    if (current_address == 0xFFFF) {
        current_address = 0x8000;
    } else {
        current_address++;
    }

    current_length -= 1;
    if (current_length == 0) {
        if (dmc_loop) {
            reset();
        }
        
        if (irq_enabled) {
            irq_flag = true;
        }
    }
}

void NesAPU2::DMC::step_shifter()
{
    if (bit_count == 0) {
        return;
    }

    if (shift_register & 0x01) {
        if (buffer <= 125) {
            buffer += 2;
        }
    } else {
        if (buffer >= 2) {
            buffer -= 2;
        }
    }

    shift_register >>= 1;
    bit_count -= 1;
}

void NesAPU2::DMC::step_timer(NesCPU *cpu)
{
    if (!enabled) {
        return;
    }

    //debug!("timer_value={}, timer_period={}", timer_value, timer_period);

    step_reader(cpu);
    if (timer_value == 0) {
        timer_value = timer_period;
        step_shifter();
    } else {
        timer_value -= 1;
    }
}

NesAPU2::NesAPU2()
{
}

NesAPU2::~NesAPU2()
{

}

void NesAPU2::reset()
{
    square1.reset();
    square2.reset();
    noise.reset();
    triangle.reset();
    dmc.reset();
}

void NesAPU2::cpuWrite(uint16_t addr, uint8_t data)
{
	switch (addr)
	{
	case 0x4000: square1.write_control(data); break;
	case 0x4001: square1.write_sweep(data); break;
	case 0x4002: square1.write_timer_low(data); break;
	case 0x4003: square1.write_timer_high(data); break;

	case 0x4004: square2.write_control(data); break;
	case 0x4005: square2.write_sweep(data); break;
	case 0x4006: square2.write_timer_low(data); break;
	case 0x4007: square2.write_timer_high(data); break;

	case 0x4008: triangle.write_control(data); break;
	case 0x4009: break;
    case 0x400A: triangle.write_timer_low(data); break;
    case 0x400B: triangle.write_timer_high(data); break;

	case 0x400C: noise.write_control(data); break;
    case 0x400D: break;
    case 0x400E: noise.write_mode(data); break;
    case 0x400F: noise.write_length_index(data); break;

    case 0x4010: dmc.write_control(data); break;
    case 0x4011: dmc.write_dac(data); break;
    case 0x4012: dmc.write_address(data); break;
    case 0x4013: dmc.write_length(data); break;

	case 0x4015: // APU STATUS
        square1.enabled  = data & 0b00000001;
        square2.enabled  = data & 0b00000010;
        triangle.enabled = (data & 0b00000100) != 0;
        noise.enabled    = (data & 0b00001000) != 0;
        dmc.enabled      = (data & 0b00010000) != 0;

        if (!square1.enabled) {
            square1.length_value = 0;
        }

        if (!square2.enabled) {
            square2.length_value = 0;
        }

        if (!triangle.enabled) {
            triangle.length_value = 0;
        }

        if (!noise.enabled) {
            noise.length_value = 0;
        }

        if (dmc.enabled) {
            if (dmc.current_length == 0) {
                dmc.reset();
            }
        } else {
            dmc.current_length = 0;
        }

        // When $4015 is written to, the DMC's IRQ occurred flag is cleared.
        dmc.clear_irq_flag();
		break;
    case 0x4017: // Frame counter
        write_frame_counter(data);
        break;
	}
}

uint8_t NesAPU2::cpuRead(uint16_t addr)
{
    //  $4015   if-d nt21   DMC IRQ, frame IRQ, length counter statuses
    if (addr != 0x4015) return 0;
    uint8_t data = 0x00;
    if (square1.length_value) {
        data |= 1;
    }

    if (square2.length_value) {
        data |= 2;
    }

    if (triangle.length_value) {
        data |= 4;
    }

    if (noise.length_value > 0) {
        data |= 8;
    }

    if (dmc.buffer) {
        data |= 16;
    }

    // When $4015 is read from, the frame IRQ flag is cleared, but not the DMC IRQ flag.
    frame_irq = false;
    return data;
}

void NesAPU2::write_frame_counter(uint8_t data)
{
    // MI-- ----       mode, IRQ disable

    // Mode (0 = 4-step, 1 = 5-step)
    sequencer_mode = (data & 0b10000000) == 0 ? SequencerMode::FourStep : SequencerMode::FiveStep;

    // IRQ inhibit flag. If this is set, we DON'T want to generate an IRQ.
    // Hello, double-negatives.
    irq = (data & 0b01000000) == 0;

    // If the mode flag is clear, the 4-step sequence is selected,
    // otherwise the 5-step sequence is selected and the sequencer is
    // immediately clocked once.
    if (sequencer_mode == SequencerMode::FiveStep) {
        step_envelopes();
        step_sweeps();
        step_lengths();
    }
}

void NesAPU2::step_envelopes()
{
    square1.step_envelope();
    square2.step_envelope();
    triangle.step_counter();
    noise.step_envelope();
}
        
void NesAPU2::step_sweeps()
{
    square1.step_sweep();
    square2.step_sweep();
}

void NesAPU2::step_lengths()
{
    square1.step_length();
    square2.step_length();
    triangle.step_length();
    noise.step_length();
}

void NesAPU2::step_timers(NesCPU *cpu)
{
    // The triangle channel ticks on every cycle. The other channels tick on
    // every other cycle.

    triangle.step_timer();

    if (cycles % 2 == 0) {
        square1.step_timer();
        square2.step_timer();
        noise.step_timer();
        dmc.step_timer(cpu);
    }
}

void NesAPU2::step_sequencer()
{
    switch (sequencer_mode)
    {
    case SequencerMode::FiveStep:
        sequencer_value = (sequencer_value+1)%5;
        // mode 1: 5-step
        // ---------------------------------------
        //     - - - - -   IRQ flag (never set)
        //     l - l - -   length counter + sweep
        //     e e e e -   envelope + linear counter
        switch (sequencer_value)
        {
        case 1:
        case 3:
            step_envelopes();
            step_sweeps();
            step_lengths();
            break;
        case 0:
        case 2:
            step_envelopes();
            break;
        default:
            break;
        }
        break;
    case SequencerMode::FourStep:
        sequencer_value = (sequencer_value+1)%4;
        // mode 0: 4-step
        // ---------------------------------------
        //     - - - f     IRQ flag
        //     - l - l     length counter + sweep
        //     e e e e     envelope + linear counter
        switch (sequencer_value)
        {
        case 1:
            step_envelopes();
            step_sweeps();
            step_lengths();
            break;
        case 3:
            step_envelopes();
            step_sweeps();
            step_lengths();
            if (irq) {
                frame_irq = true;
            }
            break;
        case 0:
        case 2:
            step_envelopes();
            break;
        default:
            break;
        }    
        break;
    default:
        break;
    }
}

void NesAPU2::clock(NesCPU *cpu)
{
    double cycle1 = (double)cycles;
    cycles += 1;
    double cycle2 = (double)cycles;

    step_timers(cpu);

    // https://wiki.nesdev.com/w/index.php/APU_Frame_Counter
    //
    // The sequencer is stepped at a rate of 240Hz or 192Hz, depending on
    // the mode. The way this happens is that we check if the previous
    // cycle and the current cycle crosses a multiple of 240. And if it
    // does, the sequencer is clocked.
    //
    // The five-step sequence is clocked at 192Hz, but this is achieved by
    // doing nothing on one of the steps, as 192 is 4/5 of 240.
    double sequencer_rate = 1789773.0 / 240.0;
    uint32_t f1 = (uint32_t)(cycle1 / sequencer_rate);
    uint32_t f2 = (uint32_t)(cycle2 / sequencer_rate);
    if (f1 != f2) {
        step_sequencer();
    }

    // The sampling rate is 44.1kHz. The way we do this is the same as the
    // sequencer (see explanation above).
    // double sample_rate = 1789773.0 / 44100.0;
    // uint32_t s1 = cycle1 / sample_rate;
    // uint32_t s2 = cycle2 / sample_rate;
    // if (s1 != s2) {
    //     res.signal = Some(self.signal());
    // }

    // res.trigger_irq = self.frame_irq || self.dmc.irq_flag();   
}

double NesAPU2::GetOutputSample()
{
    // https://www.nesdev.org/wiki/APU_Mixer
    double pulse = square1.signal() + square2.signal();
    double n = noise.signal();
    double tr = triangle.signal();
    double dmc = this->dmc.signal();

    //** linear approximation
    // double pulse_out = 0.00752 * pulse;
    // double tnd_out = 0.00851 * tr + 0.00494 * n + 0.00335 * dmc;
    
    double pulse_out = 95.88 / (100.0 + (8128.0 / pulse));
    double tnd_out = 159.79 / (100.0
                            + (1.0 / (  (tr / 8227.0)
                                        + (n / 12241.0)
                                        + (dmc / 22638.0))));
    double out = pulse_out + tnd_out;                                        

    return out;
    // return passFilters[2].process(passFilters[1].process(passFilters[0].process(out)));
    // return passFilters[0].process(passFilters[1].process(passFilters[2].process(pulse_val)));
}