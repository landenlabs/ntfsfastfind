// ------------------------------------------------------------------------------------------------
// Program to scan NTFS's Master File Table for matching file by name, date or size filters.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// https://landenlabs.com
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
// Web links
// https://www.writeblocked.org/resources/NTFS_CHEAT_SHEETS.pdf
// https://docs.velociraptor.app/docs/forensic/ntfs/
// https://medium.com/search?q=A+Journey+into+NTFS
// ------------------------------------------------------------------------------------------------

#include <iostream>
#include <sys/stat.h>
#include <math.h>

#include "BaseTypes.h"

#include "FsUtil.h"
#include "FsFilter.h"
#include "NtfsUtil.h"

#include "GetOpts.h"

#define _VERSION "v3.1"

char sUsage[] =
    "\n"
    "NTFS Fast File Find " _VERSION " - " __DATE__ "\n\n"
    "By: Dennis Lang\n"
    "https://landenlabs.com/console/ntfsfastfind/ntfsfastfind.html\n"
    "\n\n"
    "Description:\n"
    "   NTFSfastFind searches NTFS Master File Table (MFT) rather then iterating across directories.\n"
    "   NTFSfastFind does not use or maintain an index database\n"
    "   By reading the MFT directly, NTFSfastFind can locate files anywhere on a disk quickly.\n"
    "   Note: Standard directory searching is faster if you know the directory to search.\n"
    "   If you don't know the directory and need to search the entire disk drive, NTFSfastFind is fast.\n"
    "\n"
    "   If you use the -z switch, it will iterate across the directories rather then using MFT.\n"
    "\n"
    "Use:\n"
    "   NTFSfastFind [options] <localNTFSdrivetoSearch>... \n"
    "\n"
    " Filter:\n"
    "   -d <count>                        ; Filter by data stream count  \n"
    "   -f <fileFilter>                   ; Filter by filename, use * or ? patterns \n"
    "   -s <size>                         ; Filter by file size  \n"
    "   -t <relativeModifyDate>           ; Filter by time modified, value is relative days \n"
    "   -z                                ; Force slow style directory search \n"
    "   -v                                ; Verbose (used with -Q ) \n"
    "\n"
    " Report:\n"
    "   -A[=s|h|r|d|f|c]                  ; Include attributes, filter on attributes \n"
    "        s=system, h=hidden, r=readonly, d=directory, f=file, c=compressed\n"
    "   -D                                ; Include directory \n"
    "   -I                                ; Include mft index \n"
    "   -S                                ; Include size \n"
    "   -T                                ; Include time \n"
    "   -V                                ; Include VCN array \n"
    "   -X                                ; Only deleted entries \n"
    "   -#                                ; Include stream and name counts \n"
    "\n"
    " Query Drive status only, no file search\n"
    "   -Q                                ; Query / Display MFT information only (see -v) \n"
    "\n"
    " Examples:\n"
    "    c: d:                  ; List entire c and d drive, display filenames. \n"
    "    -ITSA  d:              ; List entire d drive, display mft index, time, size, attributes, directory. \n"
    "\n"
    "  Filter examples (precede 'f' command letter with ! to invert rule):\n"
    "    -f *.txt d:                 ; Files ending in .txt on d: drive \n"
    "    -f \\*\\foo*\\*.txt d:      ; Files ending in .txt on d: drive in directory starting with foo \n"
    "    -f Map1.* -f Map2.*  c:     ; Files matching two patterns on c drive \n"
    "    -T -S -f *cache -t -0.1  c: ; Files ending in cache, modified less than 0.1 days ago \n"
    "    -!f *.txt d:                ; Files NOT ending in .txt on d: drive \n" 
    "    -t 2.5 -f *.log             ; Modified more than 2.5 days and ending in .log on c drive \n"
    "    -t -0.2 e:                  ; Modified less than 0.2 days ago on e drive \n"
    "    -s 1000 d:                  ; File size greater than 1000 bytes on d drive \n"
    "    -s -1000 d: e:              ; File size less than 1000 bytes on d and e drive \n"
    "    -f F* c: d:                 ; Limit scan to files starting with F on either C or D \n"
    "    -d 1 d:                     ; Files with more than 1 data stream on d: drive \n"
    "\n"
    "    -X -f * c:                  ; All deleted entries on c: drive \n"
    "    -X -T -S -f *cache  c:      ; Delete files ending in cache, show modify time and size \n"
    "    -X  -f *cache -t -1 c:      ; Deleted files modifies less than 1 day ago \n"
    "\n"
    "    -Q c:                       ; Display special NTFS files\n"
    "\n"
    "    -z c:\\windows\\system32\\*.dll   ; Force slow directory search. \n"
    "\n";

