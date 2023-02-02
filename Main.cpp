/*

1. startup, enter run loop, display HowToUse information and own IP

-> one peer wants to send a file via "C:\Users\xxx\test.txt 106.85.104.85"
2. wait for peer to accept
3. try to open file, catch any errors
4. read filename, write filename into buffer, send buffer
5. read file byte by byte, write character into buffer, send buffer
6. when done, display success message and return to run loop

-> one peer receives a send request
2. accept yes or no

-> when yes
3. receive filename, open file with it, catch any errors
4. read buffer, write buffer into file
5. when done, display success message and return to run loop

-> when no
3. return to run loop

*/

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

#include "Server.h"
#include "Client.h"

int main()
{
	// display welcome message and how to use information
	std::cout << "#####################################################################" << std::endl;
	std::cout << "#####################################################################" << std::endl;
	std::cout << "#####################################################################" << std::endl;
	std::cout << "######		Peer-2-Peer File Exchanger		 ######" << std::endl;
	std::cout << "#####################################################################" << std::endl;
	std::cout << "#####################################################################" << std::endl;
	std::cout << "#####################################################################" << std::endl;
	std::cout << "\nTo send a file to a peer, type send <Path-To-File> <IP-Adress>" << std::endl;
	std::cout << "If the filepath contains a whitespace use "" around it\n\n";

	// debug
	std::string debugStr;
	std::getline(std::cin, debugStr);

	if (!debugStr.compare("s"))
	{
		// debug
		printf("Chosen server\n");

		// init Server object
		Server* server = new Server();
		// TODO: maybe move to startServerThread
		server->init();

		// init server thread
		DWORD serverThreadID;
		HANDLE serverThread = CreateThread(NULL, 0, Server::startServerThread, server, 0, &serverThreadID);
		if (!serverThread)
		{
			printf("Starting server thread failed!");
			return 0;
		}
	}

	std::string userInput;
	// main loop
	do
	{
		// get user input
		std::getline(std::cin, userInput);

		// filter user input
		// send filepath ip
		// 4c + 1c + 256c + 1c + 15c = max 277 characters
		// something evil is happening
		if (userInput.size() > 277)
			continue;

		// process user input into tokens
		std::istringstream iss(userInput);
		std::vector<std::string> tokens;
		std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(tokens));

		// validate token count = 3
		if (tokens.size() != 3)
			continue;

		if (!tokens[0].compare("send"))
		{
			/*
			// init client object and argument structure for thread calling
			Client* client = new Client();
			PCLIENTTHREADARGUMENTS args;
			args->client = client;
			args->filePath = tokens[1];
			args->serverIP = tokens[2];

			// init client thread
			DWORD clientThreadID;
			HANDLE clientThread = CreateThread(NULL, 0, Client::startClientThread, args, 0, &clientThreadID);
			if (!clientThread)
			{
				printf("Starting client thread failed!");
				return 0;
			}
			*/

			Client* client = new Client();
			client->run(tokens[2], tokens[1]);
		}
		else
			continue;

		// compare will return 0 if userinput equals exit
	} while (userInput.compare("exit"));
	
	return 1;
}