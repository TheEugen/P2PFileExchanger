#include "client.h"


int Client::run(std::wstring const& serverIP, std::wstring filePath)
{
    // init FileIO
    FileIO* fileIO = new FileIO(filePath, 'r');

    // check if given file exists
    if (!fileIO->doesFileExist())
    {
        std::cout << "File doesnt exist" << std::endl;
        return 0;
    }

    int result = 0;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result)
    {
        printf("Client: WSAStartup failed with error: %d\n", result);
        return 0;
    }

    ZeroMemory(&hints, sizeof(hints));
    //hints.ai_family = AF_UNSPEC;   <-- ip4 and ip6
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // convert wstr to str
    auto strIP = std::string(serverIP.begin(), serverIP.end());

    // Resolve the server address and port
    result = getaddrinfo(strIP.c_str(), DEFAULT_PORT, &hints, &serverInfo);
    if (result)
    {
        doErrorCleanup("Client: getaddrinfo failed", true, nullptr, 0, NULL);
        return 0;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = serverInfo; ptr != nullptr; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            doErrorCleanup("Client: socket failed", true, nullptr, 0, NULL);
            return 0;
        }

        // Connect to server.  
        result = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR)
        {
            std::cout << "WSAError: " << std::to_string(WSAGetLastError()) << std::endl;
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
        
    }

    freeaddrinfo(serverInfo);

    if (ConnectSocket == INVALID_SOCKET)
    {
        doErrorCleanup("Client: unable to connect to server!", true, NULL, 0, NULL);
        return 0;
    }

    std::wstring fileName;
    std::wstring tmp;
    for (int i = filePath.size() - 1; i > 0; --i)
    {
        tmp = filePath.at(i);
        if (tmp.compare(L"\\") != 0)
            fileName.insert(0, tmp);
            //fileName.append(tmp);
        else
            break;
    }
    // write number of bytes and filepath into request buffer
    unsigned long long byteCount = fileIO->getByteCount();
    std::cout << "Client: file has " << std::to_string(byteCount) << " bytes" << std::endl;

    memcpy(&m_requestBuffer[0], &byteCount, sizeof(unsigned long long));
    wcstombs(&m_requestBuffer[8], fileName.c_str(), 512);
   // memcpy(&m_requestBuffer[8], &fileName, fileName.size());

    // send the request
    result = send(ConnectSocket, m_requestBuffer, m_requestBufferLen, 0);
    if (result == SOCKET_ERROR)
    {
        doErrorCleanup("Client: send failed", true, nullptr, 1, ConnectSocket);
        return 0;
    }

    // cleanup request buffer
    memset(&m_requestBuffer[0], 0, m_requestBufferLen);

    // wait for answer, write it into request buffer
    // receive answer (first byte 1 = yes 0 = no)
    result = recv(ConnectSocket, m_requestBuffer, m_requestBufferLen, 0);

    // received answer
    if (result > 0)
    {
        std::cout << "Received answer from server!" << std::endl;
        // peer accepted
        if (m_requestBuffer[0])
        {
            std::cout << "Server accepted transmission!" << std::endl;

            DWORD bytesToRead = PACKAGE_LENGTH;
            DWORD bytesRed = 0;
            int readResult = 0;

            // read and send loop
            // read 4096 bytes, write into buffer, send
            unsigned int countRead = 0;
            unsigned int countSend = 0;
            do
            {
                // try to read file, if succeeds
                // read up to 4096 bytes and write into buffer
                if (!(readResult = ReadFile(fileIO->getFile(), m_packageBuffer, bytesToRead, &bytesRed, nullptr)))
                {
                    // do manually or change doErrorCleanup arguments
                    doErrorCleanup("Client: readfile failed with error code: " + std::to_string(GetLastError()), false, fileIO, 1, ConnectSocket);
                    return 0;
                }

                // send up to 4096 bytes from buffer to peer
                result = send(ConnectSocket, m_packageBuffer, bytesRed, 0);
                if (result == SOCKET_ERROR)
                {
                    doErrorCleanup("Client: send failed", true, fileIO, 1, ConnectSocket);
                    return 0;
                }

                // ?? cleanup buffer
                memset(&m_requestBuffer[0], 0, m_requestBufferLen);

                countSend += result;
                countRead += bytesRed;
            } while (!(readResult && bytesRed == 0));

            // we are done reading and sending
            std::cout << "Client: red " << std::to_string(countRead) << " bytes\n";
            std::cout << "Client: send " << std::to_string(countSend) << " bytes" << std::endl;

            // wait for server to send success message
            result = recv(ConnectSocket, &m_requestBuffer[0], m_requestBufferLen, 0);
            if (result > 0)
            {
                std::cout << "Received answer from server!\n";
                std::cout << "Transmission successfully completed!" << std::endl;
            }
            // TODO: server doesnt send success message
            else
            {

            }

            doCleanup(fileIO, 1, ConnectSocket);

            return 1;
        }
        // peer declined
        else
        {
            std::cout << "Peer declined transmission!" << std::endl;
            doCleanup(fileIO, 1, ConnectSocket);

            return 1;
        }
    }
    // connection closed
    else if (result == 0)
    {
        std::cout << "Connection closed!" << std::endl;
        doCleanup(nullptr, 1, ConnectSocket);
    }
    // error
    else
        doErrorCleanup("Client: recv failed", true, nullptr, 1, ConnectSocket);


    return 0;
}