// ------------------------------------------------------------------------------------------------
// Convert error number to semi-readable string.
std::wstring ErrorMsg(DWORD error)
{
	wchar_t *lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, 
        NULL);

    std::wstring msg(lpMsgBuf);
	LocalFree(lpMsgBuf);
    return msg;
}


#include "stackwalker.h"

// Specialized stackwalker-output classes
// Console (printf):
class StackWalkerToConsole : public StackWalker
{
protected:
    virtual void OnOutput(LPCSTR szText) { printf("%s", szText); }
};

static wchar_t s_szExceptionLogFileName[_MAX_PATH] = L"\\exceptions.log"; // default
static bool  s_bUnhandledExeptionFilterSet = FALSE;
static long __stdcall CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pExPtrs)
{
    StackWalkerToConsole sw; // output to console
    sw.ShowCallstack(GetCurrentThread(), pExPtrs->ContextRecord);
#if 0
    wchar_t lString[500];
    wsprintf(lString,
        L"*** Unhandled Exception! See console output for more infos!\n"
        L"   ExpCode: 0x%8.8X\n"
        L"   ExpFlags: %d\n"
#if _MSC_VER >= 1900
        L"   ExpAddress: 0x%8.8p\n"
#else
        L"   ExpAddress: 0x%8.8X\n"
#endif
        L"   Please report!",
        pExPtrs->ExceptionRecord->ExceptionCode, pExPtrs->ExceptionRecord->ExceptionFlags,
        pExPtrs->ExceptionRecord->ExceptionAddress);
    FatalAppExit(-1, lString);
#else
    FatalAppExit(-1, NULL);
#endif
    return EXCEPTION_CONTINUE_SEARCH;
}

static void InitUnhandledExceptionFilter()
{
    wchar_t szModName[_MAX_PATH];
    if (GetModuleFileName(NULL, szModName, sizeof(szModName) / sizeof(TCHAR)) != 0)
    {
        wcscpy_s(s_szExceptionLogFileName, szModName);
        wcscat_s(s_szExceptionLogFileName, L".exp.log");
    }
    if (s_bUnhandledExeptionFilterSet == FALSE)
    {
        // set global exception handler (for handling all unhandled exceptions)
        SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
        s_bUnhandledExeptionFilterSet = TRUE;
    }
}

// ------------------------------------------------------------------------------------------------
// see https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file?redirectedfrom=MSDN#win32-device-namespaces
//   Win32 Device Namespace
//      \\.\<device|file>    access device namespace instead of file namespace
//      \\.\C:               access drive C: instead of file namespace
//      \\.\PhyscialDriveX   X is valid integer, allow access to drive bypassing file system. 

