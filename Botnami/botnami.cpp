/*
    This file is part of Botnami.
    Copyright (C) 2022 BueniaDev.

    Botnami is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Botnami is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Botnami.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "botnami.h"
using namespace botnami;
using namespace std;

namespace botnami
{
    BotnamiInterface::BotnamiInterface()
    {

    }

    BotnamiInterface::~BotnamiInterface()
    {

    }

    BotnamiCPU::BotnamiCPU()
    {

    }

    BotnamiCPU::~BotnamiCPU()
    {

    }

    void BotnamiCPU::init()
    {
	rega = 0;
	regb = 0;
	regx = 0;
	regy = 0;
	regdp = 0;
	status_reg = 0;
	pc = readWord(0xFFFE);
    }

    void BotnamiCPU::shutdown()
    {
	return;
    }

    void BotnamiCPU::reset()
    {
	return;
    }

    void BotnamiCPU::setinterface(BotnamiInterface &cb)
    {
	inter = unique_ptr<BotnamiInterface>(&cb);
    }

    uint8_t BotnamiCPU::readByte(uint16_t addr)
    {
	uint8_t data = 0;

	if (inter)
	{
	    data = inter->readByte(addr);
	}

	return data;
    }

    uint16_t BotnamiCPU::readWord(uint16_t addr)
    {
	// The Motorola 6809 is a big-endian system,
	// so the 16-bit value is constructed as follows:
	// val_16 = (mem[addr] << 8) | mem[addr + 1])
	uint8_t high = readByte(addr);
	uint8_t low = readByte(addr + 1);
	return ((high << 8) | low);
    }

    void BotnamiCPU::writeByte(uint16_t addr, uint8_t data)
    {
	if (inter)
	{
	    inter->writeByte(addr, data);
	}
    }

    void BotnamiCPU::writeWord(uint16_t addr, uint16_t data)
    {
	// The Motorola 6809 is a big-endian system,
	// so the 16-bit value is written as follows:
	// mem[addr] = high_byte(val)
	// mem[addr + 1] = low_byte(val)

	writeByte(addr, (data >> 8));
	writeByte((addr + 1), (data & 0xFF));
    }

    uint8_t BotnamiCPU::getimmByte()
    {
	return readByte(pc++);
    }

    uint16_t BotnamiCPU::getimmWord()
    {
	uint16_t value = readWord(pc);
	pc += 2;
	return value;
    }

    uint8_t BotnamiCPU::readOpcode()
    {
	return readOpcode(pc++);
    }

    uint8_t BotnamiCPU::readOpcode(uint16_t addr)
    {
	uint8_t data = 0;
	if (inter)
	{
	    if (inter->isSeperateOps())
	    {
		data = inter->readOpcode(addr);
	    }
	    else
	    {
		data = inter->readByte(addr);
	    }
	}

	return data;
    }

    void BotnamiCPU::setLines(uint8_t data)
    {
	if (inter)
	{
	    inter->setLines(data);
	}
    }

    void BotnamiCPU::fireIRQ(bool line)
    {
	is_irq_pending = line;
    }

    int BotnamiCPU::handleIRQ()
    {
	return 0;
    }

    bool BotnamiCPU::isIRQPending()
    {
	if (!is_irq() && is_irq_pending)
	{
	    return true;
	}
	else
	{
	    return false;
	}
    }

    int BotnamiCPU::executenextinstr()
    {
	int cycles = 0;
	if (isIRQPending())
	{
	    cycles = handleIRQ();
	}
	else
	{
	    uint8_t opcode = readOpcode();
	    cycles = executeinstr(opcode);
	}

	setStatus();
	return cycles;
    }

    void BotnamiCPU::debugoutput(bool print_disassembly)
    {
	(void)print_disassembly;
	return;
    }

    size_t BotnamiCPU::disassembleinstr(ostream &stream, size_t pc)
    {
	(void)pc;
	stream << "unk";
	return 0;
    }

    int BotnamiCPU::executeinstr(uint8_t instr)
    {
	unrecognizedinstr(instr);
	return 0;
    }

    void BotnamiCPU::unrecognizedinstr(uint8_t instr)
    {
	cout << "Unrecognized opcode of " << hex << int(instr) << endl;
	debugoutput(false);
	exit(1);
    }
};