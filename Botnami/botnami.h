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

#ifndef BOTNAMI_H
#define BOTNAMI_H

#include <iostream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <array>
#include <functional>
#include <memory>
#include "botnami_api.h"
using namespace std;
using namespace std::placeholders;

namespace botnami
{
    class BOTNAMI_API BotnamiInterface
    {
	public:
	    BotnamiInterface();
	    virtual ~BotnamiInterface();

	    virtual uint8_t readByte(uint16_t addr)
	    {
		cout << "Reading value from address of " << hex << int(addr) << endl;
		exit(0);
		return 0;
	    }

	    virtual bool isSeperateOps()
	    {
		return false;
	    }

	    virtual uint8_t readOpcode(uint16_t addr)
	    {
		cout << "Reading opcode from address of " << hex << int(addr) << endl;
		exit(0);
		return 0;
	    }

	    virtual void writeByte(uint16_t addr, uint8_t data)
	    {
		cout << "Writing value of " << hex << int(data) << " to address of " << hex << int(addr) << endl;
		exit(0);
	    }

	    virtual void setLines(uint8_t data)
	    {
		cout << "Writing value of " << hex << int(data) << " to KONAMI-2 lines callback" << endl;
		exit(0);
	    }
    };

    class BOTNAMI_API BotnamiCPU
    {
	public:
	    BotnamiCPU();
	    ~BotnamiCPU();

	    virtual void init();
	    virtual void shutdown();
	    virtual void reset();

	    int executenextinstr();
	    virtual void debugoutput(bool print_disassembly = true);
	    virtual size_t disassembleinstr(ostream &stream, size_t pc);

	    void fireIRQ(bool line = true);

	    void setinterface(BotnamiInterface &cb);

	    bool is_half()
	    {
		return testbit(status_reg, 5);
	    }

	    bool is_irq()
	    {
		return testbit(status_reg, 4);
	    }

	    bool is_sign()
	    {
		return testbit(status_reg, 3);
	    }

	    bool is_zero()
	    {
		return testbit(status_reg, 2);
	    }

	    bool is_overflow()
	    {
		return testbit(status_reg, 1);
	    }

	    bool is_carry()
	    {
		return testbit(status_reg, 0);
	    }

	protected:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 1) ? true : false;
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

	    uint8_t rega = 0;
	    uint8_t regb = 0;
	    uint16_t regx = 0;
	    uint16_t regy = 0;
	    uint8_t regdp = 0;
	    uint8_t status_reg = 0;
	    uint16_t pc = 0;
	    uint16_t ssp = 0;
	    uint16_t usp = 0;

	    bool is_irq_pending = false;

	    virtual int handleIRQ();

	    bool isIRQPending();

	    virtual void setStatus()
	    {
		return;
	    }

	    void set_half(bool is_set)
	    {
		status_reg = changebit(status_reg, 5, is_set);
	    }

	    void set_sign(bool is_set)
	    {
		status_reg = changebit(status_reg, 3, is_set);
	    }

	    void set_zero(bool is_set)
	    {
		status_reg = changebit(status_reg, 2, is_set);
	    }

	    void set_overflow(bool is_set)
	    {
		status_reg = changebit(status_reg, 1, is_set);
	    }

	    void set_carry(bool is_set)
	    {
		status_reg = changebit(status_reg, 0, is_set);
	    }

	    template<typename T>
	    void set_n(T data)
	    {
		int sign_bit = ((sizeof(T) * 8) - 1);
		set_sign(testbit(data, sign_bit));
	    }

	    template<typename T>
	    void set_z(T data)
	    {
		set_zero(data == 0);
	    }

	    template<typename T>
	    void set_nz(T data)
	    {
		set_n(data);
		set_z(data);
	    }

	    template<typename T>
	    void set_nzv(T data)
	    {
		set_n(data);
		set_z(data);
		set_v(data, T(0), uint32_t(data));
	    }

	    template<typename T>
	    void set_c(uint32_t result)
	    {
		int high_bit = (sizeof(T) * 8);
		set_carry(testbit(result, high_bit));
	    }

	    template<typename T>
	    void set_v(T op1, T op2, uint32_t result)
	    {
		int high_bit = ((sizeof(T) * 8) - 1);
		uint32_t overflow_res = (op1 ^ op2 ^ result ^ (result >> 1));
		set_overflow(testbit(overflow_res, high_bit));
	    }

