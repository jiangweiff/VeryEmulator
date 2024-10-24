#include "NesRom.h"

class Mapper_000 : public Mapper
{
public:
	Mapper_000(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks){}
	~Mapper_000(){}

public:
	bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override
    {
      	// if PRGROM is 16KB
        //     CPU Address Bus          PRG ROM
        //     0x8000 -> 0xBFFF: Map    0x0000 -> 0x3FFF
        //     0xC000 -> 0xFFFF: Mirror 0x0000 -> 0x3FFF
        // if PRGROM is 32KB
        //     CPU Address Bus          PRG ROM
        //     0x8000 -> 0xFFFF: Map    0x0000 -> 0x7FFF	
        if (addr >= 0x8000 && addr <= 0xFFFF)
        {
            mapped_addr = addr & (nPRGBanks > 1 ? 0x7FFF : 0x3FFF);
            return true;
        }

        return false;
    }

	bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data = 0) override
    {
        if (addr >= 0x8000 && addr <= 0xFFFF)
        {
            mapped_addr = addr & (nPRGBanks > 1 ? 0x7FFF : 0x3FFF);
            return true;
        }

        return false;        
    }

	bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override
    {
        // There is no mapping required for PPU
        // PPU Address Bus          CHR ROM
        // 0x0000 -> 0x1FFF: Map    0x0000 -> 0x1FFF
        if (addr >= 0x0000 && addr <= 0x1FFF)
        {
            mapped_addr = addr;
            return true;
        }

        return false;
    }

	bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override
    {
        if (addr >= 0x0000 && addr <= 0x1FFF)
        {
            if (nCHRBanks == 0)
            {
                // Treat as RAM
                mapped_addr = addr;
                return true;
            }
        }

        return false;
    }

	void reset() override
    {
    }
};


class Mapper_001 : public Mapper
{
public:
	Mapper_001(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks)
    {
       	vRAMStatic.resize(32 * 1024);
    }
	~Mapper_001(){}

	bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override
    {
        if (addr >= 0x6000 && addr <= 0x7FFF)
        {
            // Read is from static ram on cartridge
            mapped_addr = 0xFFFFFFFF;

            // Read data from RAM
            data = vRAMStatic[addr & 0x1FFF];

            // Signal mapper has handled request
            return true;
        }

        if (addr >= 0x8000)
        {
            if (nControlRegister & 0b01000)
            {
                // 16K Mode
                if (addr >= 0x8000 && addr <= 0xBFFF)
                {
                    mapped_addr = nPRGBankSelect16Lo * 0x4000 + (addr & 0x3FFF);
                    return true;
                }

                if (addr >= 0xC000 && addr <= 0xFFFF)
                {
                    mapped_addr = nPRGBankSelect16Hi * 0x4000 + (addr & 0x3FFF);
                    return true;
                }
            }
            else
            {
                // 32K Mode
                mapped_addr = nPRGBankSelect32 * 0x8000 + (addr & 0x7FFF);
                return true;
            }
        }

        return false;
    }

	bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data = 0) override
    {
        if (addr >= 0x6000 && addr <= 0x7FFF)
        {
            // Write is to static ram on cartridge
            mapped_addr = 0xFFFFFFFF;

            // Write data to RAM
            vRAMStatic[addr & 0x1FFF] = data;

            // Signal mapper has handled request
            return true;
        }

        if (addr >= 0x8000)
        {
            if (data & 0x80)
            {
                // MSB is set, so reset serial loading
                nLoadRegister = 0x00;
                nLoadRegisterCount = 0;
                nControlRegister = nControlRegister | 0x0C;
            }
            else
            {
                // Load data in serially into load register
                // It arrives LSB first, so implant this at
                // bit 5. After 5 writes, the register is ready
                nLoadRegister >>= 1;
                nLoadRegister |= (data & 0x01) << 4;
                nLoadRegisterCount++;

                if (nLoadRegisterCount == 5)
                {
                    // Get Mapper Target Register, by examining
                    // bits 13 & 14 of the address
                    uint8_t nTargetRegister = (addr >> 13) & 0x03;

                    if (nTargetRegister == 0) // 0x8000 - 0x9FFF
                    {
                        // Set Control Register
                        nControlRegister = nLoadRegister & 0x1F;

                        switch (nControlRegister & 0x03)
                        {
                        case 0:	mirrormode = ONESCREEN_LO; break;
                        case 1: mirrormode = ONESCREEN_HI; break;
                        case 2: mirrormode = VERTICAL;     break;
                        case 3:	mirrormode = HORIZONTAL;   break;
                        }
                    }
                    else if (nTargetRegister == 1) // 0xA000 - 0xBFFF
                    {
                        // Set CHR Bank Lo
                        if (nControlRegister & 0b10000) 
                        {
                            // 4K CHR Bank at PPU 0x0000
                            nCHRBankSelect4Lo = nLoadRegister & 0x1F;
                        }
                        else
                        {
                            // 8K CHR Bank at PPU 0x0000
                            nCHRBankSelect8 = nLoadRegister & 0x1E;
                        }
                    }
                    else if (nTargetRegister == 2) // 0xC000 - 0xDFFF
                    {
                        // Set CHR Bank Hi
                        if (nControlRegister & 0b10000)
                        {
                            // 4K CHR Bank at PPU 0x1000
                            nCHRBankSelect4Hi = nLoadRegister & 0x1F;
                        }
                    }
                    else if (nTargetRegister == 3) // 0xE000 - 0xFFFF
                    {
                        // Configure PRG Banks
                        uint8_t nPRGMode = (nControlRegister >> 2) & 0x03;

                        if (nPRGMode == 0 || nPRGMode == 1)
                        {
                            // Set 32K PRG Bank at CPU 0x8000
                            nPRGBankSelect32 = (nLoadRegister & 0x0E) >> 1;
                        }
                        else if (nPRGMode == 2)
                        {
                            // Fix 16KB PRG Bank at CPU 0x8000 to First Bank
                            nPRGBankSelect16Lo = 0;
                            // Set 16KB PRG Bank at CPU 0xC000
                            nPRGBankSelect16Hi = nLoadRegister & 0x0F;
                        }
                        else if (nPRGMode == 3)
                        {
                            // Set 16KB PRG Bank at CPU 0x8000
                            nPRGBankSelect16Lo = nLoadRegister & 0x0F;
                            // Fix 16KB PRG Bank at CPU 0xC000 to Last Bank
                            nPRGBankSelect16Hi = nPRGBanks - 1;
                        }
                    }

                    // 5 bits were written, and decoded, so
                    // reset load register
                    nLoadRegister = 0x00;
                    nLoadRegisterCount = 0;
                }

            }

        }

        // Mapper has handled write, but do not update ROMs
        return false;
    }

	bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override
    {
        if (addr < 0x2000)
        {
            if (nCHRBanks == 0)
            {
                mapped_addr = addr;
                return true;
            }
            else
            {
                if (nControlRegister & 0b10000)
                {
                    // 4K CHR Bank Mode
                    if (addr >= 0x0000 && addr <= 0x0FFF)
                    {
                        mapped_addr = nCHRBankSelect4Lo * 0x1000 + (addr & 0x0FFF);
                        return true;
                    }

                    if (addr >= 0x1000 && addr <= 0x1FFF)
                    {
                        mapped_addr = nCHRBankSelect4Hi * 0x1000 + (addr & 0x0FFF);
                        return true;
                    }
                }
                else
                {
                    // 8K CHR Bank Mode
                    mapped_addr = nCHRBankSelect8 * 0x2000 + (addr & 0x1FFF);
                    return true;
                }
            }
        }	

        return false;        
    }
	bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override
    {
        if (addr < 0x2000)
        {
            if (nCHRBanks == 0)
            {
                mapped_addr = addr;
                return true;
            }

            return true;
        }
        else
            return false;        
    }
	void reset() override
    {
        nControlRegister = 0x1C;
        nLoadRegister = 0x00;
        nLoadRegisterCount = 0x00;
        
        nCHRBankSelect4Lo = 0;
        nCHRBankSelect4Hi = 0;
        nCHRBankSelect8 = 0;

        nPRGBankSelect32 = 0;
        nPRGBankSelect16Lo = 0;
        nPRGBankSelect16Hi = nPRGBanks - 1;
    }
	MIRROR mirror()
    {
       	return mirrormode;
    }

private:
	uint8_t nCHRBankSelect4Lo = 0x00;
	uint8_t nCHRBankSelect4Hi = 0x00;
	uint8_t nCHRBankSelect8 = 0x00;

	uint8_t nPRGBankSelect16Lo = 0x00;
	uint8_t nPRGBankSelect16Hi = 0x00;
	uint8_t nPRGBankSelect32 = 0x00;

	uint8_t nLoadRegister = 0x00;
	uint8_t nLoadRegisterCount = 0x00;
	uint8_t nControlRegister = 0x00;

	MIRROR mirrormode = MIRROR::HORIZONTAL;

	std::vector<uint8_t> vRAMStatic;
};

