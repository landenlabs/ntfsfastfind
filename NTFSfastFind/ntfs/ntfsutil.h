// ------------------------------------------------------------------------------------------------
// Class to read NTFS Master File Table and scan for matching files.
//
// Original code from T.YogaRamanan's Undelete project posted to CodeProject 13-Jan-2005.
// http://www.codeproject.com/KB/files/NTFSUndelete.aspx
// License:  none
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

#pragma once

#include "BaseTypes.h"
#include "FsUtil.h"
#include "MFTRecord.h"
#include "FsFilter.h"

#include <string>
#include <stack>


// ------------------------------------------------------------------------------------------------
// Class to scan the NTFS file system and report files which match 'FsFilter' criteria and 
// present output in 'ReportCfg' format.

class NtfsUtil
{
public:
    NtfsUtil(void);

    struct ReportCfg
    {
        ReportCfg() : 
            queryInfo(false), mftIndex(false)
            , modifyTime(false)
            , diskSize(false), fileSize(false)
            , attribute(false), directory(true), name(true)
            , nameCnt(false), streamCnt(false), showVcn(false), 

            showDetail(false), deleted(false),

            attributes((DWORD)-1),
            slash('\\'), separator(L" "), volume(L""),
            readFilter(new AndFilter()),
            postFilter(new AnyFilter())
        { }

        // Special mode
        bool        queryInfo;         // Display information about MFT
        
        // File scan report columns
        bool        mftIndex;
        bool        modifyTime;        // include modify time
        bool        diskSize;
        bool        fileSize;
        bool        attribute;
        bool        directory;         // include full directory path
        bool        directoryFilter;   // load directory so it can filtered.
        bool        name;

        bool        nameCnt;           // include #names associated with file
        bool        streamCnt;         // include #streams associated with file.
        bool        showVcn;           // show VCN array StartVcn#Vcn...

        bool        showDetail;        // When in 'Q' mode show all MFT record details.
        bool        deleted;           // Must be deleted 

        DWORD       attributes;        // Limit output to items with these attributes

        // Global values.
        wchar_t     slash;
        wchar_t*    separator;
        wchar_t*    volume;

        SharePtr<FsFilter> readFilter; // Filter while reading MFT.
        SharePtr<FsFilter> postFilter; // Filter while presenting results (directory filter).

        std::stack<SharePtr<FsFilter>> stackFilter;
        void PushFilter()
        {
            stackFilter.push(readFilter);
            stackFilter.push(postFilter);
        }

        void PopFilter()
        {
            postFilter = stackFilter.top(); stackFilter.pop();
            readFilter = stackFilter.top(); stackFilter.pop();
        }
    };

    DWORD ScanFiles(
        const wchar_t* volume, 
        const wchar_t* phyDrv,          // path to physcal drive to scan, ex: \\.\Physical0
        const DiskInfo& drive,          // drive/partition info which has offset to MFT
        const ReportCfg& reportCfg,     // Report configuration
        std::wostream& wout,            // Wide Output stream 
        StreamFilter* pStreamFilter,
        DWORD maxFiles);                // -1 for all matching files

    DWORD QueryMFT(
        const wchar_t* volume, 
        const wchar_t* phyDrv,          // path to physcal drive to scan, ex: \\.\Physical0
        const DiskInfo& drive,          // drive/partition info which has offset to MFT
        const ReportCfg& reportCfg,     // Report configuration
        std::wostream& wout,           // Wide Output stream 
        StreamFilter* pStreamFilter);

	struct FileInfo // Collect up file details from scan (MFT or dir)
	{
		LONGLONG	 n64Create;		// Creation time
		LONGLONG	 n64Modify;		// Last Modify time
		LONGLONG	 n64Modfil;		// Last modify of MFT record
		LONGLONG	 n64Access;		// Last Access time
		LONGLONG	 diskSize;		// Actual size of file on disk
		LONGLONG	 fileSize;		// Logical size of file
		DWORD		 dwAttributes;	// File attribute 
        // https://learn.microsoft.com/en-us/windows/win32/fileio/file-attribute-constants
        //   1 = Ronly, 2 = Hidden, 4 = Sys, 16 = dir, 
        // 32 = arch, 64 = dev, 128 = norm, 256 = temp, 
        // 512 = sparse, 1024 = reparse, 
        // 2048 = compress, 4096 = offline

		bool 		 bDeleted;		// True when deleted
        bool         bSparse;       // True if sparse file
		std::wstring filename;      // File name

        DWORD        parentSeq;      // Parent directory seq.
        std::wstring directory;

        DWORD        nameCnt;       // number of names associated with this file (DOS, unicode, etc)
        DWORD        streamCnt;     // number of alternate data streams.

        // Start VCN and #of VCN per fragment.
        typedef std::vector<std::pair<LONGLONG,LONGLONG>> FileOnDiskList;
        FileOnDiskList  m_fileOnDisk;
	};

    // Filter selection, return 0 on success, else last error.
    int GetSelectedFile(DWORD nFileSeq, const SharePtr<FsFilter>& filter, FileInfo& fileInfo, 
        bool dir=false, StreamFilter* pStreamFilter=NULL);

    int GetDirectory(std::wstring& directory, LONGLONG mftIndex);
    int GetDiskPosition(LONGLONG findLCN, LONGLONG& n64LCN, LONGLONG& n64Len); 

#if 0
    // Return details on file entry.
    // Return 0 on success, else last error.
	int GetFileDetail(DWORD nFileSeq, FileInfo& fileInfo);

