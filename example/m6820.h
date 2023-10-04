#ifndef M6820_H
#define M6820_H

#include <iostream>
#include <cstdint>
using namespace std;

namespace botnami
{
    struct M6820Pins
    {
	bool pin_e = false; // Pin 25
	bool pin_nres = true; // Pin 34
	bool pin_cs0 = false; // Pin 22
	bool pin_cs1 = false; // Pin 24
	bool pin_ncs2 = true; // Pin 23
	bool pin_rnw = false; // Pin 21
	bool pin_rs0 = false; // Pin 36
	bool pin_rs1 = false; // Pin 35
	uint8_t data = 0; // Pins 26 (D7) - 33 (D0)
	uint8_t porta_data = 0; // Pins 2 (PA0) - 9 (PA7)
	uint8_t portb_data = 0; // Pins 10 (PB0) - 17 (PB7)
	bool pin_nirqa = true; // Pin 37
	bool pin_nirqb = true; // Pin 38
	bool pin_ca1 = true; // Pin 40
	bool pin_ca2 = true; // Pin 39
	bool pin_cb1 = true; // Pin 18
	bool pin_cb2 = true; // Pin 19
    };

    class M6820
    {
	public:
	    M6820();
	    ~M6820();

	    void init();
	    void tick();

	    M6820Pins &getPins()
	    {
		return current_pins;
	    }

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 0x1) ? true : false;
	    }

	    M6820Pins current_pins;

	    void resetPIA();
	    void tickPIA();

	    void tickWrites();

	    bool is_cs = false;

	    uint8_t control_a = 0;
	    uint8_t control_b = 0;
    };
};


#endif // M6820_H