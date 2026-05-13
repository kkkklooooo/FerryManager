#pragma once
#include <windows.h>

// Run the startup config phase. Returns true when World is ready, false if user quit.
bool RunSetupPhase(HWND hWnd, bool& quitRequested);
