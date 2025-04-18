#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <urlmon.h>
#include <shlwapi.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shlwapi.lib")

const char* downloadUrl = "https://github.com/torbenconto/dryadhookv1/releases/latest/download/dryadhookv1.dll";
const wchar_t* targetProcess = L"Terraria.exe";

const char* dryadhookLogo = R"(
 ____                       _ _   _             _   __     ___ 
|  _ \ _ __ _   _  __ _  __| | | | | ___   ___ | | _\ \   / / | 
| | | | '__| | | |/ _` |/ _` | |_| |/ _ \ / _ \| |/ /\ \ / /| |
| |_| | |  | |_| | (_| | (_| |  _  | (_) | (_) |   <  \ V / | |
|____/|_|   \__, |\__,_|\__,_|_| |_|\___/ \___/|_|\_\  \_/  |_|
            |___/

)";

bool getTempDllPath(char* outPath, size_t size) {
    char tempPath[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath) == 0) {
        std::cerr << "[!] Failed to get temp path. Error: " << GetLastError() << "\n";
        return false;
    }

    // Use fixed filename to avoid path-too-long issues for LoadLibraryA
    if (!PathCombineA(outPath, tempPath, "dryadhookv1.dll")) {
        std::cerr << "[!] Failed to create full temp DLL path.\n";
        return false;
    }

    return true;
}

bool downloadDll(const char* url, const char* path) {
    HRESULT hr = URLDownloadToFileA(nullptr, url, path, 0, nullptr);
    return SUCCEEDED(hr);
}

bool doesDllExist(const char* dllPath) {
    std::ifstream file(dllPath);
    return file.good();
}

int getProcessId(const wchar_t* procname) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe)) {
        CloseHandle(hSnapshot);
        return 0;
    }

    int pid = 0;
    do {
        if (wcscmp(procname, pe.szExeFile) == 0) {
            pid = pe.th32ProcessID;
            break;
        }
    } while (Process32NextW(hSnapshot, &pe));

    CloseHandle(hSnapshot);
    return pid;
}

bool injectDll(int pid, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        std::cerr << "[!] Failed to open process. Error: " << GetLastError() << "\n";
        return false;
    }

    void* allocMem = VirtualAllocEx(hProcess, nullptr, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!allocMem) {
        std::cerr << "[!] Failed to allocate memory in target process. Error: " << GetLastError() << "\n";
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, allocMem, dllPath, strlen(dllPath) + 1, nullptr)) {
        std::cerr << "[!] Failed to write memory in target process. Error: " << GetLastError() << "\n";
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    FARPROC pLoadLibrary = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!pLoadLibrary) {
        std::cerr << "[!] LoadLibraryA address not found. Error: " << GetLastError() << "\n";
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(
        hProcess, nullptr, 0,
        (LPTHREAD_START_ROUTINE)pLoadLibrary,
        allocMem, 0, nullptr
    );

    if (!hThread) {
        std::cerr << "[!] Failed to create remote thread. Error: " << GetLastError() << "\n";
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    std::cout << "[+] DLL injected successfully!\n";

    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}

int main() {
    char tempDllPath[MAX_PATH];
    if (!getTempDllPath(tempDllPath, sizeof(tempDllPath))) {
        return 1;
    }

    std::cout << dryadhookLogo;
    std::cout << "[*] DryadHook v1.0 - Injector Module\n";

    std::cout << "[*] Downloading DLL to temporary directory...\n";
    if (!downloadDll(downloadUrl, tempDllPath)) {
        std::cerr << "[!] Failed to download DLL from GitHub.\n";
        return 1;
    }

    if (!doesDllExist(tempDllPath)) {
        std::cerr << "[!] DLL does not exist at the specified path: " << tempDllPath << "\n";
        return 1;
    }

    std::cout << "[*] Searching for Terraria process...\n";
    int pid = getProcessId(targetProcess);

    if (pid == 0) {
        std::cerr << "[!] Terraria process not found.\n";
        return 1;
    }

    std::cout << "[*] Found Terraria (PID: " << pid << "). Injecting DLL...\n";

    if (!injectDll(pid, tempDllPath)) {
        std::cerr << "[!] DLL injection failed.\n";
        return 1;
    }

    std::cout << "[*] Press any key to exit...\n";
    std::cin.get();
    return 0;
}
