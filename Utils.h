#include <Windows.h>
#include <winhttp.h>
#include <string>
#include <locale>
#include <codecvt>
#include <IPTypes.h>
#include <iphlpapi.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "IPHLPAPI.lib")

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
                        do
                        {
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

static std::string getLocalIPs()
{
    DWORD size = 32000;
    DWORD result = 0;
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    char ip[INET_ADDRSTRLEN];
    pAddresses = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), 0, size);
    result = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_ALL_INTERFACES, nullptr, pAddresses, &size);

    if(result == NO_ERROR)
    {
        PIP_ADAPTER_ADDRESSES currentAddr = pAddresses;
        
        while(currentAddr)
        {
            if(wcscmp(currentAddr->FriendlyName, L"Radmin VPN") == 0)
            {
                PIP_ADAPTER_UNICAST_ADDRESS unicastAddr = currentAddr->FirstUnicastAddress;

                while (unicastAddr)
                {
                    if (unicastAddr->Address.lpSockaddr->sa_family == AF_INET)
                    {
                        auto socketAddr = (struct sockaddr_in*)unicastAddr->Address.lpSockaddr;
                        
                        if (inet_ntop(AF_INET, &socketAddr->sin_addr, ip, INET_ADDRSTRLEN))
                            break;
                    }

                    unicastAddr = unicastAddr->Next;
                }

                break;
            }
            currentAddr = currentAddr->Next;
        }
    }

    HeapFree(GetProcessHeap(), 0, pAddresses);
    
    return std::string(ip);
}