class Mapper_002 : public Mapper
{
public:
	Mapper_002(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks)
    {
        nPRGBankSelectLo = 0;
        nPRGBankSelectHi = nPRGBanks-1;
    }
	~Mapper_002()
    {

    }

	bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override
    {
        if (addr >= 0x8000 && addr <= 0xBFFF)
        {
            mapped_addr = nPRGBankSelectLo * 0x4000 + (addr & 0x3FFF);
            return true;
        }

        if (addr >= 0xC000 && addr <= 0xFFFF)
        {
            mapped_addr = nPRGBankSelectHi * 0x4000 + (addr & 0x3FFF);
            return true;
        }
        
        return false;        
    }
	bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data = 0) override
    {
        if (addr >= 0x8000 && addr <= 0xFFFF)
        {		
            nPRGBankSelectLo = data & 0x0F;
            return true;
        }

        // Mapper has handled write, but do not update ROMs
        return false;        
    }
	bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override
    {
        if (addr >= 0x0000 && addr < 0x2000)
        {
            mapped_addr = addr;
            return true;
        }
        else
            return false;        
    }
	bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override
    {
        if (addr >= 0x0000 && addr < 0x2000)
        {
            if (nCHRBanks == 0) // Treating as RAM
            {
                mapped_addr = addr;
                return true;
            }
        }
        return false;        
    }
	void reset() override
    {
        nPRGBankSelectLo = 0;
        nPRGBankSelectHi = nPRGBanks - 1;
    }

private:
	uint8_t nPRGBankSelectLo = 0x00;
	uint8_t nPRGBankSelectHi = 0x00;
};

class Mapper_003 : public Mapper
{
public:
	Mapper_003(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks)
    {

    }
	~Mapper_003()
    {

    }

	bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override
    {
        if (addr >= 0x8000 && addr <= 0xFFFF)
        {
            if (nPRGBanks == 1) // 16K ROM 
                mapped_addr = addr & 0x3FFF;
            if (nPRGBanks == 2) // 32K ROM
                mapped_addr = addr & 0x7FFF;
            return true;
        }
        else
            return false;
    }

	bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data = 0) override
    {
        if (addr >= 0x8000 && addr <= 0xFFFF)
        {
            nCHRBankSelect = data & 0x03;
            mapped_addr = addr;		
        }

        // Mapper has handled write, but do not update ROMs
        return false;
    }
	bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override
    {
        if (addr < 0x2000)
        {
            mapped_addr = nCHRBankSelect * 0x2000 + addr;
            return true;
        }
        else
            return false;        
    }
	bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override
    {
	    return false;
    }
	void reset() override
    {
        nCHRBankSelect = 0;
    }

private:
	uint8_t nCHRBankSelect = 0x00;
};


class Mapper_004 : public Mapper
{
public:
	Mapper_004(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks)
    {

    }
	~Mapper_004()
    {

    }

	bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override
    {
        if (addr >= 0x6000 && addr <= 0x7FFF)
        {
            // Write is to static ram on cartridge
            mapped_addr = 0xFFFFFFFF;

            // Write data to RAM
            data = vRAMStatic[addr & 0x1FFF];

            // Signal mapper has handled request
            return true;
        }


        if (addr >= 0x8000 && addr <= 0x9FFF)
        {
            mapped_addr = pPRGBank[0] + (addr & 0x1FFF);
            return true;
        }

        if (addr >= 0xA000 && addr <= 0xBFFF)
        {
            mapped_addr = pPRGBank[1] + (addr & 0x1FFF);
            return true;
        }

        if (addr >= 0xC000 && addr <= 0xDFFF)
        {
            mapped_addr = pPRGBank[2] + (addr & 0x1FFF);
            return true;
        }

        if (addr >= 0xE000 && addr <= 0xFFFF)
        {
            mapped_addr = pPRGBank[3] + (addr & 0x1FFF);
            return true;
        }

        return false;
    }

	bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data = 0) override
    {
        if (addr >= 0x6000 && addr <= 0x7FFF)
        {
            // Write is to static ram on cartridge
            mapped_addr = 0xFFFFFFFF;

            // Write data to RAM
            vRAMStatic[addr & 0x1FFF] = data;

            // Signal mapper has handled request
            return true;
        }

        if (addr >= 0x8000 && addr <= 0x9FFF)
        {
            // Bank Select
            if (!(addr & 0x0001))
            {
                nTargetRegister = data & 0x07;
                bPRGBankMode = (data & 0x40);
                bCHRInversion = (data & 0x80);
            }
            else
            {
                // Update target register
                pRegister[nTargetRegister] = data;

                // Update Pointer Table
                if (bCHRInversion)
                {
                    pCHRBank[0] = pRegister[2] * 0x0400;
                    pCHRBank[1] = pRegister[3] * 0x0400;
                    pCHRBank[2] = pRegister[4] * 0x0400;
                    pCHRBank[3] = pRegister[5] * 0x0400;
                    pCHRBank[4] = (pRegister[0] & 0xFE) * 0x0400;
                    pCHRBank[5] = pRegister[0] * 0x0400 + 0x0400;
                    pCHRBank[6] = (pRegister[1] & 0xFE) * 0x0400;
                    pCHRBank[7] = pRegister[1] * 0x0400 + 0x0400;
                }
                else
                {
                    pCHRBank[0] = (pRegister[0] & 0xFE) * 0x0400;
                    pCHRBank[1] = pRegister[0] * 0x0400 + 0x0400;
                    pCHRBank[2] = (pRegister[1] & 0xFE) * 0x0400;
                    pCHRBank[3] = pRegister[1] * 0x0400 + 0x0400;
                    pCHRBank[4] = pRegister[2] * 0x0400;
                    pCHRBank[5] = pRegister[3] * 0x0400;
                    pCHRBank[6] = pRegister[4] * 0x0400;
                    pCHRBank[7] = pRegister[5] * 0x0400;
                }

                if (bPRGBankMode)
                {
                    pPRGBank[2] = (pRegister[6] & 0x3F) * 0x2000;
                    pPRGBank[0] = (nPRGBanks * 2 - 2) * 0x2000;
                }
                else
                {
                    pPRGBank[0] = (pRegister[6] & 0x3F) * 0x2000;
                    pPRGBank[2] = (nPRGBanks * 2 - 2) * 0x2000;
                }

                pPRGBank[1] = (pRegister[7] & 0x3F) * 0x2000;
                pPRGBank[3] = (nPRGBanks * 2 - 1) * 0x2000;

            }

            return false;
        }

        if (addr >= 0xA000 && addr <= 0xBFFF)
        {
            if (!(addr & 0x0001))
            {
                // Mirroring
                if (data & 0x01)
                    mirrormode = MIRROR::HORIZONTAL;
                else
                    mirrormode = MIRROR::VERTICAL;
            }
            else
            {
                // PRG Ram Protect
                // TODO:
            }
            return false;
        }

        if (addr >= 0xC000 && addr <= 0xDFFF)
        {
            if (!(addr & 0x0001))
            {
                nIRQReload = data;
            }
            else
            {
                nIRQCounter = 0x0000;
            }
            return false;
        }

        if (addr >= 0xE000 && addr <= 0xFFFF)
        {
            if (!(addr & 0x0001))
            {
                bIRQEnable = false;
                bIRQActive = false;
            }
            else
            {
                bIRQEnable = true;
            }
            return false;
        }

        return false;
    }

	bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override
    {
        if (addr >= 0x0000 && addr <= 0x03FF)
        {
            mapped_addr = pCHRBank[0] + (addr & 0x03FF);
            return true;
        }

        if (addr >= 0x0400 && addr <= 0x07FF)
        {
            mapped_addr = pCHRBank[1] + (addr & 0x03FF);
            return true;
        }

        if (addr >= 0x0800 && addr <= 0x0BFF)
        {
            mapped_addr = pCHRBank[2] + (addr & 0x03FF);
            return true;
        }

        if (addr >= 0x0C00 && addr <= 0x0FFF)
        {
            mapped_addr = pCHRBank[3] + (addr & 0x03FF);
            return true;
        }

        if (addr >= 0x1000 && addr <= 0x13FF)
        {
            mapped_addr = pCHRBank[4] + (addr & 0x03FF);
            return true;
        }

        if (addr >= 0x1400 && addr <= 0x17FF)
        {
            mapped_addr = pCHRBank[5] + (addr & 0x03FF);
            return true;
        }

        if (addr >= 0x1800 && addr <= 0x1BFF)
        {
            mapped_addr = pCHRBank[6] + (addr & 0x03FF);
            return true;
        }

        if (addr >= 0x1C00 && addr <= 0x1FFF)
        {
            mapped_addr = pCHRBank[7] + (addr & 0x03FF);
            return true;
        }

        return false;        
    }
	bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override
    {
        return false;
    }
	void reset() override
    {
        nTargetRegister = 0x00;
        bPRGBankMode = false;
        bCHRInversion = false;
        mirrormode = MIRROR::HORIZONTAL;

        bIRQActive = false;
        bIRQEnable = false;
        bIRQUpdate = false;
        nIRQCounter = 0x0000;
        nIRQReload = 0x0000;

        for (int i = 0; i < 4; i++)	pPRGBank[i] = 0;
        for (int i = 0; i < 8; i++) { pCHRBank[i] = 0; pRegister[i] = 0; }

        pPRGBank[0] = 0 * 0x2000;
        pPRGBank[1] = 1 * 0x2000;
        pPRGBank[2] = (nPRGBanks * 2 - 2) * 0x2000;
        pPRGBank[3] = (nPRGBanks * 2 - 1) * 0x2000;        
    }

	bool irqState() override
    {
    	return bIRQActive;
    }
	void irqClear() override
    {
      	bIRQActive = false;
    }

	void scanline() override
    {
        if (nIRQCounter == 0)
        {		
            nIRQCounter = nIRQReload;
        }
        else
            nIRQCounter--;

        if (nIRQCounter == 0 && bIRQEnable)
        {
            bIRQActive = true;
        }        
    }
	MIRROR mirror() override
    {
        return mirrormode;
    }

