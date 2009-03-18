#include "../include/tjcore.h"

#ifdef TJ_OS_WIN
	#include <shellapi.h>
	#include <shlobj.h>
	#include <shobjidl.h>
	#include <shlguid.h>
	#include <shlwapi.h>
	#include <atlbase.h>
#endif

using namespace tj::shared;

String File::GetDirectory(const String& pathToFile) {
	wchar_t* buf = _wcsdup(pathToFile.c_str());
	PathRemoveFileSpec(buf);
	String dir = buf;
	delete[] buf;
	return dir;
}

String File::GetFileName(const String& pathToFile) {
	return String(PathFindFileName(pathToFile.c_str()));
}

String File::GetExtension(const String& pathToFile) {
	return String(PathFindExtension(pathToFile.c_str()));
}

bool File::Exists(const String& st) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	return GetFileAttributes(st.c_str())!=INVALID_FILE_ATTRIBUTES;
}

bool File::Move(const String& from, const String& to, bool silent) {
	ZoneEntry ze(Zones::LocalFileAdministrationZone);
	// If the target directory doesn't exist, create it
	String dir = File::GetDirectory(to);
	SHCreateDirectory(NULL, dir.c_str());
	return MoveFile(from.c_str(), to.c_str())==TRUE;
}

bool File::Copy(const String& from, const String& to, bool silent) {
	ZoneEntry ze(Zones::LocalFileAdministrationZone);

	// TODO: this should try to use IFileOperation first too
	SHFILEOPSTRUCT op;
	op.pFrom = from.c_str();
	op.pTo = to.c_str();
	op.wFunc = FO_COPY;
	op.hwnd = NULL;
	op.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|(silent ? FOF_SILENT : 0);
	op.fAnyOperationsAborted = 0;
	op.hNameMappings = 0;
	op.lpszProgressTitle = 0L;

	return SHFileOperation(&op) == 0;
}

void File::DeleteFiles(const String& dir, const String& pattern) {
	ZoneEntry ze(Zones::LocalFileAdministrationZone);

	if(dir.size()>0) {
		String wc = dir + L"\\" + pattern;
		// Empty the cache
		SHFILEOPSTRUCT shop;
		shop.hwnd = 0L;
		shop.fAnyOperationsAborted = FALSE;
		shop.hNameMappings = NULL;
		shop.lpszProgressTitle = 0L;
		shop.wFunc = FO_DELETE;
		shop.pFrom = wc.c_str();
		shop.pTo = 0;
		shop.fFlags = FOF_SILENT|FOF_NOCONFIRMATION;
		SHFileOperation(&shop);
	}
}

Bytes File::GetFileSize(const String& filePath) {
	ZoneEntry ze(Zones::LocalFileInfoZone);

	HANDLE file = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if(file!=INVALID_HANDLE_VALUE) {
		LARGE_INTEGER li;
		if(GetFileSizeEx(file, &li)==TRUE) {
			return (Bytes)li.QuadPart;
		}
		CloseHandle(file);
	}
	return -1;
}

Bytes File::GetDirectorySize(const String& dirPath) {
	ZoneEntry ze(Zones::LocalFileInfoZone);
	WIN32_FIND_DATA fd;
	String searchPath = dirPath+L"*";
	HANDLE search = FindFirstFile(searchPath.c_str(), &fd);

	if(search==INVALID_HANDLE_VALUE) {
		Log::Write(L"TJShared/File", L"Could not get directory size for '"+dirPath+L"'");
		return 0;
	}

	LARGE_INTEGER totalSize;
	totalSize.QuadPart = 0;

	do {
		if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0 && fd.cFileName[0]!=L'.') {
			// calculate directory size
			String subDirPath = dirPath + fd.cFileName + L"\\";
			Bytes dirSize = GetDirectorySize(subDirPath);
			totalSize.QuadPart += dirSize;
		}
		else {
			LARGE_INTEGER size;
			size.LowPart = fd.nFileSizeLow;
			size.HighPart = fd.nFileSizeHigh;
			totalSize.QuadPart += size.QuadPart;
		}
	} 
	while(FindNextFile(search, &fd));

	FindClose(search);
	return (Bytes)totalSize.QuadPart;
}
