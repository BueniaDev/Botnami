#ifndef M6850_H
#define M6850_H

#include <iostream>
#include <cstdint>
using namespace std;

namespace botnami
{
    struct M6850Pins
    {
	bool pin_e = false; // Pin 14
	bool pin_rx_clk = false; // Pin 3
	bool pin_tx_clk = false; // Pin 4
	bool pin_rx_data = true; // Pin 2
	bool pin_tx_data = true; // Pin 6
	bool pin_rts = true; // Pin 5
	bool pin_cs0 = false; // Pin 8
	bool pin_cs1 = false; // Pin 10
	bool pin_ncs2 = true; // Pin 9
	bool pin_nirq = true; // Pin 7
	bool pin_rnw = true; // Pin 13
	bool pin_dcd = false; // Pin 23
	bool pin_cts = false; // Pin 24
	bool pin_rs = false; // Pin 11
	uint8_t data = 0; // Pins 15 (D7) - 22 (D0)
    };

    class M6850
    {
	enum TXState : int
	{
	    TxIdle = 0,
	    TxLoadShift = 1,
	    TxStart = 2,
	    TxShiftOut = 3,
	    TxParity = 4,
	    TxStop1 = 5,
	    TxStop2 = 6
	};

	enum RXState : int
	{
	    RxIdle = 0,
	    RxWaitStart = 1,
	    RxSample = 2,
	    RxParity = 3,
	    RxStop1 = 4,
	    RxStop2 = 5,
	    RxSync = 6
	};

	public:
	    M6850();
	    ~M6850();

	    void init();
	    void tick();

	    M6850Pins &getPins()
	    {
		return current_pins;
	    }

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 0x1) ? true : false;
	    }

	    M6850Pins current_pins;

	    bool prev_clk = false;
	    bool current_clk = false;
	    bool clk_rise = false;

	    bool is_cs = false;

	    void tickControl();
	    void tickTX();

	    uint8_t control_reg = 0;
	    bool is_mclr = false;

	    int reg_ws = 0;
	    int reg_cds = 0;

	    uint8_t tx_data_reg = 0;

	    bool is_tdre = false;

	    // Returns true if the number of set bits is odd
	    bool calcParity(uint8_t val)
	    {
		uint8_t data = val;
		data ^= (data >> 4);
		data ^= (data >> 2);
		data ^= (data >> 1);

		return testbit(data, 0);
	    }
    };
};

#endif // M6850_H