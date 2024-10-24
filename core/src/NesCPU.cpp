#include "NesCPU.h"
#include "NesBus.h"
#include "Util/Hex.h"

NesCPU::NesCPU()
{
    opLookup = 
    {
        { "BRK", &NesCPU::BRK, &NesCPU::IMM, 7 },
        { "ORA", &NesCPU::ORA, &NesCPU::IZX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 3 },
        { "ORA", &NesCPU::ORA, &NesCPU::ZP0, 3 },
        { "ASL", &NesCPU::ASL, &NesCPU::ZP0, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "PHP", &NesCPU::PHP, &NesCPU::IMP, 3 },
        { "ORA", &NesCPU::ORA, &NesCPU::IMM, 2 },
        { "ASL", &NesCPU::ASL, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "ORA", &NesCPU::ORA, &NesCPU::ABS, 4 },
        { "ASL", &NesCPU::ASL, &NesCPU::ABS, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "BPL", &NesCPU::BPL, &NesCPU::REL, 2 },
        { "ORA", &NesCPU::ORA, &NesCPU::IZY, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "ORA", &NesCPU::ORA, &NesCPU::ZPX, 4 },
        { "ASL", &NesCPU::ASL, &NesCPU::ZPX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "CLC", &NesCPU::CLC, &NesCPU::IMP, 2 },
        { "ORA", &NesCPU::ORA, &NesCPU::ABY, 4 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "ORA", &NesCPU::ORA, &NesCPU::ABX, 4 },
        { "ASL", &NesCPU::ASL, &NesCPU::ABX, 7 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "JSR", &NesCPU::JSR, &NesCPU::ABS, 6 },
        { "AND", &NesCPU::AND, &NesCPU::IZX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "BIT", &NesCPU::BIT, &NesCPU::ZP0, 3 },
        { "AND", &NesCPU::AND, &NesCPU::ZP0, 3 },
        { "ROL", &NesCPU::ROL, &NesCPU::ZP0, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "PLP", &NesCPU::PLP, &NesCPU::IMP, 4 },
        { "AND", &NesCPU::AND, &NesCPU::IMM, 2 },
        { "ROL", &NesCPU::ROL, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "BIT", &NesCPU::BIT, &NesCPU::ABS, 4 },
        { "AND", &NesCPU::AND, &NesCPU::ABS, 4 },
        { "ROL", &NesCPU::ROL, &NesCPU::ABS, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "BMI", &NesCPU::BMI, &NesCPU::REL, 2 },
        { "AND", &NesCPU::AND, &NesCPU::IZY, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "AND", &NesCPU::AND, &NesCPU::ZPX, 4 },
        { "ROL", &NesCPU::ROL, &NesCPU::ZPX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "SEC", &NesCPU::SEC, &NesCPU::IMP, 2 },
        { "AND", &NesCPU::AND, &NesCPU::ABY, 4 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "AND", &NesCPU::AND, &NesCPU::ABX, 4 },
        { "ROL", &NesCPU::ROL, &NesCPU::ABX, 7 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "RTI", &NesCPU::RTI, &NesCPU::IMP, 6 },
        { "EOR", &NesCPU::EOR, &NesCPU::IZX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 3 },
        { "EOR", &NesCPU::EOR, &NesCPU::ZP0, 3 },
        { "LSR", &NesCPU::LSR, &NesCPU::ZP0, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "PHA", &NesCPU::PHA, &NesCPU::IMP, 3 },
        { "EOR", &NesCPU::EOR, &NesCPU::IMM, 2 },
        { "LSR", &NesCPU::LSR, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "JMP", &NesCPU::JMP, &NesCPU::ABS, 3 },
        { "EOR", &NesCPU::EOR, &NesCPU::ABS, 4 },
        { "LSR", &NesCPU::LSR, &NesCPU::ABS, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "BVC", &NesCPU::BVC, &NesCPU::REL, 2 },
        { "EOR", &NesCPU::EOR, &NesCPU::IZY, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "EOR", &NesCPU::EOR, &NesCPU::ZPX, 4 },
        { "LSR", &NesCPU::LSR, &NesCPU::ZPX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "CLI", &NesCPU::CLI, &NesCPU::IMP, 2 },
        { "EOR", &NesCPU::EOR, &NesCPU::ABY, 4 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "EOR", &NesCPU::EOR, &NesCPU::ABX, 4 },
        { "LSR", &NesCPU::LSR, &NesCPU::ABX, 7 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "RTS", &NesCPU::RTS, &NesCPU::IMP, 6 },
        { "ADC", &NesCPU::ADC, &NesCPU::IZX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 3 },
        { "ADC", &NesCPU::ADC, &NesCPU::ZP0, 3 },
        { "ROR", &NesCPU::ROR, &NesCPU::ZP0, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "PLA", &NesCPU::PLA, &NesCPU::IMP, 4 },
        { "ADC", &NesCPU::ADC, &NesCPU::IMM, 2 },
        { "ROR", &NesCPU::ROR, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "JMP", &NesCPU::JMP, &NesCPU::IND, 5 },
        { "ADC", &NesCPU::ADC, &NesCPU::ABS, 4 },
        { "ROR", &NesCPU::ROR, &NesCPU::ABS, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "BVS", &NesCPU::BVS, &NesCPU::REL, 2 },
        { "ADC", &NesCPU::ADC, &NesCPU::IZY, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "ADC", &NesCPU::ADC, &NesCPU::ZPX, 4 },
        { "ROR", &NesCPU::ROR, &NesCPU::ZPX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "SEI", &NesCPU::SEI, &NesCPU::IMP, 2 },
        { "ADC", &NesCPU::ADC, &NesCPU::ABY, 4 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "ADC", &NesCPU::ADC, &NesCPU::ABX, 4 },
        { "ROR", &NesCPU::ROR, &NesCPU::ABX, 7 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "STA", &NesCPU::STA, &NesCPU::IZX, 6 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "STY", &NesCPU::STY, &NesCPU::ZP0, 3 },
        { "STA", &NesCPU::STA, &NesCPU::ZP0, 3 },
        { "STX", &NesCPU::STX, &NesCPU::ZP0, 3 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 3 },
        { "DEY", &NesCPU::DEY, &NesCPU::IMP, 2 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "TXA", &NesCPU::TXA, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "STY", &NesCPU::STY, &NesCPU::ABS, 4 },
        { "STA", &NesCPU::STA, &NesCPU::ABS, 4 },
        { "STX", &NesCPU::STX, &NesCPU::ABS, 4 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 4 },
        { "BCC", &NesCPU::BCC, &NesCPU::REL, 2 },
        { "STA", &NesCPU::STA, &NesCPU::IZY, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "STY", &NesCPU::STY, &NesCPU::ZPX, 4 },
        { "STA", &NesCPU::STA, &NesCPU::ZPX, 4 },
        { "STX", &NesCPU::STX, &NesCPU::ZPY, 4 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 4 },
        { "TYA", &NesCPU::TYA, &NesCPU::IMP, 2 },
        { "STA", &NesCPU::STA, &NesCPU::ABY, 5 },
        { "TXS", &NesCPU::TXS, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 5 },
        { "STA", &NesCPU::STA, &NesCPU::ABX, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "LDY", &NesCPU::LDY, &NesCPU::IMM, 2 },
        { "LDA", &NesCPU::LDA, &NesCPU::IZX, 6 },
        { "LDX", &NesCPU::LDX, &NesCPU::IMM, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "LDY", &NesCPU::LDY, &NesCPU::ZP0, 3 },
        { "LDA", &NesCPU::LDA, &NesCPU::ZP0, 3 },
        { "LDX", &NesCPU::LDX, &NesCPU::ZP0, 3 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 3 },
        { "TAY", &NesCPU::TAY, &NesCPU::IMP, 2 },
        { "LDA", &NesCPU::LDA, &NesCPU::IMM, 2 },
        { "TAX", &NesCPU::TAX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "LDY", &NesCPU::LDY, &NesCPU::ABS, 4 },
        { "LDA", &NesCPU::LDA, &NesCPU::ABS, 4 },
        { "LDX", &NesCPU::LDX, &NesCPU::ABS, 4 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 4 },
        { "BCS", &NesCPU::BCS, &NesCPU::REL, 2 },
        { "LDA", &NesCPU::LDA, &NesCPU::IZY, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "LDY", &NesCPU::LDY, &NesCPU::ZPX, 4 },
        { "LDA", &NesCPU::LDA, &NesCPU::ZPX, 4 },
        { "LDX", &NesCPU::LDX, &NesCPU::ZPY, 4 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 4 },
        { "CLV", &NesCPU::CLV, &NesCPU::IMP, 2 },
        { "LDA", &NesCPU::LDA, &NesCPU::ABY, 4 },
        { "TSX", &NesCPU::TSX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 4 },
        { "LDY", &NesCPU::LDY, &NesCPU::ABX, 4 },
        { "LDA", &NesCPU::LDA, &NesCPU::ABX, 4 },
        { "LDX", &NesCPU::LDX, &NesCPU::ABY, 4 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 4 },
        { "CPY", &NesCPU::CPY, &NesCPU::IMM, 2 },
        { "CMP", &NesCPU::CMP, &NesCPU::IZX, 6 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "CPY", &NesCPU::CPY, &NesCPU::ZP0, 3 },
        { "CMP", &NesCPU::CMP, &NesCPU::ZP0, 3 },
        { "DEC", &NesCPU::DEC, &NesCPU::ZP0, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "INY", &NesCPU::INY, &NesCPU::IMP, 2 },
        { "CMP", &NesCPU::CMP, &NesCPU::IMM, 2 },
        { "DEX", &NesCPU::DEX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "CPY", &NesCPU::CPY, &NesCPU::ABS, 4 },
        { "CMP", &NesCPU::CMP, &NesCPU::ABS, 4 },
        { "DEC", &NesCPU::DEC, &NesCPU::ABS, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "BNE", &NesCPU::BNE, &NesCPU::REL, 2 },
        { "CMP", &NesCPU::CMP, &NesCPU::IZY, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "CMP", &NesCPU::CMP, &NesCPU::ZPX, 4 },
        { "DEC", &NesCPU::DEC, &NesCPU::ZPX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "CLD", &NesCPU::CLD, &NesCPU::IMP, 2 },
        { "CMP", &NesCPU::CMP, &NesCPU::ABY, 4 },
        { "NOP", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "CMP", &NesCPU::CMP, &NesCPU::ABX, 4 },
        { "DEC", &NesCPU::DEC, &NesCPU::ABX, 7 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "CPX", &NesCPU::CPX, &NesCPU::IMM, 2 },
        { "SBC", &NesCPU::SBC, &NesCPU::IZX, 6 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "CPX", &NesCPU::CPX, &NesCPU::ZP0, 3 },
        { "SBC", &NesCPU::SBC, &NesCPU::ZP0, 3 },
        { "INC", &NesCPU::INC, &NesCPU::ZP0, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 5 },
        { "INX", &NesCPU::INX, &NesCPU::IMP, 2 },
        { "SBC", &NesCPU::SBC, &NesCPU::IMM, 2 },
        { "NOP", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::SBC, &NesCPU::IMP, 2 },
        { "CPX", &NesCPU::CPX, &NesCPU::ABS, 4 },
        { "SBC", &NesCPU::SBC, &NesCPU::ABS, 4 },
        { "INC", &NesCPU::INC, &NesCPU::ABS, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "BEQ", &NesCPU::BEQ, &NesCPU::REL, 2 },
        { "SBC", &NesCPU::SBC, &NesCPU::IZY, 5 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 8 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "SBC", &NesCPU::SBC, &NesCPU::ZPX, 4 },
        { "INC", &NesCPU::INC, &NesCPU::ZPX, 6 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 6 },
        { "SED", &NesCPU::SED, &NesCPU::IMP, 2 },
        { "SBC", &NesCPU::SBC, &NesCPU::ABY, 4 },
        { "NOP", &NesCPU::NOP, &NesCPU::IMP, 2 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
        { "???", &NesCPU::NOP, &NesCPU::IMP, 4 },
        { "SBC", &NesCPU::SBC, &NesCPU::ABX, 4 },
        { "INC", &NesCPU::INC, &NesCPU::ABX, 7 },
        { "???", &NesCPU::XXX, &NesCPU::IMP, 7 },
    };
}

// This is the disassembly function. Its workings are not required for emulation.
// It is merely a convenience function to turn the binary instruction code into
// human readable form. Its included as part of the emulator because it can take
// advantage of many of the CPUs internal operations to do this.
std::map<uint16_t, std::string> NesCPU::Disassemble(uint16_t nStart, uint16_t nStop)
{
    uint32_t addr = nStart;
    uint8_t value = 0x00, lo = 0x00, hi = 0x00;
    std::map<uint16_t, std::string> mapLines;
    uint16_t line_addr = 0;

    // A convenient utility to convert variables into
    // hex strings because "modern C++"'s method with 
    // streams is atrocious

    // Starting at the specified address we read an instruction
    // uint8_t, which in turn yields information from the lookup table
    // as to how many additional uint8_ts we need to read and what the
    // addressing mode is. I need this info to assemble human readable
    // syntax, which is different depending upon the addressing mode

    // As the instruction is decoded, a std::string is assembled
    // with the readable output
    while (addr <= nStop)
    {
        line_addr = addr;

        // Prefix line with instruction address
        std::string sInst = "$" + hex(addr, 4) + ": ";

        // Read instruction, and get its readable name
        uint8_t opcode = read(addr, true);
        addr++;
        sInst += opLookup[opcode].name + " ";

        // Get oprands from desired locations, and form the
        // instruction based upon its addressing mode. These
        // routines mimmick the actual fetch routine of the
        // 6502 in order to get accurate data as part of the
        // instruction
        if (opLookup[opcode].addrmode == &NesCPU::IMP)
        {
            sInst += " {IMP}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::IMM)
        {
            value = read(addr, true); addr++;
            sInst += "#$" + hex(value, 2) + " {IMM}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::ZP0)
        {
            lo = read(addr, true); addr++;
            hi = 0x00;												
            sInst += "$" + hex(lo, 2) + " {ZP0}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::ZPX)
        {
            lo = read(addr, true); addr++;
            hi = 0x00;														
            sInst += "$" + hex(lo, 2) + ", X {ZPX}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::ZPY)
        {
            lo = read(addr, true); addr++;
            hi = 0x00;														
            sInst += "$" + hex(lo, 2) + ", Y {ZPY}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::IZX)
        {
            lo = read(addr, true); addr++;
            hi = 0x00;								
            sInst += "($" + hex(lo, 2) + ", X) {IZX}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::IZY)
        {
            lo = read(addr, true); addr++;
            hi = 0x00;								
            sInst += "($" + hex(lo, 2) + "), Y {IZY}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::ABS)
        {
            lo = read(addr, true); addr++;
            hi = read(addr, true); addr++;
            sInst += "$" + hex(((hi << 8) | lo), 4) + " {ABS}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::ABX)
        {
            lo = read(addr, true); addr++;
            hi = read(addr, true); addr++;
            sInst += "$" + hex((hi << 8) | lo, 4) + ", X {ABX}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::ABY)
        {
            lo = read(addr, true); addr++;
            hi = read(addr, true); addr++;
            sInst += "$" + hex((hi << 8) | lo, 4) + ", Y {ABY}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::IND)
        {
            lo = read(addr, true); addr++;
            hi = read(addr, true); addr++;
            sInst += "($" + hex((hi << 8) | lo, 4) + ") {IND}";
        }
        else if (opLookup[opcode].addrmode == &NesCPU::REL)
        {
            value = read(addr, true); addr++;
            sInst += "$" + hex(value, 2) + " [$" + hex(addr + value, 4) + "] {REL}";
        }

        // Add the formed string to a std::map, using the instruction's
        // address as the key. This makes it convenient to look for later
        // as the instructions are variable in length, so a straight up
        // incremental index is not sufficient.
        mapLines[line_addr] = sInst;
    }

    return mapLines;
}

void NesCPU::Reset()
{
    // Get address to set program counter to
    addr_abs = 0xFFFC;
    uint16_t lo = read(addr_abs + 0);
    uint16_t hi = read(addr_abs + 1);

    // Set it
    pc = (hi << 8) | lo;

    // Reset internal registers
    a = 0;
    x = 0;
    y = 0;
    stkp = 0xFD;
    status = 0x00 | U;

    // Clear internal helper variables
    addr_rel = 0x0000;
    addr_abs = 0x0000;
    fetched = 0x00;

    // Reset takes time
    cycles = 8;
}

uint8_t NesCPU::read(uint16_t a, bool bReadOnly)
{
    return bus->cpuRead(a, bReadOnly);
}

void NesCPU::write(uint16_t a, uint8_t d)
{
    bus->cpuWrite(a,d);
}

uint8_t NesCPU::fetch()
{
    if (!(opLookup[opcode].addrmode == &NesCPU::IMP)) {
        fetched = read(addr_abs);
    }
    return fetched;
}

void NesCPU::doBranch()
{
    cycles++;
    addr_abs = pc + addr_rel;
    if ((addr_abs & 0xFF00) != (pc & 0xFF00)) {
        cycles++;
    }
    pc = addr_abs;
}

void NesCPU::clock()
{
    // Each instruction requires a variable number of clock cycles to execute.
    // In my emulation, I only care about the final result and so I perform
    // the entire computation in one hit. In hardware, each clock cycle would
    // perform "microcode" style transformations of the CPUs state.
    //
    // To remain compliant with connected devices, it's important that the 
    // emulation also takes "time" in order to execute instructions, so I
    // implement that delay by simply counting down the cycles required by 
    // the instruction. When it reaches 0, the instruction is complete, and
    // the next one is ready to be executed.
    if (cycles == 0)
    {
        // Read next instruction uint8_t. This 8-bit value is used to index
        // the translation table to get the relevant information about
        // how to implement the instruction
        opcode = read(pc);

// #ifdef LOGMODE
//         uint16_t log_pc = pc;
// #endif
        
        // Always set the unused status flag bit to 1
        SetFlag(U, true);
        
        // Increment program counter, we read the opcode uint8_t
        pc++;

        // Get Starting number of cycles
        cycles = opLookup[opcode].cycles;

        // Perform fetch of intermmediate data using the
        // required addressing mode
        uint8_t additional_cycle1 = (this->*opLookup[opcode].addrmode)();

        // Perform operation
        uint8_t additional_cycle2 = (this->*opLookup[opcode].operate)();

        // The addressmode and opcode may have altered the number
        // of cycles this instruction requires before its completed
        cycles += additional_cycle1 & additional_cycle2;

        // Always set the unused status flag bit to 1
        SetFlag(U, true);

// #ifdef LOGMODE
//         // This logger dumps every cycle the entire processor state for analysis.
//         // This can be used for debugging the emulation, but has little utility
//         // during emulation. Its also very slow, so only use if you have to.
//         if (logfile == nullptr)	logfile = fopen("olc6502.txt", "wt");
//         if (logfile != nullptr)
//         {
//             fprintf(logfile, "%10d:%02d PC:%04X %s A:%02X X:%02X Y:%02X %s%s%s%s%s%s%s%s STKP:%02X\n",
//                 clock_count, 0, log_pc, "XXX", a, x, y,	
//                 GetFlag(N) ? "N" : ".",	GetFlag(V) ? "V" : ".",	GetFlag(U) ? "U" : ".",	
//                 GetFlag(B) ? "B" : ".",	GetFlag(D) ? "D" : ".",	GetFlag(I) ? "I" : ".",	
//                 GetFlag(Z) ? "Z" : ".",	GetFlag(C) ? "C" : ".",	stkp);
//         }
// #endif
    }
    
    // Increment global clock count - This is actually unused unless logging is enabled
    // but I've kept it in because its a handy watch variable for debugging
    clock_count++;

    // Decrement the number of cycles remaining for this instruction
    cycles--;
}

// Addressing Modes =============================================
// The 6502 has a variety of addressing modes to access data in 
// memory, some of which are direct and some are indirect (like
// pointers in C++). Each opcode contains information about which
// addressing mode should be employed to facilitate the 
// instruction, in regards to where it reads/writes the data it
// uses. The address mode changes the number of bytes that
// makes up the full instruction, so we implement addressing
// before executing the instruction, to make sure the program
// counter is at the correct location, the instruction is
// primed with the addresses it needs, and the number of clock
// cycles the instruction requires is calculated. These functions
// may adjust the number of cycles required depending upon where
// and how the memory is accessed, so they return the required
// adjustment.

// Address Mode: Implied
// There is no additional data required for this instruction. The instruction
// does something very simple like like sets a status bit. However, we will
// target the accumulator, for instructions like PHA
uint8_t NesCPU::IMP()
{
    fetched = a;
    return 0;
}

// Address Mode: Immediate
// The instruction expects the next byte to be used as a value, so we'll prep
// the read address to point to the next byte    
uint8_t NesCPU::IMM()
{
    addr_abs = pc++;
    return 0;
}

// Address Mode: Zero Page
// To save program bytes, zero page addressing allows you to absolutely address
// a location in first 0xFF bytes of address range. Clearly this only requires
// one byte instead of the usual two.
uint8_t NesCPU::ZP0()
{
    addr_abs = read(pc);
    pc++;
    addr_abs &= 0x00FF;
    return 0;
}

// Address Mode: Zero Page with X Offset
// Fundamentally the same as Zero Page addressing, but the contents of the X Register
// is added to the supplied single byte address. This is useful for iterating through
// ranges within the first page.
uint8_t NesCPU::ZPX()
{
    addr_abs = read(pc) + x;
    pc++;
    addr_abs &= 0x00FF;
    return 0;
}

// Address Mode: Zero Page with Y Offset
// Same as above but uses Y Register for offset
uint8_t NesCPU::ZPY()
{
    addr_abs = read(pc) + y;
    pc++;
    addr_abs &= 0x00FF;
    return 0;
}

// Address Mode: Relative
// This address mode is exclusive to branch instructions. The address
// must reside within -128 to +127 of the branch instruction, i.e.
// you cant directly branch to any address in the addressable range.    
uint8_t NesCPU::REL()
{
    addr_rel = read(pc);
    pc++;
    if ((addr_rel & 0x80) > 0) {
        addr_rel |= 0xFF00;
    }
    return 0;
}

// Address Mode: Absolute 
// A full 16-bit address is loaded and used
uint8_t NesCPU::ABS()
{
    uint16_t lo = read(pc++);
    uint16_t hi = read(pc++);
    addr_abs = (hi << 8) | lo;
    return 0;
}

// Address Mode: Absolute with X Offset
// Fundamentally the same as absolute addressing, but the contents of the X Register
// is added to the supplied two byte address. If the resulting address changes
// the page, an additional clock cycle is required    
uint8_t NesCPU::ABX()
{
    uint16_t lo = read(pc++);
    uint16_t hi = read(pc++);
    addr_abs = (hi << 8) | lo;
    addr_abs += x;

    if ((addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }

    return 0;
}

// Address Mode: Absolute with Y Offset
// Fundamentally the same as absolute addressing, but the contents of the Y Register
// is added to the supplied two byte address. If the resulting address changes
// the page, an additional clock cycle is required    
uint8_t NesCPU::ABY()
{
    uint16_t lo = read(pc++);
    uint16_t hi = read(pc++);
    addr_abs = (hi << 8) | lo;
    addr_abs += y;

    if ((addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }

    return 0;
}

// Note: The next 3 address modes use indirection (aka Pointers!)

// Address Mode: Indirect
// The supplied 16-bit address is read to get the actual 16-bit address. This is
// instruction is unusual in that it has a bug in the hardware! To emulate its
// function accurately, we also need to emulate this bug. If the low byte of the
// supplied address is 0xFF, then to read the high byte of the actual address
// we need to cross a page boundary. This doesnt actually work on the chip as 
// designed, instead it wraps back around in the same page, yielding an 
// invalid actual address    
uint8_t NesCPU::IND()
{
    uint16_t ptr_lo = read(pc++);
    uint16_t ptr_hi = read(pc++);
    uint16_t ptr = (ptr_hi << 8) | ptr_lo;

    if (ptr_lo == 0x00FF) //Simulate page boundary hardware bug
    {
        addr_abs = read(ptr & 0xFF00) << 8 | read(ptr);
    }
    else {
        addr_abs = read(ptr + 1) << 8 | read(ptr);
    }

    return 0;
}

// Address Mode: Indirect X
// The supplied 8-bit address is offset by X Register to index
// a location in page 0x00. The actual 16-bit address is read 
// from this location
uint8_t NesCPU::IZX()
{
    uint16_t t = read(pc++);
    uint16_t lo = read((t + x) & 0x00FF);
    uint16_t hi = read((t + x + 1) & 0x00FF);
    addr_abs = (hi << 8) | lo;
    return 0;
}

// Address Mode: Indirect Y
// The supplied 8-bit address indexes a location in page 0x00. From 
// here the actual 16-bit address is read, and the contents of
// Y Register is added to it to offset it. If the offset causes a
// change in page then an additional clock cycle is required.    
uint8_t NesCPU::IZY()
{
    uint16_t t = read(pc++);
    uint16_t lo = read(t & 0x00FF);
    uint16_t hi = read((t + 1) & 0x00FF);
    addr_abs = (hi << 8) | lo;
    addr_abs += y;
    if ((addr_abs & 0xFF00) != (hi << 8)) {
        return 1;
    }
    return 0;
}

// Opcodes ======================================================
// There are 56 "legitimate" opcodes provided by the 6502 CPU. I
// have not modelled "unofficial" opcodes. As each opcode is 
// defined by 1 byte, there are potentially 256 possible codes.
// Codes are not used in a "switch case" style on a processor,
// instead they are repsonisble for switching individual parts of
// CPU circuits on and off. The opcodes listed here are official, 
// meaning that the functionality of the chip when provided with
// these codes is as the developers intended it to be. Unofficial
// codes will of course also influence the CPU circuitry in 
// interesting ways, and can be exploited to gain additional
// functionality!
//
// These functions return 0 normally, but some are capable of
// requiring more clock cycles when executed under certain
// conditions combined with certain addressing modes. If that is 
// the case, they return 1.
//
// I have included detailed explanations of each function in 
// the class implementation file. Note they are listed in
// alphabetical order here for ease of finding.

///////////////////////////////////////////////////////////////////////////////
// INSTRUCTION IMPLEMENTATIONS

// Note: Ive started with the two most complicated instructions to emulate, which
// ironically is addition and subtraction! Ive tried to include a detailed 
// explanation as to why they are so complex, yet so fundamental. Im also NOT
// going to do this through the explanation of 1 and 2's complement.

// Instruction: Add with Carry In
// Function:    A = A + M + C
// Flags Out:   C, V, N, Z
//
// Explanation:
// The purpose of this function is to add a value to the accumulator and a carry bit. If
// the result is > 255 there is an overflow setting the carry bit. Ths allows you to
// chain together ADC instructions to add numbers larger than 8-bits. This in itself is
// simple, however the 6502 supports the concepts of Negativity/Positivity and Signed Overflow.
//
// 10000100 = 128 + 4 = 132 in normal circumstances, we know this as unsigned and it allows
// us to represent numbers between 0 and 255 (given 8 bits). The 6502 can also interpret 
// this word as something else if we assume those 8 bits represent the range -128 to +127,
// i.e. it has become signed.
//
// Since 132 > 127, it effectively wraps around, through -128, to -124. This wraparound is
// called overflow, and this is a useful to know as it indicates that the calculation has
// gone outside the permissable range, and therefore no longer makes numeric sense.
//
// Note the implementation of ADD is the same in binary, this is just about how the numbers
// are represented, so the word 10000100 can be both -124 and 132 depending upon the 
// context the programming is using it in. We can prove this!
//
//  10000100 =  132  or  -124
// +00010001 = + 17      + 17
//  ========    ===       ===     See, both are valid additions, but our interpretation of
//  10010101 =  149  or  -107     the context changes the value, not the hardware!
//
// In principle under the -128 to 127 range:
// 10000000 = -128, 11111111 = -1, 00000000 = 0, 00000000 = +1, 01111111 = +127
// therefore negative numbers have the most significant set, positive numbers do not
//
// To assist us, the 6502 can set the overflow flag, if the result of the addition has
// wrapped around. V <- ~(A^M) & A^(A+M+C) :D lol, let's work out why!
//
// Let's suppose we have A = 30, M = 10 and C = 0
//          A = 30 = 00011110
//          M = 10 = 00001010+
//     RESULT = 40 = 00101000
//
// Here we have not gone out of range. The resulting significant bit has not changed.
// So let's make a truth table to understand when overflow has occurred. Here I take
// the MSB of each component, where R is RESULT.
//
// A  M  R | V | A^R | A^M |~(A^M) | 
// 0  0  0 | 0 |  0  |  0  |   1   |
// 0  0  1 | 1 |  1  |  0  |   1   |
// 0  1  0 | 0 |  0  |  1  |   0   |
// 0  1  1 | 0 |  1  |  1  |   0   |  so V = ~(A^M) & (A^R)
// 1  0  0 | 0 |  1  |  1  |   0   |
// 1  0  1 | 0 |  0  |  1  |   0   |
// 1  1  0 | 1 |  1  |  0  |   1   |
// 1  1  1 | 0 |  0  |  0  |   1   |
//
// We can see how the above equation calculates V, based on A, M and R. V was chosen
// based on the following hypothesis:
//       Positive Number + Positive Number = Negative Result -> Overflow
//       Negative Number + Negative Number = Positive Result -> Overflow
//       Positive Number + Negative Number = Either Result -> Cannot Overflow
//       Positive Number + Positive Number = Positive Result -> OK! No Overflow
//       Negative Number + Negative Number = Negative Result -> OK! NO Overflow

uint8_t NesCPU::ADC()
{
    fetch();
    temp = a + fetched + GetFlag(C);

    SetFlag(C, temp > 255);
    SetFlag(Z, (temp & 0x00FF) == 0);
    SetFlag(V, (((~(a^fetched)) & (a^temp)) & 0x0080) > 0);
    SetFlag(N, (temp & 0x80) > 0);
    a = temp & 0x00FF;
    return 1;
}

// Instruction: Bitwise Logic AND
// Function:    A = A & M
// Flags Out:   N, Z   
uint8_t NesCPU::AND()
{
    fetch();
    a = a & fetched;
    SetFlag(Z, a == 0x00);
    SetFlag(N, a & 0x80);
    return 1;
}

// Instruction: Arithmetic Shift Left
// Function:    A = C <- (A << 1) <- 0
// Flags Out:   N, Z, C
uint8_t NesCPU::ASL()
{
    fetch();
    temp = fetched << 1;
    SetFlag(C, (temp & 0xFF00) > 0);
    SetFlag(Z, (temp & 0x00FF) == 0);
    SetFlag(N, (temp & 0x80) > 0);
    if (opLookup[opcode].addrmode == &NesCPU::IMP) {
        a = temp & 0x00FF;
    } else {
        write(addr_abs, temp & 0x00FF);
    }
    return 0;
}

// Instruction: Branch if Carry Clear
// Function:    if(C == 0) pc = address 
uint8_t NesCPU::BCC()
{
    if (GetFlag(C) == 0) {
        doBranch();
    }
    return 0;
}

// Instruction: Branch if Carry Set
// Function:    if(C == 1) pc = address
uint8_t NesCPU::BCS()
{
    if (GetFlag(C) == 1) {
        doBranch();
    }
    return 0;
}

// Instruction: Branch if Equal
// Function:    if(Z == 1) pc = address    
uint8_t NesCPU::BEQ()
{
    if (GetFlag(Z) == 1) {
        doBranch();
    }
    return 0;
}
uint8_t NesCPU::BIT()
{
    fetch();
    temp = a & fetched;
    SetFlag(Z, (temp & 0x00FF) == 0);
    SetFlag(N, (fetched & (1 << 7)) > 0);
    SetFlag(V, (fetched & (1 << 6)) > 0);
    return 0;
}

// Instruction: Branch if Negative
// Function:    if(N == 1) pc = address
uint8_t NesCPU::BMI()
{
    if (GetFlag(N) == 1) {
        doBranch();
    }
    return 0;
}

// Instruction: Branch if Not Equal
// Function:    if(Z == 0) pc = address
uint8_t NesCPU::BNE()
{
    if (GetFlag(Z) == 0) {
        doBranch();
    }
    return 0;
}

// Instruction: Branch if Positive
// Function:    if(N == 0) pc = address
uint8_t NesCPU::BPL()
{
    if (GetFlag(N) == 0) {
        doBranch();
    }
    return 0;
}

// Instruction: Break
// Function:    Program Sourced Interrupt
uint8_t NesCPU::BRK()
{
    pc++;
    SetFlag(I, true);
    write(0x0100 + stkp, pc >> 8);
    stkp--;
    write(0x0100 + stkp, pc & 0x00FF);
    stkp--;

    SetFlag(B, true);
    write(0x0100 + stkp, status);
    stkp--;
    SetFlag(B, false);

    pc = read(0xFFFE) | (read(0xFFFF) << 8);
    return 0;
}

// Instruction: Branch if Overflow Clear
// Function:    if(V == 0) pc = address
uint8_t NesCPU::BVC()
{
    if (GetFlag(V) == 0) {
        doBranch();
    }
    return 0;
}

// Instruction: Branch if Overflow Set
// Function:    if(V == 1) pc = address
uint8_t NesCPU::BVS()
{
    if (GetFlag(V) == 1) {
        doBranch();
    }
    return 0;
}

// Instruction: Clear Carry Flag
// Function:    C = 0
uint8_t NesCPU::CLC()
{
    SetFlag(C, false);
    return 0;
}

// Instruction: Clear Decimal Flag
// Function:    D = 0
uint8_t NesCPU::CLD()
{
    SetFlag(D, false);
    return 0;
}

// Instruction: Disable Interrupts / Clear Interrupt Flag
// Function:    I = 0
uint8_t NesCPU::CLI()
{
    SetFlag(I, false);
    return 0;
}

// Instruction: Clear Overflow Flag
// Function:    V = 0
uint8_t NesCPU::CLV()
{
    SetFlag(V, false);
    return 0;
}

// Instruction: Compare Accumulator
// Function:    C <- A >= M      Z <- (A - M) == 0
// Flags Out:   N, C, Z
uint8_t NesCPU::CMP()
{
    fetch();
    temp = a - fetched;
    SetFlag(C, a >= fetched);
    SetFlag(Z, temp == 0);
    SetFlag(N, (temp & 0x0080) > 0);
    return 1;
}

// Instruction: Compare X Register
// Function:    C <- X >= M      Z <- (X - M) == 0
// Flags Out:   N, C, Z
uint8_t NesCPU::CPX()
{
    fetch();
    temp = x - fetched;
    SetFlag(C, x >= fetched);
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x0080);
    return 0;
}

// Instruction: Compare Y Register
// Function:    C <- Y >= M      Z <- (Y - M) == 0
// Flags Out:   N, C, Z
uint8_t NesCPU::CPY()
{
    fetch();
    temp = y - fetched;
    SetFlag(C, y >= fetched);
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x0080);
    return 0;
}

// Instruction: Decrement Value at Memory Location
// Function:    M = M - 1
// Flags Out:   N, Z
uint8_t NesCPU::DEC()
{
    fetch();
    temp = fetched - 1;
    write(addr_abs, temp & 0x00FF);
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x0080);
    return 0;
}

// Instruction: Decrement X Register
// Function:    X = X - 1
// Flags Out:   N, Z
uint8_t NesCPU::DEX()
{
    x--;
    SetFlag(Z, x == 0);
    SetFlag(N, x & 0x80);
    return 0;
}

// Instruction: Decrement Y Register
// Function:    Y = Y - 1
// Flags Out:   N, Z    
uint8_t NesCPU::DEY()
{
    y--;
    SetFlag(Z, y == 0);
    SetFlag(N, y & 0x80);
    return 0;
}

// Instruction: Bitwise Logic XOR
// Function:    A = A xor M
// Flags Out:   N, Z
uint8_t NesCPU::EOR()
{
    fetch();
    a = a ^ fetched;
    SetFlag(Z, a == 0);
    SetFlag(N, a & 0x80);
    return 1;
}

// Instruction: Increment Value at Memory Location
// Function:    M = M + 1
// Flags Out:   N, Z
uint8_t NesCPU::INC()
{
    fetch();
    temp = fetched + 1;
    write(addr_abs, temp & 0x00FF);
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x0080);
    return 0;
}

// Instruction: Increment X Register
// Function:    X = X + 1
// Flags Out:   N, Z
uint8_t NesCPU::INX()
{
    x++;
    SetFlag(Z, x == 0);
    SetFlag(N, x & 0x80);
    return 0;
}

// Instruction: Increment Y Register
// Function:    Y = Y + 1
// Flags Out:   N, Z
uint8_t NesCPU::INY()
{
    y++;
    SetFlag(Z, y == 0);
    SetFlag(N, y & 0x80);
    return 0;
}

// Instruction: Jump To Location
// Function:    pc = address
uint8_t NesCPU::JMP()
{
    pc = addr_abs;
    return 0;
}

// Instruction: Jump To Sub-Routine
// Function:    Push current pc to stack, pc = address
uint8_t NesCPU::JSR()
{
    pc--;
    write(0x0100 + stkp, pc >> 8);
    stkp--;
    write(0x0100 + stkp, pc & 0x00FF);
    stkp--;
    
    pc = addr_abs;
    return 0;
}

// Instruction: Load The Accumulator
// Function:    A = M
// Flags Out:   N, Z
uint8_t NesCPU::LDA()
{
    fetch();
    a = fetched;
    SetFlag(Z, a == 0);
    SetFlag(N, a & 0x80);
    return 1;
}

// Instruction: Load The X Register
// Function:    X = M
// Flags Out:   N, Z
uint8_t NesCPU::LDX()
{
    fetch();
    x = fetched;
    SetFlag(Z, x == 0);
    SetFlag(N, x & 0x80);
    return 1;
}

// Instruction: Load The Y Register
// Function:    Y = M
// Flags Out:   N, Z
uint8_t NesCPU::LDY()
{
    fetch();
    y = fetched;
    SetFlag(Z, y == 0);
    SetFlag(N, y & 0x80);
    return 1;
}
uint8_t NesCPU::LSR()
{
    fetch();
    SetFlag(C, fetched & 0x01);
    temp = fetched >> 1;
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x0080);
    if (opLookup[opcode].addrmode == &NesCPU::IMP) {
        a = temp & 0x00FF;
    } else {
        write(addr_abs, temp & 0x00FF);
    }
    return 0;
}
uint8_t NesCPU::NOP()
{
    // Sadly not all NOPs are equal, Ive added a few here
    // based on https://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
    // and will add more based on game compatibility, and ultimately
    // I'd like to cover all illegal opcodes too
    switch (opcode) {
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC:
        return 1;
    }
    return 0;
}

// Instruction: Bitwise Logic OR
// Function:    A = A | M
// Flags Out:   N, Z
uint8_t NesCPU::ORA()
{
    fetch();
    a = a | fetched;
    SetFlag(Z, a == 0);
    SetFlag(N, a & 0x80);
    return 1;
}

// Instruction: Push Accumulator to Stack
// Function:    A -> stack
uint8_t NesCPU::PHA()
{
    write(0x0100 + stkp, a);
    stkp--;
    return 0;
}

// Instruction: Push Status Register to Stack
// Function:    status -> stack
// Note:        Break flag is set to 1 before push
uint8_t NesCPU::PHP()
{
    write(0x0100 + stkp, status | B | U);
    SetFlag(B, false);
    SetFlag(U, false);
    stkp--;
    return 0;
}

// Instruction: Pop Accumulator off Stack
// Function:    A <- stack
// Flags Out:   N, Z
uint8_t NesCPU::PLA()
{
    stkp++;
    a = read(0x0100 + stkp);
    SetFlag(Z, a == 0);
    SetFlag(N, a & 0x80);
    return 0;
}

// Instruction: Pop Status Register off Stack
// Function:    Status <- stack    
uint8_t NesCPU::PLP()
{
    stkp++;
    status = read(0x0100 + stkp);
    SetFlag(U, true);
    return 0;
}
uint8_t NesCPU::ROL()
{
    fetch();
    temp = (fetched << 1) | GetFlag(C);
    SetFlag(C, temp & 0xFF00);
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x0080);
    if (opLookup[opcode].addrmode == &NesCPU::IMP) {
        a = temp & 0x00FF;
    } else {
        write(addr_abs, temp & 0x00FF);
    }
    return 0;
}
uint8_t NesCPU::ROR()
{
    fetch();
    temp = (fetched >> 1) | (GetFlag(C) << 7);
    SetFlag(C, fetched & 0x01);
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x0080);
    if (opLookup[opcode].addrmode == &NesCPU::IMP) {
        a = temp & 0x00FF;
    } else {
        write(addr_abs, temp & 0x00FF);
    }
    return 0;
}
uint8_t NesCPU::RTI()
{
    stkp++;
    status = read(0x0100 + stkp);
    status &= ~B;
    status &= ~U;
    stkp++;
    pc = read(0x0100 + stkp);
    stkp++;
    pc |= read(0x0100 + stkp) << 8;
    return 0;
}
uint8_t NesCPU::RTS()
{
    stkp++;
    pc = read(0x0100 + stkp);
    stkp++;
    pc |= read((0x0100 + stkp)) << 8;
    pc++;
    return 0;
}

// Instruction: Subtraction with Borrow In
// Function:    A = A - M - (1 - C)
// Flags Out:   C, V, N, Z
//
// Explanation:
// Given the explanation for ADC above, we can reorganise our data
// to use the same computation for addition, for subtraction by multiplying
// the data by -1, i.e. make it negative
//
// A = A - M - (1 - C)  ->  A = A + -1 * (M - (1 - C))  ->  A = A + (-M + 1 + C)
//
// To make a signed positive number negative, we can invert the bits and add 1
// (OK, I lied, a little bit of 1 and 2s complement :P)
//
//  5 = 00000101
// -5 = 11111010 + 00000001 = 11111011 (or 251 in our 0 to 255 range)
//
// The range is actually unimportant, because if I take the value 15, and add 251
// to it, given we wrap around at 256, the result is 10, so it has effectively 
// subtracted 5, which was the original intention. (15 + 251) % 256 = 10
//
// Note that the equation above used (1-C), but this got converted to + 1 + C.
// This means we already have the +1, so all we need to do is invert the bits
// of M, the data(!) therfore we can simply add, exactly the same way we did 
// before.

uint8_t NesCPU::SBC()
{
    fetch();
    uint16_t value = fetched ^ 0x00FF;
    
    temp = a + value + GetFlag(C);
    SetFlag(C, temp & 0xFF00);
    SetFlag(Z, temp == 0);
    SetFlag(V, (temp ^ a) & (temp ^ value) & 0x0080);
    SetFlag(N, temp & 0x0080);
    a = temp & 0x00FF;
    return 1;
}

// Instruction: Set Carry Flag
// Function:    C = 1
uint8_t NesCPU::SEC()
{
    SetFlag(C, true);
    return 0;
}

// Instruction: Set Decimal Flag
// Function:    D = 1
uint8_t NesCPU::SED()
{
    SetFlag(D, true);
    return 0;
}

// Instruction: Set Interrupt Flag / Enable Interrupts
// Function:    I = 1
uint8_t NesCPU::SEI()
{
    SetFlag(I, true);
    return 0;
}

// Instruction: Store Accumulator at Address
// Function:    M = A
uint8_t NesCPU::STA()
{
    write(addr_abs, a);
    return 0;
}

// Instruction: Store X Register at Address
// Function:    M = X
uint8_t NesCPU::STX()
{
    write(addr_abs, x);
    return 0;
}

// Instruction: Store Y Register at Address
// Function:    M = Y
uint8_t NesCPU::STY()
{
    write(addr_abs, y);
    return 0;
}

// Instruction: Transfer Accumulator to X Register
// Function:    X = A
// Flags Out:   N, Z
uint8_t NesCPU::TAX()
{
    x = a;
    SetFlag(Z, x == 0);
    SetFlag(N, x & 0x80);
    return 0;
}
uint8_t NesCPU::TAY()
{
    y = a;
    SetFlag(Z, y == 0);
    SetFlag(N, y & 0x80);
    return 0;
}

// Instruction: Transfer Stack Pointer to X Register
// Function:    X = stack pointer
// Flags Out:   N, Z
uint8_t NesCPU::TSX()
{
    x = stkp;
    SetFlag(Z, x == 0);
    SetFlag(N, x & 0x80);
    return 0;
}

// Instruction: Transfer X Register to Accumulator
// Function:    A = X
// Flags Out:   N, Z
uint8_t NesCPU::TXA()
{
    a = x;
    SetFlag(Z, a == 0);
    SetFlag(N, a & 0x80);
    return 0;
}


// Instruction: Transfer X Register to Stack Pointer
// Function:    stack pointer = X
uint8_t NesCPU::TXS()
{
    stkp = x;
    return 0;
}

// Instruction: Transfer Y Register to Accumulator
// Function:    A = Y
// Flags Out:   N, Z
uint8_t NesCPU::TYA()
{
    a = y;
    SetFlag(Z, a == 0);
    SetFlag(N, a & 0x80);
    return 0;
}
// I capture all "unofficial" opcodes with this function. It is
// functionally identical to a NOP
uint8_t NesCPU::XXX()
{
    return 0;
}

void NesCPU::nmi()
{
    write(0x0100 + stkp, (pc >> 8) & 0x00FF);
    stkp--;
    write(0x0100 + stkp, pc & 0x00FF);
    stkp--;

    SetFlag(B, 0);
    SetFlag(U, 1);
    SetFlag(I, 1);
    write(0x0100 + stkp, status);
    stkp--;

    addr_abs = 0xFFFA;
    uint16_t lo = read(addr_abs + 0);
    uint16_t hi = read(addr_abs + 1);
    pc = (hi << 8) | lo;

    cycles = 8;
}