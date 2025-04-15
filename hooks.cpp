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