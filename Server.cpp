#include "Server.h"


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
    receivedFilePath = "C:\\Users\\GREG\\Desktop\\testerfolgreich.exe";

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
        tmp = recv(m_clientSocket, m_packageBuffer, m_packageBufferLen, 0);
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
        // TODO: clear buffer?
            
    } while (cBytesReceived < receivedByteCount);

    // send message to client that we are done so it can close it's socket
    memset(&m_requestBuffer[0], 1, m_requestBufferLen);
    tmp = send(m_clientSocket, &m_requestBuffer[0], m_requestBufferLen, 0);
    
    std::cout << "Transmission complete" << std::endl;
    std::cout << "Received " << std::to_string(cBytesReceived) << " bytes" << std::endl;

    // cleanup
    delete fileIO;

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
    } while (userInput.compare("y") != 0 && userInput.compare("n") != 0);
    

    // send answer to peer/client
    int result = 0;
    // accepting request
    if (userInput.compare("y") == 0)
    {
        printf("Server: user accepted\n");
        memset(&m_requestAnswerBuffer[0], 1, m_requestAnswerBufferLen);
        result = send(m_clientSocket, m_requestAnswerBuffer, m_requestAnswerBufferLen, 0);
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
        result = send(m_clientSocket, m_requestAnswerBuffer, m_requestAnswerBufferLen, 0);
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

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
    if (result)
    {
        printf("WSAStartup failed with error: %d\n", result);
        return 0;
    }

    ZeroMemory(&m_socketCreateHints, sizeof(m_socketCreateHints));
    m_socketCreateHints.ai_family = AF_INET;
    m_socketCreateHints.ai_socktype = SOCK_STREAM;
    m_socketCreateHints.ai_protocol = IPPROTO_TCP;
    m_socketCreateHints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    result = getaddrinfo(NULL, DEFAULT_PORT, &m_socketCreateHints, &m_serverSocketInfo);
    if (result)
    {
        //printf("getaddrinfo failed with error: %d\n", result);
        //WSACleanup();
        doErrorCleanup("Server: getaddrinfo failed with error: " + std::to_string(result), false, NULL, 0, NULL);
        return 0;
    }

    std::cout << "Your IP: " << m_serverSocketInfo->ai_addr << std::endl;

    m_isRunning = true;

    // Create a SOCKET for the server to listen for client connections.
    m_listenSocket = socket(m_serverSocketInfo->ai_family, m_serverSocketInfo->ai_socktype, m_serverSocketInfo->ai_protocol);
    if (m_listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(m_serverSocketInfo);
        WSACleanup();
        return 0;
    }

    // Setup the TCP listening socket
    result = bind(m_listenSocket, m_serverSocketInfo->ai_addr, (int)m_serverSocketInfo->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(m_serverSocketInfo);
        closesocket(m_listenSocket);
        WSACleanup();
        return 0;
    }

    freeaddrinfo(m_serverSocketInfo);

    int cBytesReceived = 0;

    // listen for incoming tcp request
    result = listen(m_listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(m_listenSocket);
        WSACleanup();
        return 0;
    }

    // Accept a client socket
    sockaddr connectingClientInfo;
    m_clientSocket = accept(m_listenSocket, &connectingClientInfo, NULL);
    if (m_clientSocket == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(m_listenSocket);
        WSACleanup();
        return 0;
    }

    // get request message
    // receive 264 bytes and write it into requestBuffer
    cBytesReceived = recv(m_clientSocket, m_requestBuffer, m_requestBufferLen, 0);

    // something went wrong
    if (cBytesReceived < 0)
    {
        printf("recv failed with error: %d\n", WSAGetLastError());
        closesocket(m_clientSocket);
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
        result = listen(m_listenSocket, SOMAXCONN);
        if (result == SOCKET_ERROR) {
            printf("listen failed with error: %d\n", WSAGetLastError());
            closesocket(m_listenSocket);
            WSACleanup();
            return 0;
        }

        // Accept a client socket
        sockaddr connectingClientInfo;
        m_clientSocket = accept(m_listenSocket, &connectingClientInfo, NULL);
        if (m_clientSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(m_listenSocket);
            WSACleanup();
            return 0;
        }

        // get request message
        // receive 264 bytes and write it into requestBuffer
        cBytesReceived = recv(m_clientSocket, m_requestBuffer, m_requestBufferLen, 0);

        // something went wrong
        if (cBytesReceived < 0)
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(m_clientSocket);
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