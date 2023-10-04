#ifndef KUJOCLOCK_H
#define KUJOCLOCK_H

#include <iostream>
#include <cstdint>
#include <algorithm>
using namespace std;

namespace kujoclock
{
    class KujoClock
    {
	public:
	    KujoClock()
	    {

	    }

	    ~KujoClock()
	    {

	    }

	    void init(uint64_t hz)
	    {
		hertz = max<uint64_t>(1, hz);
		inc_ps = ((1e12 / hertz) / 2);
		now_ps = (inc_ps + 1);
		last_edge_ps = inc_ps;
	    }

	    uint64_t timeToTick()
	    {
		uint64_t ul = 0;

		if (last_edge_ps > now_ps)
		{
		    ul = (last_edge_ps - now_ps);
		    ul /= inc_ps;
		    ul = (now_ps + ul * inc_ps);
		}
		else if (last_edge_ps == now_ps)
		{
		    ul = inc_ps;
		}
		else if ((last_edge_ps + inc_ps) == now_ps)
		{
		    ul = inc_ps;
		}
		else if ((last_edge_ps + inc_ps) > now_ps)
		{
		    ul = (last_edge_ps + inc_ps - now_ps);
		}
		else
		{
		    ul = (last_edge_ps + 2 * inc_ps - now_ps);
		}

		return ul;
	    }

	    bool advance(uint64_t itime)
	    {
		bool clk = false;

		now_ps += itime;

		if (now_ps >= (last_edge_ps + (2 * inc_ps)))
		{
		    last_edge_ps += (2 * inc_ps);
		    clk = true;
		}
		else if (now_ps >= (last_edge_ps + inc_ps))
		{
		    clk = false;
		}
		else
		{
		    clk = true;
		}

		return clk;
	    }

	    bool isRisingEdge()
	    {
		return (now_ps == last_edge_ps);
	    }

	    bool isFallingEdge()
	    {
		return (now_ps == (last_edge_ps + inc_ps));
	    }

	private:
	    uint64_t hertz = 0;
	    uint64_t inc_ps = 0;
	    uint64_t now_ps = 0;
	    uint64_t last_edge_ps = 0;
    };
};


#endif // KUJOCLOCK_H