	    template<typename T>
	    void set_h(T op1, T op2, uint32_t result)
	    {
		uint32_t half_res = (op1 ^ op2 ^ result);
		set_half(testbit(half_res, 4));
	    }

	    bool is_cond_ne()
	    {
		return !is_zero();
	    }

	    bool is_cond_eq()
	    {
		return is_zero();
	    }

	    bool is_cond_cc()
	    {
		return !is_carry();
	    }

	    bool is_cond_cs()
	    {
		return is_carry();
	    }

	    bool is_cond_vc()
	    {
		return !is_overflow();
	    }

	    bool is_cond_vs()
	    {
		return is_overflow();
	    }

	    bool is_cond_pl()
	    {
		return !is_sign();
	    }

	    bool is_cond_mi()
	    {
		return is_sign();
	    }

	    bool is_cond_hi()
	    {
		return (!is_carry() && !is_zero());
	    }

	    bool is_cond_ls()
	    {
		return (is_carry() || is_zero());
	    }

	    bool is_cond_ge()
	    {
		return (is_sign() == is_overflow());
	    }

	    bool is_cond_lt()
	    {
		return (is_sign() != is_overflow());
	    }

	    bool is_cond_gt()
	    {
		return (!is_zero() && (is_sign() == is_overflow()));
	    }

	    bool is_cond_le()
	    {
		return (is_zero() || (is_sign() != is_overflow()));
	    }

	    int branch(bool is_cond = true)
	    {
		int8_t offs = getimmByte();

		if (is_cond)
		{
		    pc += offs;
		}

		return 3;
	    }

	    int lbranch(bool is_cond = true)
	    {
		int16_t offs = getimmWord();

		if (is_cond)
		{
		    pc += offs;
		    return 6;
		}

		return 5;
	    }

	    int lbranchk(bool is_cond = true)
	    {
		int16_t offs = getimmWord();

		if (is_cond)
		{
		    pc += offs;
		    return 5;
		}

		return 4;
	    }

	    template<bool set_hc = true>
	    uint8_t add_internal8(uint8_t source, uint8_t operand, bool is_carry = false)
	    {
		uint16_t result = (source + operand + is_carry);
		set_nz<uint8_t>(result);
		set_v<uint8_t>(source, operand, result);
		set_c<uint8_t>(result);

		if (set_hc)
		{
		    set_h<uint8_t>(source, operand, result);
		}

		return result;
	    }

	    uint16_t add_internal16(uint16_t source, uint16_t operand, bool is_carry = false)
	    {
		uint32_t result = (source + operand + is_carry);
		set_nz<uint16_t>(result);
		set_v<uint16_t>(source, operand, result);
		set_c<uint16_t>(result);
		return result;
	    }

	    uint8_t sub_internal8(uint8_t source, uint8_t operand, bool is_carry = false)
	    {
		uint16_t result = (source - operand - is_carry);
		set_nz<uint8_t>(result);
		set_v<uint8_t>(source, operand, result);
		set_c<uint8_t>(result);
		return result;
	    }

	    uint16_t sub_internal16(uint16_t source, uint16_t operand, bool is_carry = false)
	    {
		uint32_t result = (source - operand - is_carry);
		set_nz<uint16_t>(result);
		set_v<uint16_t>(source, operand, result);
		set_c<uint16_t>(result);
		return result;
	    }

	    uint8_t and_internal8(uint8_t source, uint8_t operand)
	    {
		uint8_t result = (source & operand);
		set_overflow(false);
		set_nz(result);
		return result;
	    }

	    uint8_t or_internal8(uint8_t source, uint8_t operand)
	    {
		uint8_t result = (source | operand);
		set_overflow(false);
		set_nz(result);
		return result;
	    }

	    uint8_t asl_internal8(uint8_t source)
	    {
		uint16_t result = (source << 1);
		set_nz<uint8_t>(result);
		set_v<uint8_t>(source, source, result);
		set_c<uint8_t>(result);
		return result;
	    }

	    uint8_t asr_internal8(uint8_t source)
	    {
		set_carry(testbit(source, 0));
		uint8_t result = (int8_t(source) >> 1);
		set_nz<uint8_t>(result);
		return result;
	    }

