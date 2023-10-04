#include "m6850.h"
using namespace botnami;
using namespace std;

namespace botnami
{
    M6850::M6850()
    {

    }

    M6850::~M6850()
    {

    }

    void M6850::init()
    {
	current_pins = {};
	control_reg = 0x40;
	is_tdre = true;
    }

    void M6850::tick()
    {
	clk_rise = (!prev_clk && current_pins.pin_e);
	is_cs = (current_pins.pin_cs0 && current_pins.pin_cs1 && !current_pins.pin_ncs2);

	tickControl();
	tickTX();

	prev_clk = current_pins.pin_e;
    }

    void M6850::tickControl()
    {
	uint8_t status_reg = 0;
	status_reg |= (current_pins.pin_cts << 3);
	status_reg |= ((is_tdre && !current_pins.pin_cts) << 1);

	if (clk_rise && is_cs && current_pins.pin_rnw && !current_pins.pin_rs)
	{
	    current_pins.data = status_reg;
	}

	is_mclr = ((control_reg & 0x3) == 0x3);
	reg_cds = (control_reg & 0x3);
	reg_ws = ((control_reg >> 2) & 0x7);

	if (clk_rise && is_cs && !current_pins.pin_rnw && !current_pins.pin_rs)
	{
	    cout << "Writing value of " << hex << int(current_pins.data) << " to MC6850 control register" << endl;
	    control_reg = current_pins.data;
	}
    }

    void M6850::tickTX()
    {
	if (clk_rise)
	{
	    if (is_mclr)
	    {
		tx_data_reg = 0;
	    }
	    else if (is_cs && !current_pins.pin_rnw && current_pins.pin_rs)
	    {
		cout << "Writing value of " << hex << int(current_pins.data) << " to MC6850 transfer data register" << endl;
		tx_data_reg = current_pins.data;

		if (!testbit(reg_ws, 2))
		{
		    tx_data_reg &= 0x7F;
		}
	    }
	}
    }
};