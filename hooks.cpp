#include "hooks.h"
#include "minhook/minhook.h"

void hooks::Setup() {
	if (MH_Initialize() != MH_OK) {
		MessageBoxA(0, "Failed to initialize MinHook", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Hook Here
}

void hooks::Destroy() {
	// Unhook Here
	MH_Uninitialize();
}