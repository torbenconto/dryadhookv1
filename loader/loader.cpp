#include "loader.h"
#include <string>
#include <windows.h>
#include <winhttp.h>
#include <d3d9.h>
#include <tchar.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx9.h"

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "d3d9.lib")

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS g_d3dpp = {};

POINT guiMousePos = { 0, 0 };

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool CreateDeviceD3D(HWND hWnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    return g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) >= 0;
}

void CleanupDeviceD3D() {
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

// Reset Direct3D device
void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    g_pd3dDevice->Reset(&g_d3dpp);
    ImGui_ImplDX9_CreateDeviceObjects();
}

std::string GetLatestDLLUrl(const std::string& user, const std::string& repo, const std::string& assetName) {
    std::string result;

    HINTERNET hSession = WinHttpOpen(L"Dryadhook Loader/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return "";

    HINTERNET hConnect = WinHttpConnect(hSession, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    std::string path = "/repos/" + user + "/" + repo + "/releases/latest";
    std::wstring wPath = std::wstring(path.begin(), path.end());

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wPath.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    BOOL sent = WinHttpSendRequest(hRequest, L"User-Agent: Dryadhook Loader/1.0\r\n", -1L,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!sent || !WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    DWORD dwSize = 0;
    do {
        DWORD dwDownloaded = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize) || dwSize == 0) break;

        char* buffer = new char[dwSize + 1];
        ZeroMemory(buffer, dwSize + 1);

        if (!WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded)) {
            delete[] buffer;
            break;
        }

        result.append(buffer, dwDownloaded);
        delete[] buffer;
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    size_t pos = result.find("\"name\":\"" + assetName + "\"");
    if (pos == std::string::npos) return "";

    pos = result.rfind("{", pos);
    if (pos == std::string::npos) return "";

    size_t urlPos = result.find("\"browser_download_url\":\"", pos);
    if (urlPos == std::string::npos) return "";

    urlPos += strlen("\"browser_download_url\":\"");
    size_t endQuote = result.find("\"", urlPos);
    if (endQuote == std::string::npos) return "";

    return result.substr(urlPos, endQuote - urlPos);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, NULL, NULL, NULL, NULL, _T("Dryadhook Loader"), NULL };
    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow(_T("Dryadhook Loader"), _T("Dryadhook Loader"), WS_POPUP, 100, 100, 400, 300, NULL, NULL, wc.hInstance, NULL);
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    if (!CreateDeviceD3D(hWnd)) {
        CleanupDeviceD3D();
        UnregisterClass(_T("Dryadhook Loader"), wc.hInstance);
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    bool running = true;
    while (running) {
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                running = false;
        }

        // Start frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Window layout
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(400, 300));
        ImGui::Begin("Dryadhook Loader", NULL,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Dryadhook Loader");
        ImGui::Text("Checking for updates...");
        std::string latestDLLUrl = GetLatestDLLUrl(loader::githubUsername, loader::githubRepo, loader::githubAssetName);
        if (!latestDLLUrl.empty()) {
            ImGui::Text("Latest DLL URL: %s", latestDLLUrl.c_str());
        }

        ImGui::End();

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hWnd);
    UnregisterClass(_T("Dryadhook Loader"), wc.hInstance);

    return 0;
}



// Window procedure for handling messages
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
        // Allow dragging the window by clicking anywhere
        ReleaseCapture();
        SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return 0;
    }



    return DefWindowProc(hWnd, msg, wParam, lParam);
}
