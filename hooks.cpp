#include "hooks.h"

DWORD hooks::GetFunctionFromMemorySignature(std::vector<int> signature, DWORD startAddress, DWORD endAddress) {
	MEMORY_BASIC_INFORMATION mbi;
	for (DWORD i = startAddress; i < endAddress - signature.size(); i++)
	{
		if (VirtualQuery((LPCVOID)i, &mbi, sizeof(mbi)))
		{
			if (mbi.Protect & (PAGE_GUARD | PAGE_NOCACHE | PAGE_NOACCESS) || !(mbi.State & MEM_COMMIT))
			{
				i += mbi.RegionSize;
				continue;
			}
			for (DWORD k = (DWORD)mbi.BaseAddress; k < (DWORD)mbi.BaseAddress + mbi.RegionSize - signature.size(); k++)
			{
				for (DWORD j = 0; j < signature.size(); j++)
				{
					if (signature.at(j) != -1 && signature.at(j) != *(BYTE*)(k + j))
					{
						break;
					}
					if (j + 1 == signature.size())
					{
						return k;
					}
				}
			}
			i = (DWORD)mbi.BaseAddress + mbi.RegionSize;
		}
	}
	return 0;
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