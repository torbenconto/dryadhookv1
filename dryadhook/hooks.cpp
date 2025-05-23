#include "hooks.h"

DWORD hooks::GetAddressFromMemorySignature(const std::vector<int> signature, DWORD startAddress, DWORD endAddress) {
    MEMORY_BASIC_INFORMATION mbi;
    DWORD current = startAddress;

    while (current < endAddress) {
        if (VirtualQuery((LPCVOID)current, &mbi, sizeof(mbi))) {
            if ((mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) || !(mbi.State & MEM_COMMIT)) {
                current += mbi.RegionSize;
                continue;
            }

            BYTE* buffer = (BYTE*)mbi.BaseAddress;
            DWORD regionSize = mbi.RegionSize;

            for (DWORD i = 0; i < regionSize - signature.size(); ++i) {
                bool found = true;
                for (DWORD j = 0; j < signature.size(); ++j) {
                    if (signature[j] != -1 && signature[j] != buffer[i + j]) {
                        found = false;
                        break;
                    }
                }
                if (found) {
                    return (DWORD)(buffer + i);
                }
            }

            current += regionSize;
        }
        else {
            break;
        }
    }

    return 0;
}
DWORD cachedLocalPlayer = 0;
DWORD lastCheckTime = 0;

DWORD hooks::GetLocalPlayer()
{
    DWORD currentTime = GetTickCount();
    if (currentTime - lastCheckTime < 5000) {
        return cachedLocalPlayer;
    }

    DWORD localPlayerAddress = hooks::GetAddressFromMemorySignature(signatures::localPlayerSignature, 0x26000000, 0x35000000);

    if (localPlayerAddress)
    {
        DWORD eax = *(DWORD*)(*(DWORD*)(localPlayerAddress + 0x1));
        DWORD edx = *(DWORD*)(*(DWORD*)(localPlayerAddress + 0x7));

        cachedLocalPlayer = *(DWORD*)(eax + (edx * 4) + 0x8);
    }
    else {
        cachedLocalPlayer = 0;
    }

    lastCheckTime = currentTime;
    return cachedLocalPlayer;
}

static bool* cached = nullptr;

bool* hooks::GetGameMenuFlag() {
    if (cached)
        return cached;

    DWORD gameMenuAddress = hooks::GetAddressFromMemorySignature(signatures::gameMenuSig, 0x26000000, 0x35000000);
    if (gameMenuAddress) {
        cached = (bool*)(*(DWORD*)(gameMenuAddress + 2));
    }
    return cached;
}

float __fastcall hooks::hurtFunction(void* __1, void* damageSource, int damage, int hitDirection, bool pvp, bool quiet, bool crit, int cooldownCounter, bool dodgeable)
{
	if (dryadhook::fGODMODE)
	{
		return 0.0f;
	}
	else
	{
		return oHurtFunction(__1, damageSource, damage, hitDirection, pvp, quiet, crit, cooldownCounter, dodgeable);
	}
}

void __fastcall hooks::updateFunction(void* __1, int w)
{
    DWORD localPlayer = hooks::GetLocalPlayer();
    if (localPlayer != 0)
    {
        // Read health and position
        int health = *(int*)(localPlayer + 0x408);
        int maxHealth = *(int*)(localPlayer + 0x400);

        float posX = *(float*)(localPlayer + 0x28);
        float posY = *(float*)(localPlayer + 0x2C);

        float velX = 0.0f;
        float velY = 0.0f;

        const float speed = 5.0f * dryadhook::fVelocityMultiplier;

        if (dryadhook::fNOCLIP)
        {
            *(float*)(localPlayer + 0x30) = 0.0f;  // velX
            *(float*)(localPlayer + 0x34) = -0.4f; // velY

            if (GetAsyncKeyState(0x57)) posY -= speed; // W
            if (GetAsyncKeyState(0x53)) posY += speed; // S
            if (GetAsyncKeyState(0x41)) posX -= speed; // A
            if (GetAsyncKeyState(0x44)) posX += speed; // D

            *(float*)(localPlayer + 0x28) = posX;
            *(float*)(localPlayer + 0x2C) = posY;
        }
        else if (dryadhook::fFLYHACK)
        {
            *(float*)(localPlayer + 0x34) = -0.4f;

            if (GetAsyncKeyState(0x57)) velY = -speed; // W = up
            if (GetAsyncKeyState(0x53)) velY = speed;  // S = down
            if (GetAsyncKeyState(0x41)) velX = -speed; // A = left
            if (GetAsyncKeyState(0x44)) velX = speed;  // D = right

            *(float*)(localPlayer + 0x30) = velX;
            *(float*)(localPlayer + 0x34) = velY;
        }
		if (dryadhook::fGODMODE)
		{
            *(float*)(localPlayer + 0x408) = *(int*)(localPlayer + 0x400);

		}

        player::health = health;
        player::maxHealth = maxHealth;
        player::posX = *(float*)(localPlayer + 0x28);
        player::posY = *(float*)(localPlayer + 0x2C);
        player::velX = *(float*)(localPlayer + 0x30);
        player::velY = *(float*)(localPlayer + 0x34);
    }

    oUpdateFunction(__1, w);
}
