#pragma once

#include <windows.h>
#include <iostream>
#include <stdexcept>


class FileIO
{
	HANDLE m_file;
	OFSTRUCT m_fileInfo;
	std::wstring m_filePath;
	unsigned long long m_byteCount;
	std::string m_filePath2;

public:

	FileIO(std::wstring const& filePath, char mode);
	FileIO(std::string const& filePath, char mode);
	~FileIO();

	bool doesFileExist() const;
	HANDLE getFile() const;
	unsigned long long getByteCount() const;
};



//void makeFile();
//void readFromFile(std::string filepath);
//void writeToFile(std::string filepath);