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
	    void debugoutput(bool print_disassembly = false);
	    // size_t disassmebleinstr(ostream &stream, uint32_t pc);

	    void setinterface(BotnamiInterface &cb);

	    bool is_zero()
	    {
		return testbit(status_reg, 2);
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
	    uint8_t status_reg = 0;
	    uint16_t pc = 0;

	    void set_sign(bool is_set)
	    {
		status_reg = changebit(status_reg, 3, is_set);
	    }

	    void set_zero(bool is_set)
	    {
		status_reg = changebit(status_reg, 2, is_set);
	    }

	    template<typename T>
	    void set_nz(T data)
	    {
		int sign_bit = ((sizeof(T) * 8) - 1);
		set_sign(testbit(data, sign_bit));
		set_zero(data == 0);
	    }

	    uint8_t add_internal8(uint8_t source, uint8_t operand, bool is_carry = false)
	    {
		uint16_t result = (source + operand + is_carry);
		set_nz<uint8_t>(result);
		return result;
	    }

	    uint8_t sub_internal8(uint8_t source, uint8_t operand, bool is_carry = false)
	    {
		uint16_t result = (source - operand - is_carry);
		set_nz<uint8_t>(result);
		return result;
	    }

	    uint8_t add8(uint8_t source, uint8_t operand)
	    {
		return add_internal8(source, operand);
	    }

	    uint8_t dec_internal8(uint8_t data)
	    {
		return sub_internal8(data, 1);
	    }

	    void cmp8(uint8_t source, uint8_t operand)
	    {
		sub_internal8(source, operand);
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

	protected:
	    void init_6809()
	    {
		BotnamiCPU::init();
		status_reg = 0x41;
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
		status_reg = 0x41;
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

	private:
	    uint16_t extended_address = 0;

	    int indexed_mode();
    };
};


#endif // BOTNAMI_H