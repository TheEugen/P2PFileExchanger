#pragma once


#define MAX_THREADS 2

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

// 8 bytes for number of bytes, 256 bytes filepath
#define REQUEST_LENGTH 264
#define PACKAGE_LENGTH 4096

class Client;

typedef struct ClientThreadArguments
{
	Client* client;
	std::string serverIP;
	std::string filePath;
} CLIENTTHREADARGUMENTS, *PCLIENTTHREADARGUMENTS;

