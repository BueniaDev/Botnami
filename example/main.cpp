
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <array>
#include <botnami6809.h>
#include "m6820.h"
#include "m6850.h"
#include "kujouart.h"
#include "kujoclock.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL2/SDL.h>
using namespace botnami;
using namespace kujouart;
using namespace kujoclock;
using namespace std;

/*
class BotnamiUART : public KujoUART
{
    public:
	BotnamiUART()
	{
	    setRate(7372800, 115200);
	    setNumDataBits(Num8Bits);
	    setParity(None);
	    setNumStopBits(Num1Bit);
	}

	~BotnamiUART()
	{

	}

	void clearData()
	{
	    input_data.clear();
	}

	void outputRX(uint8_t data)
	{
	    output_data.push_back(data);
	}

	bool pollTX()
	{
	    return !input_data.empty();
	}

	uint8_t inputTX()
	{
	    if (input_data.empty())
	    {
		cout << "Input FIFO is empty" << endl;
		throw runtime_error("UART error");
	    }

	    uint8_t data = input_data.front();
	    input_data.pop_front();
	    return data;
	}

	void appendInputData(string str)
	{
	    for (int i = 0; i < str.size(); i++)
	    {
		appendInputData(str.at(i));
	    }
	}

	void appendInputData(uint8_t data)
	{
	    input_data.push_back(data);
	}

	bool isOutput()
	{
	    return !output_data.empty();
	}

	uint8_t getOutputData()
	{
	    if (output_data.empty())
	    {
		cout << "Output FIFO is empty" << endl;
		throw runtime_error("UART error");
	    }

	    uint8_t data = output_data.front();
	    output_data.pop_front();
	    return data;
	}

    private:
	deque<uint8_t> input_data;
	deque<uint8_t> output_data;
};
*/

class ImGuiBackend
{
    public:
	ImGuiBackend()
	{

	}

	~ImGuiBackend()
	{

	}

	virtual bool init(SDL_Window*, int, int)
	{
	    return true;
	}

	virtual void shutdown()
	{
	    return;
	}

	virtual void newFrame()
	{
	    return;
	}

	virtual void render(ImDrawData *draw_data)
	{
	    (void)draw_data;
	    return;
	}
};

class SDL2Backend : public ImGuiBackend
{
    public:
	SDL2Backend()
	{

	}

	bool init(SDL_Window *win, int width, int height)
	{
	    window_width = width;
	    window_height = height;
	    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	    if (renderer == NULL)
	    {
		return false;
	    }

	    ImGui_ImplSDLRenderer2_Init(renderer);
	    return true;
	}

	void shutdown()
	{
	    ImGui_ImplSDLRenderer2_Shutdown();

	    if (renderer != NULL)
	    {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	    }
	}

	void newFrame()
	{
	    ImGuiIO &io = ImGui::GetIO();

	    ImGui_ImplSDLRenderer2_NewFrame();
	    io.DisplaySize.x = static_cast<float>(window_width);
	    io.DisplaySize.y = static_cast<float>(window_height);
	}

	void render(ImDrawData *draw_data)
	{
	    SDL_SetRenderDrawColor(renderer, 114, 144, 154, 255);
	    SDL_RenderClear(renderer);
	    ImGui_ImplSDLRenderer2_RenderDrawData(draw_data);
	    SDL_RenderPresent(renderer);
	}

    private:
	SDL_Renderer *renderer = NULL;
	int window_width = 0;
	int window_height = 0;
};

class BotnamiTest
{
    public:
	BotnamiTest()
	{
	    rom_file.resize(0x4000, 0);
	    backend = new SDL2Backend();
	}

	~BotnamiTest()
	{
	    rom_file.clear();
	}

	bool loadROM(string filename)
	{
	    ifstream file(filename, ios::in | ios::binary | ios::ate);

	    if (!file.is_open())
	    {
		cout << "Could not open file" << endl;
		return false;
	    }

	    streampos size = file.tellg();
	    file.seekg(0, ios::beg);

	    if (size > 0x4000)
	    {
		cout << "ROM file is too big" << endl;
		file.close();
		return false;
	    }

	    file.read((char*)rom_file.data(), size);
	    file.close();
	    cout << "File succesfully loaded." << endl;
	    return true;
	}

