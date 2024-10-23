#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include <vector>

enum MIRROR
{
	HARDWARE,
	HORIZONTAL,
	VERTICAL,
	ONESCREEN_LO,
	ONESCREEN_HI,
};


class Mapper
{
public:
	Mapper(uint8_t prgBanks, uint8_t chrBanks)
    {
        nPRGBanks = prgBanks;
        nCHRBanks = chrBanks;
        reset();
    }
	~Mapper()
    {       
    }

public:
	// Transform CPU bus address into PRG ROM offset
	virtual bool cpuMapRead(uint16_t addr, uint32_t &mapped_addr, uint8_t &data)	 = 0;
	virtual bool cpuMapWrite(uint16_t addr, uint32_t &mapped_addr, uint8_t data = 0)	 = 0;
	
	// Transform PPU bus address into CHR ROM offset
	virtual bool ppuMapRead(uint16_t addr, uint32_t &mapped_addr)	 = 0;
	virtual bool ppuMapWrite(uint16_t addr, uint32_t &mapped_addr)	 = 0;

	// Reset mapper to known state
	virtual void reset() = 0;

	// Get Mirror mode if mapper is in control
	virtual MIRROR mirror() { return MIRROR::HARDWARE; }

	// IRQ Interface
	virtual bool irqState() { return false; }
	virtual void irqClear() {}

	// Scanline Counting
	virtual void scanline() {}

protected:
	// These are stored locally as many of the mappers require this information
	uint8_t nPRGBanks = 0;
	uint8_t nCHRBanks = 0;
};

class NesRom
{
public:	
	NesRom(const std::string& sFileName);
	~NesRom();

public:
	bool ImageValid();

private:
	bool bImageValid = false;
	MIRROR hw_mirror = HORIZONTAL;

	uint8_t nMapperID = 0;
	uint8_t nPRGBanks = 0;
	uint8_t nCHRBanks = 0;

	std::vector<uint8_t> vPRGMemory;
	std::vector<uint8_t> vCHRMemory;

	std::shared_ptr<Mapper> pMapper;

public:
	// Communication with Main Bus
	bool cpuRead(uint16_t addr, uint8_t &data);
	bool cpuWrite(uint16_t addr, uint8_t data);

	// Communication with PPU Bus
	bool ppuRead(uint16_t addr, uint8_t &data);
	bool ppuWrite(uint16_t addr, uint8_t data);

	// Permits system rest of mapper to know state
	void reset();

	// Get Mirror configuration
	MIRROR Mirror();

	std::shared_ptr<Mapper> GetMapper();
};