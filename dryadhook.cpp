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

#include "dryadhook.h"
#include "hooks.h"
#include <intrin.h>


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

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (dryadhook::bShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	return CallWindowProc(dryadhook::oWndProc, hWnd, msg, wParam, lParam);
}

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
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.BackBufferWidth = 0;
	d3dpp.BackBufferHeight = 0;
	d3dpp.BackBufferCount = 1;

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

HRESULT __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    HRESULT result;

    static void* dwAllowedReturn = nullptr;
    void* dwReturnAddress = _ReturnAddress();

    result = dryadhook::oEndScene(pDevice);

    if (dwAllowedReturn == 0 || dwAllowedReturn == dwReturnAddress)
    {
        dwAllowedReturn = dwReturnAddress;

        if (!dryadhook::bInitialized)
        {
            dryadhook::oWndProc = (WNDPROC)SetWindowLongPtr(dryadhook::hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();

            io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
            ImGui::StyleColorsDark();

            ImGui_ImplWin32_Init(dryadhook::hWnd);
            ImGui_ImplDX9_Init(pDevice);
            dryadhook::bInitialized = true;
        }

        // Keyboard toggle
        if (GetAsyncKeyState(VK_INSERT) & 1)
            dryadhook::bShowMenu = !dryadhook::bShowMenu;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (dryadhook::bShowMenu)
        {
            ImGui::GetIO().MouseDrawCursor = true;
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 2.0f;
            style.FrameRounding = 2.0f;
            style.GrabRounding = 2.0f;
            style.ScrollbarRounding = 2.0f;
            style.FramePadding = ImVec2(8, 4);
            style.ItemSpacing = ImVec2(8, 8);
            style.IndentSpacing = 16.0f;
            style.WindowBorderSize = 0.0f;
            style.FrameBorderSize = 0.0f;

            ImVec4* colors = style.Colors;
            colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
            colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
            colors[ImGuiCol_Border] = ImVec4(0.10f, 0.10f, 0.10f, 0.30f);
            colors[ImGuiCol_Button] = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.22f, 0.27f, 1.00f);
            colors[ImGuiCol_Header] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.28f, 0.33f, 1.00f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.23f, 0.26f, 0.30f, 1.00f);
            colors[ImGuiCol_Text] = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);

            ImVec2 windowSize = ImVec2(550, 300);
            ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

            ImGui::Begin("Dryad Hook V1", &dryadhook::bShowMenu,
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

            float titleBarHeight = 30.0f;
            ImVec2 pos = ImGui::GetWindowPos();
            ImVec2 size = ImGui::GetWindowSize();

            ImGui::GetWindowDrawList()->AddRectFilled(
                pos, ImVec2(pos.x + size.x, pos.y + titleBarHeight),
                IM_COL32(40, 45, 55, 255), 4.0f, ImDrawFlags_RoundCornersTop);

            ImGui::SetCursorPos(ImVec2(10, 7));
            ImGui::TextColored(ImVec4(1, 1, 1, 1), "Dryad Hook V1");

            ImGui::SetCursorPos(ImVec2(size.x - 30, 5));
            if (ImGui::Button("X"))
                dryadhook::bShowMenu = false;

            ImGui::SetCursorPosY(titleBarHeight + 5);
            ImGui::BeginChild("MainFrame", ImVec2(0, 0), false);

            ImGui::BeginChild("Sidebar", ImVec2(120, 0), true);
            {
                auto TabButton = [](const char* label, bool selected) {
                    if (selected)
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                    else
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));

                    bool clicked = ImGui::Button(label, ImVec2(-1, 35));
                    ImGui::PopStyleColor();
                    return clicked;
                    };

                if (TabButton("Player", menu::bShowTab1)) {
                    menu::bShowTab1 = true;
                    menu::bShowTab2 = false;
                    menu::bShowTab3 = false;
                }
                if (TabButton("World", menu::bShowTab2)) {
                    menu::bShowTab1 = false;
                    menu::bShowTab2 = true;
                    menu::bShowTab3 = false;
                }
                if (TabButton("Misc", menu::bShowTab3)) {
                    menu::bShowTab1 = false;
                    menu::bShowTab2 = false;
                    menu::bShowTab3 = true;
                }
            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Content", ImVec2(0, 0), true);
            {
                if (menu::bShowTab1) {
                    ImGui::Text("Player Tab");
                    ImGui::Separator();

                    DWORD localPlayer = hooks::GetLocalPlayer();
                    ImGui::Text("localPlayer: 0x%X", localPlayer);

                    if (localPlayer == 0) {
                        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Player not found.");
                    }
                    else {
                        float posX = *(float*)(localPlayer + 0x28);
                        float posY = *(float*)(localPlayer + 0x2C);

                        ImGui::SliderFloat("Position X", &posX, 0, 60000);
                        ImGui::SliderFloat("Position Y", &posY, 600, 25000);

						ImGui::Checkbox("God Mode", &dryadhook::fGODMODE);

                        *(float*)(localPlayer + 0x28) = posX;
                        *(float*)(localPlayer + 0x2C) = posY;
                    }
                }

                if (menu::bShowTab2) {
                    ImGui::Text("World Tab");
                    ImGui::Separator();
                    ImGui::Text("World-related options go here.");
                }
                if (menu::bShowTab3) {
                    ImGui::Text("Misc Tab");
                    ImGui::Separator();
                    ImGui::Text("Miscellaneous options go here.");
                }
            }
            ImGui::EndChild();

            ImGui::EndChild();
            ImGui::End();
        }
        else {
            ImGui::GetIO().MouseDrawCursor = false;
        }

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    return result;
}

DWORD WINAPI Main(LPVOID lpParam)
{
	dryadhook::hWnd = GetProcessWindow();

	void* vtable[119] = { nullptr };
	if (!GetD3D9Device(vtable, sizeof(vtable)))
		return 0;

	if (MH_Initialize() != MH_OK)
		return 0;

	dryadhook::oEndScene = (HRESULT(__stdcall*)(LPDIRECT3DDEVICE9))vtable[42];

	if (MH_CreateHook(vtable[42], &hkEndScene, reinterpret_cast<void**>(&dryadhook::oEndScene)) != MH_OK) {
		MH_Uninitialize();
		return 0;
	}

	if (MH_EnableHook(vtable[42]) != MH_OK) {
		MH_RemoveHook(vtable[42]);
		MH_Uninitialize();
		return 0;
	}

	if (MH_CreateHook((LPVOID)hooks::GetAddressFromMemorySignature(signatures::hurtFunctionSignature, 0x25000000, 0x30000000), &hooks::hurtFunction, (LPVOID*)&hooks::oHurtFunction)) {
		MessageBoxA(nullptr, "Failed to create hook for hurt function", "Error", MB_OK | MB_ICONERROR);
	}
	
	MH_EnableHook(MH_ALL_HOOKS);

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
		CreateThread(nullptr, 0, Main, hModule, 0, nullptr);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}