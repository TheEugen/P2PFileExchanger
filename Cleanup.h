#pragma once

#include <WinSock2.h>
#include <cstdarg>
#include <string>
#include <iostream>

#include "FileIO.h"


static void doErrorCleanup(std::string const& errorMsg, bool getLastWSAError, FileIO* fileIO, int socketCount, SOCKET sockets...)
{
	// display error message
	if (getLastWSAError)
		std::cout << "\n" + errorMsg + "with error code: " << WSAGetLastError() << std::endl;
	else
		std::cout << "\n" + errorMsg << std::endl;

	// does this work?
	//doCleanup(fileIO, socketCount, sockets);

	// FileIO
	//if (fileIO)
	//{
	//	CloseHandle(fileIO->getFile());
	//	delete fileIO;
	//}

	// Sockets
	if (socketCount)
	{
		std::va_list args;
		va_start(args, sockets);
		for (int i = 0; i < socketCount; ++i)
			closesocket(va_arg(args, SOCKET));
		va_end(args);
	}

	// WSA
	WSACleanup();
}

static void doCleanup(FileIO* fileIO, int socketCount, SOCKET sockets...)
{
	// FileIO
	if (fileIO)
	{
		CloseHandle(fileIO->getFile());
		delete fileIO;
	}

	// Sockets
	if (socketCount)
	{
		std::va_list args;
		va_start(args, sockets);
		for (int i = 0; i < socketCount; ++i)
			closesocket(va_arg(args, SOCKET));
		va_end(args);
	}

	// WSA
	WSACleanup();
}