#pragma once

//  DX9
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")

//  IMGUI
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

// MINHOOK
#include "minhook/MinHook.h"

// WINDOWS
#include <Windows.h>
#include <tchar.h>


namespace dryadhook
{
	inline LPDIRECT3D9 pD3D = nullptr;

	inline LPDIRECT3DDEVICE9 pDevice = nullptr;
	inline HWND hWnd = nullptr;
	inline WNDPROC oWndProc = nullptr;
	inline bool bShowMenu = false;
	inline bool bInitialized = false;

	inline HRESULT(__stdcall* oEndScene)(LPDIRECT3DDEVICE9) = nullptr;


	// Cheats
	inline bool fGODMODE = false;
	inline bool fFLYHACK = false;
	inline bool fNOCLIP = false;
	

	inline float fVelocityMultiplier = 1.0f;

}

namespace menu {
	inline bool bShowTab1 = true;
	inline bool bShowTab2 = false;
	inline bool bShowTab3 = false;
}