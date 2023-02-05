#include "client.h"


int Client::run(std::string serverIP, std::string filePath)
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

    // Resolve the server address and port
    result = getaddrinfo(serverIP.c_str(), DEFAULT_PORT, &hints, &serverInfo);
    if (result)
    {
        //printf("getaddrinfo failed with error: %d\n", result);
        //WSACleanup();
        doErrorCleanup("Client: getaddrinfo failed", true, NULL, 0, NULL);
        return 0;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = serverInfo; ptr != NULL; ptr = ptr->ai_next)
    {

    
        
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    //ConnectSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            //printf("socket failed with error: %ld\n", WSAGetLastError());
            //WSACleanup();
            doErrorCleanup("Client: socket failed", true, NULL, 0, NULL);
            return 0;
        }

        // bind outgoing and receiving on port 27015
        struct sockaddr_in sendAddr;
        ZeroMemory(&sendAddr, sizeof(struct sockaddr_in));
        sendAddr.sin_family = AF_INET;
        sendAddr.sin_port = htons(27015);
        sendAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        result = bind(ConnectSocket, (SOCKADDR*)&sendAddr, sizeof(sendAddr));
        if (result == SOCKET_ERROR)
        {
            std::cout << "WSAError: " << std::to_string(WSAGetLastError()) << std::endl;
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            return 0;
        }
        // set timeout to 30 seconds
        //int timeout = 30 * 1000;
        //setsockopt(ConnectSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(int));

        // Connect to server.  
        result = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        //result = connect(ConnectSocket, serverInfo->ai_addr, (int)serverInfo->ai_addrlen);
        if (result == SOCKET_ERROR)
        {
            std::cout << "WSAError: " << std::to_string(WSAGetLastError()) << std::endl;
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            //return 0;
            continue;
        }
        break;
        
    }

    freeaddrinfo(serverInfo);

    if (ConnectSocket == INVALID_SOCKET)
    {
        //printf("Unable to connect to server!\n");
        //WSACleanup();
        doErrorCleanup("Client: unable to connect to server!", true, NULL, 0, NULL);
        return 0;
    }

    // write number of bytes and filepath into request buffer
    unsigned long long byteCount = fileIO->getByteCount();
    std::cout << "Client: file has " << std::to_string(byteCount) << " bytes" << std::endl;
    memcpy(&m_requestBuffer[0], &byteCount, sizeof(unsigned long long));
    memcpy(&m_requestBuffer[8], &filePath, filePath.size());

    // send the request
    result = send(ConnectSocket, m_requestBuffer, m_requestBufferLen, 0);
    if (result == SOCKET_ERROR)
    {
        //printf("send failed with error: %d\n", WSAGetLastError());
        //closesocket(ConnectSocket);
        //WSACleanup();

        doErrorCleanup("Client: send failed", true, NULL, 1, ConnectSocket);
        return 0;
    }

    // cleanup request buffer
    memset(&m_requestBuffer[0], 0, m_requestBufferLen);

    // wait for answer, write it into request buffer
    // receive answer (first byte 1 = yes 0 = no)
    // recv should be blocking
    result = recv(ConnectSocket, m_requestBuffer, m_requestBufferLen, 0);

    // received answer
    if (result > 0)
    {
        printf("Client: received answer!\n");
        // peer accepted
        if (m_requestBuffer[0])
        {
            printf("Client: received accept!\n");
            // open file
            //HANDLE file = fileIO->getFile();
            //file = (HANDLE)OpenFile(filePath.c_str(), (LPOFSTRUCT)fileIO->getFileInfoPtr(), OF_READ);

            DWORD bytesToRead = PACKAGE_LENGTH;
            DWORD bytesRed = 0;
            int readResult = 0;

            printf("Client: readfile attempt!\n");

            // read and send loop
            // read 4096 bytes, write into buffer, send
            int tmp = 0;
            int tmp2 = 0;
            do
            {
                // try to read file, if succeeds
                // read up to 4096 bytes and write into buffer
                //if (!(readResult = ReadFile(file, m_packageBuffer, bytesToRead, &bytesRed, NULL)))

                if (!(readResult = ReadFile(fileIO->getFile(), m_packageBuffer, bytesToRead, &bytesRed, NULL)))
                {
                    // readfile failed
                    //printf("readfile failed with error: %d\n", GetLastError());
                    //closesocket(ConnectSocket);
                    //WSACleanup();

                    // do manually or change doErrorCleanup arguments
                    doErrorCleanup("Client: readfile failed with error code: " + std::to_string(GetLastError()), false, fileIO, 1, ConnectSocket);
                    return 0;
                }

                // send up to 4096 bytes from buffer to peer
                result = send(ConnectSocket, m_packageBuffer, bytesRed, 0);
                if (result == SOCKET_ERROR)
                {
                    //printf("send failed with error: %d\n", WSAGetLastError());
                    //closesocket(ConnectSocket);
                    //WSACleanup();

                    doErrorCleanup("Client: send failed", true, fileIO, 1, ConnectSocket);
                    return 0;
                }

                // ?? cleanup buffer
                memset(&m_requestBuffer[0], 0, m_requestBufferLen);

                tmp2 += result;
                tmp += bytesRed;

            } while (tmp < fileIO->getByteCount());
            //} while (bytesRed != 0);
            //} while (!(readResult && bytesRed == 0));

            std::cout << "Client: red " << std::to_string(tmp) << " bytes" << std::endl;
            std::cout << "Client: send " << std::to_string(tmp2) << " bytes" << std::endl;

            // we are done reading and sending
            
            
            // TODO: wait until server sends answer that he's done
            result = recv(ConnectSocket, &m_requestBuffer[0], m_requestBufferLen, 0);
            if (result > 0)
            {
                std::cout << "\n Transmission successfully completed!" << std::endl;
            }
            else
            // WSASendDisconnect WSARecvDisconnect
            //doCleanup(fileIO, 1, ConnectSocket);

            return 1;
        }
        // peer declined
        else
        {
            std::cout << "\nPeer declined transmission!" << std::endl;
            //closesocket(ConnectSocket);
            //WSACleanup();

            doCleanup(fileIO, 1, ConnectSocket);

            return 1;
        }
    }
    // connection closed
    else if (result == 0)
    {
        printf("Connection closed\n");
        doCleanup(NULL, 1, ConnectSocket);
    }
    // error
    else
    {
        //printf("recv failed with error: %d\n", WSAGetLastError());
        doErrorCleanup("Client: recv failed", true, NULL, 1, ConnectSocket);
    }

    //closesocket(ConnectSocket);
    //WSACleanup();

    return 0;
}


bool Client::shutdownClient()
{
    int result;

    // shutdown the connection since no more data will be sent
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR)
    {
        //printf("shutdown failed with error: %d\n", WSAGetLastError());
        //closesocket(ConnectSocket);
        //WSACleanup();
        doErrorCleanup("shutdown failed", true, NULL, 1, ConnectSocket);
        return false;
    }

    return true;
}
