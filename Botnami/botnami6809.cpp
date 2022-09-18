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

// Motorola 6809 opcode logic (WIP)

namespace botnami
{
    int Botnami6809::indexed_mode()
    {
	int cycles = 1;

	uint8_t opcode = getimmByte();
	uint16_t address = 0;

	if (testbit(opcode, 7))
	{
	    cout << "Unimplemented indexed mode of " << hex << int(opcode & 0x7F) << endl;
	    exit(0);
	}
	else
	{
	    address = (get_ireg(opcode) + int8_t((opcode & 0xF) | (testbit(opcode, 4) ? 0xF0 : 0x00)));
	    cycles += 2;
	}

	extended_address = address;

	return cycles;
    }

    int Botnami6809::executeinstr(uint8_t instr)
    {
	int cycles = 0;
	switch (instr)
	{
	    case 0x1A:
	    {
		status_reg |= getimmByte();
		cycles = 3;
	    }
	    break; // ORCC imm
	    case 0x1C:
	    {
		status_reg &= getimmByte();
		cycles = 3;
	    }
	    break; // ANDCC imm
	    case 0x26:
	    {
		cycles = branch(is_cond_ne());
	    }
	    break; // BNE rel8
	    case 0x3D:
	    {
		cycles = mul();
	    }
	    break; // MUL
	    case 0x4A:
	    case 0x4B:
	    {
		rega = dec8(rega);
		cycles = 2;
	    }
	    break; // DECA
	    case 0x4F:
	    {
		rega = clr8();
		cycles = 2;
	    }
	    break; // CLRA
	    case 0x5A:
	    case 0x5B:
	    {
		regb = dec8(regb);
		cycles = 2;
	    }
	    break; // DECB
	    case 0x5F:
	    {
		regb = clr8();
		cycles = 2;
	    }
	    break; // CLRB
	    case 0x6F:
	    {
		int index_cycles = indexed_mode();
		writeByte(extended_address, clr8());
		cycles = (4 + index_cycles);
	    }
	    break; // CLR8 indexed
	    case 0x7E:
	    {
		uint16_t address = getimmWord();
		pc = address;
		cycles = 4;
	    }
	    break; // JMP extended
	    case 0x7F:
	    {
		uint16_t address = getimmWord();
		writeByte(address, clr8());
		cycles = 7;
	    }
	    break; // CLR8 extended
	    case 0x81:
	    {
		uint8_t operand = getimmByte();
		cmp8(rega, operand);
		cycles = 2;
	    }
	    break; // CMPA imm
	    case 0x86:
	    {
		uint8_t operand = getimmByte();
		rega = load8(operand);
		cycles = 2;
	    }
	    break; // LDA imm
	    case 0x8A:
	    {
		uint8_t operand = getimmByte();
		rega = or8(rega, operand);
		cycles = 2;
	    }
	    break; // ORA imm
	    case 0x8B:
	    {
		uint8_t operand = getimmByte();
		rega = add8(rega, operand);
		cycles = 2;
	    }
	    break; // ADDA imm
	    case 0x8C:
	    {
		uint16_t operand = getimmWord();
		cmp16(regx, operand);
		cycles = 3;
	    }
	    break; // CMPX imm
	    case 0x8E:
	    {
		uint16_t operand = getimmWord();
		regx = load16(operand);
		cycles = 3;
	    }
	    break; // LDX imm16
	    case 0x97:
	    {
		uint8_t operand = getimmByte();
		uint16_t address = ((regdp << 8) | operand);
		writeByte(address, rega);
		cycles = 4;
	    }
	    break; // STA direct
	    case 0xA7:
	    {
		int index_cycles = indexed_mode();
		store8(extended_address, rega);
		cycles = (2 + index_cycles);
	    }
	    break; // STA indexed
	    case 0xB7:
	    {
		uint16_t address = getimmWord();
		writeByte(address, rega);
		cycles = 5;
	    }
	    break; // STA extended
	    case 0xC6:
	    {
		uint8_t operand = getimmByte();
		regb = load8(operand);
		cycles = 2;
	    }
	    break; // LDB imm
	    default: unrecognizedinstr(instr); break;
	}

	return cycles;
    }

    void Botnami6809::debugoutput(bool print_disassembly)
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