	    uint8_t lsr_internal8(uint8_t source)
	    {
		set_carry(testbit(source, 0));
		uint8_t result = (source >> 1);
		set_nz<uint8_t>(result);
		return result;
	    }

	    uint8_t inc_internal8(uint8_t data)
	    {
		uint16_t result = (data + 1);
		set_nz<uint8_t>(result);
		set_v<uint8_t>(data, 1, result);
		return uint8_t(result);
	    }

	    uint8_t dec_internal8(uint8_t data)
	    {
		uint16_t result = (data - 1);
		set_nz<uint8_t>(result);
		set_v<uint8_t>(data, 1, result);
		return uint8_t(result);
	    }

	    uint8_t com_internal8(uint8_t data)
	    {
		uint8_t result = ~data;
		set_overflow(false);
		set_carry(true);
		set_nz<uint8_t>(result);
		return result;
	    }

	    uint8_t load8(uint8_t data)
	    {
		set_nzv<uint8_t>(data);
		return data;
	    }

	    uint16_t load16(uint16_t data)
	    {
		set_nzv<uint16_t>(data);
		return data;
	    }

	    void store8(uint16_t addr, uint8_t data)
	    {
		set_nzv<uint8_t>(data);
		writeByte(addr, data);
	    }

	    void store16(uint16_t addr, uint16_t data)
	    {
		set_nzv<uint16_t>(data);
		writeWord(addr, data);
	    }

	    uint8_t add8(uint8_t source, uint8_t operand)
	    {
		return add_internal8(source, operand);
	    }

	    uint16_t add16(uint16_t source, uint16_t operand)
	    {
		return add_internal16(source, operand);
	    }

	    uint8_t adc8(uint8_t source, uint8_t operand)
	    {
		return add_internal8(source, operand, is_carry());
	    }

	    uint8_t sub8(uint8_t source, uint8_t operand)
	    {
		return sub_internal8(source, operand);
	    }

	    uint16_t sub16(uint16_t source, uint16_t operand)
	    {
		return sub_internal16(source, operand);
	    }

	    uint8_t sbc8(uint8_t source, uint8_t operand)
	    {
		return sub_internal8(source, operand, is_carry());
	    }

	    void cmp8(uint8_t source, uint8_t operand)
	    {
		sub_internal8(source, operand);
	    }

	    void cmp16(uint16_t source, uint16_t operand)
	    {
		sub_internal16(source, operand);
	    }

	    uint8_t and8(uint8_t source, uint8_t operand)
	    {
		return and_internal8(source, operand);
	    }

	    uint8_t or8(uint8_t source, uint8_t operand)
	    {
		return or_internal8(source, operand);
	    }

	    void bit8(uint8_t source, uint8_t operand)
	    {
		and_internal8(source, operand);
	    }

	    uint8_t asl8(uint8_t source)
	    {
		return asl_internal8(source);
	    }

	    uint8_t asr8(uint8_t source)
	    {
		return asr_internal8(source);
	    }

	    uint8_t lsr8(uint8_t source)
	    {
		return lsr_internal8(source);
	    }

	    uint8_t com8(uint8_t source)
	    {
		return com_internal8(source);
	    }

	    int pushs()
	    {
		uint8_t stack_reg = getimmByte();

		int cycles = 5;

		if (testbit(stack_reg, 7))
		{
		    pushsp16(pc);
		    cycles += 2;
		}

		if (testbit(stack_reg, 6))
		{
		    pushsp16(usp);
		    cycles += 2;
		}

		if (testbit(stack_reg, 5))
		{
		    pushsp16(regy);
		    cycles += 2;
		}

		if (testbit(stack_reg, 4))
		{
		    pushsp16(regx);
		    cycles += 2;
		}

		if (testbit(stack_reg, 3))
		{
		    pushsp(regdp);
		    cycles += 1;
		}

		if (testbit(stack_reg, 2))
		{
		    pushsp(regb);
		    cycles += 1;
		}

		if (testbit(stack_reg, 1))
		{
		    pushsp(rega);
		    cycles += 1;
		}

		if (testbit(stack_reg, 0))
		{
		    pushsp(status_reg);
		    cycles += 1;
		}

		return cycles;
	    }

