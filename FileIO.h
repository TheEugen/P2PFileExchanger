#pragma once

#include <windows.h>
#include <iostream>
#include <stdexcept>


class FileIO
{
	HANDLE m_file;
	OFSTRUCT m_fileInfo;
	std::string m_filePath;
	unsigned long long m_byteCount;

public:

	FileIO(std::string filePath, char mode);

	bool doesFileExist();
	HANDLE getFile();
	OFSTRUCT* getFileInfoPtr();
	unsigned long long getByteCount();


};



//void makeFile();
//void readFromFile(std::string filepath);
//void writeToFile(std::string filepath);