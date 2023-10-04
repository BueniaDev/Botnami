/*
    This file is part of Botnami.
    Copyright (C) 2023 BueniaDev.

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

    
#ifndef BOTNAMI6809_H
#define BOTNAMI6809_H

#include <iostream>
#include <cstdint>
using namespace std;

namespace botnami
{
    struct Botnami6809Pins
    {
	uint8_t data = 0;
	uint16_t addr = 0;
	bool pin_rnw = false;
	bool pin_e = false;
	bool pin_q = false;
	bool pin_bs = false;
	bool pin_ba = false;
	bool pin_mrdy = true;
	bool pin_irq = true;
	bool pin_firq = true;
	bool pin_nmi = true;
	bool pin_avma = false;
	bool pin_busy = false;
	bool pin_lic = false;
	bool pin_res = true;
	bool pin_halt = true;
	bool pin_dma_breq = true;
    };

    class Botnami6809
    {
	enum InstState
	{
	    Reset = 0,
	    Reset0,
	    Reset2,
	    FetchI1,
	    FetchI1V2,
	    FetchI2,
	    Sync,
	    Imm16Lo,
	    Imm16DontCare,
	    IndexedBase,
	    Idx16OffsDontCare0,
	    Idx16OffsDontCare1,
	    Idx16OffsDontCare2,
	    Idx16OffsDontCare3,
	    IdxDontCare3,
	    ExtendedAddrLow,
	    ExtendedDontCare,
	    DirectDontCare,
	    AluEA,
	    AluDontCare,
	    AluWriteback,
	    Alu16Low,
	    Alu16DontCare,
	    Ld16Low,
	    TstDontCare1,
	    BraDontCare,
	    BsrDontCare1,
	    BsrDontCare2,
	    BsrReturnLow,
	    BsrReturnHigh,
	    LBraOffsetLow,
	    LBraDontCare,
	    LBraDontCare2,
	    JsrDontCare,
	    JsrReturnLow,
	    JsrReturnHigh,
	    RtsHigh,
	    RtsLow,
	    RtsDontCare2,
	    SwiStart,
	    IrqDontCare,
	    IrqDontCare2,
	    IrqVectorHigh,
	    IrqVectorLow,
	    IntDontCare,
	    PushDontCare1,
	    PushDontCare2,
	    PushDontCare3,
	    PushAction,
	    PullDontCare1,
	    PullDontCare2,
	    PullAction
	};

	enum AddrMode
	{
	    Inherent = 0,
	    Immediate = 1,
	    Direct = 2,
	    Relative = 3,
	    Indexed = 4,
	    Extended = 5,
	    Invalid = 7
	};

	enum IndexMode
	{
	    PostInc1 = 0,
	    PostInc2 = 1,
	    PreDec1 = 2,
	    PreDec2 = 3,
	    NoOffs = 4,
	    BOffs = 5,
	    AOffs = 6,
	    Bit5Offs = 7,
	    Bit8Offs = 8,
	    Bit16Offs = 9,
	    Invalid0 = 10,
	    ModeDOffs = 11,
	    Bit8OffsPC = 12,
	    Bit16OffsPC = 13,
	    Invalid1 = 14,
	    ExtendedIndirect = 15
	};

	enum ALU8Op
	{
	    Neg = 0x0,
	    Com = 0x3,
	    Lsr = 0x4,
	    Ror = 0x6,
	    Asr = 0x7,
	    Lsl = 0x8,
	    Rol = 0x9,
	    Dec = 0xA,
	    Inc = 0xC,
	    Tst = 0xD,
	    Clr = 0xF,
	    Sub = 0x10,
	    Cmp = 0x11,
	    Sbc = 0x12,
	    And = 0x14,
	    Bit = 0x15,
	    Ld = 0x16,
	    Eor = 0x18,
	    Adc = 0x19,
	    Or = 0x1A,
	    Add = 0x1B,
	    InvalidALU = 0x1F
	};

	enum ALU16Op
	{
	    Sub16 = 0,
	    Add16 = 1,
	    Ld16 = 2,
	    Cmp16 = 3,
	    Lea16 = 4,
	    InvalidALU16 = 7
	};

	enum ALU16Reg
	{
	    RegX = 0,
	    RegY = 1,
	    RegU = 2,
	    RegS = 3,
	    RegD = 4,
	    RegInvalid = 5
	};

	enum IndexReg
	{
	    IdxRegX = 0,
	    IdxRegY = 1,
	    IdxRegU = 2,
	    IdxRegS = 3,
	    IdxRegPC = 4,
	    IdxRegInvalid = 5
	};

	enum IRQType
	{
	    Nmi = 0,
	    Irq = 1,
	    Firq = 2,
	    Swi = 3,
	    Swi2 = 4,
	    Swi3 = 5,
	    None = 7
	};

	public:
	    Botnami6809();
	    ~Botnami6809();

	    void init();
	    void tick();
	    void tickClk(bool clk);
	    void tickE();

	    void debugOutput();

	    bool isFetch()
	    {
		if (is_fetch)
		{
		    return (inst_state == FetchI1);
		}
		else
		{
		    return false;
		}
	    }

	    uint8_t getA()
	    {
		return rega;
	    }

	    uint8_t getB()
	    {
		return regb;
	    }

	    uint8_t getDP()
	    {
		return reg_dp;
	    }

	    uint16_t getX()
	    {
		return regx;
	    }

	    uint16_t getY()
	    {
		return regy;
	    }

	    uint16_t getU()
	    {
		return reg_usp;
	    }

	    uint16_t getS()
	    {
		return reg_ssp;
	    }

	    uint16_t getPC()
	    {
		return reg_pc;
	    }

	    Botnami6809Pins &getPins()
	    {
		return current_pins;
	    }

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 0x1) ? true : false;
	    }

	    template<typename T>
	    T setbit(T reg, int bit)
	    {
		return (reg | (1 << bit));
	    }

	    template<typename T>
	    T resetbit(T reg, int bit)
	    {
		return (reg & ~(1 << bit));
	    }

	    template<typename T>
	    T changebit(T reg, int bit, bool is_set)
	    {
		return (is_set) ? setbit(reg, bit) : resetbit(reg, bit);
	    }

	    Botnami6809Pins current_pins;

	    int clk_phase = 0;

	    bool prev_clk = false;

	    bool prev_q = false;
	    bool prev_e = false;

	    bool is_fetch = false;

	    InstState inst_state = Reset;
	    InstState inst_state_next = Reset;

	    InstState next_state = Reset;
	    InstState next_state_next = Reset;

	    void setState(InstState state)
	    {
		inst_state_next = state;
	    }

	    void setNextState(InstState state)
	    {
		next_state_next = state;
	    }

	    void read()
	    {
		current_pins.pin_rnw = true;
	    }

	    void write()
	    {
		current_pins.pin_rnw = false;
	    }

	    void instIncomplete()
	    {
		current_pins.pin_lic = false;
	    }

	    void instComplete()
	    {
		current_pins.pin_lic = true;
	    }

	    void memAccess()
	    {
		current_pins.pin_avma = true;
	    }

	    void syncAccess()
	    {
		current_pins.pin_avma = false;
	    }

	    void setAddress(uint16_t address)
	    {
		addr_next = address;
	    }

	    void incAddress()
	    {
		addr_next = (addr + 1);
	    }

	    uint8_t getData()
	    {
		return current_pins.data;
	    }

	    void setData(uint8_t data)
	    {
		current_pins.data = data;
	    }

	    uint8_t rega = 0;
	    uint8_t regb = 0;
	    uint8_t reg_dp = 0;
	    uint8_t reg_cc = 0;
	    uint16_t regx = 0;
	    uint16_t regy = 0;
	    uint16_t reg_usp = 0;
	    uint16_t reg_ssp = 0;
	    uint16_t reg_pc = 0;

	    uint8_t rega_next = 0;
	    uint8_t regb_next = 0;
	    uint8_t reg_dp_next = 0;
	    uint8_t reg_cc_next = 0;
	    uint16_t regx_next = 0;
	    uint16_t regy_next = 0;
	    uint16_t reg_usp_next = 0;
	    uint16_t reg_ssp_next = 0;
	    uint16_t reg_pc_next = 0;

	    uint16_t addr = 0;
	    uint16_t addr_next = 0;

	    uint16_t ea = 0;
	    uint16_t ea_next = 0;

	    uint16_t temp = 0;
	    uint16_t temp_next = 0;

	    uint8_t instr1 = 0;
	    uint8_t instr2 = 0;
	    uint8_t instr3 = 0;

	    uint8_t instr1_next = 0;
	    uint8_t instr2_next = 0;
	    uint8_t instr3_next = 0;

	    bool is_page2 = false;
	    bool is_page3 = false;

	    bool is_page2_next = false;
	    bool is_page3_next = false;

	    IRQType int_type = None;
	    IRQType int_type_next = None;

	    AddrMode addr_mode = Invalid;
	    bool is_special_imm = false;
	    bool is_alu8_set0 = false;
	    bool is_alu8_set1 = false;
	    bool is_alu_8bit = false;
	    bool is_one_byte = false;
	    bool is_rega = false;

	    ALU8Op alu8_op = InvalidALU;
	    bool alu8_wb = false;

	    uint8_t alu8_a = 0;
	    uint8_t alu8_b = 0;
	    uint8_t alu8_cc = 0;
	    uint8_t alu8_res = 0;

	    ALU16Op alu16_op = InvalidALU16;
	    bool alu16_wb = false;

	    ALU16Reg alu16_reg = RegInvalid;
	    ALU16Reg alu16_store_reg = RegInvalid;

	    uint16_t alu16_a = 0;
	    uint16_t alu16_b = 0;
	    uint8_t alu16_cc = 0;
	    uint16_t alu16_res = 0;

	    uint8_t alu_cc_out = 0;

	    bool is_jump = false;
	    bool is_store8 = false;
	    bool is_store16 = false;
	    bool is_store_b = false;
	    bool is_branch_cond = false;

	    IndexReg indexed_reg = IdxRegInvalid;
	    IndexMode indexed_mode = Invalid0;
	    bool is_idx_indirect = false;

	    bool isHalf()
	    {
		return testbit(reg_cc, 5);
	    }

	    void setHalf(bool is_set)
	    {
		alu_cc_out = changebit(alu_cc_out, 5, is_set);
	    }

	    bool isSign()
	    {
		return testbit(reg_cc, 3);
	    }

	    void setSign(bool is_set)
	    {
		alu_cc_out = changebit(alu_cc_out, 3, is_set);
	    }

	    bool isZero()
	    {
		return testbit(reg_cc, 2);
	    }

	    void setZero(bool is_set)
	    {
		alu_cc_out = changebit(alu_cc_out, 2, is_set);
	    }

	    bool isOverflow()
	    {
		return testbit(reg_cc, 1);
	    }

	    void setOverflow(bool is_set)
	    {
		alu_cc_out = changebit(alu_cc_out, 1, is_set);
	    }

	    bool isCarry()
	    {
		return testbit(reg_cc, 0);
	    }

	    void setCarry(bool is_set)
	    {
		alu_cc_out = changebit(alu_cc_out, 0, is_set);
	    }

	    void fetchOneByteInstr(int instr_hi, int instr_lo)
	    {
		switch (instr_hi)
		{
		    case 0x4:
		    case 0x5: is_one_byte = true; break;
		    case 0x1:
		    {
			switch (instr_lo)
			{
			    case 0x2:
			    case 0x3:
			    case 0x9:
			    case 0xD: is_one_byte = true; break;
			    default: is_one_byte = false; break;
			}
		    }
		    break;
		    case 0x3:
		    {
			switch (instr_lo)
			{
			    case 0x9:
			    case 0xA:
			    case 0xB:
			    case 0xD:
			    case 0xE:
			    case 0xF: is_one_byte = true; break;
			    default: is_one_byte = false; break;
			}
		    }
		    break;
		    default: is_one_byte = false; break;
		}
	    }

	    void fetchBranch(uint8_t instr)
	    {
		is_branch_cond = false;
		int instr_lo = (instr & 0xF);

		if ((instr == 0x8D) || (instr == 0x17) || (instr == 0x16))
		{
		    is_branch_cond = true;
		}
		else
		{
		    switch (instr_lo)
		    {
			case 0x0: is_branch_cond = true; break; // AL
			case 0x1: is_branch_cond = false; break; // NV
			case 0x4: is_branch_cond = !isCarry(); break; // CC
			case 0x5: is_branch_cond = isCarry(); break; // CS
			case 0x6: is_branch_cond = !isZero(); break; // NE
			case 0x7: is_branch_cond = isZero(); break; // EQ
			case 0xF: is_branch_cond = ((isSign() != isOverflow()) || isZero()); break; // LE
			default:
			{
			    cout << "Unrecognized branch condition of " << hex << int(instr_lo) << endl;
			    throw runtime_error("Botnami error");
			}
			break;
		    }
		}
	    }

	    void fetchAddrMode(int instr_hi, int instr_lo)
	    {
		switch (instr_hi)
		{
		    case 0x1:
		    {
			switch (instr_lo)
			{
			    case 0x2:
			    case 0x3:
			    case 0x9:
			    case 0xD: addr_mode = Inherent; break;
			    case 0x6:
			    case 0x7: addr_mode = Relative; break;
			    case 0xA:
			    case 0xC:
			    case 0xE:
			    case 0xF: addr_mode = Immediate; break;
			    default: addr_mode = Invalid; break;
			}
		    }
		    break;
		    case 0x2: addr_mode = Relative; break;
		    case 0x3:
		    {
			switch (instr_lo)
			{
			    case 0x8:
			    case 0xE: addr_mode = Invalid; break;
			    default:
			    {
				int hi_bits = ((instr_lo >> 2) & 0x3);

				switch (hi_bits)
				{
				    case 0: addr_mode = Indexed; break;
				    case 1: addr_mode = Immediate; break;
				    default: addr_mode = Inherent; break;
				}
			    }
			    break;
			}
		    }
		    break;
		    case 0x4:
		    case 0x5: addr_mode = Inherent; break;
		    case 0x6: addr_mode = Indexed; break;
		    case 0x8:
		    {
			switch (instr_lo)
			{
			    case 0x7:
			    case 0xF: addr_mode = Invalid; break;
			    case 0xD: addr_mode = Relative; break;
			    default: addr_mode = Immediate; break;
			}
		    }
		    break;
		    case 0x9: addr_mode = Direct; break;
		    case 0xA: addr_mode = Indexed; break;
		    case 0xB: addr_mode = Extended; break;
		    case 0xC: addr_mode = Immediate; break;
		    case 0xE: addr_mode = Indexed; break;
		    case 0xF: addr_mode = Extended; break;
		    default:
		    {
			cout << "Unrecognized instruction high bits of " << hex << int(instr_hi) << endl;
			throw runtime_error("Botnami error");
		    }
		    break;
		}
	    }

	    void decodeIndexed(uint8_t instr)
	    {
		is_idx_indirect = false;
		indexed_mode = Invalid0;
		indexed_reg = IdxRegInvalid;

		if (!testbit(instr, 7))
		{
		    indexed_mode = Bit5Offs;
		}
		else
		{
		    indexed_mode = static_cast<IndexMode>(instr & 0xF);
		    is_idx_indirect = testbit(instr, 4);
		}

		if ((indexed_mode != Bit8OffsPC) && (indexed_mode != Bit16OffsPC))
		{
		    int reg_num = ((instr >> 5) & 0x3);
		    indexed_reg = static_cast<IndexReg>(reg_num);
		}
		else
		{
		    indexed_reg = IdxRegPC;
		}
	    }

	    void fetchJmp(int instr_hi, int instr_lo)
	    {
		switch (instr_hi)
		{
		    case 0x0:
		    case 0x6:
		    case 0x7:
		    {
			is_jump = (instr_lo == 0xE);
		    }
		    break;
		    default: is_jump = false; break;
		}
	    }

	    void fetchStore8(uint8_t instr)
	    {
		switch (instr)
		{
		    case 0x97:
		    case 0xA7:
		    case 0xB7:
		    {
			is_store8 = true;
			is_store_b = false;
		    }
		    break;
		    case 0xD7:
		    case 0xE7:
		    case 0xF7:
		    {
			is_store8 = true;
			is_store_b = true;
		    }
		    break;
		    default:
		    {
			is_store8 = false;
			is_store_b = true;
		    }
		    break;
		}
	    }

	    void fetchStore16(uint8_t instr)
	    {
		switch (instr)
		{
		    case 0x9F:
		    case 0xAF:
		    case 0xBF:
		    {
			is_store16 = true;
			alu16_store_reg = is_page2 ? RegY : RegX;
		    }
		    break;
		    case 0xDF:
		    case 0xEF:
		    case 0xFF:
		    {
			is_store16 = true;
			alu16_store_reg = is_page2 ? RegS : RegU;
		    }
		    break;
		    case 0xDD:
		    case 0xED:
		    case 0xFD:
		    {
			is_store16 = true;
			alu16_store_reg = RegD;
		    }
		    break;
		    default:
		    {
			is_store16 = false;
			alu16_store_reg = RegInvalid;
		    }
		    break;
		}
	    }

	    void fetchSpecialImm(int instr_hi, int instr_lo)
	    {
		switch (instr_hi)
		{
		    case 0x1:
		    {
			switch (instr_lo)
			{
			    case 0xA:
			    case 0xC:
			    case 0xE:
			    case 0xF: is_special_imm = true; break;
			    default: is_special_imm = false; break;
			}
		    }
		    break;
		    case 0x3:
		    {
			switch (instr_lo)
			{
			    case 0x3:
			    case 0x4:
			    case 0x5:
			    case 0x6:
			    case 0x7: is_special_imm = true; break;
			    default: is_special_imm = false; break;
			}
		    }
		    break;
		    default:
		    {
			is_special_imm = false;
		    }
		    break;
		}
	    }

	    void fetchALU8Bit(int instr_hi, int instr_lo)
	    {
		switch (instr_hi)
		{
		    case 0x0:
		    case 0x4:
		    case 0x5:
		    case 0x6:
		    case 0x7:
		    {
			switch (instr_lo)
			{
			    case 0x1:
			    case 0x2:
			    case 0x5:
			    case 0xB:
			    case 0xE: is_alu8_set0 = false; break;
			    default: is_alu8_set0 = true; break;
			}
		    }
		    break;
		    default: is_alu8_set0 = false; break;
		}

		if (instr_hi >= 0x8)
		{
		    switch (instr_lo)
		    {
			case 0x3:
			case 0x7:
			case 0xC:
			case 0xD:
			case 0xE:
			case 0xF: is_alu8_set1 = false; break;
			default: is_alu8_set1 = true; break;
		    }
		}
		else
		{
		    is_alu8_set1 = false;
		}

		is_alu_8bit = (is_alu8_set0 || is_alu8_set1);
	    }

	    void fetchRegA(int instr_hi)
	    {
		switch (instr_hi)
		{
		    case 0x4:
		    case 0x8:
		    case 0x9:
		    case 0xA:
		    case 0xB: is_rega = true; break;
		    default: is_rega = false; break;
		}
	    }

	    void fetchALU8(uint8_t instr)
	    {
		int op = ((testbit(instr, 7) << 4) | (instr & 0xF));
		alu8_op = static_cast<ALU8Op>(op);

		switch (alu8_op)
		{
		    case Cmp:
		    case Tst:
		    case Bit: alu8_wb = false; break;
		    default: alu8_wb = true; break;
		}
	    }

	    uint32_t calcOverflow(uint16_t opa, uint16_t opb, uint32_t res)
	    {
		return (opa ^ opb ^ res ^ (res >> 1));
	    }

	    bool calcHalfCarry(uint16_t opa, uint16_t opb, uint32_t res)
	    {
		uint16_t half_res = (opa ^ opb ^ res);
		return testbit(half_res, 4);
	    }

	    void runALU8()
	    {
		alu_cc_out = alu8_cc;

		switch (alu8_op)
		{
		    case Add:
		    {
			uint16_t res32 = (alu8_a + alu8_b);
			alu8_res = (res32 & 0xFF);
			setCarry(testbit(res32, 8));

			uint32_t overflow_res = calcOverflow(alu8_a, alu8_b, alu8_res);
			setOverflow(testbit(overflow_res, 7));
			setHalf(calcHalfCarry(alu8_a, alu8_b, alu8_res));
		    }
		    break;
		    case Sub:
		    {
			uint16_t res32 = (alu8_a - alu8_b);
			alu8_res = (res32 & 0xFF);
			setCarry(testbit(res32, 8));

			uint32_t overflow_res = calcOverflow(alu8_a, alu8_b, alu8_res);
			setOverflow(testbit(overflow_res, 7));
		    }
		    break;
		    case Cmp:
		    {
			uint16_t res32 = (alu8_a - alu8_b);
			alu8_res = (res32 & 0xFF);
			setCarry(testbit(res32, 8));

			uint32_t overflow_res = calcOverflow(alu8_a, alu8_b, alu8_res);
			setOverflow(testbit(overflow_res, 7));
		    }
		    break;
		    case Dec:
		    {
			uint16_t res = (alu8_a - 1);
			alu8_res = (res & 0xFF);
			uint32_t overflow_res = calcOverflow(alu8_a, 1, alu8_res);
			setOverflow(testbit(overflow_res, 7));
		    }
		    break;
		    case Clr:
		    {
			alu8_res = 0;
			setOverflow(false);
			setCarry(false);
		    }
		    break;
		    case Ld:
		    {
			alu8_res = alu8_b;
			setOverflow(false);
		    }
		    break;
		    case And:
		    {
			alu8_res = (alu8_a & alu8_b);
			setOverflow(false);
		    }
		    break;
		    case Or:
		    {
			alu8_res = (alu8_a | alu8_b);
			setOverflow(false);
		    }
		    break;
		    case Lsl:
		    {
			setCarry(testbit(alu8_a, 7));
			alu8_res = (alu8_a << 1);
			uint32_t overflow_res = calcOverflow(alu8_a, alu8_a, alu8_res);
			setOverflow(testbit(overflow_res, 7));
		    }
		    break;
		    case Lsr:
		    {
			setCarry(testbit(alu8_a, 0));
			alu8_res = (alu8_a >> 1);
		    }
		    break;
		    default:
		    {
			cout << "Unrecognized ALU 8-bit opcode of " << hex << int(alu8_op) << endl;
			throw runtime_error("Botnami error");
		    }
		    break;
		}

		setSign(testbit(alu8_res, 7));
		setZero(alu8_res == 0);
	    }

	    void fetchALU16(bool p2, bool p3, uint8_t instr)
	    {
		alu16_wb = true;
		uint16_t instr_val = ((p2 << 9) | (p3 << 8) | instr);

		switch (instr_val)
		{
		    case 0x08C:
		    case 0x09C:
		    case 0x0AC:
		    case 0x0BC:
		    {
			alu16_op = Cmp16;
			alu16_reg = RegX;
			alu16_wb = false;
		    }
		    break;
		    case 0x0CC:
		    case 0x0DC:
		    case 0x0EC:
		    case 0x0FC:
		    {
			alu16_op = Ld16;
			alu16_reg = RegD;
		    }
		    break;
		    case 0x08E:
		    case 0x09E:
		    case 0x0AE:
		    case 0x0BE:
		    {
			alu16_op = Ld16;
			alu16_reg = RegX;
		    }
		    break;
		    case 0x28E:
		    case 0x29E:
		    case 0x2AE:
		    case 0x2BE:
		    {
			alu16_op = Ld16;
			alu16_reg = RegY;
		    }
		    break;
		    case 0x2CE:
		    case 0x2DE:
		    case 0x2EE:
		    case 0x2FE:
		    {
			alu16_op = Ld16;
			alu16_reg = RegS;
		    }
		    break;
		    case 0x030:
		    {
			alu16_op = Lea16;
			alu16_reg = RegX;
		    }
		    break;
		    case 0x031:
		    {
			alu16_op = Lea16;
			alu16_reg = RegY;
		    }
		    break;
		    case 0x032:
		    {
			alu16_op = Lea16;
			alu16_reg = RegS;
		    }
		    break;
		    case 0x033:
		    {
			alu16_op = Lea16;
			alu16_reg = RegU;
		    }
		    break;
		    case 0x283:
		    case 0x293:
		    case 0x2A3:
		    case 0x2B3:
		    case 0x28C:
		    case 0x29C:
		    case 0x2AC:
		    case 0x2BC:
		    case 0x183:
		    case 0x193:
		    case 0x1A3:
		    case 0x1B3:
		    case 0x18C:
		    case 0x19C:
		    case 0x1AC:
		    case 0x1BC:
		    case 0x0C3:
		    case 0x0D3:
		    case 0x0E3:
		    case 0x0F3:
		    case 0x0CE:
		    case 0x0DE:
		    case 0x0EE:
		    case 0x0FE:
		    case 0x083:
		    case 0x093:
		    case 0x0A3:
		    case 0x0B3:
		    case 0x03A:
		    {
			cout << "Unrecognized ALU 16-bit instruction of " << hex << int(instr_val) << endl;
			throw runtime_error("Botnami error");
		    }
		    break;
		    default:
		    {
			alu16_op = InvalidALU16;
		    }
		    break;
		}
	    }

	    void runALU16()
	    {
		alu_cc_out = alu16_cc;

		switch (alu16_op)
		{
		    case Add16:
		    {
			uint32_t res32 = (alu16_a + alu16_b);
			alu16_res = (res32 & 0xFFFF);
			setCarry(testbit(res32, 16));

			uint32_t overflow_res = calcOverflow(alu16_a, alu16_b, alu16_res);
			setOverflow(testbit(overflow_res, 15));
		    }
		    break;
		    case Cmp16:
		    {
			uint32_t res32 = (alu16_a - alu16_b);
			alu16_res = (res32 & 0xFFFF);
			setCarry(testbit(res32, 16));

			uint32_t overflow_res = calcOverflow(alu16_a, alu16_b, alu16_res);
			setOverflow(testbit(overflow_res, 15));
		    }
		    break;
		    case Ld16:
		    {
			alu16_res = alu16_b;
			setOverflow(false);
		    }
		    break;
		    case Lea16:
		    {
			alu16_res = alu16_a;
		    }
		    break;
		    default:
		    {
			cout << "Unrecognized ALU 16-bit opcode of " << hex << int(alu16_op) << endl;
			throw runtime_error("Botnami error");
		    }
		    break;
		}

		setZero(alu16_res == 0);

		if (alu16_op != Lea16)
		{
		    setSign(testbit(alu16_res, 15));
		}
	    }

	    void decodeInstruction(uint8_t instr)
	    {
		int instr_hi = ((instr >> 4) & 0xF);
		int instr_lo = (instr & 0xF);

		alu8_op = InvalidALU;
		alu16_op = InvalidALU16;

		fetchAddrMode(instr_hi, instr_lo);
		fetchSpecialImm(instr_hi, instr_lo);
		fetchALU8Bit(instr_hi, instr_lo);
		fetchRegA(instr_hi);
		fetchOneByteInstr(instr_hi, instr_lo);
		fetchJmp(instr_hi, instr_lo);
		fetchStore8(instr);
		fetchStore16(instr);
	    }

	    void execInst();
	    void latchValues();
	    void setValues();

	    inline void mapInstr()
	    {
		uint8_t map_instr = getData();

		int inst_hi = (map_instr >> 4);
		int inst_lo = (map_instr & 0xF);

		switch (inst_hi)
		{
		    case 0x0:
		    case 0x4:
		    case 0x5:
		    case 0x6:
		    case 0x7:
		    {
			map_instr = (inst_hi << 4);
			switch (inst_lo)
			{
			    case 0x1: map_instr |= 0x0; break;
			    case 0x2: map_instr |= 0x3; break;
			    case 0x5: map_instr |= 0x4; break;
			    case 0xB: map_instr |= 0xA; break;
			    default: map_instr |= inst_lo; break;
			}
		    }
		    break;
		}

		instr1 = map_instr;
	    }

	    void decSP()
	    {
		uint16_t sp_addr = testbit(temp, 14) ? (reg_usp - 1) : (reg_ssp - 1);
		setAddress(sp_addr);

		if (testbit(temp, 14))
		{
		    reg_usp_next = sp_addr;
		}
		else
		{
		    reg_ssp_next = sp_addr;
		}
	    }

	    void incSP()
	    {
		uint16_t sp_addr = testbit(temp, 14) ? reg_usp : reg_ssp;
		setAddress(sp_addr);

		if (testbit(temp, 14))
		{
		    reg_usp_next = (sp_addr + 1);
		}
		else
		{
		    reg_ssp_next = (sp_addr + 1);
		}
	    }
    };
};


#endif // BOTNAMI6809_H