int Client::run2(std::string const& serverIP, std::string filePath)
{
    // init FileIO
    FileIO* fileIO = new FileIO(filePath, 'r');

    // check if given file exists
    if (!fileIO->doesFileExist())
    {
        std::cout << "File doesnt exist" << std::endl;
        return 0;
    }

    int result = 0;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result)
    {
        printf("Client: WSAStartup failed with error: %d\n", result);
        return 0;
    }

    ZeroMemory(&hints, sizeof(hints));
    //hints.ai_family = AF_UNSPEC;   <-- ip4 and ip6
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // convert wstr to str
    auto strIP = std::string(serverIP.begin(), serverIP.end());

    // Resolve the server address and port
    result = getaddrinfo(strIP.c_str(), DEFAULT_PORT, &hints, &serverInfo);
    if (result)
    {
        doErrorCleanup("Client: getaddrinfo failed", true, nullptr, 0, NULL);
        return 0;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = serverInfo; ptr != nullptr; ptr = ptr->ai_next)
    {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            doErrorCleanup("Client: socket failed", true, nullptr, 0, NULL);
            return 0;
        }

        // Connect to server.  
        result = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (result == SOCKET_ERROR)
        {
            std::cout << "WSAError: " << std::to_string(WSAGetLastError()) << std::endl;
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;

    }

    freeaddrinfo(serverInfo);

    if (ConnectSocket == INVALID_SOCKET)
    {
        doErrorCleanup("Client: unable to connect to server!", true, NULL, 0, NULL);
        return 0;
    }

    std::wstring fileName;
    std::wstring tmp;
    for (int i = filePath.size() - 1; i > 0; --i)
    {
        tmp = filePath.at(i);
        if (tmp.compare(L"\\") != 0)
            fileName.insert(0, tmp);
        //fileName.append(tmp);
        else
            break;
    }
    // write number of bytes and filepath into request buffer
    unsigned long long byteCount = fileIO->getByteCount();
    std::cout << "Client: file has " << std::to_string(byteCount) << " bytes" << std::endl;

    memcpy(&m_requestBuffer[0], &byteCount, sizeof(unsigned long long));
    wcstombs(&m_requestBuffer[8], fileName.c_str(), 512);
    // memcpy(&m_requestBuffer[8], &fileName, fileName.size());

     // send the request
    result = send(ConnectSocket, m_requestBuffer, m_requestBufferLen, 0);
    if (result == SOCKET_ERROR)
    {
        doErrorCleanup("Client: send failed", true, nullptr, 1, ConnectSocket);
        return 0;
    }

    // cleanup request buffer
    memset(&m_requestBuffer[0], 0, m_requestBufferLen);

    // wait for answer, write it into request buffer
    // receive answer (first byte 1 = yes 0 = no)
    result = recv(ConnectSocket, m_requestBuffer, m_requestBufferLen, 0);

    // received answer
    if (result > 0)
    {
        std::cout << "Received answer from server!" << std::endl;
        // peer accepted
        if (m_requestBuffer[0])
        {
            std::cout << "Server accepted transmission!" << std::endl;

            DWORD bytesToRead = PACKAGE_LENGTH;
            DWORD bytesRed = 0;
            int readResult = 0;

            // read and send loop
            // read 4096 bytes, write into buffer, send
            unsigned int countRead = 0;
            unsigned int countSend = 0;
            do
            {
                // try to read file, if succeeds
                // read up to 4096 bytes and write into buffer
                if (!(readResult = ReadFile(fileIO->getFile(), m_packageBuffer, bytesToRead, &bytesRed, nullptr)))
                {
                    // do manually or change doErrorCleanup arguments
                    doErrorCleanup("Client: readfile failed with error code: " + std::to_string(GetLastError()), false, fileIO, 1, ConnectSocket);
                    return 0;
                }

                // send up to 4096 bytes from buffer to peer
                result = send(ConnectSocket, m_packageBuffer, bytesRed, 0);
                if (result == SOCKET_ERROR)
                {
                    doErrorCleanup("Client: send failed", true, fileIO, 1, ConnectSocket);
                    return 0;
                }

                // ?? cleanup buffer
                memset(&m_requestBuffer[0], 0, m_requestBufferLen);

                countSend += result;
                countRead += bytesRed;
            } while (!(readResult && bytesRed == 0));

            // we are done reading and sending
            std::cout << "Client: red " << std::to_string(countRead) << " bytes\n";
            std::cout << "Client: send " << std::to_string(countSend) << " bytes" << std::endl;

            // wait for server to send success message
            result = recv(ConnectSocket, &m_requestBuffer[0], m_requestBufferLen, 0);
            if (result > 0)
            {
                std::cout << "Received answer from server!\n";
                std::cout << "Transmission successfully completed!" << std::endl;
            }
            // TODO: server doesnt send success message
            else
            {

            }

            doCleanup(fileIO, 1, ConnectSocket);

            return 1;
        }
        // peer declined
        else
        {
            std::cout << "Peer declined transmission!" << std::endl;
            doCleanup(fileIO, 1, ConnectSocket);

            return 1;
        }
    }
    // connection closed
    else if (result == 0)
    {
        std::cout << "Connection closed!" << std::endl;
        doCleanup(nullptr, 1, ConnectSocket);
    }
    // error
    else
        doErrorCleanup("Client: recv failed", true, nullptr, 1, ConnectSocket);


    return 0;
}