	bool init()
	{
	    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	    {
		return sdlError("SDL could not be initialized!");
	    }

	    window = SDL_CreateWindow("Botnami-test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_SHOWN);

	    if (window == NULL)
	    {
		return sdlError("Window could not be created!");
	    }

	    
	    ImGui::CreateContext();
	    if (!backend->init(window, window_width, window_height))
	    {
		cout << "Backend could not be initialized!" << endl;
		return false;
	    }

	    SDL_StartTextInput();

	    core.init();
	    initUART();	
	    initPIA();
	    main_clock.init(8000000);
	    uart_clock.init(7372800);
	    main_ram.fill(0);
	    return true;
	}

	void shutdown()
	{
	    uart_str = "";
	    backend->shutdown();
	    ImGui::DestroyContext();
	    SDL_StopTextInput();

	    if (window != NULL)
	    {
		SDL_DestroyWindow(window);
		window = NULL;
	    }

	    SDL_Quit();
	}

	void run()
	{
	    while (!quit)
	    {
		pollEvents();
		runCore();
		render();
	    }
	}

    private:
	template<typename T>
	bool testbit(T reg, int bit)
	{
	    return ((reg >> bit) & 0x1) ? true : false;
	}

	vector<uint8_t> rom_file;
	array<uint8_t, 0x8000> main_ram;

	bool prev_e = false;

	Botnami6809 core;

	SDL_Window *window = NULL;

	bool quit = false;
	SDL_Event event;

	M6820 pia;

	M6850 uart;

	string uart_str = "";

	bool uart_clk = false;

	bool is_pia_cs = false;
	bool is_pia_rs0 = false;
	bool is_pia_rs1 = false;

	int window_width = 800;
	int window_height = 600;

	int clk_div = 0;

	uint8_t debug_latch = 0;

	ImGuiBackend *backend = NULL;

	int clock_phase = 0;

	bool is_run_core = false;

	uint64_t total_clock = 0;

	KujoClock main_clock;
	KujoClock uart_clock;

	int uart_clk_div = 0;

	void pollEvents()
	{
	    ImGuiIO &io = ImGui::GetIO();
	    int wheel = 0;

	    while (SDL_PollEvent(&event))
	    {
		switch (event.type)
		{
		    case SDL_QUIT: quit = true; break;
		    case SDL_WINDOWEVENT:
		    {
			switch (event.window.event)
			{
			    case SDL_WINDOWEVENT_SIZE_CHANGED:
			    {
				io.DisplaySize.x = static_cast<float>(event.window.data1);
				io.DisplaySize.y = static_cast<float>(event.window.data2);
			    }
			    break;
			}
		    }
		    break;
		    case SDL_MOUSEWHEEL:
		    {
			wheel = event.wheel.y;
		    }
		    break;
		    case SDL_KEYDOWN:
		    {
			switch (event.key.keysym.sym)
			{
			    case SDLK_RETURN:
			    {
				// append(0x0D);
			    }
			    break;
			}
		    }
		    break;
		    case SDL_TEXTINPUT:
		    {
			// append(event.text.text);
		    }
		    break;
		}
	    }

	    int mouseX = 0;
	    int mouseY = 0;
	    const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

	    io.DeltaTime = (1.0f / 60.0f);
	    io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
	    io.MouseDown[0] = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT));
	    io.MouseDown[1] = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT));
	    io.MouseWheel = static_cast<float>(wheel);
	}

	void newFrame()
	{
	    backend->newFrame();
	    ImGui::NewFrame();
	}

	void renderUI()
	{
	    if (ImGui::BeginMainMenuBar())
	    {
		if (ImGui::BeginMenu("File"))
		{
		    if (ImGui::MenuItem("Run..."))
		    {
			if (!is_run_core)
			{
			    // clearData();
			    is_run_core = true;
			}
		    }

		    if (ImGui::MenuItem("Quit"))
		    {
			quit = true;
		    }

		    ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug"))
		{
		    ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	    }

	    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

	    ImGui::Begin("Monitor");
	    ImGui::BeginChild("Monitor", ImVec2((ImGui::GetWindowWidth() - 16), (ImGui::GetWindowHeight() - 38)), true);
	    ImGui::TextUnformatted(uart_str.c_str());
	    ImGui::EndChild();
	    ImGui::End();
	}

	void render()
	{
	    newFrame();
	    renderUI();
	    ImGui::Render();
	    backend->render(ImGui::GetDrawData());
	}

	uint8_t readByte(uint16_t addr)
	{
	    uint8_t data = 0;
	    if (addr < 0x8000)
	    {
		data = main_ram.at(addr & 0x7FFF);
	    }
	    else if (addr < 0xC000)
	    {
		data = readIO(addr);
	    }
	    else
	    {
		data = rom_file.at(addr & 0x3FFF);
	    }

	    return data;
	}

	void writeByte(uint16_t addr, uint8_t data)
	{
	    if (addr < 0x8000)
	    {
		main_ram.at(addr & 0x7FFF) = data;
	    }
	    else if (addr < 0xC000)
	    {
		writeIO(addr, data);
	    }
	    else
	    {
		return;
	    }
	}

	uint8_t readIO(uint16_t addr)
	{
	    uint8_t data = 0;
	    addr &= 0x3FFF;

	    if (addr < 0x100)
	    {
		updateUART(testbit(addr, 0));
		data = uart.getPins().data;
	    }
	    else if ((addr >= 0x200) && (addr < 0x300))
	    {
		data = debug_latch;
	    }
	    else if ((addr >= 0x300) && (addr < 0x400))
	    {
		cout << "Reading value from M6820 address of " << hex << int(addr & 0x3) << endl;

		is_pia_cs = true;
		is_pia_rs0 = testbit(addr, 0);
		is_pia_rs1 = testbit(addr, 1);

		tickPIA();
		data = pia.getPins().data;
	    }
	    else
	    {
		cout << "Reading value from I/O address of " << hex << int(addr) << endl;
		data = 0;
	    }


	    return data;
	}

	void writeIO(uint16_t addr, uint8_t data)
	{
	    addr &= 0x3FFF;

	    if (addr < 0x100)
	    {
		uart.getPins().data = data;
		updateUART(testbit(addr, 0));
	    }
	    else if ((addr >= 0x200) && (addr < 0x300))
	    {
		debug_latch = data;
	    }
	    else if ((addr >= 0x300) && (addr < 0x400))
	    {
		cout << "Writing value of " << hex << int(data) << " to M6820 address of " << hex << int(addr & 0x3) << endl;

		is_pia_rs0 = testbit(addr, 0);
		is_pia_rs1 = testbit(addr, 1);
		is_pia_cs = true;

		pia.getPins().data = data;
		tickPIA();
	    }
	    else
	    {
		cout << "Writing value of " << hex << int(data) << " to I/O address of " << hex << int(addr) << endl;
		return;
	    }
	}

	void debugOutput()
	{
	    // core.debugOutput();
	    return;
	}

	void tickCore(bool clk)
	{
	    if (core.isFetch())
	    {
		debugOutput();
	    }

	    core.tickClk(clk);

	    auto &pins = core.getPins();

	    if (!prev_e && pins.pin_e)
	    {
		const uint16_t addr = pins.addr;

		if (pins.pin_rnw)
		{
		    pins.data = readByte(addr);
		}
		else
		{
		    writeByte(addr, pins.data);
		}
	    }

	    prev_e = pins.pin_e;
	}

	void tick(bool clk)
	{
	    static bool prev_clk = false;
	    tickCore(clk);

	    if (!prev_clk && clk)
	    {
		tickPIA();
	    }

	    prev_clk = clk;
	}

	void initUART()
	{
	    uart.init();

	    auto &uart_pins= uart.getPins();
	    uart_pins.pin_cs0 = true;
	    uart_pins.pin_cs1 = true;
	}

	void initPIA()
	{
	    pia.init();

	    auto &pia_pins = pia.getPins();
	    pia_pins.pin_cs0 = true;
	    pia_pins.pin_cs1 = true;

	    pia_pins.pin_nres = false;
	    tickPIA();
	    pia_pins.pin_nres = true;
	}

	void updateUART(bool is_a0)
	{
	    auto &uart_pins= uart.getPins();
	    uart_pins.pin_ncs2 = false;
	    uart_pins.pin_rs = is_a0;
	    uart_pins.pin_rnw = core.getPins().pin_rnw;

	    uart.tick();
	}

	void tickUART(bool clk)
	{
	    uart_clk = clk;
	    static bool prev_clk = false;

	    if (prev_clk && !clk)
	    {
		uart_clk_div += 1;

		if (uart_clk_div == 2)
		{
		    uart_clk_div = 0;
		    uart_clk = !uart_clk;
		}
	    }

	    auto &uart_pins= uart.getPins();
	    uart_pins.pin_e = core.getPins().pin_e;
	    uart_pins.pin_rx_clk = uart_clk;
	    uart_pins.pin_tx_clk = uart_clk;
	    uart.tick();

	    if (prev_clk && !clk)
	    {
	    	// uart.getPins().pin_rx_data = clock(uart.getPins().pin_tx_data);
		// outputData();
	    }

	    uart_pins.pin_ncs2 = true;

	    prev_clk = clk;
	}

	void tickPIA()
	{
	    auto &core_pins = core.getPins();
	    auto &pia_pins = pia.getPins();

	    pia_pins.pin_e = core_pins.pin_e;
	    pia_pins.pin_rnw = core_pins.pin_rnw;

	    pia_pins.pin_ncs2 = !is_pia_cs;
	    pia_pins.pin_rs0 = is_pia_rs0;
	    pia_pins.pin_rs1 = is_pia_rs1;
	    pia.tick();

	    is_pia_cs = false;
	}

	void runCore()
	{
	    if (!is_run_core)
	    {
		return;
	    }

	    total_clock = 0;

	    while (total_clock < getFrameTime())
	    {
		tickClocks(getMinTime());
	    }
	}

	uint64_t getMinTime()
	{
	    uint64_t min_time = main_clock.timeToTick();

	    uint64_t uart_time = uart_clock.timeToTick();

	    if (uart_time < min_time)
	    {
		min_time = uart_time;
	    }

	    return min_time;
	}

	void tickClocks(uint64_t min_time)
	{
	    bool cpu_clk = main_clock.advance(min_time);
	    bool uart_clk = uart_clock.advance(min_time);
	    tick(cpu_clk);
	    tickUART(uart_clk);

	    total_clock += min_time;
	}

	uint64_t getFrameTime()
	{
	    return ((1e12 / 60));
	}

	bool sdlError(string msg)
	{
	    cout << msg << " SDL_Error: " << SDL_GetError() << endl;
	    return false;
	}
};


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
	cout << "Usage: botnami-test <ROM>" << endl;
	return 1;
    }

    BotnamiTest core;

    if (!core.loadROM(argv[1]))
    {
	return 1;
    }

    if (!core.init())
    {
	return 1;
    }

    core.run();
    core.shutdown();

    return 0;
}