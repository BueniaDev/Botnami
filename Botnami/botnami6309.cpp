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

// Hitachi HD6309 opcode logic (WIP)

namespace botnami
{
    int Botnami6309::executeinstr(uint8_t instr)
    {
	bool is_native = is_native_mode();
	int cycles = 0;
	switch (instr)
	{
	    case 0x4A:
	    {
		rega = dec_internal8(rega);
		cycles = is_native ? 1 : 2;
	    }
	    break; // DECA
	    case 0x5A:
	    {
		regb = dec_internal8(regb);
		cycles = is_native ? 1 : 2;
	    }
	    break; // DECB
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
};