	    int pulls()
	    {
		uint8_t stack_reg = getimmByte();
		int cycles = 5;

		if (testbit(stack_reg, 0))
		{
		    status_reg = pullsp();
		    cycles += 1;
		}

		if (testbit(stack_reg, 1))
		{
		    rega = pullsp();
		    cycles += 1;
		}

		if (testbit(stack_reg, 2))
		{
		    regb = pullsp();
		    cycles += 1;
		}

		if (testbit(stack_reg, 3))
		{
		    regdp = pullsp();
		    cycles += 1;
		}

		if (testbit(stack_reg, 4))
		{
		    regx = pullsp16();
		    cycles += 2;
		}

		if (testbit(stack_reg, 5))
		{
		    regy = pullsp16();
		    cycles += 2;
		}

		if (testbit(stack_reg, 6))
		{
		    usp = pullsp16();
		    cycles += 2;
		}

		if (testbit(stack_reg, 7))
		{
		    pc = pullsp16();
		    cycles += 2;
		}

		return cycles;
	    }

	    void pushsp(uint8_t val)
	    {
		ssp -= 1;
		writeByte(ssp, val);
	    }

	    uint8_t pullsp()
	    {
		uint8_t value = readByte(ssp);
		ssp += 1;
		return value;
	    }

	    void pushsp16(uint16_t val)
	    {
		pushsp((val & 0xFF));
		pushsp((val >> 8));
	    }

	    uint16_t pullsp16()
	    {
		uint8_t high = pullsp();
		uint8_t low = pullsp();
		return ((high << 8) | low);
	    }

	    int rts()
	    {
		pc = pullsp16();
		return 5;
	    }

	    int rti()
	    {
		int cycles = 6;
		status_reg = pullsp();

		if (testbit(status_reg, 7))
		{
		    rega = pullsp();
		    regb = pullsp();
		    regdp = pullsp();
		    regx = pullsp16();
		    regy = pullsp16();
		    usp = pullsp16();
		    cycles += 9;
		}

		pc = pullsp16();
		return cycles;
	    }

	    int bsr()
	    {
		int8_t offs = getimmByte();
		uint16_t addr = (pc + offs);
		pushsp16(pc);
		pc = addr;
		return 7;
	    }

	    virtual int executeinstr(uint8_t instr);

	    uint8_t readOpcode();
	    virtual uint8_t readOpcode(uint16_t addr);
	    uint8_t readByte(uint16_t addr);
	    uint16_t readWord(uint16_t addr);

	    void writeByte(uint16_t addr, uint8_t data);
	    void writeWord(uint16_t addr, uint16_t data);

	    uint8_t getimmByte();
	    uint16_t getimmWord();

	    void setLines(uint8_t data);

	    void unrecognizedinstr(uint8_t instr);

