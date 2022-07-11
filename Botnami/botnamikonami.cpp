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

// KONAMI-2 opcode logic
//
// TODO list:
// Implement remaining instructions
// Add FIRQ and NMI support

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
	    case 0x24:
	    case 0x34:
	    case 0x54:
	    case 0x64:
	    case 0x74:
	    {
		extended_address = get_ireg(opcode);
		int8_t offs = getimmByte();
		address = (extended_address + offs);
		cycles += 2;
	    }
	    break;
	    case 0x25:
	    case 0x35:
	    case 0x55:
	    case 0x65:
	    case 0x75:
	    {
		extended_address = get_ireg(opcode);
		int16_t offs = getimmWord();
		address = (extended_address + offs);
		cycles += 4;
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
	    case 0xA0:
	    case 0xB0:
	    case 0xD0:
	    case 0xE0:
	    case 0xF0:
	    {
		int8_t offs = rega;
		address = (get_ireg(opcode) + offs);
		cycles += 1;
	    }
	    break;
	    case 0xA1:
	    case 0xB1:
	    case 0xD1:
	    case 0xE1:
	    case 0xF1:
	    {
		int8_t offs = regb;
		address = (get_ireg(opcode) + offs);
		cycles += 1;
	    }
	    break;
	    case 0xA7:
	    case 0xB7:
	    case 0xD7:
	    case 0xE7:
	    case 0xF7:
	    {
		int16_t offs = getRegD();
		address = (get_ireg(opcode) + offs);
		cycles += 4;
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
	    extended_address = address;
	    address = readWord(extended_address);
	    cycles += 2;
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
		uint8_t operand = getimmByte();
		rega = load8(operand);
		cycles = 2;
	    }
	    break; // LDA imm
	    case 0x11:
	    {
		uint8_t operand = getimmByte();
		regb = load8(operand);
		cycles = 2;
	    }
	    break; // LDB imm
	    case 0x12:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		rega = load8(operand);
		cycles = (2 + index_cycles);
	    }
	    break; // LDA indexed
	    case 0x13:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		regb = load8(operand);
		cycles = (2 + index_cycles);
	    }
	    break; // LDB indexed
	    case 0x14:
	    {
		uint8_t operand = getimmByte();
		rega = add8(rega, operand);
		cycles = 2;
	    }
	    break; // ADDA imm
	    case 0x15:
	    {
		uint8_t operand = getimmByte();
		regb = add8(regb, operand);
		cycles = 2;
	    }
	    break; // ADDB imm
	    case 0x16:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		rega = add8(rega, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // ADDA indexed
	    case 0x17:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		regb = add8(regb, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // ADDB indexed
	    case 0x18:
	    {
		uint8_t operand = getimmByte();
		rega = adc8(rega, operand);
		cycles = 2;
	    }
	    break; // ADCA imm
	    case 0x19:
	    {
		uint8_t operand = getimmByte();
		regb = adc8(regb, operand);
		cycles = 2;
	    }
	    break; // ADCB imm
	    case 0x1A:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		rega = adc8(rega, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // ADCA indexed
	    case 0x1B:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		regb = adc8(regb, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // ADCB indexed
	    case 0x1C:
	    {
		uint8_t operand = getimmByte();
		rega = sub8(rega, operand);
		cycles = 2;
	    }
	    break; // SUBA imm
	    case 0x1D:
	    {
		uint8_t operand = getimmByte();
		regb = sub8(regb, operand);
		cycles = 2;
	    }
	    break; // SUBB imm
	    case 0x1E:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		rega = sub8(rega, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // SUBA indexed
	    case 0x1F:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		regb = sub8(regb, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // SUBB indexed
	    case 0x20:
	    {
		uint8_t operand = getimmByte();
		rega = sbc8(rega, operand);
		cycles = 2;
	    }
	    break; // SBCA imm
	    case 0x21:
	    {
		uint8_t operand = getimmByte();
		regb = sbc8(regb, operand);
		cycles = 2;
	    }
	    break; // SBCB imm
	    case 0x22:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		rega = sbc8(rega, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // SBCA indexed
	    case 0x23:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		regb = sbc8(regb, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // SBCB indexed
	    case 0x24:
	    {
		uint8_t operand = getimmByte();
		rega = and8(rega, operand);
		cycles = 2;
	    }
	    break; // ANDA imm
	    case 0x25:
	    {
		uint8_t operand = getimmByte();
		regb = and8(regb, operand);
		cycles = 2;
	    }
	    break; // ANDB imm
	    case 0x26:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		rega = and8(rega, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // ANDA indexed
	    case 0x27:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		regb = and8(regb, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // ANDB indexed
	    case 0x28:
	    {
		uint8_t operand = getimmByte();
		bit8(rega, operand);
		cycles = 2;
	    }
	    break; // BITA imm
	    case 0x29:
	    {
		uint8_t operand = getimmByte();
		bit8(regb, operand);
		cycles = 2;
	    }
	    break; // BITB imm
	    case 0x2A:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		bit8(rega, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // BITA indexed
	    case 0x2B:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		bit8(regb, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // BITB indexed
	    case 0x2C:
	    {
		uint8_t operand = getimmByte();
		rega = eor8(rega, operand);
		cycles = 2;
	    }
	    break; // EORA imm
	    case 0x2D:
	    {
		uint8_t operand = getimmByte();
		regb = eor8(regb, operand);
		cycles = 2;
	    }
	    break; // EORB imm
	    case 0x2E:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		rega = eor8(rega, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // EORA indexed
	    case 0x2F:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		regb = eor8(regb, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // EORB indexed
	    case 0x30:
	    {
		uint8_t operand = getimmByte();
		rega = or8(rega, operand);
		cycles = 2;
	    }
	    break; // ORA imm
	    case 0x31:
	    {
		uint8_t operand = getimmByte();
		regb = or8(regb, operand);
		cycles = 2;
	    }
	    break; // ORB imm
	    case 0x32:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		rega = or8(rega, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // ORA indexed
	    case 0x33:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		regb = or8(regb, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // ORB indexed
	    case 0x34:
	    {
		uint8_t operand = getimmByte();
		cmp8(rega, operand);
		cycles = 2;
	    }
	    break; // CMPA imm
	    case 0x35:
	    {
		uint8_t operand = getimmByte();
		cmp8(regb, operand);
		cycles = 2;
	    }
	    break; // CMPB imm
	    case 0x36:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		cmp8(rega, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // CMPA indexed
	    case 0x37:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		cmp8(regb, operand);
		cycles = (2 + index_cycles);
	    }
	    break; // CMPB indexed
	    case 0x38:
	    {
		uint8_t value = getimmByte();
		setLines(value);
		cycles = 2;
	    }
	    break; // SETLINES imm
	    case 0x39:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		setLines(operand);
		cycles = (2 + index_cycles);
	    }
	    break; // SETLINES indexed
	    case 0x3A:
	    {
		int index_cycles = indexed_mode();
		store8(extended_address, rega);
		cycles = (2 + index_cycles);
	    }
	    break; // STA indexed
	    case 0x3B:
	    {
		int index_cycles = indexed_mode();
		store8(extended_address, regb);
		cycles = (2 + index_cycles);
	    }
	    break; // STB indexed
	    case 0x3C:
	    {
		status_reg &= getimmByte();
		cycles = 3;
	    }
	    break; // ANDCC imm
	    case 0x3D:
	    {
		status_reg |= getimmByte();
		cycles = 3;
	    }
	    break; // ORCC imm
	    case 0x3E:
	    {
		cycles = exchange();
	    }
	    break; // EXG
	    case 0x3F:
	    {
		cycles = tfr();
	    }
	    break; // EXG
	    case 0x40:
	    {
		uint16_t value = getimmWord();
		setRegD(load16(value));
		cycles = 3;
	    }
	    break; // LDD imm16
	    case 0x41:
	    {
		int index_cycles = indexed_mode();
		uint16_t operand = readWord(extended_address);
		setRegD(load16(operand));
		cycles = (3 + index_cycles);
	    }
	    break; // LDD indexed
	    case 0x42:
	    {
		uint16_t value = getimmWord();
		regx = load16(value);
		cycles = 3;
	    }
	    break; // LDX imm16
	    case 0x43:
	    {
		int index_cycles = indexed_mode();
		uint16_t operand = readWord(extended_address);
		regx = load16(operand);
		cycles = (3 + index_cycles);
	    }
	    break; // LDX indexed
	    case 0x44:
	    {
		uint16_t value = getimmWord();
		regy = load16(value);
		cycles = 3;
	    }
	    break; // LDY imm16
	    case 0x45:
	    {
		int index_cycles = indexed_mode();
		uint16_t operand = readWord(extended_address);
		regy = load16(operand);
		cycles = (3 + index_cycles);
	    }
	    break; // LDY indexed
	    case 0x46:
	    {
		uint16_t value = getimmWord();
		usp = load16(value);
		cycles = 3;
	    }
	    break; // LDU imm16
	    case 0x47:
	    {
		int index_cycles = indexed_mode();
		uint16_t operand = readWord(extended_address);
		usp = load16(operand);
		cycles = (3 + index_cycles);
	    }
	    break; // LDU indexed
	    case 0x48:
	    {
		uint16_t value = getimmWord();
		ssp = load16(value);
		cycles = 3;
	    }
	    break; // LDS imm16
	    case 0x49:
	    {
		int index_cycles = indexed_mode();
		uint16_t operand = readWord(extended_address);
		ssp = load16(operand);
		cycles = (3 + index_cycles);
	    }
	    break; // LDS indexed
	    case 0x4A:
	    {
		uint16_t operand = getimmWord();
		cmp16(getRegD(), operand);
		cycles = 3;
	    }
	    break; // CMPD imm
	    case 0x4B:
	    {
		int index_cycles = indexed_mode();

		uint16_t operand = readWord(extended_address);
		cmp16(getRegD(), operand);
		cycles = (4 + index_cycles);
	    }
	    break; // CMPD indexed
	    case 0x4C:
	    {
		uint16_t operand = getimmWord();
		cmp16(regx, operand);
		cycles = 3;
	    }
	    break; // CMPX imm
	    case 0x4D:
	    {
		int index_cycles = indexed_mode();

		uint16_t operand = readWord(extended_address);
		cmp16(regx, operand);
		cycles = (4 + index_cycles);
	    }
	    break; // CMPX indexed
	    case 0x4E:
	    {
		uint16_t operand = getimmWord();
		cmp16(regy, operand);
		cycles = 3;
	    }
	    break; // CMPY imm
	    case 0x4F:
	    {
		int index_cycles = indexed_mode();

		uint16_t operand = readWord(extended_address);
		cmp16(regy, operand);
		cycles = (4 + index_cycles);
	    }
	    break; // CMPY indexed
	    case 0x50:
	    {
		uint16_t operand = getimmWord();
		cmp16(usp, operand);
		cycles = 3;
	    }
	    break; // CMPU imm
	    case 0x51:
	    {
		int index_cycles = indexed_mode();

		uint16_t operand = readWord(extended_address);
		cmp16(usp, operand);
		cycles = (4 + index_cycles);
	    }
	    break; // CMPU indexed
	    case 0x52:
	    {
		uint16_t operand = getimmWord();
		cmp16(ssp, operand);
		cycles = 3;
	    }
	    break; // CMPS imm
	    case 0x53:
	    {
		int index_cycles = indexed_mode();

		uint16_t operand = readWord(extended_address);
		cmp16(ssp, operand);
		cycles = (4 + index_cycles);
	    }
	    break; // CMPS indexed
	    case 0x54:
	    {
		uint16_t operand = getimmWord();
		setRegD(add16(getRegD(), operand));
		cycles = 4;
	    }
	    break; // ADDD imm
	    case 0x55:
	    {
		int index_cycles = indexed_mode();

		uint16_t operand = readWord(extended_address);
		setRegD(add16(getRegD(), operand));
		cycles = (4 + index_cycles);
	    }
	    break; // ADDD indexed
	    case 0x56:
	    {
		uint16_t operand = getimmWord();
		setRegD(sub16(getRegD(), operand));
		cycles = 4;
	    }
	    break; // SUBD imm
	    case 0x57:
	    {
		int index_cycles = indexed_mode();

		uint16_t operand = readWord(extended_address);
		setRegD(sub16(getRegD(), operand));
		cycles = (4 + index_cycles);
	    }
	    break; // SUBD indexed
	    case 0x58:
	    {
		int index_cycles = indexed_mode();
		store16(extended_address, getRegD());
		cycles = (3 + index_cycles);
	    }
	    break; // STD indexed
	    case 0x59:
	    {
		int index_cycles = indexed_mode();
		store16(extended_address, regx);
		cycles = (3 + index_cycles);
	    }
	    break; // STX indexed
	    case 0x5A:
	    {
		int index_cycles = indexed_mode();
		store16(extended_address, regy);
		cycles = (3 + index_cycles);
	    }
	    break; // STY indexed
	    case 0x5B:
	    {
		int index_cycles = indexed_mode();
		store16(extended_address, usp);
		cycles = (3 + index_cycles);
	    }
	    break; // STU indexed
	    case 0x5C:
	    {
		int index_cycles = indexed_mode();
		store16(extended_address, ssp);
		cycles = (3 + index_cycles);
	    }
	    break; // STS indexed
	    case 0x60:
	    {
		cycles = branch(true);
	    }
	    break; // BRA rel8
	    case 0x61:
	    {
		cycles = branch(is_cond_hi());
	    }
	    break; // BHI rel8
	    case 0x62:
	    {
		cycles = branch(is_cond_cc());
	    }
	    break; // BCC rel8
	    case 0x63:
	    {
		cycles = branch(is_cond_ne());
	    }
	    break; // BNE rel8
	    case 0x64:
	    {
		cycles = branch(is_cond_vc());
	    }
	    break; // BVC rel8
	    case 0x65:
	    {
		cycles = branch(is_cond_pl());
	    }
	    break; // BPL rel8
	    case 0x66:
	    {
		cycles = branch(is_cond_ge());
	    }
	    break; // BGE rel8
	    case 0x67:
	    {
		cycles = branch(is_cond_gt());
	    }
	    break; // BGT rel8
	    case 0x68:
	    {
		cycles = lbranchk(true);
	    }
	    break; // LBRA rel16
	    case 0x69:
	    {
		cycles = lbranchk(is_cond_hi());
	    }
	    break; // LBHI rel16
	    case 0x6A:
	    {
		cycles = lbranchk(is_cond_cc());
	    }
	    break; // LBCC rel16
	    case 0x6B:
	    {
		cycles = lbranchk(is_cond_ne());
	    }
	    break; // LBNE rel16
	    case 0x6C:
	    {
		cycles = lbranchk(is_cond_vc());
	    }
	    break; // LBVC rel16
	    case 0x6D:
	    {
		cycles = lbranchk(is_cond_pl());
	    }
	    break; // LBPL rel16
	    case 0x6E:
	    {
		cycles = lbranchk(is_cond_ge());
	    }
	    break; // LBGE rel16
	    case 0x6F:
	    {
		cycles = lbranchk(is_cond_gt());
	    }
	    break; // LBGT rel16
	    case 0x70:
	    {
		cycles = branch(false);
	    }
	    break; // BNV rel8
	    case 0x71:
	    {
		cycles = branch(is_cond_ls());
	    }
	    break; // BLS rel8
	    case 0x72:
	    {
		cycles = branch(is_cond_cs());
	    }
	    break; // BCS rel8
	    case 0x73:
	    {
		cycles = branch(is_cond_eq());
	    }
	    break; // BEQ rel8
	    case 0x74:
	    {
		cycles = branch(is_cond_vs());
	    }
	    break; // BVS rel8
	    case 0x75:
	    {
		cycles = branch(is_cond_mi());
	    }
	    break; // BMI rel8
	    case 0x76:
	    {
		cycles = branch(is_cond_lt());
	    }
	    break; // BLT rel8
	    case 0x77:
	    {
		cycles = branch(is_cond_le());
	    }
	    break; // BLE rel8
	    case 0x78:
	    {
		cycles = lbranchk(false);
	    }
	    break; // LBNV rel16
	    case 0x79:
	    {
		cycles = lbranchk(is_cond_ls());
	    }
	    break; // LBNV rel16
	    case 0x7A:
	    {
		cycles = lbranchk(is_cond_cs());
	    }
	    break; // LBCS rel16
	    case 0x7B:
	    {
		cycles = lbranchk(is_cond_eq());
	    }
	    break; // LBEQ rel16
	    case 0x7C:
	    {
		cycles = lbranchk(is_cond_vs());
	    }
	    break; // LBVS rel16
	    case 0x7D:
	    {
		cycles = lbranchk(is_cond_mi());
	    }
	    break; // LBMI rel16
	    case 0x7E:
	    {
		cycles = lbranchk(is_cond_lt());
	    }
	    break; // LBLT rel16
	    case 0x7F:
	    {
		cycles = lbranchk(is_cond_le());
	    }
	    break; // LBLE rel16
	    case 0x80:
	    {
		rega = clear<uint8_t>();
		cycles = 2;
	    }
	    break; // CLRA
	    case 0x81:
	    {
		regb = clear<uint8_t>();
		cycles = 2;
	    }
	    break; // CLRB
	    case 0x82:
	    {
		int index_cycles = indexed_mode();
		writeByte(extended_address, clear<uint8_t>());
		cycles = (4 + index_cycles);
	    }
	    break; // CLR indexed
	    case 0x83:
	    {
		rega = com8(rega);
		cycles = 2;
	    }
	    break; // COMA
	    case 0x84:
	    {
		regb = com8(regb);
		cycles = 2;
	    }
	    break; // COMB
	    case 0x85:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		writeByte(extended_address, com8(operand));
		cycles = (4 + index_cycles);
	    }
	    break; // INC indexed
	    case 0x86:
	    {
		rega = neg8(rega);
		cycles = 2;
	    }
	    break; // NEGA
	    case 0x87:
	    {
		regb = neg8(regb);
		cycles = 2;
	    }
	    break; // NEGB
	    case 0x88:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		writeByte(extended_address, neg8(operand));
		cycles = (4 + index_cycles);
	    }
	    break; // INC indexed
	    case 0x89:
	    {
		rega = inc_internal8(rega);
		cycles = 2;
	    }
	    break; // INCA
	    case 0x8A:
	    {
		regb = inc_internal8(regb);
		cycles = 2;
	    }
	    break; // INCB
	    case 0x8B:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		writeByte(extended_address, inc_internal8(operand));
		cycles = (4 + index_cycles);
	    }
	    break; // INC indexed
	    case 0x8C:
	    {
		rega = dec_internal8(rega);
		cycles = 2;
	    }
	    break; // DECA
	    case 0x8D:
	    {
		regb = dec_internal8(regb);
		cycles = 2;
	    }
	    break; // DECB
	    case 0x8E:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		writeByte(extended_address, dec_internal8(operand));
		cycles = (4 + index_cycles);
	    }
	    break; // DEC indexed
	    case 0x8F:
	    {
		cycles = rts();
	    }
	    break; // RTS
	    case 0x90:
	    {
		tst8(rega);
		cycles = 4;
	    }
	    break; // TSTA
	    case 0x91:
	    {
		tst8(rega);
		cycles = 4;
	    }
	    break; // TSTB
	    case 0x92:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		tst8(operand);
		cycles = (4 + index_cycles);
	    }
	    break; // TST indexed
	    case 0x93:
	    {
		rega = lsr8(rega);
		cycles = 2;
	    }
	    break; // LSRA
	    case 0x94:
	    {
		regb = lsr8(regb);
		cycles = 2;
	    }
	    break; // LSRB
	    case 0x95:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		writeByte(extended_address, lsr8(operand));
		cycles = (4 + index_cycles);
	    }
	    break; // LSR indexed
	    case 0x96:
	    {
		rega = ror8(rega);
		cycles = 2;
	    }
	    break; // RORA
	    case 0x97:
	    {
		regb = ror8(regb);
		cycles = 2;
	    }
	    break; // RORB
	    case 0x98:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		writeByte(extended_address, ror8(operand));
		cycles = (4 + index_cycles);
	    }
	    break; // ROR indexed
	    case 0x99:
	    {
		rega = asr8(rega);
		cycles = 2;
	    }
	    break; // ASRA
	    case 0x9A:
	    {
		regb = asr8(regb);
		cycles = 2;
	    }
	    break; // ASRB
	    case 0x9B:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		writeByte(extended_address, asr8(operand));
		cycles = (4 + index_cycles);
	    }
	    break; // ASR indexed
	    case 0x9C:
	    {
		rega = asl8(rega);
		cycles = 2;
	    }
	    break; // ASLA
	    case 0x9D:
	    {
		regb = asl8(regb);
		cycles = 2;
	    }
	    break; // ASLB
	    case 0x9E:
	    {
		int index_cycles = indexed_mode();
		uint8_t operand = readByte(extended_address);
		writeByte(extended_address, asl8(operand));
		cycles = (4 + index_cycles);
	    }
	    break; // ASL indexed
	    case 0x9F:
	    {
		cycles = rti();
	    }
	    break; // RTI
	    case 0xA0:
	    {
		rega = rol8(rega);
		cycles = 2;
	    }
	    break; // ROLA
	    case 0xA1:
	    {
		regb = rol8(regb);
		cycles = 2;
	    }
	    break; // ROLB
	    case 0xA8:
	    {
		int index_cycles = indexed_mode();
		pc = extended_address;
		cycles = index_cycles;
	    }
	    break; // JMP indexed
	    case 0xA9:
	    {
		int index_cycles = indexed_mode();
		pushsp16(pc);
		pc = extended_address;
		cycles = (4 + index_cycles);
	    }
	    break; // JSR indexed
	    case 0xAA:
	    {
		cycles = bsr();
	    }
	    break; // BSR
	    case 0xAB:
	    {
		cycles = lbsr();
	    }
	    break; // LBSR
	    case 0xAC:
	    {
		cycles = decbjnz();
	    }
	    break; // DECB, JNZ
	    case 0xAE:
	    {
		cycles = 2;
	    }
	    break; // NOP
	    case 0xB0:
	    {
		cycles = abx();
	    }
	    break; // ABX
	    case 0xB2:
	    {
		cycles = sign_ext();
	    }
	    break; // SEX
	    case 0xB3:
	    {
		cycles = mul();
	    }
	    break; // MUL
	    case 0xB6:
	    {
		cycles = bmove();
	    }
	    break; // BMOVE
	    case 0xC2:
	    {
		setRegD(clear<uint16_t>());
		cycles = 2;
	    }
	    break; // CLRD
	    case 0xC3:
	    {
		int index_cycles = indexed_mode();
		writeWord(extended_address, clear<uint16_t>());
		cycles = (4 + index_cycles);
	    }
	    break; // CLR16 indexed
	    case 0xC4:
	    {
		setRegD(neg16(getRegD()));
		cycles = 2;
	    }
	    break; // NEGD
	    case 0xC5:
	    {
		int index_cycles = indexed_mode();
		uint16_t operand = readWord(extended_address);
		writeWord(extended_address, neg16(operand));
		cycles = (6 + index_cycles);
	    }
	    break; // NEG16 indexed
	    case 0xC6:
	    {
		setRegD(inc_internal16(getRegD()));
		cycles = 2;
	    }
	    break; // INCD
	    case 0xC7:
	    {
		int index_cycles = indexed_mode();
		uint16_t operand = readWord(extended_address);
		writeWord(extended_address, inc_internal16(operand));
		cycles = (6 + index_cycles);
	    }
	    break; // INC16 indexed
	    case 0xC8:
	    {
		setRegD(dec_internal16(getRegD()));
		cycles = 2;
	    }
	    break; // DECD
	    case 0xC9:
	    {
		int index_cycles = indexed_mode();
		uint16_t operand = readWord(extended_address);
		writeWord(extended_address, dec_internal16(operand));
		cycles = (6 + index_cycles);
	    }
	    break; // DEC16 indexed
	    case 0xCC:
	    {
		rega = abs8(rega);
		cycles = 2;
	    }
	    break; // ABSA
	    case 0xCD:
	    {
		regb = abs8(regb);
		cycles = 2;
	    }
	    break; // ABSB
	    case 0xCE:
	    {
		setRegD(abs16(getRegD()));
		cycles = 2;
	    }
	    break; // ABSD
	    case 0xCF:
	    {
		cycles = bset();
	    }
	    break; // BSET
	    case 0xD0:
	    {
		cycles = bset2();
	    }
	    break; // BSET2
	    default: unrecognizedinstr(instr); break;
	}

	return cycles;
    }

    int BotnamiKonami2::handleIRQ()
    {
	status_reg = setbit(status_reg, 7);
	pushsp16(pc);
	pushsp16(usp);
	pushsp16(regy);
	pushsp16(regx);
	pushsp(regdp);
	pushsp(regb);
	pushsp(rega);
	pushsp(status_reg);
	status_reg = setbit(status_reg, 4);
	pc = readWord(0xFFF8);
	return 19;
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

	uint8_t status_reg = status.status_reg;
	stringstream flags;
	flags << (testbit(status_reg, 7) ? "E" : ".");
	flags << (testbit(status_reg, 6) ? "F" : ".");
	flags << (testbit(status_reg, 5) ? "H" : ".");
	flags << (testbit(status_reg, 4) ? "I" : ".");
	flags << (testbit(status_reg, 3) ? "N" : ".");
	flags << (testbit(status_reg, 2) ? "Z" : ".");
	flags << (testbit(status_reg, 1) ? "V" : ".");
	flags << (testbit(status_reg, 0) ? "C" : ".");
	cout << "Flags: " << flags.str() << endl;

	if (print_disassembly)
	{
	    stringstream ss;
	    disassembleinstr(ss, status.pc);
	    cout << "Disassembly: " << ss.str() << endl;
	}

	cout << endl;
    }

    size_t BotnamiKonami2::disassembleinstr(ostream &stream, size_t pc)
    {
	size_t prev_pc = pc;

	uint8_t opcode = readByte(pc++);
	uint8_t arg = readByte(pc);
	uint16_t arg16 = readWord(pc);

	uint16_t branch_offs = ((pc + 1) + int8_t(arg));
	uint16_t lbranch_offs = ((pc + 2) + int16_t(arg16));

	switch (opcode)
	{
	    case 0x08:
	    {
		pc += 1;
		stream << "leax ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x09:
	    {
		pc += 1;
		stream << "leay ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x0A:
	    {
		pc += 1;
		stream << "leau ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x0B:
	    {
		pc += 1;
		stream << "leay ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x0C:
	    {
		pc += 1;
		stream << "pushs";
	    }
	    break;
	    case 0x0E:
	    {
		pc += 1;
		stream << "pulls";
	    }
	    break;
	    case 0x10:
	    {
		pc += 1;
		stream << "lda #$" << hex << int(arg);
	    }
	    break;
	    case 0x11:
	    {
		pc += 1;
		stream << "ldb #$" << hex << int(arg);
	    }
	    break;
	    case 0x12:
	    {
		pc += 1;
		stream << "lda ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x13:
	    {
		pc += 1;
		stream << "ldb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x14:
	    {
		pc += 1;
		stream << "adda #$" << hex << int(arg);
	    }
	    break;
	    case 0x15:
	    {
		pc += 1;
		stream << "addb #$" << hex << int(arg);
	    }
	    break;
	    case 0x16:
	    {
		pc += 1;
		stream << "adda ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x17:
	    {
		pc += 1;
		stream << "addb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x18:
	    {
		pc += 1;
		stream << "adca #$" << hex << int(arg);
	    }
	    break;
	    case 0x19:
	    {
		pc += 1;
		stream << "adcb #$" << hex << int(arg);
	    }
	    break;
	    case 0x1A:
	    {
		pc += 1;
		stream << "adca ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x1B:
	    {
		pc += 1;
		stream << "adcb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x1C:
	    {
		pc += 1;
		stream << "suba #$" << hex << int(arg);
	    }
	    break;
	    case 0x1D:
	    {
		pc += 1;
		stream << "subb #$" << hex << int(arg);
	    }
	    break;
	    case 0x1E:
	    {
		pc += 1;
		stream << "suba ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x1F:
	    {
		pc += 1;
		stream << "subb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x20:
	    {
		pc += 1;
		stream << "sbca #$" << hex << int(arg);
	    }
	    break;
	    case 0x21:
	    {
		pc += 1;
		stream << "sbcb #$" << hex << int(arg);
	    }
	    break;
	    case 0x22:
	    {
		pc += 1;
		stream << "sbca ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x23:
	    {
		pc += 1;
		stream << "sbcb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x24:
	    {
		pc += 1;
		stream << "anda #$" << hex << int(arg);
	    }
	    break;
	    case 0x25:
	    {
		pc += 1;
		stream << "andb #$" << hex << int(arg);
	    }
	    break;
	    case 0x26:
	    {
		pc += 1;
		stream << "anda ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x27:
	    {
		pc += 1;
		stream << "andb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x28:
	    {
		pc += 1;
		stream << "bita #$" << hex << int(arg);
	    }
	    break;
	    case 0x29:
	    {
		pc += 1;
		stream << "bitb #$" << hex << int(arg);
	    }
	    break;
	    case 0x2A:
	    {
		pc += 1;
		stream << "bita ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x2B:
	    {
		pc += 1;
		stream << "bitb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x2C:
	    {
		pc += 1;
		stream << "eora #$" << hex << int(arg);
	    }
	    break;
	    case 0x2D:
	    {
		pc += 1;
		stream << "eorb #$" << hex << int(arg);
	    }
	    break;
	    case 0x2E:
	    {
		pc += 1;
		stream << "eora ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x2F:
	    {
		pc += 1;
		stream << "eorb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x30:
	    {
		pc += 1;
		stream << "ora #$" << hex << int(arg);
	    }
	    break;
	    case 0x31:
	    {
		pc += 1;
		stream << "orb #$" << hex << int(arg);
	    }
	    break;
	    case 0x32:
	    {
		pc += 1;
		stream << "ora ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x33:
	    {
		pc += 1;
		stream << "orb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x34:
	    {
		pc += 1;
		stream << "cmpa #$" << hex << int(arg);
	    }
	    break;
	    case 0x35:
	    {
		pc += 1;
		stream << "cmpb #$" << hex << int(arg);
	    }
	    break;
	    case 0x36:
	    {
		pc += 1;
		stream << "cmpa ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x37:
	    {
		pc += 1;
		stream << "cmpb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x38:
	    {
		pc += 1;
		stream << "setlines #$" << hex << int(arg);
	    }
	    break;
	    case 0x39:
	    {
		pc += 1;
		stream << "setlines ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x3A:
	    {
		pc += 1;
		stream << "sta ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x3B:
	    {
		pc += 1;
		stream << "stb ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x3C:
	    {
		pc += 1;
		stream << "andcc #$" << hex << int(arg);
	    }
	    break;
	    case 0x3D:
	    {
		pc += 1;
		stream << "orcc #$" << hex << int(arg);
	    }
	    break;
	    case 0x3E:
	    {
		pc += 1;
		stream << "exg";
	    }
	    break;
	    case 0x3F:
	    {
		pc += 1;
		stream << "tfr";
	    }
	    break;
	    case 0x40:
	    {
		pc += 2;
		stream << "ldd #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x41:
	    {
		pc += 1;
		stream << "ldd ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x42:
	    {
		pc += 2;
		stream << "ldx #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x43:
	    {
		pc += 1;
		stream << "ldx ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x44:
	    {
		pc += 2;
		stream << "ldy #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x45:
	    {
		pc += 1;
		stream << "ldy ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x46:
	    {
		pc += 2;
		stream << "ldu #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x47:
	    {
		pc += 1;
		stream << "ldu ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x48:
	    {
		pc += 2;
		stream << "lds #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x49:
	    {
		pc += 1;
		stream << "lds ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x4A:
	    {
		pc += 2;
		stream << "cmpd #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x4B:
	    {
		pc += 1;
		stream << "cmpd ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x4C:
	    {
		pc += 2;
		stream << "cmpx #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x4D:
	    {
		pc += 1;
		stream << "cmpx ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x4E:
	    {
		pc += 2;
		stream << "cmpy #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x4F:
	    {
		pc += 1;
		stream << "cmpy ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x50:
	    {
		pc += 2;
		stream << "cmpu #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x51:
	    {
		pc += 1;
		stream << "cmpu ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x52:
	    {
		pc += 2;
		stream << "cmps #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x53:
	    {
		pc += 1;
		stream << "cmps ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x54:
	    {
		pc += 2;
		stream << "addd #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x55:
	    {
		pc += 1;
		stream << "addd ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x56:
	    {
		pc += 2;
		stream << "subd #$" << hex << int(arg16) << endl;
	    }
	    break;
	    case 0x57:
	    {
		pc += 1;
		stream << "subd ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x58:
	    {
		pc += 1;
		stream << "std ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x59:
	    {
		pc += 1;
		stream << "stx ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x5A:
	    {
		pc += 1;
		stream << "sty ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x5B:
	    {
		pc += 1;
		stream << "stu ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x5C:
	    {
		pc += 1;
		stream << "sts ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x60:
	    {
		stream << "bra #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x61:
	    {
		stream << "bhi #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x62:
	    {
		stream << "bcc #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x63:
	    {
		stream << "bne #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x64:
	    {
		stream << "bvc #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x65:
	    {
		stream << "bpl #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x66:
	    {
		stream << "bge #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x67:
	    {
		stream << "bgt #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x68:
	    {
		stream << "lbra #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x69:
	    {
		stream << "lbhi #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x6A:
	    {
		stream << "lbcc #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x6B:
	    {
		stream << "lbne #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x6C:
	    {
		stream << "lbvc #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x6D:
	    {
		stream << "lbpl #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x6E:
	    {
		stream << "lbge #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x6F:
	    {
		stream << "lbgt #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x70:
	    {
		stream << "brn #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x71:
	    {
		stream << "bls #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x72:
	    {
		stream << "bcs #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x73:
	    {
		stream << "beq #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x74:
	    {
		stream << "bvs #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x75:
	    {
		stream << "bmi #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x76:
	    {
		stream << "blt #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x77:
	    {
		stream << "ble #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0x78:
	    {
		stream << "lbrn #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x79:
	    {
		stream << "lbls #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x7A:
	    {
		stream << "lbcs #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x7B:
	    {
		stream << "lbeq #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x7C:
	    {
		stream << "lbvs #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x7D:
	    {
		stream << "lbmi #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x7E:
	    {
		stream << "lblt #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x7F:
	    {
		stream << "lble #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0x80: stream << "clra"; break;
	    case 0x81: stream << "clrb"; break;
	    case 0x82:
	    {
		pc += 1;
		stream << "clr ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x83: stream << "coma"; break;
	    case 0x84: stream << "comb"; break;
	    case 0x85:
	    {
		pc += 1;
		stream << "com ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x86: stream << "nega"; break;
	    case 0x87: stream << "negb"; break;
	    case 0x88:
	    {
		pc += 1;
		stream << "neg ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x89: stream << "inca"; break;
	    case 0x8A: stream << "incb"; break;
	    case 0x8B:
	    {
		pc += 1;
		stream << "inc ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x8C: stream << "deca"; break;
	    case 0x8D: stream << "decb"; break;
	    case 0x8E:
	    {
		pc += 1;
		stream << "dec ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x8F: stream << "rts"; break;
	    case 0x90: stream << "tsta"; break;
	    case 0x91: stream << "tstb"; break;
	    case 0x92:
	    {
		pc += 1;
		stream << "tst ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x93: stream << "lsra"; break;
	    case 0x94: stream << "lsrb"; break;
	    case 0x95:
	    {
		pc += 1;
		stream << "lsr ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x96: stream << "rora"; break;
	    case 0x97: stream << "rorb"; break;
	    case 0x98:
	    {
		pc += 1;
		stream << "ror ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x99: stream << "asra"; break;
	    case 0x9A: stream << "asrb"; break;
	    case 0x9B:
	    {
		pc += 1;
		stream << "asr ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x9C: stream << "asla"; break;
	    case 0x9D: stream << "aslb"; break;
	    case 0x9E:
	    {
		pc += 1;
		stream << "asl ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0x9F: stream << "rti"; break;
	    case 0xA0: stream << "rola"; break;
	    case 0xA1: stream << "rolb"; break;
	    case 0xA8:
	    {
		pc += 1;
		stream << "jmp ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0xA9:
	    {
		pc += 1;
		stream << "jsr ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0xAA:
	    {
		stream << "bsr #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0xAB:
	    {
		stream << "lbsr #$" << hex << int(lbranch_offs);
	    }
	    break;
	    case 0xAC:
	    {
		stream << "decb, jnz #$" << hex << int(branch_offs);
	    }
	    break;
	    case 0xAE: stream << "nop"; break;
	    case 0xB0: stream << "abx"; break;
	    case 0xB2: stream << "sex"; break;
	    case 0xB3: stream << "mul"; break;
	    case 0xB6: stream << "bmove"; break;
	    case 0xC2: stream << "clrd"; break;
	    case 0xC3:
	    {
		pc += 1;
		stream << "clr16 ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0xC4: stream << "negd"; break;
	    case 0xC5:
	    {
		pc += 1;
		stream << "neg16 ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0xC6: stream << "incd"; break;
	    case 0xC7:
	    {
		pc += 1;
		stream << "inc16 ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0xC8: stream << "decd"; break;
	    case 0xC9:
	    {
		pc += 1;
		stream << "dec16 ";
		indexed_mode_dasm(stream, arg, pc);
	    }
	    break;
	    case 0xCC: stream << "absa"; break;
	    case 0xCD: stream << "absb"; break;
	    case 0xCE: stream << "absd"; break;
	    case 0xCF: stream << "bset"; break;
	    case 0xD0: stream << "bset2"; break;
	    default: stream << "unk"; break;
	}

	return (pc - prev_pc);
    }

    void BotnamiKonami2::indexed_mode_dasm(ostream &stream, uint8_t mode, size_t &pc)
    {
	array<string, 8> index_reg = 
	{
	    "?", "?", "x", "y",
	    "?", "u", "s", "pc"
	};

	int idx = ((mode >> 4) & 0x7);
	int type = (mode & 0xF);

	if (testbit(mode, 7))
	{
	    if (testbit(type, 3))
	    {
		switch (type & 0x7)
		{
		    case 0x0: stream << "[a, " << index_reg.at(idx) << "]"; break;
		    case 0x1: stream << "[b, " << index_reg.at(idx) << "]"; break;
		    case 0x4:
		    {
			uint8_t imm = readByte(pc++);
			stream << "[#$" << hex << int(imm) << "]";
		    }
		    break;
		    case 0x7: stream << "[d, " << index_reg.at(idx) << "]"; break;
		    default: stream << "[?, " << index_reg.at(idx) << "]"; break;
		}
	    }
	    else
	    {
		switch (type & 0x7)
		{
		    case 0x0: stream << "a, " << index_reg.at(idx); break;
		    case 0x1: stream << "b, " << index_reg.at(idx); break;
		    case 0x4:
		    {
			uint8_t imm = readByte(pc++);
			stream << "#$" << hex << int(imm);
		    }
		    break;
		    case 0x7: stream << "d, " << index_reg.at(idx); break;
		    default: stream << "?, " << index_reg.at(idx); break;
		}
	    }
	}
	else
	{
	    if (testbit(type, 3))
	    {
		switch (type & 0x7)
		{
		    case 0x0: stream << "[, " << index_reg.at(idx) << "+]"; break;
		    case 0x1: stream << "[, " << index_reg.at(idx) << "++]"; break;
		    case 0x2: stream << "[, -" << index_reg.at(idx) << "]"; break;
		    case 0x3: stream << "[, --" << index_reg.at(idx) << "]"; break;
		    case 0x4:
		    {
			uint8_t imm = readByte(pc++);

			if (testbit(imm, 7))
			{
			    stream << "[#$-" << hex << int(0x100 - imm) << ", " << index_reg.at(idx) << "]";
			}
			else
			{
			    stream << "[#$" << hex << int(imm) << ", " << index_reg.at(idx) << "]";
			}
		    }
		    break;
		    case 0x5:
		    {
			uint16_t imm = readWord(pc);
			pc += 2;

			if (testbit(imm, 15))
			{
			    stream << "[#$-" << hex << int(0x10000 - imm) << ", " << index_reg.at(idx) << "]";
			}
			else
			{
			    stream << "[#$" << hex << int(imm) << ", " << index_reg.at(idx) << "]";
			}
		    }
		    break;
		    case 0x6:
		    {
			stream << "[," << index_reg.at(idx) << "]";
		    }
		    break;
		    case 0x7:
		    {
			uint16_t imm = readWord(pc);
			pc += 2;

			stream << "[$" << hex << int(imm) << "]";
		    }
		    break;
		}
	    }
	    else
	    {
		switch (type & 0x7)
		{
		    case 0x0: stream << ", " << index_reg.at(idx) << "+"; break;
		    case 0x1: stream << ", " << index_reg.at(idx) << "++"; break;
		    case 0x2: stream << ", -" << index_reg.at(idx); break;
		    case 0x3: stream << ", --" << index_reg.at(idx); break;
		    case 0x4:
		    {
			uint8_t imm = readByte(pc++);

			if (testbit(imm, 7))
			{
			    stream << "#$-" << hex << int(0x100 - imm) << ", " << index_reg.at(idx);
			}
			else
			{
			    stream << "#$" << hex << int(imm) << ", " << index_reg.at(idx);
			}
		    }
		    break;
		    case 0x5:
		    {
			uint16_t imm = readWord(pc);
			pc += 2;

			if (testbit(imm, 15))
			{
			    stream << "#$-" << hex << int(0x10000 - imm) << ", " << index_reg.at(idx);
			}
			else
			{
			    stream << "#$" << hex << int(imm) << ", " << index_reg.at(idx);
			}
		    }
		    break;
		    case 0x6:
		    {
			stream << "," << index_reg.at(idx);
		    }
		    break;
		    case 0x7:
		    {
			uint16_t imm = readWord(pc);
			pc += 2;

			stream << "$" << hex << int(imm);
		    }
		    break;
		}
	    }
	}
    }
};