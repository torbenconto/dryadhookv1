#pragma once

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <dxgi.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

namespace window 
{
	inline bool open = false;
	constexpr const char* WINDOW_TITLE = "DriadHook - Terraria Internal";
	constexpr const char* WINDOW_CLASS = "DriadHook";

	inline HWND hWnd = nullptr;
	inline ID3D11Device* pDevice = nullptr;
	inline ID3D11DeviceContext* pDeviceContext = nullptr;
	inline IDXGISwapChain* pSwapChain = nullptr;

	bool Setup();
	void Destroy();

	bool CreateDeviceD3D(HWND hWnd);
	void DestroyDeviceD3D();

	void SetupImGui();
	void DestroyImGui();

	void Render();
}