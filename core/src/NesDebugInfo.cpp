#include "NesDebugInfo.h"
#include "imgui.h"
#include "Util/Hex.h"
#include <string>
#include "Nes.h"
#include "NesBus.h"

void DrawRam(NesBus* bus, int nAddr, int nRows, int nColumns)
{
    for (int row = 0; row < nRows; row++)
    {
        std::string sOffset = "$" + hex(nAddr, 4) + ":";
        for (int col = 0; col < nColumns; col++)
        {
            uint8_t v = bus->cpuRead(nAddr, true);
            sOffset += " " + hex(v, 2);
            nAddr += 1;
        }
        ImGui::Text(sOffset.c_str());
    }
}

void DrawVram(NesBus* bus, int nAddr, int nRows, int nColumns)
{
    for (int row = 0; row < nRows; row++)
    {
        std::string sOffset = "$" + hex(nAddr,4) + ":";
        for (int col = 0; col < nColumns; col++)
        {
            uint8_t v = bus->ppu->ppuRead(nAddr, true);
            sOffset += " " + hex(v,2);
            nAddr += 1;
        }
        ImGui::Text(sOffset.c_str());
    }
}

void DrawCpu(NesBus* bus)
{
    NesCPU* cpu = bus->cpu;
    ImVec4 colGreen(0,1,0,1), colRed(1,0,0,1);
    ImGui::Text("STATUS: ");
    ImGui::SameLine();
    ImGui::TextColored(cpu->GetFlag(NesCPU::FLAGS6502::N) > 0 ? colGreen : colRed, "N ");
    ImGui::SameLine();
    ImGui::TextColored(cpu->GetFlag(NesCPU::FLAGS6502::V) > 0 ? colGreen : colRed, "V ");
    ImGui::SameLine();
    ImGui::TextColored(cpu->GetFlag(NesCPU::FLAGS6502::U) > 0 ? colGreen : colRed, "U ");
    ImGui::SameLine();
    ImGui::TextColored(cpu->GetFlag(NesCPU::FLAGS6502::B) > 0 ? colGreen : colRed, "B ");
    ImGui::SameLine();
    ImGui::TextColored(cpu->GetFlag(NesCPU::FLAGS6502::D) > 0 ? colGreen : colRed, "D ");
    ImGui::SameLine();
    ImGui::TextColored(cpu->GetFlag(NesCPU::FLAGS6502::I) > 0 ? colGreen : colRed, "I ");
    ImGui::SameLine();
    ImGui::TextColored(cpu->GetFlag(NesCPU::FLAGS6502::Z) > 0 ? colGreen : colRed, "Z ");
    ImGui::SameLine();
    ImGui::TextColored(cpu->GetFlag(NesCPU::FLAGS6502::C) > 0 ? colGreen : colRed, "C ");
    ImGui::Text("PC: %04x", cpu->pc);
    ImGui::Text("A: %02x", cpu->a);
    ImGui::Text("X: %02x", cpu->x);
    ImGui::Text("Y: %02x", cpu->y);
    ImGui::Text("STKP: %04x", cpu->stkp);
}


void ImGuiNesDebug(Nes* nes)
{
    ImGui::Begin("Nes Debug Info");
    DrawRam(nes->bus, 0x0000, 16, 16);
    DrawVram(nes->bus, 0x2000, 16, 16);
    DrawCpu(nes->bus);

    ImGui::Text("Audio: %f", nes->bus->apu->GetOutputSample());

    ImGui::End();
}