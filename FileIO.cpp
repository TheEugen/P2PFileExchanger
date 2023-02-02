#include "FileIO.h"

FileIO::FileIO(std::string filePath, char mode)
{
	m_filePath = filePath;

	if (mode == 'r')
		//m_file = (HANDLE)OpenFile(m_filePath.c_str(), &m_fileInfo, OF_READ);
		m_file = CreateFileA(m_filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	else if (mode == 'w')
		//m_file = (HANDLE)OpenFile(m_filePath.c_str(), &m_fileInfo, OF_WRITE);
		m_file = CreateFileA(m_filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	else
		// TODO: throw error
		printf("ll");

	// low dword 0 high dword 1
	DWORD bytes[2] = { 0 };
	bytes[0] = GetFileSize(m_file, &bytes[1]);
	// file > 2^32 bytes
	if (bytes[1])
		// does this work? yes
		memcpy(&m_byteCount, &bytes[0], sizeof(unsigned long long));
	// file <= 2^32 bytes
	else
		m_byteCount = bytes[0];

	/*
	// get number of bytes from file
	DWORD lowByte = 0;
	DWORD highByte = 0;
	lowByte = GetFileSize(m_file, &highByte);

	// file > 2^32 bytes
	if (highByte)
	{

	}
	// file <= 2^32 bytes
	else
		m_byteCount = lowByte;

	*/
}

bool FileIO::doesFileExist()
{
	DWORD fileAttributes = GetFileAttributesA(m_filePath.c_str());

	// check if filepath is a directory or something broken
	return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

HANDLE FileIO::getFile()
{
	return m_file;
}

OFSTRUCT* FileIO::getFileInfoPtr()
{
	return &m_fileInfo;
}

unsigned long long FileIO::getByteCount()
{
	return m_byteCount;
}


/*
void FileIO::readFromFile(std::string filepath)
{
	// open file
	m_file = (HANDLE)OpenFile(filepath.c_str(), (LPOFSTRUCT)&m_fileInfo, OF_READ);
	// if file == HFILE_ERROR
}

HANDLE FileIO::getFile()
{
	return m_file;
}

void FileIO::makeFile()
{
	m_file = CreateFileA(m_filePath.c_str(),			// name of the write
						GENERIC_WRITE,				// open for writing
						0,							// do not share
						NULL,						// default security
						CREATE_NEW,					// create new file only
						FILE_ATTRIBUTE_NORMAL,		// normal file
						NULL);						// no attr. template

	if (m_file == INVALID_HANDLE_VALUE)
	{
		// error handling
	}
}
*/
