#include "loader.h"

#include <string>
#include <windows.h>
#include <winhttp.h>

std::string GetLatestDLLUrl(const std::string& user, const std::string& repo, const std::string& assetName) {
    HINTERNET hSession = WinHttpOpen(L"Dryadhook Loader/1.0", 
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
                                    WINHTTP_NO_PROXY_NAME, 
                                    WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        return "";
    }
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/repos/" + user + "/" + repo + "/releases/latest", 
                                            NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 
                                            WINHTTP_FLAG_SECURE);


    BOOL sent = WinHttpSendRequest(request, L"User-Agent: DryadLoader/1.0\r\n", -1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!sent || !WinHttpReceiveResponse(request, NULL)) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);
        return "";
    }
    

    DWORD dwSize = 0;
    do {
        DWORD dwDownloaded = 0;
        WinHttpQueryDataAvailable(request, &dwSize);
        if (dwSize == 0) {
            break;
        }

        char * buffer = new char[dwSize + 1];
        ZeroMemory(buffer, dwSize + 1);

        WinHttpReadData(request, (LPVOID)buffer, dwSize, &dwDownloaded);
        result.append(buffer, dwDownloaded);
        delete[] buffer;
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // Parse the JSON response to find the asset URL
    // size_t pos = result.find(assetName);
    // if (pos == std::string::npos) {
    //     return "";
    // }
    // pos = result.find("browser_download_url", pos);
    // if (pos == std::string::npos) {
    //     return "";
    // }
    // pos = result.find("\"", pos);
    // if (pos == std::string::npos) {
    //     return "";
    // }
    // size_t endPos = result.find("\"", pos + 1);
    // if (endPos == std::string::npos) {
    //     return "";
    // }

    // return result.substr(pos + 1, endPos - pos - 1);
}
