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
    int Botnami6809::executeinstr(uint8_t instr)
    {
	int cycles = 0;
	switch (instr)
	{
	    case 0x4A:
	    case 0x4B:
	    {
		rega = dec_internal8(rega);
		cycles = 2;
	    }
	    break; // DECA
	    case 0x4F:
	    {
		set_nz<uint8_t>(0);
		rega = 0;
		cycles = 2;
	    }
	    break; // CLRA
	    case 0x5A:
	    case 0x5B:
	    {
		regb = dec_internal8(regb);
		cycles = 2;
	    }
	    break; // DECB
	    case 0x5F:
	    {
		set_nz<uint8_t>(0);
		regb = 0;
		cycles = 2;
	    }
	    break; // CLRB
	    case 0x81:
	    {
		cmp8(rega, getimmByte());
		cycles = 2;
	    }
	    break; // CMPA
	    case 0x86:
	    {
		rega = getimmByte();
		set_nz(rega);
		cycles = 2;
	    }
	    break; // LDA
	    case 0x8B:
	    {
		rega = add8(rega, getimmByte());
		cycles = 2;
	    }
	    break; // ADDA
	    case 0x8E:
	    {
		regx = getimmWord();
		set_nz(regx);
		cycles = 3;
	    }
	    break; // LDX imm16
	    case 0xC6:
	    {
		regb = getimmByte();
		set_nz(regb);
		cycles = 2;
	    }
	    break; // LDB
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