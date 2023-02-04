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


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


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
    
    int run(std::string serverIP, std::string filePath);
    bool shutdownClient();

    static DWORD WINAPI startClientThread(LPVOID param)
    {
        // TODO: validate
        PCLIENTTHREADARGUMENTS args = (PCLIENTTHREADARGUMENTS)param;

        Client* client = args->client;

        client->run(args->serverIP, args->filePath);
    }
};