private:
	// Control variables
	uint8_t nTargetRegister = 0x00;
	bool bPRGBankMode = false;
	bool bCHRInversion = false;
	MIRROR mirrormode = MIRROR::HORIZONTAL;

	uint32_t pRegister[8];
	uint32_t pCHRBank[8];
	uint32_t pPRGBank[4];

	bool bIRQActive = false;
	bool bIRQEnable = false;
	bool bIRQUpdate = false;
	uint16_t nIRQCounter = 0x0000;
	uint16_t nIRQReload = 0x0000;

	std::vector<uint8_t> vRAMStatic;
};

class Mapper_066 :	public Mapper
{
public:
	Mapper_066(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks)
    {}
	~Mapper_066()
    {}

	bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr, uint8_t &data) override
    {
        if (addr >= 0x8000 && addr <= 0xFFFF)
        {
            mapped_addr = nPRGBankSelect * 0x8000 + (addr & 0x7FFF);
            return true;
        }
        else
            return false;
    }
	bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data = 0) override
    {
        if (addr >= 0x8000 && addr <= 0xFFFF)
        {
            nCHRBankSelect = data & 0x03;
            nPRGBankSelect = (data & 0x30) >> 4;
        }
        
        // Mapper has handled write, but do not update ROMs
        return false;
    }
	bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr) override
    {
        if (addr < 0x2000)
        {
            mapped_addr = nCHRBankSelect * 0x2000 + addr;
            return true;
        }
        else
            return false;        
    }
	bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr) override
    {
        return false;
    }
	void reset() override
    {
        nCHRBankSelect = 0;
        nPRGBankSelect = 0;
    }

private:
	uint8_t nCHRBankSelect = 0x00;
	uint8_t nPRGBankSelect = 0x00;
};


