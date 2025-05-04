// ------------------------------------------------------------------------------------------------
// Slow scan dos/windows file system.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// https://lanenlabs.com
//
// ----- License ----
//
// Copyright (c) 2014 Dennis Lang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------------


#include "dosslowfind.h"

#include "WinErrHandlers.h"
 

// ------------------------------------------------------------------------------------------------
void DirSlowFind::ScanFiles() {
    WIN32_FIND_DATA FileData;    // Data structure describes the file found
    HANDLE     hSearch;          // Search handle returned by FindFirstFile

    m_fileInfo.directory = m_path + 2;

    size_t dirLen = wcslen(m_path);
    wcscpy_s(m_path + dirLen, ARRAYSIZE(m_path) - dirLen, L"\\*");
    dirLen++;

    // Start searching for folder (directories), starting with srcdir directory.

    hSearch = FindFirstFile(m_path, &FileData);
    if (hSearch == INVALID_HANDLE_VALUE) {
        std::wcerr << "Error " << WinErrHandlers::ErrorMsg(GetLastError())
            << "\nFailed to open directory " << m_path << std::endl;
        m_error = GetLastError();
        return;
    }

    bool isMore = true;
    while (isMore) {
        if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            if (FileData.cFileName[0] != L'.' || isalnum(FileData.cFileName[1])) {
                wcscpy_s(m_path + dirLen, ARRAYSIZE(m_path) - dirLen, FileData.cFileName);
                ScanFiles();
                m_path[dirLen - 1] = L'\0';
                m_fileInfo.directory = m_path + 2;
                m_path[dirLen - 1] = L'\\';
                m_path[dirLen] = L'\0';
            }
        } else {
            if (m_reportCfg.postFilter->IsMatch(
                m_mftRecord.m_attrStandard, m_mftRecord.m_attrFilename, &m_fileInfo)) {
                wcscpy_s(m_mftRecord.m_attrFilename.wFilename,
                    ARRAYSIZE(m_mftRecord.m_attrFilename.wFilename),
                    FileData.cFileName);

                m_mftRecord.m_attrFilename.chFileNameLength = (BYTE)wcslen(FileData.cFileName);
                m_mftRecord.m_attrFilename.n64Modify =
                    m_mftRecord.m_attrStandard.n64Modify = *(LONGLONG*)&FileData.ftLastWriteTime;

                LARGE_INTEGER fileSize;
                fileSize.HighPart = FileData.nFileSizeHigh;
                fileSize.LowPart = FileData.nFileSizeLow;
                m_mftRecord.m_attrFilename.n64RealSize = fileSize.QuadPart;

                if (m_reportCfg.readFilter->IsMatch(
                    m_mftRecord.m_attrStandard, m_mftRecord.m_attrFilename, &m_fileInfo)) {
                    m_wout << m_path << "\\" << FileData.cFileName << std::endl;
                }
            }
        }

        isMore = (FindNextFile(hSearch, &FileData) != 0) ? true : false;
    }

    // Close directory before calling callback incase client wants to delete dir.
    FindClose(hSearch);
}