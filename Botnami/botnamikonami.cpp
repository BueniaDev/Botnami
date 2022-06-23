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

// KONAMI-2 opcode logic (WIP)

namespace botnami
{
    int BotnamiKonami2::indexed_mode()
    {
	int cycles = 1;
	uint8_t opcode = getimmByte();

	uint16_t address = 0;

	switch ((opcode & 0xF7))
	{
	    case 0x07:
	    {
		address = getimmWord();
		cycles += 2;
	    }
	    break;
	    case 0x20:
	    case 0x30:
	    case 0x50:
	    case 0x60:
	    case 0x70:
	    {
		// Auto-increment
		address = get_ireg(opcode);
		inc_ireg(opcode);
		cycles += 2;
	    }
	    break;
	    case 0x21:
	    case 0x31:
	    case 0x51:
	    case 0x61:
	    case 0x71:
	    {
		// Double auto-increment
		address = get_ireg(opcode);
		inc_ireg(opcode, true);
		cycles += 3;
	    }
	    break;
	    case 0x22:
	    case 0x32:
	    case 0x52:
	    case 0x62:
	    case 0x72:
	    {
		// Auto-decrement
		dec_ireg(opcode);
		address = get_ireg(opcode);
		cycles += 2;
	    }
	    break;
	    case 0x23:
	    case 0x33:
	    case 0x53:
	    case 0x63:
	    case 0x73:
	    {
		// Double auto-decrement
		dec_ireg(opcode, true);
		address = get_ireg(opcode);
		cycles += 3;
	    }
	    break;
	    case 0x26:
	    case 0x36:
	    case 0x56:
	    case 0x66:
	    case 0x76:
	    {
		address = get_ireg(opcode);
	    }
	    break;
	    case 0xC4:
	    {
		uint8_t lsb = getimmByte();
		address = ((regdp << 8) | lsb);
		cycles += 1;
	    }
	    break;
	    default:
	    {
		cout << "Unrecognized indexed opcode of " << hex << int(opcode) << endl;
		exit(1);
	    }
	    break;
	}

	if (testbit(opcode, 3))
	{
	    cout << "Indirect mode unimplemented" << endl;
	    exit(0);
	}

	extended_address = address;
	return cycles;
    }

    int BotnamiKonami2::executeinstr(uint8_t instr)
    {
	int cycles = 0;

	switch (instr)
	{
	    case 0x08:
	    {
		int index_cycles = indexed_mode();
		regx = extended_address;
		set_z(regx);
		cycles = (2 + index_cycles);
	    }
	    break; // LEA x
	    case 0x09:
	    {
		int index_cycles = indexed_mode();
		regy = extended_address;
		set_z(regy);
		cycles = (2 + index_cycles);
	    }
	    break; // LEA y
	    case 0x0A:
	    {
		int index_cycles = indexed_mode();
		usp = extended_address;
		set_z(usp);
		cycles = (2 + index_cycles);
	    }
	    break; // LEA u
	    case 0x0B:
	    {
		int index_cycles = indexed_mode();
		ssp = extended_address;
		set_z(ssp);
		cycles = (2 + index_cycles);
	    }
	    break; // LEA s
	    case 0x0C:
	    {
		cycles = pushs();
	    }
	    break; // PUSHS
	    case 0x0E:
	    {
		cycles = pulls();
	    }
	    break; // PULLS
	    case 0x10:
	    {
		rega = getimmByte();
		set_nz(rega);
		cycles = 2;
	    }
	    break; // LDA
	    case 0x11:
	    {
		regb = getimmByte();
		set_nz(regb);
		cycles = 2;
	    }
	    break; // LDB
	    case 0x38:
	    {
		uint8_t value = getimmByte();
		setLines(value);
		cycles = 2;
	    }
	    break; // SETLINES
	    case 0x3A:
	    {
		int index_cycles = indexed_mode();
		set_nz(rega);
		writeByte(extended_address, rega);
		cycles = (2 + index_cycles);
	    }
	    break; // STA indexed
	    case 0x3B:
	    {
		int index_cycles = indexed_mode();
		set_nz(regb);
		writeByte(extended_address, regb);
		cycles = (2 + index_cycles);
	    }
	    break; // STB indexed
	    case 0x40:
	    {
		uint16_t value = getimmWord();
		setRegD(value);
		set_nz(value);
		cycles = 3;
	    }
	    break; // LDD imm16
	    case 0x42:
	    {
		regx = getimmWord();
		set_nz(regx);
		cycles = 3;
	    }
	    break; // LDX imm16
	    case 0x43:
	    {
		int index_cycles = indexed_mode();
		regx = readWord(extended_address);
		set_nz(regx);
		cycles = (3 + index_cycles);
	    }
	    break; // LDX indexed
	    case 0x44:
	    {
		regy = getimmWord();
		set_nz(regy);
		cycles = 3;
	    }
	    break; // LDY imm16
	    case 0x45:
	    {
		int index_cycles = indexed_mode();
		regy = readWord(extended_address);
		set_nz(regy);
		cycles = (3 + index_cycles);
	    }
	    break; // LDY indexed
	    case 0x46:
	    {
		uint16_t value = getimmWord();
		usp = value;
		set_nz(value);
		cycles = 3;
	    }
	    break; // LDU imm16
	    case 0x48:
	    {
		uint16_t value = getimmWord();
		ssp = value;
		set_nz(value);
		cycles = 3;
	    }
	    break; // LDS imm16
	    case 0x80:
	    {
		set_nz<uint8_t>(0);
		rega = 0;
		cycles = 2;
	    }
	    break; // CLRA
	    case 0x81:
	    {
		set_nz<uint8_t>(0);
		regb = 0;
		cycles = 2;
	    }
	    break; // CLRB
	    case 0x82:
	    {
		int index_cycles = indexed_mode();
		set_nz<uint8_t>(0);
		writeByte(extended_address, 0);
		cycles = (4 + index_cycles);
	    }
	    break; // CLR indexed
	    default: unrecognizedinstr(instr); break;
	}

	return cycles;
    }

    void BotnamiKonami2::debugoutput(bool print_disassembly)
    {
	cout << "PC: " << hex << int(status.pc) << endl;
	cout << "S: " << hex << int(status.ssp) << endl;
	cout << "CC: " << hex << int(status.status_reg) << endl;
	cout << "DP: " << hex << int(status.regdp) << endl;
	cout << "A: " << hex << int(status.rega) << endl;
	cout << "B: " << hex << int(status.regb) << endl;
	cout << "D: " << hex << int(status.regd) << endl;
	cout << "X: " << hex << int(status.regx) << endl;
	cout << "Y: " << hex << int(status.regy) << endl;
	cout << "U: " << hex << int(status.usp) << endl;
	cout << endl;
    }
};