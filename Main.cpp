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

/*

(MAYBE) TODOs:

	detect if router has UPnP activated und automatically forward port to server machine
	make getPublicIP call in seperate non-blocking thread


*/

#define UNICODE

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <memory>
#include <iomanip>
#include <locale>

#include "Server.h"
#include "Client.h"
#include "Utils.h"


std::wstring convertInput(std::wstring const& rawInput)
{
	std::string s;
	s.resize(::WideCharToMultiByte(CP_UTF8, 0, rawInput.c_str(), -1, 0, 0, 0, 0));
	::WideCharToMultiByte(CP_UTF8, 0, rawInput.c_str(), -1, const_cast<char*>(s.data()), s.size(), 0, 0);
	std::vector<wchar_t> buf(::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, 0, 0));
	::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buf.data(), buf.size());

	return std::wstring(buf.data());
}

int main()
{
	// display welcome message and how to use information
	std::cout << "#####################################################################\n";
	std::cout << "#####################################################################\n";
	std::cout << "#####################################################################\n";
	std::cout << "######		   Peer-2-Peer File Exchanger		       ######\n";
	std::cout << "#####################################################################\n";
	std::cout << "#####################################################################\n";
	std::cout << "#####################################################################\n";
	std::cout << "\nType s for receiving files, type c for sending files, type quit to close the program" << std::endl;
	
	std::wstring userInput;

	/*
	std::string s;
	std::wstring tStr;
	std::getline(std::wcin, tStr);
	s.resize(::WideCharToMultiByte(CP_UTF8, 0, tStr.c_str(), -1, 0, 0, 0, 0));
	::WideCharToMultiByte(CP_UTF8, 0, tStr.c_str(), -1, const_cast<char*>(s.data()), s.size(), 0, 0);
	std::vector<wchar_t> buf(::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, 0, 0));
	::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buf.data(), buf.size());
	for(wchar_t t : buf)
		std::wcout << t << std::endl;
	auto f = std::wstring(buf.data());
	//std::cout << "s: " << s;
	*/

	// obj ptrs
	auto server = std::make_unique<Server>();
	auto client = std::make_unique<Client>();

	do
	{
		std::cout << "Enter command: ";
		std::getline(std::wcin, userInput);
		if (userInput.size() > 277)
			continue;

		if (userInput.compare(L"s") == 0)
		{	
			std::cout << "\nPublic Router IP: " << getPublicIP() << "\n";
			std::cout << "Radmin VPN IP: " << getLocalIPs() << "\n" << std::endl;
			
			server->run();
		}
		else if (userInput.compare(L"c") == 0)
		{
			std::cout << "\nTo send a file to a peer, type send <Path-To-File> <IP-Address>\n";
			std::cout << "If the filepath contains a whitespace use double quotes around it\n" << std::endl;

			do
			{
				std::cout << "Enter command: ";
				std::getline(std::wcin, userInput);

				// process user input into tokens
				std::wstringstream wss(L"send C:\\ööö.exe localhost");
				std::vector<std::wstring> tokens;
				std::copy(std::istream_iterator<std::wstring, wchar_t>(wss), std::istream_iterator<std::wstring, wchar_t>(), std::back_inserter(tokens));
				std::wcout << tokens.size();
				

				// validate token count = 3
				if (tokens.size() == 3 && tokens[0].compare(L"send") == 0)
					client->run(tokens[2], tokens[1]);

			} while (userInput.compare(L"quit") != 0);
		}
	} while (userInput.compare(L"quit") != 0);
	


	return 1;
}