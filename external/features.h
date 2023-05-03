#include <iostream>

static bool trigger;
static int trigger_min_delay = 30; // använt i tidigare version av program
static int trigger_max_delay = 200; // använt i tidigare version av program

static bool aimbot;

namespace hack {
	void hotkey_manager();
	void trigger_bot();
	void aim_bot();
}

namespace arduino {
	void ini();
	void move(int a, int b);
	void click(bool c);
	void send();
}


namespace math {
	int sin(int a, int b, int c);
	int cos(int a, int b, int c);
	int tan(int a, int b, int c);
}

struct bone_matrix
{
	int _junk1[0xC];
	float x;
	int _junk2[0xC];
	float y;
	int _junk3[0xC];
	float z;
};

