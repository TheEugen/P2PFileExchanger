#include "Server.h"


// user got a peer request and accepted it
int Server::doTransmission()
{
    // build long long from first 8 byte
    unsigned long long receivedByteCount = 0;
    memcpy(&receivedByteCount, &m_requestBuffer[0], sizeof(unsigned long long));

    std::cout << "Server: message says file has " << std::to_string(receivedByteCount) << " bytes" << std::endl;



    //std::wstring test = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes();
    //std::wcout << "Test-String: " << "\" " + test + " \" " << std::endl;
    // build str from 9th byte to end
    
    wchar_t* wFileName = new wchar_t[256];
    //std::wstring receivedFileName;
    //receivedFileName.resize(253);
    mbstowcs(wFileName, &m_requestBuffer[8], 256);
    auto receivedFileName = std::wstring(wFileName);
    
   //for (int i = 8; i < REQUEST_LENGTH; i += 2)
   //{
   //    // end of string
   //    if (m_requestBuffer[i] < 0x20 || m_requestBuffer[i] > 0x7E)
   //        break;
   //
   //    receivedFileName.append(std::wstring(1, m_requestBuffer[i]));
   //}
    
    std::wcout << "Server: message says name of file is " << receivedFileName << std::endl;

    DWORD dirBufferLen = 506;
    std::wstring dirBuffer;
    dirBuffer.resize(dirBufferLen);
    GetCurrentDirectoryW(dirBufferLen, (LPWSTR)dirBuffer.c_str());//const_cast<char *>((dirBuffer.c_str())));
    std::wcout << "Server: working directory is " << dirBuffer << " #####" << std::endl;

    std::cout << "String size: " << dirBuffer.length() << std::endl;
    
    // erase all zeros from dirbuffer
    for (int i = dirBuffer.size() - 1; i > -1; --i)
    {
        if (dirBuffer[i] != 0)
            break;
        dirBuffer.pop_back();
    }
    
    //dirBuffer = dirBuffer.c_str();
    
    // build filepath for the new file
    std::wstring finaleFilePath = std::wstring(dirBuffer);
    finaleFilePath.resize(finaleFilePath.size() - 1);
    finaleFilePath.shrink_to_fit();
    finaleFilePath += L"\\";
    finaleFilePath += receivedFileName;
    //std::wstring finaleFilePath = dirBuffer + L"\\" + receivedFileName;
    //auto wDirBuffer = std::wstring(dirBuffer);
    //auto wBS = std::wstring(L"\\");
    //auto finaleFilePath = wDirBuffer + wBS + receivedFileName;

    std::wcout << "finalpath: " << finaleFilePath << std::endl;

    // init FileIO object
    //finalFilePath = "C:\\Users\\Eugen\\Desktop\\test.exe";
    FileIO* fileIO = new FileIO(finaleFilePath, 'w');
    //FileIO* fileIO = new FileIO(receivedFilePath, 'w');

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
            // TODO: catch errors, especially if adminrights are necessary
            if (!WriteFile(fileIO->getFile(), (void*)m_packageBuffer, tmp, &bytesWritten, nullptr))
            {
                std::cout << "Error writefile: " << GetLastError() << std::endl;
            }
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
    //delete fileIO;
    doCleanup(fileIO, 1, m_clientSocket);

    return 1;
}

int Server::handleRequest()
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

    m_isRunning = true;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
    if (result)
    {
        printf("WSAStartup failed with error: %d\n", result);
        return 0;
    }

    
    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(m_serverSocketInfo);
        WSACleanup();
        return 0;
    }

    ZeroMemory(&m_socketInCreateHints, sizeof(m_socketCreateHints));
    m_socketInCreateHints.sin_family = AF_INET;
    m_socketInCreateHints.sin_addr.S_un.S_addr = INADDR_ANY;
    m_socketInCreateHints.sin_port = htons(27015);

    result = bind(m_listenSocket, (SOCKADDR*)&m_socketInCreateHints, sizeof(m_socketInCreateHints));
    if (result == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(m_serverSocketInfo);
        closesocket(m_listenSocket);
        WSACleanup();
        return 0;
    }

    if(m_serverSocketInfo)
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

    std::cout << "Listening for transmission requests.." << std::endl;

    m_clientSocket = accept(m_listenSocket, nullptr, nullptr);
    if (m_clientSocket == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(m_listenSocket);
        WSACleanup();
        return 0;
    }

    // listen socket no longer needed
    //doCleanup(nullptr, 1, m_listenSocket);

    // get request message
    // receive 264 bytes and write it into requestBuffer
    cBytesReceived = recv(m_clientSocket, m_requestBuffer, m_requestBufferLen, 0);
    if (cBytesReceived < 0)
    {
        doErrorCleanup("Server: recv failed", true, nullptr, 2, m_listenSocket, m_clientSocket);
        return 0;
    }

    // handle request
    handleRequest();

    return 1;
}