	private:
	    unique_ptr<BotnamiInterface> inter;
    };

    class BOTNAMI_API Botnami6800 : public BotnamiCPU
    {
	public:
	    Botnami6800();
	    virtual ~Botnami6800();

	    void init()
	    {
		BotnamiCPU::init();
		cout << "Botnami6800::Initialized" << endl;
	    }

	    void shutdown()
	    {
		cout << "Botnami6800::Shutting down..." << endl;
		BotnamiCPU::shutdown();
	    }

	    void reset()
	    {
		cout << "Botnami6800::Resetting..." << endl;
		init();
	    }

	    int executeinstr(uint8_t instr);
    };

    struct Botnami6809Status
    {
	uint8_t rega;
	uint8_t regb;
	uint8_t status_reg;
	uint16_t regd;
	uint8_t regdp;
	uint16_t regx;
	uint16_t regy;
	uint16_t usp;
	uint16_t ssp;
	uint16_t pc;
    };

    class BOTNAMI_API Botnami6809 : public BotnamiCPU
    {
	public:
	    Botnami6809();
	    virtual ~Botnami6809();

	    void init()
	    {
		init_6809();
		cout << "Botnami6809::Initialized" << endl;
	    }

	    void shutdown()
	    {
		cout << "Botnami6809::Shutting down..." << endl;
		BotnamiCPU::shutdown();
	    }

	    void reset()
	    {
		cout << "Botnami6809::Resetting..." << endl;
		init();
	    }

	    int executeinstr(uint8_t instr);
	    void debugoutput(bool print_disassembly = false);

	    Botnami6809Status getStatus()
	    {
		return status;
	    }

	protected:
	    void init_6809()
	    {
		BotnamiCPU::init();
		status_reg = 0x50;
		setStatus();
	    }

	    void setStatus()
	    {
		status.rega = rega;
		status.regb = regb;
		status.regd = getRegD();
		status.regx = regx;
		status.regy = regy;
		status.regdp = regdp;
		status.status_reg = status_reg;
		status.pc = pc;
		status.usp = usp;
		status.ssp = ssp;
	    }

	    Botnami6809Status status;

	private:
	    uint16_t getRegD()
	    {
		return ((rega << 8) | regb);
	    }

	    void setRegD(uint16_t value)
	    {
		rega = (value >> 8);
		regb = (value & 0xFF);
	    }
    };

    class BOTNAMI_API Botnami6309 : public BotnamiCPU
    {
	public:
	    Botnami6309();
	    virtual ~Botnami6309();

	    void init()
	    {
		init_6309();
		cout << "Botnami6309::Initialized" << endl;
	    }

	    void shutdown()
	    {
		cout << "Botnami6309::Shutting down..." << endl;
		BotnamiCPU::shutdown();
	    }

	    void reset()
	    {
		cout << "Botnami6309::Resetting..." << endl;
		init();
	    }

	    int executeinstr(uint8_t instr);

	protected:
	    void init_6309()
	    {
		BotnamiCPU::init();
		status_reg = 0x50;
		reg_md = 0;
	    }

	private:
	    bool is_native_mode()
	    {
		return testbit(reg_md, 0);
	    }

	    uint8_t reg_md = 0;
    };

    class BOTNAMI_API BotnamiKonami1 : public Botnami6809
    {
	public:
	    BotnamiKonami1()
	    {

	    }

	    ~BotnamiKonami1()
	    {

	    }

	    void init()
	    {
		Botnami6809::init_6809();
		enc_boundary = 0;
		cout << "BotnamiKonami1::Initialized" << endl;
	    }

	    void shutdown()
	    {
		cout << "BotnamiKonami1::Shutting down..." << endl;
		BotnamiCPU::shutdown();
	    }

	    void reset()
	    {
		cout << "BotnamiKonami1::Resetting..." << endl;
		init();
	    }

	    void setEncryptionBoundary(uint16_t val)
	    {
		enc_boundary = val;
	    }

	    uint8_t readOpcode(uint16_t addr)
	    {
		uint8_t opcode = BotnamiCPU::readOpcode(addr);

		if (addr < enc_boundary)
		{
		    return opcode;
		}

		uint8_t xor_mask = 0x00;

		xor_mask |= (!testbit(addr, 3) << 1);
		xor_mask |= (testbit(addr, 3) << 3);
		xor_mask |= (!testbit(addr, 1) << 5);
		xor_mask |= (testbit(addr, 1) << 7);

		return (opcode ^ xor_mask);
	    }

	private:
	    uint16_t enc_boundary = 0;
    };

    class BOTNAMI_API BotnamiKonami2 : public BotnamiCPU
    {
	public:
	    BotnamiKonami2()
	    {

	    }

	    ~BotnamiKonami2()
	    {

	    }

	    void init()
	    {
		BotnamiCPU::init();
		status_reg = 0x50;
		setStatus();
		cout << "BotnamiKonami2::Initialized" << endl;
	    }

	    void shutdown()
	    {
		cout << "BotnamiKonami2::Shutting down..." << endl;
		BotnamiCPU::shutdown();
	    }

	    void reset()
	    {
		cout << "BotnamiKonami2::Resetting..." << endl;
		BotnamiCPU::init();
	    }

	    int executeinstr(uint8_t instr);
	    void debugoutput(bool print_disassembly = true);
	    size_t disassembleinstr(ostream &stream, size_t pc);

	    Botnami6809Status getStatus()
	    {
		return status;
	    }

	private:
	    uint16_t extended_address = 0;

	    int handleIRQ();

	    void setStatus()
	    {
		status.rega = rega;
		status.regb = regb;
		status.regd = getRegD();
		status.regx = regx;
		status.regy = regy;
		status.regdp = regdp;
		status.status_reg = status_reg;
		status.pc = pc;
		status.usp = usp;
		status.ssp = ssp;
	    }

	    Botnami6809Status status;

	    int indexed_mode();

	    int handleInterrupts();

	    int sign_ext()
	    {
		uint16_t data = int8_t(regb);
		set_nz<uint16_t>(data);
		setRegD(data);
		return 2;
	    }

	    uint16_t getRegD()
	    {
		return ((rega << 8) | regb);
	    }

	    void setRegD(uint16_t value)
	    {
		rega = (value >> 8);
		regb = (value & 0xFF);
	    }

	    uint16_t get_ireg(uint8_t instr)
	    {
		uint16_t data = 0;
		switch (instr & 0x70)
		{
		    case 0x20: data = regx; break;
		    case 0x30: data = regy; break;
		    case 0x50: data = usp; break;
		    case 0x60: data = ssp; break;
		    case 0x70: data = pc; break;
		}

		return data;
	    }

	    void inc_ireg(uint8_t instr, bool is_double_inc = false)
	    {
		int inc = (is_double_inc) ? 2 : 1;

		switch (instr & 0x70)
		{
		    case 0x20: regx += inc; break;
		    case 0x30: regy += inc; break;
		    case 0x50: usp += inc; break;
		    case 0x60: ssp += inc; break;
		    case 0x70: pc += inc; break;
		}
	    }

	    void dec_ireg(uint8_t instr, bool is_double_dec = false)
	    {
		int dec = (is_double_dec) ? 2 : 1;

		switch (instr & 0x70)
		{
		    case 0x20: regx -= dec; break;
		    case 0x30: regy -= dec; break;
		    case 0x50: usp -= dec; break;
		    case 0x60: ssp -= dec; break;
		    case 0x70: pc -= dec; break;
		}
	    }

	    int decbjnz()
	    {
		regb = dec_internal8(regb);
		branch(is_cond_ne());
		return 4;
	    }

	    int lbsr()
	    {
		int16_t offs = getimmWord();
		uint16_t addr = (pc + offs);
		pushsp16(pc);
		pc = addr;
		return 8;
	    }

	    int bmove()
	    {
		int cycles = 1;

		while (usp > 0)
		{
		    uint8_t operand = readByte(regy++);
		    writeByte(regx++, operand);
		    usp -= 1;
		    cycles += 2;
		}

		return cycles;
	    }

	    void indexed_mode_dasm(ostream &stream, uint8_t mode, size_t &pc);

	    struct exg_reg
	    {
		uint8_t byte_val = 0;
		uint16_t word_val = 0;
	    };

	    exg_reg read_exg_reg(uint8_t reg)
	    {
		exg_reg result;
		result.word_val = 0xFF;

		switch (reg & 0x7)
		{
		    case 0: result.word_val = rega; break; // A
		    case 1: result.word_val = regb; break; // B
		    case 2: result.word_val = regx; break; // X
		    case 3: result.word_val = regy; break; // Y
		    case 4: result.word_val = ssp; break; // S
		    case 5: result.word_val = usp; break; // U
		}

		result.byte_val = uint8_t(result.word_val);
		return result;
	    }

	    void write_exg_reg(uint8_t reg, exg_reg value)
	    {
		switch (reg & 0x7)
		{
		    case 0: rega = value.byte_val; break; // A
		    case 1: regb = value.byte_val; break; // B
		    case 2: regx = value.word_val; break; // X
		    case 3: regy = value.word_val; break; // Y
		    case 4: ssp = value.word_val; break; // S
		    case 5: usp = value.word_val; break; // U
		}
	    }

	    int exchange()
	    {
		uint8_t param = getimmByte();
		int r1 = (param & 0xF);
		int r2 = (param >> 4);

		auto reg1 = read_exg_reg(r1);
		auto reg2 = read_exg_reg(r2);

		write_exg_reg(r1, reg2);
		write_exg_reg(r2, reg1);
		return 7;
	    }

	    int tfr()
	    {
		uint8_t param = getimmByte();
		int r1 = (param & 0xF);
		int r2 = (param >> 4);

		auto reg = read_exg_reg(r1);
		write_exg_reg(r2, reg);
		return 5;
	    }
	    
    };
};


#endif // BOTNAMI_H