int NTFSfastFind(
    const wchar_t* path, 
    NtfsUtil::ReportCfg& reportCfg, 
    std::wostream& wout,
    StreamFilter* pStreamFilter)
{
    wchar_t driveLetter = FsUtil::GetDriveLetter(path);
    DWORD error;
    unsigned phyDrvNum = 0;
    unsigned partitionNum = 0;

    wchar_t volumePath[] = L"\\\\.\\C:";
    volumePath[4] = towupper(driveLetter);
    reportCfg.volume = volumePath+4;
    FsUtil::DiskInfoList diskInfoList;

    InitUnhandledExceptionFilter();

  
    error = FsUtil::GetDriveAndPartitionNumber(volumePath, phyDrvNum, partitionNum);
    if (error != ERROR_SUCCESS)
    {
        std::wcerr << "Error " << ErrorMsg(error).c_str() << std::endl;
        return error;
    }

    wchar_t physicalDrive[]= L"\\\\.\\PhysicalDrive0";
    // ARRAYSIZE includes string terminating null, so backup 2 characters.
    physicalDrive[ARRAYSIZE(physicalDrive)-2] += (char)phyDrvNum;

    /* 
    error = FsUtil::GetLogicalDrives(physicalDrive, diskInfoList, FsUtil::eFsALL);
    if (error != 0)
    {
        std::wcerr << "Error " << ErrorMsg(error).c_str() << std::endl;
        return error;
    }
    */

    int diskNumber;
    LONGLONG offset;
    error = FsUtil::GetNtfsDiskNumber(volumePath, diskNumber, offset);
    if (error != 0)
    {
        std::wcerr << "Error GetNtfsDiskNumber " << ErrorMsg(error).c_str() << std::endl;
        return error;
    }
    error = FsUtil::GetDriveStartSector(volumePath, diskInfoList);
    if (error != 0)
    {
        std::wcerr << "Error GetDriveStartSector " << ErrorMsg(error).c_str() << std::endl;
        return error;
    }

#ifdef _DEBUG
    std::wcerr << physicalDrive
        << " DiskNum=" << diskNumber
        << " PhyDrvNum=" << phyDrvNum
        << " Volume(partition)=" << volumePath
        << " Partition=" << partitionNum
        << " #Partitions=" << diskInfoList.size()
        << std::endl;
#endif

    diskNumber = 0; // DiskList limited to just this drive, so always index at [0]
    if (diskNumber >= diskInfoList.size())
    {
        std::wcerr << "Failed to locate physical drive sector parameters\n";
        return -2;
    }
  
    NtfsUtil ntfsUtil;

    if (reportCfg.queryInfo)
        error = ntfsUtil.QueryMFT(volumePath, physicalDrive, diskInfoList[diskNumber], reportCfg, wout, pStreamFilter);
    else
        error = ntfsUtil.ScanFiles(volumePath, physicalDrive, diskInfoList[diskNumber], reportCfg, wout, pStreamFilter, -1);

    if (error != 0)
    {
        std::wcerr << "Error " << ErrorMsg(error).c_str() << std::endl;
    }
    return error;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Scan file system using Directory API, which is slower than using NTFS but can be
// faster if limited to a subdirectory.
class DirSlowFind
{
public:
    DirSlowFind(NtfsUtil::ReportCfg& reportCfg, std::wostream& wout) :
        m_reportCfg(reportCfg),
        m_wout(wout),
        m_error(0)
    {
    }

    void ScanFiles();
    void ScanFiles(const wchar_t* path)
    {
        wcscpy_s(m_path, ARRAYSIZE(m_path), path);
        wchar_t* pSlash = wcsrchr(m_path, '\\');
        if (pSlash)
            *pSlash = '\0';
        ScanFiles();
    }

    NtfsUtil::ReportCfg& m_reportCfg;
    std::wostream&       m_wout;
    int                  m_error;

    // Dummy objects so we can call filters in ReportCfg.
    MFTRecord            m_mftRecord;
    NtfsUtil::FileInfo   m_fileInfo;

    wchar_t              m_path[MAX_PATH];
};

// ------------------------------------------------------------------------------------------------
void DirSlowFind::ScanFiles()
{
    WIN32_FIND_DATA FileData;    // Data structure describes the file found
    HANDLE     hSearch;          // Search handle returned by FindFirstFile

    m_fileInfo.directory = m_path+2;

    size_t dirLen = wcslen(m_path);
    wcscpy_s(m_path + dirLen, ARRAYSIZE(m_path) - dirLen, L"\\*");
    dirLen++;

    // Start searching for folder (directories), starting with srcdir directory.

    hSearch = FindFirstFile(m_path, &FileData);
    if (hSearch == INVALID_HANDLE_VALUE)
    {
        std::wcerr << "Error " << ErrorMsg(GetLastError()) 
            << "\nFailed to open directory " << m_path << std::endl;
        m_error = GetLastError();
        return;
    }

    bool isMore = true;
    while (isMore)
    {
        if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            if (FileData.cFileName[0] != L'.' || isalnum(FileData.cFileName[1]))
            {
                wcscpy_s(m_path + dirLen, ARRAYSIZE(m_path) - dirLen, FileData.cFileName);
                ScanFiles();
                m_path[dirLen-1] = L'\0';
                m_fileInfo.directory = m_path+2;
                m_path[dirLen-1] = L'\\';
                m_path[dirLen] = L'\0';
            }
        }
        else
        {
            if (m_reportCfg.postFilter->IsMatch(
                    m_mftRecord.m_attrStandard, m_mftRecord.m_attrFilename, MatchInfo(NULL, & m_fileInfo)))
            {
                wcscpy_s(m_mftRecord.m_attrFilename.wFilename, 
                        ARRAYSIZE(m_mftRecord.m_attrFilename.wFilename),
                        FileData.cFileName);

                m_mftRecord.m_attrFilename.chFileNameLength = (BYTE)wcslen(FileData.cFileName);
                m_mftRecord.m_attrFilename.n64Modify =
                    m_mftRecord.m_attrStandard.n64Modify = *(LONGLONG*)&FileData.ftLastWriteTime;

                LARGE_INTEGER fileSize;
                fileSize.HighPart = FileData.nFileSizeHigh;
                fileSize.LowPart = FileData.nFileSizeLow;
                m_mftRecord.m_attrFilename.n64DiskSize = fileSize.QuadPart;
                    
                if (m_reportCfg.readFilter->IsMatch(
                        m_mftRecord.m_attrStandard, m_mftRecord.m_attrFilename, MatchInfo(NULL, & m_fileInfo)))
                {
                    m_wout << m_path << "\\" <<  FileData.cFileName << std::endl;        
                }
            }
        }

        isMore = (FindNextFile(hSearch, &FileData) != 0) ? true : false;
    }

    // Close directory before calling callback incase client wants to delete dir.
    FindClose(hSearch);
}

 
static AnyFilter* pAnyNamefilters;

