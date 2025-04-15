#pragma once

#include <vector>
#include <Windows.h>
#include <string>
#include "dryadhook.h"

namespace signatures {
	inline std::vector<int> UpdateFunctionSignature = { 0x55, 0x8b, 0xec, 0x57, 0x56, 0x53, 0x81, 0xec, -1, -1, -1, -1, 0x8b, 0xf1, 0x8d, 0xbd, -1, -1, -1, -1, 0xb9, -1, -1, -1, -1, 0x33, 0xc0, 0xf3, 0xab, 0x8b, 0xce, 0x89, 0x8d, -1, -1, -1, -1, 0x89, 0x55, 0xdc, 0x8b, 0x45, 0xdc, 0x3b, 0x05, -1, -1, -1, -1, 0x75, 0x0f };

	inline std::vector<int> gameMenuSig = { 0x80, 0x3D, -1, -1, -1, -1, -1, 0x74, -1, 0x8B, 0x45, -1, 0x8B, 0xE5, 0x5D, 0xC2, -1, -1, 0x51, 0x52, 0x8B, 0x0D, -1, -1, -1, -1, 0x8D, 0x55, -1, 0xFF, 0x15, -1, -1, -1, -1, 0x8D, 0x4D, -1, 0xE8 };

	inline std::vector<int> localPlayerSignature = { 0xA1,  -1, -1, -1, -1, 0x8B, 0x15, -1, -1, -1, -1, 0x3B, 0x50, 0x04, 0x73, 0x05, 0x8B, 0x44, 0x90, 0x08 };

	inline std::vector<int> hurtFunctionSignature = { 0x55, 0x8b, 0xec, 0x57, 0x56, 0x53, 0x81, 0xec, -1, -1, -1, -1, 0x8b, 0xf1, 0x8d, 0xbd, -1, -1, -1, -1, 0xb9, -1, -1, -1, -1, 0x33, 0xc0, 0xf3, 0xab, 0x8b, 0xce, 0x89, 0x8d, -1, -1, -1, -1, 0x89, 0x95, -1, -1, -1, -1, 0x8b, 0x85, -1, -1, -1, -1, 0x80, 0xb8, -1, -1, -1, -1, -1, 0x74, 0x25 };
}

namespace hooks {
	float __fastcall hurtFunction(void* __1, void* damageSource, int damage, int hitDirection, bool pvp, bool quiet, bool crit, int cooldownCounter, bool dodgeable);
	inline float(__fastcall* oHurtFunction)(void*, void*, int, int, bool, bool, bool, int, bool) = nullptr;
	
	void __fastcall updateFunction(void* __1, int w);
	inline void(__fastcall* oUpdateFunction)(void*, int) = nullptr;

	DWORD GetAddressFromMemorySignature(std::vector<int> signature, DWORD startAddress, DWORD endAddress);
	DWORD GetLocalPlayer();
	bool* GetGameMenuFlag();
}

namespace player {
	inline int health = 0;
	inline int maxHealth = 0;
	inline float posX = 0.0f;
	inline float posY = 0.0f;
	inline float velX = 0.0f;
	inline float velY = 0.0f;
}