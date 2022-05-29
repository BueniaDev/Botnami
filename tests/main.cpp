#define UTEST_CPP_IMPLEMENTATION
#include "upptest.h"
#include "Botnami/botnami.h"
using namespace utest;
using namespace botnami;
using namespace std;

class BotnamiTest6809 : public BotnamiInterface
{
    public:
	BotnamiTest6809(Botnami6809 &cb) : core(cb)
	{
	    core.setinterface(*this);
	}

	~BotnamiTest6809()
	{

	}

	void init_code(vector<uint8_t> code, uint16_t start_addr = 0x4000)
	{
	    memory.resize(0x10000, 0);
	    copy(code.begin(), code.end(), (memory.begin() + start_addr));
	    memory.at(0xFFFE) = (start_addr >> 8);
	    memory.at(0xFFFF) = (start_addr & 0xFF);
	    core.init();
	}

	void shutdown()
	{
	    core.shutdown();
	    memory.clear();
	}

	void run_instr()
	{
	    core.debugoutput();
	    core.executenextinstr();
	}

	uint8_t readByte(uint16_t addr)
	{
	    return memory.at(addr);
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    memory.at(addr) = data;
	}

    private:
	Botnami6809 &core;

	vector<uint8_t> memory;
};

template<class fail_handler>
class ladybot_assert
{
    public:
	template<typename T1, typename T2>
	static void eq(const T1& expected, const T2& actual, const char* file_name = "", const int line_num = 0)
	{
		if (expected == actual)
		{
			return;
		}
		std::ostringstream err;
		err << "Ladybot expected " << expected << ", but instead she got " << actual;
		fail(err.str(), file_name, line_num);
	}

	template<typename T1, typename T2>
	static void neq(const T1& expected, const T2& actual, const char* file_name = "", const int line_num = 0)
	{
		if (expected != actual)
		{
			return;
		}

		std::ostringstream err;
		err << "Ladybot expected " << expected << ", but instead she got " << actual;
		fail(err.str(), file_name, line_num);
	}

	static void is_true(const bool condition, const char* file_name = "", const int line_num = 0)
	{
		if (condition)
		{
			return;
		}
		fail("Ladybot expected [true], but instead she got [false]", file_name, line_num);
	}

	static void is_false(const bool condition, const char* file_name = "", const int line_num = 0)
	{
		if (!condition)
		{
			return;
		}
		fail("Ladybot expected [false], but instead she got [true]", file_name, line_num);
	}

	static void fail(const std::string& message, const char* file_name = "", const int line_num = 0)
	{
		fail_handler::handle(message, file_name, line_num);
	}
};

typedef ladybot_assert<default_fail_handler> botassert;


class BotnamiTest
{
    public:
	BotnamiTest()
	{
	    
	}

	~BotnamiTest()
	{

	}

	void init(vector<uint8_t> code)
	{
	    test = new BotnamiTest6809(core);
	    test->init_code(code);
	}

	void shutdown()
	{
	    if (test != NULL)
	    {
		test->shutdown();
		test = NULL;
	    }
	}

	void run(int num_instrs = 1)
	{
	    if (test == NULL)
	    {
		return;
	    }

	    for (int i = 0; i < num_instrs; i++)
	    {
		test->run_instr();
	    }
	}

	Botnami6809 &getCore()
	{
	    return core;
	}

    private:
	BotnamiTest6809 *test = NULL;
	Botnami6809 core;
};

#include "tests.inl"

string fetch_str(const utest::status &stat)
{
    if (stat == utest::status::pass)
    {
	return "Pass";
    }
    else if (stat == utest::status::fail)
    {
	return "Fail";
    }
    else
    {
	return "-";
    }
}

auto callback = [](const utest::result &tst)
{
    cout << tst.info->name << ": " << fetch_str(tst.status) << endl;

    if (tst.status == status::fail)
    {
	cout << tst.err_message << endl;
    }
};

int main(int argc, char* argv[])
{
    auto res = utest::runner::run_registered(callback);

    if (res != status::fail)
    {
	cout << "Ladybot is so happy that all of the tests have passed!" << endl;
    }
    else
    {
	cout << "Ladybot sees that there's still work to be done!" << endl;
    }

    return 0;
}