NesRom::NesRom(const std::string& sFileName)
{
	// iNES Format Header
	struct sHeader
	{
		char name[4];
		uint8_t prg_rom_chunks;
		uint8_t chr_rom_chunks;
		uint8_t mapper1;
		uint8_t mapper2;
		uint8_t prg_ram_size;
		uint8_t tv_system1;
		uint8_t tv_system2;
		char unused[5];
	} header;

	bImageValid = false;

	std::ifstream ifs;
	ifs.open(sFileName, std::ifstream::binary);
	if (ifs.is_open())
	{
		// Read file header
		ifs.read((char*)&header, sizeof(sHeader));

		// If a "trainer" exists we just need to read past
		// it before we get to the good stuff
		if (header.mapper1 & 0x04)
			ifs.seekg(512, std::ios_base::cur);

		// Determine Mapper ID
		nMapperID = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
		hw_mirror = (header.mapper1 & 0x01) ? VERTICAL : HORIZONTAL;

		// "Discover" File Format
		uint8_t nFileType = 1;
		if ((header.mapper2 & 0x0C) == 0x08) nFileType = 2;

		if (nFileType == 0)
		{

		}

		if (nFileType == 1)
		{
			nPRGBanks = header.prg_rom_chunks;
			vPRGMemory.resize(nPRGBanks * 16384);
			ifs.read((char*)vPRGMemory.data(), vPRGMemory.size());

			nCHRBanks = header.chr_rom_chunks;
			if (nCHRBanks == 0)
			{
				// Create CHR RAM
				vCHRMemory.resize(8192);
			}
			else
			{
				// Allocate for ROM
				vCHRMemory.resize(nCHRBanks * 8192);
			}
			ifs.read((char*)vCHRMemory.data(), vCHRMemory.size());
		}

		if (nFileType == 2)
		{
			nPRGBanks = ((header.prg_ram_size & 0x07) << 8) | header.prg_rom_chunks;
			vPRGMemory.resize(nPRGBanks * 16384);
			ifs.read((char*)vPRGMemory.data(), vPRGMemory.size());

			nCHRBanks = ((header.prg_ram_size & 0x38) << 8) | header.chr_rom_chunks;
			vCHRMemory.resize(nCHRBanks * 8192);
			ifs.read((char*)vCHRMemory.data(), vCHRMemory.size());
		}

		// Load appropriate mapper
		switch (nMapperID)
		{
		case   0: pMapper = std::make_shared<Mapper_000>(nPRGBanks, nCHRBanks); break;
		case   1: pMapper = std::make_shared<Mapper_001>(nPRGBanks, nCHRBanks); break;
		case   2: pMapper = std::make_shared<Mapper_002>(nPRGBanks, nCHRBanks); break;
		case   3: pMapper = std::make_shared<Mapper_003>(nPRGBanks, nCHRBanks); break;
		case   4: pMapper = std::make_shared<Mapper_004>(nPRGBanks, nCHRBanks); break;
		case  66: pMapper = std::make_shared<Mapper_066>(nPRGBanks, nCHRBanks); break;

		}

		bImageValid = true;
		ifs.close();
	}

}

NesRom::~NesRom()
{

}

bool NesRom::cpuRead(uint16_t addr, uint8_t &data)
{
	uint32_t mapped_addr = 0;
	if (pMapper->cpuMapRead(addr, mapped_addr, data))
	{
		if (mapped_addr == 0xFFFFFFFF)
		{
			// Mapper has actually set the data value, for example cartridge based RAM
			return true;
		}
		else
		{
			// Mapper has produced an offset into cartridge bank memory
			data = vPRGMemory[mapped_addr];
		}
		return true;
	}
	else
		return false;
}

bool NesRom::cpuWrite(uint16_t addr, uint8_t data)
{
	uint32_t mapped_addr = 0;
	if (pMapper->cpuMapWrite(addr, mapped_addr, data))
	{
		if (mapped_addr == 0xFFFFFFFF)
		{
			// Mapper has actually set the data value, for example cartridge based RAM
			return true;
		}
		else
		{
			// Mapper has produced an offset into cartridge bank memory
			vPRGMemory[mapped_addr] = data;
		}
		return true;
	}
	else
		return false;
}

bool NesRom::ppuRead(uint16_t addr, uint8_t & data)
{
	uint32_t mapped_addr = 0;
	if (pMapper->ppuMapRead(addr, mapped_addr))
	{
		data = vCHRMemory[mapped_addr];
		return true;
	}
	else
		return false;
}

bool NesRom::ppuWrite(uint16_t addr, uint8_t data)
{
	uint32_t mapped_addr = 0;
	if (pMapper->ppuMapWrite(addr, mapped_addr))
	{
		vCHRMemory[mapped_addr] = data;
		return true;
	}
	else
		return false;
}


void NesRom::reset()
{
	// Note: This does not reset the ROM contents,
	// but does reset the mapper.
	if (pMapper != nullptr)
		pMapper->reset();
}

MIRROR NesRom::Mirror()
{
	MIRROR m = pMapper->mirror();
	if (m == MIRROR::HARDWARE)
	{
		// Mirror configuration was defined
		// in hardware via soldering
		return hw_mirror;
	}
	else
	{
		// Mirror configuration can be
		// dynamically set via mapper
		return m;
	}
}

std::shared_ptr<Mapper> NesRom::GetMapper()
{
	return pMapper;
}
