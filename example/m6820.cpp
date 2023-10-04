#include "m6820.h"
using namespace std;

namespace botnami
{
    M6820::M6820()
    {

    }

    M6820::~M6820()
    {

    }

    void M6820::init()
    {
	current_pins = {};
    }

    void M6820::tick()
    {
	if (!current_pins.pin_nres)
	{
	    cout << "Resetting PIA..." << endl;
	    resetPIA();
	}
	else
	{
	    tickPIA();
	}
    }

    void M6820::resetPIA()
    {
	is_cs = false;
    }

    void M6820::tickPIA()
    {
	is_cs = (current_pins.pin_cs0 && current_pins.pin_cs1 && !current_pins.pin_ncs2);

	tickWrites();
    }

    void M6820::tickWrites()
    {
	if (is_cs && !current_pins.pin_rnw && current_pins.pin_e)
	{
	    int rs = ((current_pins.pin_rs1 << 1) | current_pins.pin_rs0);

	    switch (rs)
	    {
		case 0x0:
		{
		    if (testbit(control_a, 2))
		    {
			cout << "Writing value of " << hex << int(current_pins.data) << " to M6820 port A data register" << endl;
		    }
		    else
		    {
			cout << "Writing value of " << hex << int(current_pins.data) << " to M6820 port A direction register" << endl;
		    }
		}
		break;
		case 0x1:
		{
		    cout << "Writing value of " << hex << int(current_pins.data & 0x3F) << " to M6820 control register A" << endl;
		    control_a = ((control_a & 0xC0) | (current_pins.data & 0x3F));
		}
		break;
		case 0x3:
		{
		    cout << "Writing value of " << hex << int(current_pins.data & 0x3F) << " to M6820 control register B" << endl;
		    control_b = ((control_b & 0xC0) | (current_pins.data & 0x3F));
		}
		break;
	    }
	}
    }
};