#pragma once


#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "Resources.h"
#include "FileIO.h"
#include "Cleanup.h"


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")



class Client
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* serverInfo = NULL,
        * ptr = NULL,
        hints;

    char m_requestBuffer[REQUEST_LENGTH] = { 0 };
    int m_requestBufferLen = REQUEST_LENGTH;
    char m_packageBuffer[PACKAGE_LENGTH] = { 0 };
    int m_packageBufferLen = PACKAGE_LENGTH;

public:
    
    int run(std::wstring const& serverIP, std::wstring filePath);
    int run2(std::string const& serverIP, std::string filePath);

    /*
    static DWORD WINAPI startClientThread(LPVOID param)
    {
        auto args = (PCLIENTTHREADARGUMENTS)param;

        if(!args)
            return 1;

        Client* client = args->client;

        client->run(args->serverIP, args->filePath);

        return 0;
    }
    */
};