// ------------------------------------------------------------------------------------------------
void AddFileFilter(const wchar_t* argv, NtfsUtil::ReportCfg& reportCfg, bool matchOn)
{
    if (pAnyNamefilters == NULL) {
        pAnyNamefilters = new AnyFilter();
        reportCfg.readFilter->List().push_back(pAnyNamefilters);
    }

    // Determine if pattern is just name or directory and name. 
    //   directory and name = any slash, ex   dir1\file1.ext1
    //   name only          = no slash
    const wchar_t* pName = wcsrchr(argv, reportCfg.slash);
    if (pName == NULL)
    {
        // name only
        pAnyNamefilters->List().push_back(new MatchName(argv, IsNameIcase, matchOn));
    }
    else
    {
        // directory and name
        if (pName[1] != '\0' && pName[1] != '*')
            pAnyNamefilters->List().push_back(new MatchName(pName + 1, IsNameIcase, matchOn));
        std::wstring dirPat(argv, size_t(pName - argv));
        reportCfg.postFilter->List().push_back(new MatchDirectory(dirPat.c_str(), matchOn));
        reportCfg.directoryFilter = true;
    }
}

// ------------------------------------------------------------------------------------------------
int wmain(int argc, const wchar_t* argv[])
{
    const wchar_t* path = L"c:\\";
    NtfsUtil::ReportCfg reportCfg;
    bool matchOn = true;
    bool doDirIterating = false;
    StreamFilter streamFilter;  // TODO - add members and logic to class

    if (argc == 1)
    {
        std::wcout << sUsage;
        return 0;
    }

    GetOpts<wchar_t> getOpts(argc, argv, L"!#A:DIQSTVXvd:f:s:t:z?");

    while (getOpts.GetOpt())
    {
        switch (getOpts.Opt())
        {
        case '!':   // not
            matchOn = false;
            break;
        case '#':   // number of names and streams
            reportCfg.nameCnt = true;
            reportCfg.streamCnt = true;
            break;
        case 'A':   // attributes
            reportCfg.attribute = !reportCfg.attribute;
            reportCfg.attributes = -1;
            if (getOpts.OptArg() == NULL || *getOpts.OptArg() != '=') {
                std::wcerr << "Missing attribute argument, such as -A= or -A=rshdfc\n";
                return 0;
            } else if (getOpts.OptArg()[1] != '\0')
            {
                reportCfg.attributes = 0;
                for (const wchar_t* optStr = getOpts.OptArg()+1; *optStr != '\0'; optStr++)
                {
                    switch (tolower(*optStr))
                    {
                    case 'r':
                        reportCfg.attributes |= eReadOnly;
                    case 's': 
                        reportCfg.attributes |= eSystem;
                        break;
                    case 'h':   
                        reportCfg.attributes |= eHidden;
                        break;
                    case 'd':
                        reportCfg.attributes |= eDirectory;
                        break;
                    case 'f':   // files
                        reportCfg.attributes = ~eDirectory;
                        break;
                    case 'c':
                        reportCfg.attributes |= eCompressed;
                        break;
                    default:
                        std::wcerr << "Invalid attribute argument:" << getOpts.OptArg() << std::endl;
                        break;
                    }
                }
            }
            break;
        case 'D':   // directory path
            reportCfg.directory = !reportCfg.directory;
            break;
        case 'I':   // mft index
            reportCfg.mftIndex = !reportCfg.mftIndex;
            break;
        case 'Q':   // query info
            reportCfg.queryInfo = true;
            reportCfg.attributes = eSystem;
            break;
        case 'S':   // size
            reportCfg.diskSize = true;
            reportCfg.fileSize = true;
            break;
        case 'T':   // modify time
            reportCfg.modifyTime = !reportCfg.modifyTime;
            break;
        case 'V':   // show VCN array
            reportCfg.showVcn = true;
            break;
        case 'X':   // deleted 
            reportCfg.deleted = true;
            break;
        case 'v':   // verbose 
            reportCfg.showDetail = true;
            break;

        case 'd':   // data stream count
            {
                wchar_t* endPtr;
                long fileSize = wcstol(getOpts.OptArg(), &endPtr, 10);
                if (endPtr == getOpts.OptArg())
                {
                    std::wcerr << "Invalid Size argument:" << getOpts.OptArg() << std::endl;
                    return -1;
                }
                reportCfg.readFilter->List().push_back(new StreamCntMatch(labs(fileSize), fileSize > 0 ? IsCntGreater : IsCntLess, matchOn));
            }
            matchOn = true;
            break;

        case 'f':
            AddFileFilter(getOpts.OptArg(), reportCfg, matchOn);
            matchOn = true;
            break;

        case 's':   // size
            {
                wchar_t* endPtr;
                long fileSize = wcstol(getOpts.OptArg(), &endPtr, 10);
                if (endPtr == getOpts.OptArg())
                {
                    std::wcerr << "Invalid Size argument:" << getOpts.OptArg() << std::endl;
                    return -1;
                }
                reportCfg.readFilter->List().push_back(new MatchSize(labs(fileSize), fileSize > 0 ? IsSizeGreater : IsSizeLess, matchOn));
            }
            matchOn = true;
            break;

        case 't':
            {
                wchar_t* endPtr;
                double days = wcstod(getOpts.OptArg(), &endPtr);
                if (endPtr == getOpts.OptArg())
                {
                    std::wcerr << "Invalid Modify Days argument, expect floating point number\n";
                    return -1;
                }
                FILETIME  daysAgo = FsTime::TodayUTC() - FsTime::TimeSpan::Days(fabs(days));
                reportCfg.readFilter->List().push_back(new MatchDate(daysAgo, days < 0 ? IsDateModifyGreater : IsDateModifyLess, matchOn));
                // std::wcout << "Today      =" << FsTime::TodayUTC() << std::endl;
                // std::wcout << "Filter date=" << daysAgo << std::endl;
            }
            matchOn = true;
            break;

        case 'z':
            doDirIterating = true;
            break;

        default:
        case '?':
            std::wcout << sUsage;
            return 0;
        }
    }

    int error = 0;
    if (getOpts.NextIdx() < argc)
    {
        for (int optIdx = getOpts.NextIdx(); optIdx < argc; optIdx++)
        {
            reportCfg.PushFilter();
            reportCfg.directoryFilter = 
                    !reportCfg.postFilter.IsNull() && reportCfg.postFilter->List().size() != 0;

            const wchar_t* arg = argv[optIdx];
            if (wcslen(arg) > 3 && arg[1] == ':')
            {
                if (arg[2] == '\\')
                    AddFileFilter(arg + 3, reportCfg, true);
                else  
                    AddFileFilter(arg + 2, reportCfg, true);
            }

            if (doDirIterating)
            {
                DirSlowFind dirSlowFind(reportCfg, std::wcout);
                dirSlowFind.ScanFiles(argv[optIdx]);
                error |= dirSlowFind.m_error;
            }
            else
            {
                // ToDo - if multi files on same MFT, reuse previous scan !
                error |= NTFSfastFind(argv[optIdx], reportCfg, std::wcout, &streamFilter);
            }

            reportCfg.PopFilter();
        }
    }
    else
    {
        error = NTFSfastFind(path, reportCfg, std::wcout, &streamFilter);
    }

	return error;
}

