#ifndef KUJOUART_H
#define KUJOUART_H

#include <iostream>
#include <cstdint>
#include <algorithm>
using namespace std;

namespace kujouart
{
    enum KujoNumDataBits
    {
	Num8Bits = 0,
	Num7Bits = 1,
    };

    enum KujoParity
    {
	None = 0,
	Even = 1,
	Odd = 2
    };

    enum KujoNumStopBits
    {
	Num1Bit = 0,
	Num2Bits = 1
    };

    enum KujoRxState
    {
	RxIdle = 0,
	RxData = 1,
	RxParity = 2,
	RxStop = 3
    };

    enum KujoTxState
    {
	TxIdle = 0,
	TxStart = 1,
	TxData = 2,
	TxParity = 3,
	TxStop = 4
    };

    class KujoUART
    {
	public:
	    KujoUART()
	    {

	    }

	    ~KujoUART()
	    {

	    }

	    virtual void outputRX(uint8_t data)
	    {
		cout << "Outputing value of " << hex << int(data) << endl;
		return;
	    }

	    virtual bool pollTX()
	    {
		return false;
	    }

	    virtual uint8_t inputTX()
	    {
		return 0;
	    }

	    void setRate(uint32_t clk_rate, uint32_t baud_rate)
	    {
		baud_rate = max<uint32_t>(1, baud_rate);
		clk_div = (clk_rate / baud_rate);
	    }

	    void setNumDataBits(KujoNumDataBits bits)
	    {
		switch (bits)
		{
		    case Num8Bits: num_bits = 8; break;
		    case Num7Bits: num_bits = 7; break;
		    default: num_bits = 0; break;
		}
	    }

	    void setParity(KujoParity parity)
	    {
		is_parity_enabled = (parity != None);
		is_even_parity = (parity == Even);
	    }

	    void setNumStopBits(KujoNumStopBits bits)
	    {
		switch (bits)
		{
		    case Num1Bit: num_stop_bits = 1; break;
		    case Num2Bits: num_stop_bits = 2; break;
		    default: num_stop_bits = 0; break;
		}
	    }

	    bool clock(bool tx)
	    {
		bool is_rx = true;
		if (!tx && prev_tx)
		{
		    rx_change_counter = 0;
		}
		else
		{
		    rx_change_counter += 1;
		}

		prev_tx = tx;

		if (rx_state == RxIdle)
		{
		    if (!tx)
		    {
			rx_baud_counter = (clk_div + ((clk_div / 2) - 1));
			rx_baud_counter -= rx_change_counter;
			rx_state = RxData;
			is_rx_parity = false;
			rx_data = 0;
		    }
		}
		else if (rx_baud_counter <= 0)
		{
		    switch (rx_state)
		    {
			case RxIdle: break;
			case RxData:
			{
			    is_rx_parity ^= tx;
			    rx_data = ((tx << 7) | (rx_data >> 1));
			    num_rx_bits += 1;

			    if (num_rx_bits == num_bits)
			    {
				out_data = rx_data;
				num_rx_bits = 0;

				if (is_parity_enabled)
				{
				    rx_state = RxParity;
				}
				else
				{
				    rx_state = RxStop;
				}
			    }
			}
			break;
			case RxParity:
			{
			    is_rx_parity ^= tx;

			    bool is_error = false;
			    if (is_even_parity)
			    {
				is_error = is_rx_parity;
			    }
			    else
			    {
				is_error = !is_rx_parity;
			    }

			    if (is_error)
			    {
				cout << "Parity error detected" << endl;
				throw runtime_error("KujoUART error");
			    }

			    rx_state = RxStop;
			}
			break;
			case RxStop:
			{
			    stop_bits = ((tx << 7) | (stop_bits >> 1));
			    num_rx_bits += 1;

			    if (num_rx_bits == num_stop_bits)
			    {
				uint8_t stop_val = (stop_bits >> (8 - num_stop_bits));
				bool is_valid = false;

				switch (num_stop_bits)
				{
				    case 1:
				    {
					is_valid = (stop_val == 0x1);
				    }
				    break;
				    case 2:
				    {
					is_valid = (stop_val == 0x3);
				    }
				    break;
				}

				if (!is_valid)
				{
				    cout << "Invalid stop bits of " << dec << int(stop_val) << endl;
				    throw runtime_error("KujoUART error");
				}
				else
				{
				    outputRX(out_data);
				    rx_state = RxIdle;
				}

				is_rx_parity = false;
				num_rx_bits = 0;
			    }
			}
			break;
			default:
			{
			    cout << "Unrecognized RX state of " << dec << int(rx_state) << endl;
			    throw runtime_error("KujoUART error");
			}
			break;
		    }

		    rx_baud_counter = (clk_div - 1);
		}
		else
		{
		    rx_baud_counter -= 1;
		}

		if (tx_state == TxIdle && (tx_baud_counter <= 0))
		{
		    if (pollTX())
		    {
			tx_data = inputTX();
			tx_state = TxData;
			is_tx_parity = false;
			rx_bit = false;
			num_tx_bits = 0;
			tx_baud_counter = (clk_div - 1);
		    }
		    else
		    {
			rx_bit = true;
		    }
		}
		else if (tx_baud_counter <= 0)
		{
		    tx_baud_counter = (clk_div - 1);

		    switch (tx_state)
		    {
			case TxIdle: break;
			case TxStart:
			{
			    rx_bit = false;
			    tx_state = TxData;
			    num_parity_bits = 0;
			}
			break;
			case TxData:
			{
			    rx_bit = (tx_data & 0x1);

			    is_tx_parity ^= rx_bit;
			    tx_data >>= 1;

			    num_tx_bits += 1;

			    if (num_tx_bits == num_bits)
			    {
				num_tx_bits = 0;

				if (is_parity_enabled)
				{
				    tx_state = TxParity;
				}
				else
				{
				    tx_state = TxStop;
				}
			    }
			}
			break;
			case TxParity:
			{
			    if (is_even_parity)
			    {
				rx_bit = is_tx_parity;
			    }
			    else
			    {
				rx_bit = !is_tx_parity;
			    }

			    tx_state = TxStop;
			}
			break;
			case TxStop:
			{
			    rx_bit = true;

			    num_tx_bits += 1;

			    if (num_tx_bits == num_stop_bits)
			    {
				num_tx_bits = 0;
				is_tx_parity = false;
				tx_state = TxIdle;
			    }
			}
			break;
			default:
			{
			    cout << "Unrecognized TX state of " << dec << int(tx_state) << endl;
			    throw runtime_error("KujoUART error");
			}
			break;
		    }
		}
		else
		{
		    tx_baud_counter -= 1;
		}

		is_rx = rx_bit;
		return is_rx;
	    }

	private:
	    uint32_t clk_div = 0;
	    int num_bits = 0;
	    bool is_parity_enabled = false;
	    bool is_even_parity = false;
	    int num_stop_bits = 0;
	    bool prev_tx = false;
	    int rx_change_counter = 0;
	    int rx_baud_counter = 0;
	    uint8_t rx_data = 0;
	    int num_rx_bits = 0;
	    uint8_t stop_bits = 0;

	    int tx_baud_counter = 0;

	    bool is_tx_parity = false;
	    bool is_rx_parity = false;

	    uint8_t tx_data = 0;
	    uint8_t out_data = 0;

	    int num_tx_bits = 0;

	    int num_parity_bits = 0;

	    bool rx_bit = false;
	    bool is_tx_idle = false;
	    
	    KujoRxState rx_state = RxIdle;
	    KujoTxState tx_state = TxIdle;
    };
};



#endif // KUJOUART_H