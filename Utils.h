#include <Windows.h>
#include <winhttp.h>
#include <string>
#include <locale>
#include <codecvt>

#pragma comment(lib, "winhttp.lib")

static std::string getPublicIP()
{
    std::string ipAddress;

    HINTERNET hInternet = WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hInternet)
    {
        HINTERNET hConnect = WinHttpConnect(hInternet, L"api.ipify.org", INTERNET_DEFAULT_HTTP_PORT, 0);
        if (hConnect)
        {
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
            if (hRequest)
            {
                if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
                {
                    if (WinHttpReceiveResponse(hRequest, NULL))
                    {
                        DWORD dwSize = 0;
                        DWORD dwDownloaded = 0;
                        do {
                            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                            {
                                break;
                            }

                            if (dwSize == 0)
                            {
                                break;
                            }

                            LPSTR pszData = new char[dwSize + 1];
                            ZeroMemory(pszData, dwSize + 1);

                            if (!WinHttpReadData(hRequest, (LPVOID)pszData, dwSize, &dwDownloaded))
                            {
                                delete[] pszData;
                                break;
                            }

                            ipAddress = std::string(pszData);

                            //std::string strData(pszData);
                            //delete[] pszData;
                            //
                            //std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                            //ipAddress = converter.from_bytes(strData);

                        } while (dwSize > 0);
                    }
                }

                WinHttpCloseHandle(hRequest);
            }

            WinHttpCloseHandle(hConnect);
        }

        WinHttpCloseHandle(hInternet);
    }

    return ipAddress;
}