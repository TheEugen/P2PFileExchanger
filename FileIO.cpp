#include "FileIO.h"


FileIO::~FileIO()
{
	if(m_file)
	{
		CloseHandle(m_file);
		m_file = nullptr;
	}
}

FileIO::FileIO(std::string const& filePath, char mode)
{
	m_filePath2 = filePath;

	DWORD dwAccess;
	DWORD dwCreate;

	if (mode == 'r')
	{
		dwAccess = GENERIC_READ;
		dwCreate = OPEN_EXISTING;
	}
	else if (mode == 'w')
	{
		dwAccess = GENERIC_WRITE;
		dwCreate = CREATE_NEW;
	}
	else
		throw std::runtime_error(std::string("FileIO constructor: invalid mode parameter"));


	m_file = CreateFileA(m_filePath2.c_str(), dwAccess, 0, NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_file == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error(std::string("FileIO constructor: returned invalid file handle"));
	}

	// low dword 0 high dword 1
	DWORD bytes[2] = { 0 };
	bytes[0] = GetFileSize(m_file, &bytes[1]);

	// file > 2^32 bytes
	if (bytes[1])
		memcpy(&m_byteCount, &bytes[0], sizeof(unsigned long long));
	// file <= 2^32 bytes
	else
		m_byteCount = bytes[0];

}

FileIO::FileIO(std::wstring const& filePath, char mode)
{
	m_filePath = filePath;

	DWORD dwAccess;
	DWORD dwCreate;

	if (mode == 'r')
	{
		dwAccess = GENERIC_READ;
		dwCreate = OPEN_EXISTING;
	}
	else if (mode == 'w')
	{
		dwAccess = GENERIC_WRITE;
		dwCreate = CREATE_NEW;
	}
	else
		throw std::runtime_error(std::string("FileIO constructor: invalid mode parameter"));


	m_file = CreateFileW(m_filePath.c_str(), dwAccess, 0, nullptr, dwCreate, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_file == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error(std::string("FileIO constructor: returned invalid file handle"));
	}

	// low dword 0 high dword 1
	DWORD bytes[2] = { 0 };
	bytes[0] = GetFileSize(m_file, &bytes[1]);

	// file > 2^32 bytes
	if (bytes[1])
		memcpy(&m_byteCount, &bytes[0], sizeof(unsigned long long));
	// file <= 2^32 bytes
	else
		m_byteCount = bytes[0];

}

bool FileIO::doesFileExist() const
{
	DWORD fileAttributes = GetFileAttributesW(m_filePath.c_str());

	// check if filepath is a directory or something broken
	return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

HANDLE FileIO::getFile() const
{
	return m_file;
}

unsigned long long FileIO::getByteCount() const
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