	int Read_File(DWORD nFileSeq, Buffer& outFileData);
#endif

protected:
	void SetDriveHandle(HANDLE hDrive)
    {
	    m_hDrive = hDrive;
	    m_bInitialized = false;
    }

	void SetStartSector(DWORD dwStartSector, DWORD dwBytesPerSector);
  
    // Return 0 on success, else last error
    // Filter will be used to trim in memory MFT.
	int Initialize(const FsFilter& filter);

    // Load MFT into memory, removing item which fail filter test.
	int LoadMFT(LONGLONG nStartCluster, const FsFilter& filter);

    // Global objects.
    DWORD   m_error;
    bool    m_abort;
    wchar_t m_slash;                // used to build directory path.

    // Physical drive info 
	Hnd	    m_hDrive;
	bool    m_bInitialized;
	DWORD   m_startSector;          // Starting location of MFT
	DWORD   m_bytesPerCluster;      // = bytersPerSector * sectorsPerCluster
	DWORD   m_bytesPerSector;
 
    // MFT info  
	Buffer      m_copyOfMFT;        // In memory copy of MFT, optionally trimmed by filter.
    Buffer      m_oneMFTRecord;     // Helper to walk MFT on record at a time.
	DWORD       m_dwMFTRecordSz;    // MFT record size

    // Copy of MFT header record.
    MFT_FILE_HEADER m_NtfsMFT;

    // Remember on disk lcn and chuck sizes.
    MFTRecord::FileOnDiskList m_fileOnDisk;

    // Remember previous fetched directory to mftIndex mappings.
    typedef std::map<LONGLONG, std::wstring> DirMap;
    DirMap m_dirMap;

    MFTRecord::TypeCnt m_typeCnt;
};

// ------------------------------------------------------------------------------------------------
// Custom filter to count MFT records by inUse state
// ------------------------------------------------------------------------------------------------
class CountFilter : public AndFilter /* FsFilter */
{
public:
    CountFilter()  
    { }

    virtual ~CountFilter()  
    { }

    virtual bool IsMatch(const MFT_STANDARD & attr, const MFT_FILEINFO& fileInfo, const MatchInfo& matchInfo) const;
 
    virtual bool IsValid() const
    { return true; }

    struct CountInfo
    {
        CountInfo() :
            m_fileCnt(0), m_dirCnt(0),
            m_diskSize(0), m_fileSize(0)
        {
            ZeroMemory(m_attrCnt, sizeof(m_attrCnt));
            ZeroMemory(m_nameTypeCnt, sizeof(m_nameTypeCnt));
        }

        void Count(const MFT_FILEINFO& name);

        DWORD       m_attrCnt[15];          // 1=Ronly, 2=hidden, 4=System
        DWORD       m_fileCnt;
        DWORD       m_dirCnt;
        LONGLONG    m_diskSize;
        LONGLONG    m_fileSize;

        // chFileNameType  ePOSIX=0,eUnicode=1,eDOS= 2,eBoth=3
        DWORD       m_nameTypeCnt[7];
    };

    const CountInfo& GetActiveInfo() const
    {  return m_activeInfo; }

    const CountInfo& GetDeletedInfo() const
    {  return m_deletedInfo; }

private:
    mutable CountInfo m_activeInfo;
    mutable CountInfo m_deletedInfo;
};



// ------------------------------------------------------------------------------------------------
// Custom Match filter to test against data stream count.
// ------------------------------------------------------------------------------------------------

extern bool IsCntGreater(size_t inSize, size_t matchSize);
extern bool IsCntEqual(size_t inSize, size_t matchSize);
extern bool IsCntLess(size_t inSize, size_t matchSize);

class StreamCntMatch : public Match
{
public:
    typedef bool (*Test)(size_t inSize, size_t matchSize);

    StreamCntMatch(size_t size, Test test = IsCntGreater, bool matchOn = true) :
        Match(matchOn),
        m_size(size), m_test(test) 
    { }

    virtual bool IsMatch(const MFT_STANDARD &, const MFT_FILEINFO&, const MatchInfo& matchInfo) const
    {
        const MFTRecord* pMFTRecord = (const MFTRecord*)matchInfo.pMFTRecord;
        return m_test(pMFTRecord->m_streamCnt, m_size) == m_matchOn;
    }

    size_t      m_size;
    Test        m_test;
};


// ------------------------------------------------------------------------------------------------
// Custom match filter to match on directory name.
// ------------------------------------------------------------------------------------------------
#include "Pattern.h"

// ------------------------------------------------------------------------------------------------
class MatchDirectory : public Match
{
public:
    typedef bool (*Test)(const std::wstring& inDir, const std::wstring& matchDir);

    MatchDirectory(const std::wstring& dirPat, bool matchOn = true) :
        Match(matchOn),
        m_dirPat(dirPat) 
    { }

    virtual ~MatchDirectory()
    { }

    virtual bool IsMatch(const MFT_STANDARD &, const MFT_FILEINFO&, const MatchInfo& matchInfo) const
    {
        const NtfsUtil::FileInfo* pFileInfo = (const NtfsUtil::FileInfo*)matchInfo.pDirectory;
        return (pFileInfo == NULL) || Pattern::CompareNoCase(m_dirPat.c_str(), pFileInfo->directory.c_str()) == m_matchOn;
    }

    std::wstring m_dirPat;
    Test         m_test;
};