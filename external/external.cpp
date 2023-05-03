#include <iostream>
// för dwords etc.
#include <Windows.h>
// toolhelp32 - för getmodule och read funktioner
#include <TlHelp32.h>

#include <string>

// för std::clamp
#include <algorithm>
// för abs
#include <cmath>

#include "features.h"
#include "hazedumper.h"
#include "vec.h"

//-----våra variabler-----//
DWORD procid;
HWND game_wnd = NULL;
uintptr_t modulebaseaddr = NULL;
uintptr_t enginebaseaddr = NULL;
uintptr_t engine_client_state = NULL;
uintptr_t localplayer = NULL;

int flags = NULL;
int incross = NULL;
//---slutet på variabler---//

uintptr_t read(uintptr_t addr, DWORD procid) {
    // deklarera vad vi skickar tillbaka
    int val;  // bytte från uintptr och skit fixade sig :))))
    // läs memory med address och process id (process id är alltid den samma så det här är lite onödigt)
    Toolhelp32ReadProcessMemory(procid, (LPVOID)addr, &val, sizeof(val), NULL);
    //skicka tillbaka vår value
    return val;
}

// samma som den andra, men den här läser floats. (nödvändligt för att få entitypos)
uintptr_t readf(uintptr_t addr, DWORD procid) {
    float val; 
    Toolhelp32ReadProcessMemory(procid, (LPVOID)addr, &val, sizeof(val), NULL);
    return val;
}

view_matrix_t readm(uintptr_t addr, DWORD procid) {
    view_matrix_t val;
    Toolhelp32ReadProcessMemory(procid, (LPVOID)addr, &val, sizeof(val), NULL);
    return val;
}


uintptr_t GetModuleBaseAddress(const wchar_t* modName) { // snott fr�n guidedhacking.com
    // leta reda på vår handle, utan att öppna den (wow)
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procid);
    if (hSnap != INVALID_HANDLE_VALUE) { // sanitycheck
        // definera entry-point
        MODULEENTRY32 modEntry;
        // definera storlek
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) { // sanitycheck
            do {
                if (!wcscmp(modEntry.szModule, modName))
                {
                    // stäng "handle" och städa upp efter oss
                    CloseHandle(hSnap);
                    return (uintptr_t)modEntry.modBaseAddr;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
}

int main() {

    while (!game_wnd) 
        game_wnd = FindWindowA(NULL, "Counter-Strike: Global Offensive - Direct3D 9");

    while (!procid)
        GetWindowThreadProcessId(game_wnd, &procid);
    std::cout << "[setup] procid: " << std::dec << procid << "\n";

    while (!modulebaseaddr)
        modulebaseaddr = GetModuleBaseAddress(L"client.dll");
    std::cout << "[setup] modulebaseaddr: 0x" << std::hex << modulebaseaddr << "\n";

    while (!enginebaseaddr)
        enginebaseaddr = GetModuleBaseAddress(L"engine.dll");
    std::cout << "[setup] enginebaseaddr: 0x" << std::hex << enginebaseaddr << "\n";

    while (!localplayer)
        localplayer = read((modulebaseaddr + hazedumper::signatures::dwLocalPlayer), procid);
    std::cout << "[setup] localplayer: 0x" << std::hex << localplayer << "\n";

//    arduino::ini(); //    kommentera bort ifall ingen arduino ikopplad

    while (true) { // otrolig loop
        hack::hotkey_manager(); // för keybinds
        hack::aim_bot();
        hack::trigger_bot();
        arduino::send();
        Sleep(1); // så programmet inte äter upp aldelles för mycket cpu
    }

    return 0;
}

// features

void hack::hotkey_manager() {
    if (GetAsyncKeyState(VK_INSERT) & 1) {
        trigger = !trigger;
        std::cout << "[hack] trigger_bot set to: " << trigger << "\n"; //hatar cout och borde finnas snyggare sätt att printa till konsol, har c nånsin hört talas om pythons print?!?!?
    }
    
    if (GetAsyncKeyState(VK_HOME) & 1) {
        aimbot = !aimbot;
        std::cout << "[hack] aim_bot set to: " << aimbot << "\n";
    }
    
    // det här förstår du
}

void hack::trigger_bot() {
    if (trigger == false)
        return;

    if (!GetAsyncKeyState(0x06))
        return;

    // incross = fuskigt sätt att veta om vi siktar på någon (även teammates)
    // ger tillbaka id på vad vi siktar
    incross = read(localplayer + hazedumper::netvars::m_iCrosshairId, procid);

    // oftast finns det bara spelare med id 1 - 10, men man kan aldrig vara för säker
    if (incross > 0 && incross < 65) //ifall vi siktar på en spelare med player id 1 - 64 (vi struntar i 0 då det är en "world entity")
        arduino::click(1); // call arduino click
}

void hack::aim_bot() { // har inte direkt kommit jättelångt
    if (aimbot == false)
        return;

//    if (!GetAsyncKeyState(0x01))
//        return;

    static vec3 screen = { 1280, 1024 };
    static vec3 delta = { 0, 0 };
    vec3 closest = { 9999, 9999 };
    int usableclosest = 9999;

    static int max_speed = 10;
    static int max_delta = 50;

    view_matrix_t matrix = readm(modulebaseaddr + hazedumper::signatures::dwViewMatrix, procid);
    
    for (int i = 1; i < 32; i++) { // 0 = localplayer
        auto ent = read(modulebaseaddr + hazedumper::signatures::dwEntityList + (i * 0x10), procid);

        // ifall spelaren inte finns så går vi vidare till nästa call i loopen
        if (!ent) continue;

        // ifall spelare är död så går vidare (spelarna finns kvar i spelet när de är döda)
        int ent_h = read(ent + hazedumper::netvars::m_iHealth, procid);
        if (ent_h <= 0) continue;

        // ja, vi behöver använda readf trots att vi sen konverterar variablen till en int som vi sen castar till en float. ser lite roligt ut men behövs
        int x = readf(ent + hazedumper::netvars::m_vecOrigin, procid);
        int y = readf(ent + hazedumper::netvars::m_vecOrigin + 0x4, procid);
        int z = readf(ent + hazedumper::netvars::m_vecOrigin + 0x8, procid);
        auto in_duck = read(ent + hazedumper::netvars::m_fFlags, procid) & (1 << 1);
        vec3 entpos = { float(x), float(y), float(z + (in_duck ? 32 : 64)) }; // 64 = hur lång spelaren är (annorlunda när den crouchar)

        // omvandla 3d koordinater till ett 2d-space
        vec3 out;
        world_to_screen(screen, entpos, out, matrix);

        // ex: en spelare ligger på koordinaterna (1000, 1000)
        // (1000, 1000) - (1280, 1024)/2
        // (1000 - 640, 1000 - 512) == (360, 488)
        // vi behöver gå upp 488 pixlar och gå åt höger med 360 pixlar för att sikta på vår ent.
        delta = out - (screen / 2);

        int usabledelta = abs(delta.x) + abs(delta.y);
        if ((usabledelta) <= (usableclosest)) {
            usableclosest = usabledelta;
            closest = delta;
        }
    }

    // sanitycheck (var ett tag sen jag la till den här, vet inte riktigt varför den behövs. kan ha varit debug-grej men skadar inte att ha)
    if (usableclosest > max_delta * 2) return;

    // vår makeshift-smoothing
    closest.x /= 2;
    closest.y /= 2;

    // clamp
    closest.x = std::clamp(int(closest.x), max_speed * -1, max_speed);
    closest.y = std::clamp(int(closest.y), max_speed * -1, max_speed);
    
    arduino::move(closest.x, closest.y);
}