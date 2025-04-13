/*

 ____                       _ _   _             _   __     ___
|  _ \ _ __ _   _  __ _  __| | | | | ___   ___ | | _\ \   / / |
| | | | '__| | | |/ _` |/ _` | |_| |/ _ \ / _ \| |/ /\ \ / /| |
| |_| | |  | |_| | (_| | (_| |  _  | (_) | (_) |   <  \ V / | |
|____/|_|   \__, |\__,_|\__,_|_| |_|\___/ \___/|_|\_\  \_/  |_|
            |___/

        Dryad Hook V1 - Freeware Terraria Internal Cheat
			    Copyright (C) 2023 Torben Conto
*/

// BEGIN SECTION INCLUDES
#include "dryadhook.h"
// END SECTION INCLUDES

// BEGIN SECTION UTILS

HWND GetProcessWindow() {
    HWND hwnd = nullptr;
    DWORD pid = GetCurrentProcessId();

    do {
        hwnd = FindWindowEx(nullptr, hwnd, nullptr, nullptr);
        DWORD windowPid = 0;
        GetWindowThreadProcessId(hwnd, &windowPid);

        if (windowPid == pid && IsWindowVisible(hwnd))
            return hwnd;

    } while (hwnd != nullptr);

    return nullptr;
}

// END SECTION UTILS

// BEGIN SECTION IMGUI
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (dryadhook::bShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	return CallWindowProc(dryadhook::oWndProc, hWnd, msg, wParam, lParam);
}
// END SECTION IMGUI

// BEGIN SECTION DIRECTX9

bool GetD3D9Device(void **vtable, size_t size)
{
	if (!vtable)
		return false;

	dryadhook::pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!dryadhook::pD3D)
		return false;

	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetProcessWindow();

    if (FAILED(dryadhook::pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dryadhook::pDevice))) {
        dryadhook::pD3D->Release();
        return false;
    }

	memcpy(vtable, *(void***)dryadhook::pDevice, size);

	dryadhook::pDevice->Release();
	dryadhook::pD3D->Release();

	return true;
}

void ReleaseD3D9Device()
{
	if (dryadhook::pD3D)
	{
		dryadhook::pD3D->Release();
		dryadhook::pD3D = nullptr;
	}
	if (dryadhook::pDevice)
	{
		dryadhook::pDevice->Release();
		dryadhook::pDevice = nullptr;
	}
}

// END SECTION DIRECTX9

// BEGIN SECTION HOOKS

HRESULT __stdcall Hooked_EndScene(LPDIRECT3DDEVICE9 pDevice)
{
	if (!dryadhook::bInitialized)
	{
		dryadhook::oWndProc = (WNDPROC)SetWindowLongPtr(dryadhook::hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(dryadhook::hWnd);
		ImGui_ImplDX9_Init(pDevice);
		dryadhook::bInitialized = true;
	}

	// Keyboard input
	if (GetAsyncKeyState(VK_INSERT) & 1)
		dryadhook::bShowMenu = !dryadhook::bShowMenu;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (dryadhook::bShowMenu)
	{
		ImGui::Begin("Dryad Hook", &dryadhook::bShowMenu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Hello, world!");
		ImGui::End();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return dryadhook::oEndScene(pDevice);
}
// END SECTION HOOKS

// BEGIN SECTION MAIN

DWORD WINAPI HookMain(LPVOID lpParam)
{
	dryadhook::hWnd = GetProcessWindow();

	void* vtable[119] = { nullptr };
	if (!GetD3D9Device(vtable, sizeof(vtable)))
		return 0;

	if (MH_Initialize() != MH_OK)
		return 0;

	dryadhook::oEndScene = (HRESULT(__stdcall*)(LPDIRECT3DDEVICE9))vtable[42];

	if (!MH_CreateHook(vtable[42], &Hooked_EndScene, reinterpret_cast<void**>(&dryadhook::oEndScene)) == MH_OK) {
		MH_Uninitialize();
		return 0;
	}

	if (MH_EnableHook(vtable[42]) != MH_OK) {
		MH_RemoveHook(vtable[42]);
		MH_Uninitialize();
		return 0;
	}

	while (!GetAsyncKeyState(VK_END)) {
		Sleep(1);
	}

	MH_DisableHook(vtable[42]);
	MH_RemoveHook(vtable[42]);
	MH_Uninitialize();
	
	ReleaseD3D9Device();

	FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, 0, HookMain, hModule, 0, nullptr);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

// END SECTION MAIN