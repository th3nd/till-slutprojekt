#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <math.h>

#include "features.h"
#include "SerialClass.h"

bool connected;
Serial* serial_port = NULL;
char arr[3];

void arduino::ini() {
	//defina vilken port vi skickar data till. här är vår arduino connectad.
	serial_port = new Serial(L"COM4"); //	L // fixa grej automatiskt, fråga om port

	if (serial_port->IsConnected())
		std::cout << "[arduino] connected.\n";
	connected = true;
}

// move och click modifierar data som vi sedan skickar til, arduoino med hjälp av send.

void arduino::move(int a, int b) {	//x y 
	arr[0] += a;
	arr[1] += b;
}

void arduino::click(bool c) { // mousestate (0 = inget, 1 = m1s)
	arr[2] = c;
}

void arduino::send() {
	if (connected) // ifall inte connectad till arduino blir det lite errors
		serial_port->WriteData(arr, sizeof(arr));
	else {
		// när vi inte har en arduino att använda använder vi oss av windows mousemove funktion. är sämre men bättre än inget
		mouse_event(MOUSEEVENTF_MOVE, arr[0], arr[1], 0, 0);
	}
	
	for (int i = 0; i < 3; i++) // gör detta pga att def måste vara i loop ifall vi inte vill ha samma vals hela tiden
		arr[i] = 0;				// ifall vi har utanför loop resettas arr bara vid början av programmet, kan vara trevligt att t.ex sluta stjuta ibland.
}