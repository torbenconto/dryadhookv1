#include "window.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (window::open && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (window::pSwapChain != nullptr && wParam != SIZE_MINIMIZED)
			window::pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		break;
	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool window::Setup() {
	WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = (LPCWSTR)window::WINDOW_CLASS;
	wc.hIconSm = NULL;

	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	window::hWnd = CreateWindowA(window::WINDOW_CLASS, window::WINDOW_TITLE, WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, 0, 0, wc.hInstance, 0);

	if (!window::hWnd)
	{
		return false;
	}
}

void window::Destroy()
{
	if (window::hWnd) {
		DestroyWindow(window::hWnd);
		window::hWnd = nullptr;
	}
}

bool window::CreateDeviceD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//sd.BufferDesc.RefreshRate.Numerator = 60;
	//sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0 };

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		featureLevelArray,
		1,
		D3D11_SDK_VERSION,
		&sd,
		&window::pSwapChain,
		&window::pDevice,
		&featureLevel,
		&window::pDeviceContext
	);

	return SUCCEEDED(hr);
}

void window::DestroyDeviceD3D()
{
	if (window::pSwapChain) { window::pSwapChain->Release(); window::pSwapChain = nullptr; }
	if (window::pDeviceContext) { window::pDeviceContext->Release(); window::pDeviceContext = nullptr; }
	if (window::pDevice) { window::pDevice->Release(); window::pDevice = nullptr; }
}

void window::SetupImGui() {
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window::hWnd);
	ImGui_ImplDX11_Init(window::pDevice, window::pDeviceContext);
}

void window::DestroyImGui() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void window::Render() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin(window::WINDOW_TITLE, &window::open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Hello, world!");
	ImGui::Text("This is a simple window.");
	ImGui::Text("Press ESC to close.");
	ImGui::End();
	ImGui::Render();
	
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	if (window::pSwapChain) {
		window::pSwapChain->Present(0, 0);
	}
}