// TODO list:
// Implement DMABREQ, HALT and BS/BA pins
// Implement remaining interrupts (SWI already implemented)
// Finish implementing M6809 instruction set

#include "botnami6809.h"
using namespace std;

namespace botnami
{
    Botnami6809::Botnami6809()
    {

    }

    Botnami6809::~Botnami6809()
    {

    }

    void Botnami6809::init()
    {
	current_pins = {};
    }

    void Botnami6809::tick()
    {
	tickClk(true);
	tickClk(false);
    }

    void Botnami6809::latchValues()
    {
	addr = addr_next;
	inst_state = inst_state_next;
	next_state = next_state_next;
	reg_pc = reg_pc_next;

	rega = rega_next;
	regb = regb_next;
	reg_dp = reg_dp_next;
	reg_cc = reg_cc_next;

	regx = regx_next;
	regy = regy_next;

	reg_usp = reg_usp_next;
	reg_ssp = reg_ssp_next;

	instr1 = instr1_next;
	instr2 = instr2_next;
	instr3 = instr3_next;

	is_page2 = is_page2_next;
	is_page3 = is_page3_next;

	int_type = int_type_next;

	ea = ea_next;
	temp = temp_next;
    }

    void Botnami6809::setValues()
    {
	current_pins.addr = addr_next;
	addr_next = 0xFFFF;
	inst_state_next = inst_state;
	next_state_next = next_state;

	instr1_next = instr1;
	instr2_next = instr2;
	instr3_next = instr3;

	is_page2_next = is_page2;
	is_page3_next = is_page3;

	int_type_next = int_type;

	alu8_a = 0;
	alu8_b = 0;
	alu8_cc = 0;
	alu8_op = InvalidALU;

	alu16_a = 0;
	alu16_b = 0;
	alu16_cc = 0;
	alu16_op = InvalidALU16;

	rega_next = rega;
	regb_next = regb;
	reg_dp_next = reg_dp;
	regx_next = regx;
	regy_next = regy;
	reg_usp_next = reg_usp;
	reg_ssp_next = reg_ssp;
	reg_cc_next = reg_cc;
	reg_pc_next = reg_pc;

	ea_next = ea;
	temp_next = temp;
    }

    void Botnami6809::debugOutput()
    {
	uint16_t regd = ((rega << 8) | regb);

	cout << "PC: " << hex << int(reg_pc) << endl;
	cout << "S: " << hex << int(reg_ssp) << endl;
	cout << "CC: " << hex << int(reg_cc) << endl;
	cout << "DP: " << hex << int(reg_dp) << endl;
	cout << "A: " << hex << int(rega) << endl;
	cout << "B: " << hex << int(regb) << endl;
	cout << "D: " << hex << int(regd) << endl;
	cout << "X: " << hex << int(regx) << endl;
	cout << "Y: " << hex << int(regy) << endl;
	cout << "U: " << hex << int(reg_usp) << endl;
	cout << endl;
    }

    void Botnami6809::tickE()
    {
	is_fetch = (prev_e && !current_pins.pin_e);
	if (prev_e && !current_pins.pin_e)
	{
	    if (!current_pins.pin_res)
	    {
		inst_state = Reset;
	    }
	    else
	    {
		latchValues();
	    }
	}

	prev_q = current_pins.pin_q;
	prev_e = current_pins.pin_e;

	setValues();
	execInst();
    }

    void Botnami6809::execInst()
    {
	#include "instr6809.inl"
    }

    void Botnami6809::tickClk(bool clk)
    {
	if (prev_clk && !clk)
	{
	    switch (clk_phase)
	    {
		case 0: current_pins.pin_e = false; break;
		case 1: current_pins.pin_q = true; break;
		case 2: current_pins.pin_e = true; break;
		case 3: current_pins.pin_q = false; break;
	    }

	    if (current_pins.pin_mrdy)
	    {
		clk_phase += 1;

		if (clk_phase == 4)
		{
		    clk_phase = 0;
		}
	    }
	}

	tickE();

	prev_clk = clk;
    }
};