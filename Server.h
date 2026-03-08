#pragma once


//#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <processthreadsapi.h>
#include <locale>
#include <codecvt>

#include <Inaddr.h>

#include "Resources.h"
#include "FileIO.h"
#include "Cleanup.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")



class Server
{
    WSADATA m_wsaData;

    SOCKET m_listenSocket = INVALID_SOCKET;
    SOCKET m_clientSocket = INVALID_SOCKET;

    struct addrinfo* m_serverSocketInfo = NULL;
    struct addrinfo m_socketCreateHints;

    struct sockaddr_in m_socketInCreateHints;

    bool m_isRunning = false;
    char m_requestBuffer[REQUEST_LENGTH] = { 0 };
    char m_packageBuffer[PACKAGE_LENGTH] = { 0 };
    char m_requestAnswerBuffer[REQUEST_LENGTH] = { 0 };
    int m_requestBufferLen = REQUEST_LENGTH;
    int m_packageBufferLen = PACKAGE_LENGTH;
    int m_requestAnswerBufferLen = REQUEST_LENGTH;

public:

    int run();
    int handleRequest();
    int doTransmission();

    static DWORD WINAPI startServerThread(LPVOID param)
    {
        // TODO: validate
        Server* server = (Server*)param;
        if (!server)
            throw std::runtime_error("Server startServerThread: invalid param parameter");

        server->run();

        return 1;
    }
};
