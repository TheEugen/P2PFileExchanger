#include "Server.h"


int Server::init()
{
    int result;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result)
    {
        printf("WSAStartup failed with error: %d\n", result);
        return 0;
    }

    ZeroMemory(&socketCreateHints, sizeof(socketCreateHints));
    socketCreateHints.ai_family = AF_INET;
    socketCreateHints.ai_socktype = SOCK_STREAM;
    socketCreateHints.ai_protocol = IPPROTO_TCP;
    socketCreateHints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    result = getaddrinfo(NULL, DEFAULT_PORT, &socketCreateHints, &serverSocketInfo);
    if (result)
    {
        //printf("getaddrinfo failed with error: %d\n", result);
        //WSACleanup();
        doErrorCleanup("Server: getaddrinfo failed with error: " + std::to_string(result), false, NULL, 0, NULL);
        return 0;
    }

    std::cout << "Your IP: " << serverSocketInfo->ai_addr << std::endl;

    m_isRunning = true;

    return 1;
}

// user got a peer request and accepted it
int Server::doTransmission()
{
    // build long long from first 8 byte
    unsigned long long receivedByteCount = 0;
    memcpy(&receivedByteCount, &m_requestBuffer[0], sizeof(unsigned long long));

    std::cout << "Server: message says file has " << std::to_string(receivedByteCount) << " bytes" << std::endl;

    // build str from 9th byte to end
    std::string receivedFilePath;
    for (int i = 8; i < REQUEST_LENGTH; ++i)
    {
        // end of string
        if (m_requestBuffer[i] == 0)
            break;

        // append
        receivedFilePath.append(std::string(1, m_requestBuffer[i]));
    }

    std::cout << "Server: message says filepath is " << receivedFilePath << std::endl;
    receivedFilePath = "C:\\Users\\Eugen\\Desktop\\testerfolgreich.exe";
    // assert filepath len > 0
    // init FileIO object
    FileIO* fileIO = new FileIO(receivedFilePath, 'w');

    // TODO: check if file already exists in the constructor of FILEIO
    // check if file already exists
    /*if (fileIO->doesFileExist())
    {
        std::string userInput;
        // loop until users enters y or n
        do
        {
            std::cout << "\nIncoming file already exists, do you want to overwrite it?" << std::endl;

            // get answer
            std::getline(std::cin, userInput);

            // TODO: logic for y || n
            // compare will return 0 if userinput equals y or n
        } while (userInput.compare("y") != 0);

        // if user says no end transmission
        if (userInput.compare("n") == 0)
            return 0;
    }*/
    
    // TODO makefile assuming file doesnt exists or we are allowed to overwrite it
    //fileIO->makeFile();
    
    // receive and write into file
    int cBytesReceived = 0;
    int tmp = 0;
    DWORD bytesWritten = 0;
    do
    {
        tmp = recv(ClientSocket, m_packageBuffer, m_packageBufferLen, 0);
        if (tmp > 0)
        {
            WriteFile(fileIO->getFile(), (void*)m_packageBuffer, tmp, &bytesWritten, NULL);
            cBytesReceived += tmp;
        }
        else
        {
            std::cout << "Server: recv failed with error " << std::to_string(WSAGetLastError()) << std::endl;
            break;
        }
            
    } while (cBytesReceived < receivedByteCount);

    // fileIO cleanup
    std::cout << "Transmission complete" << std::endl;
    std::cout << "Received " << std::to_string(cBytesReceived) << " bytes" << std::endl;

    return 1;
}

int Server::handleRequest(sockaddr* clientInfo)
{
    // display request to user
    std::cout << "#####################################################################" << std::endl;
    std::cout << "##########                Incoming Request!!!             ###########" << std::endl;
    std::cout << "#####################################################################" << std::endl;

    std::string userInput;

    // loop until users enters y or n
    do
    {
        std::cout << "\nAccept incoming request from <IP> (y = yes, n = no)?" << std::endl;

        // get answer from user
        std::getline(std::cin, userInput);

        // compare will return 0 if userinput equals y or n
    } while (userInput.compare("y") != 0);
    //} while (userInput.compare("y") || userInput.compare("n"));
    printf("Server: exited userinput loop\n");

    // send answer to peer/client
    int result = 0;
    // accepting request
    if (!(userInput.compare("y")))
    {
        printf("Server: user accepted\n");
        memset(&m_requestAnswerBuffer[0], 1, m_requestAnswerBufferLen);
        result = send(ClientSocket, m_requestAnswerBuffer, m_requestAnswerBufferLen, 0);
        if (result < 0)
        {
            printf("Server: error sending");
        }
        doTransmission();
        return 1;
    }
    // declining request
    else
    {
        memset(&m_requestAnswerBuffer[0], 0, m_requestAnswerBufferLen);
        result = send(ClientSocket, m_requestAnswerBuffer, m_requestAnswerBufferLen, 0);
        if (result < 0)
        {
            // error handling
        }
        std::cout << "Transmission request declined!" << std::endl;
        return 0;
    }
}

int Server::run()
{
    int result;

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(serverSocketInfo->ai_family, serverSocketInfo->ai_socktype, serverSocketInfo->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(serverSocketInfo);
        WSACleanup();
        return 0;
    }

    // Setup the TCP listening socket
    result = bind(ListenSocket, serverSocketInfo->ai_addr, (int)serverSocketInfo->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(serverSocketInfo);
        closesocket(ListenSocket);
        WSACleanup();
        return 0;
    }

    freeaddrinfo(serverSocketInfo);

    int cBytesReceived = 0;

    // listen for incoming tcp request
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 0;
    }

    // Accept a client socket
    sockaddr connectingClientInfo;
    ClientSocket = accept(ListenSocket, &connectingClientInfo, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 0;
    }

    // get request message
    // receive 264 bytes and write it into requestBuffer
    cBytesReceived = recv(ClientSocket, m_requestBuffer, m_requestBufferLen, 0);

    // something went wrong
    if (cBytesReceived < 0)
    {
        printf("recv failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 0;
    }

    // handle request
    handleRequest(&connectingClientInfo);

    // free memory for new requests
    //freeaddrinfo(connectingClientInfo);
    
    /*
    do
    {
        // listen for incoming tcp request
        result = listen(ListenSocket, SOMAXCONN);
        if (result == SOCKET_ERROR) {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 0;
        }

        // Accept a client socket
        sockaddr connectingClientInfo;
        ClientSocket = accept(ListenSocket, &connectingClientInfo, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 0;
        }

        // get request message
        // receive 264 bytes and write it into requestBuffer
        cBytesReceived = recv(ClientSocket, m_requestBuffer, m_requestBufferLen, 0);

        // something went wrong
        if (cBytesReceived < 0)
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 0;
        }

        // handle request
        handleRequest(&connectingClientInfo);

        // free memory for new requests
        //freeaddrinfo(connectingClientInfo);

        // when user enters "exit" exit loop
    } while (m_isRunning);*/

    return 1;
}