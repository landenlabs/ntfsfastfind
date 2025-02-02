// ------------------------------------------------------------------------------------------------
// FileSystem filter classes used to limit output of file system scan.
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


#pragma once

#include "BaseTypes.h"
#include "NtfsTypes.h"
#include "FsTime.h"

#include <string>
#include <time.h>

// --- Helper to manage optional information
class MatchInfo {
public:
    const void* pMFTRecord; //  MFTRecord* (file and its attributes)
    const void* pDirectory; //  NtfsUtil::FileInfo*  (directory)

    MatchInfo(const void* _pMFTRecord)
        : pMFTRecord(_pMFTRecord)
        , pDirectory(NULL)
    {
    }
    MatchInfo(const void* _pMFTRecord, const void* _pDirectory)
        : pMFTRecord(NULL)
        , pDirectory(_pDirectory)
    {
    }
};



// ------------------------------------------------------------------------------------------------
// Example usage:
//      MultiFilter mFilter;
//
//      mFilter.List().push_back(new MatchName("*.txt", IsNameIcase));
//
//      double days = -2;
//      FILETIME  daysAgo = FsTime::TodayUTC() - FsTime::TimeSpan::Days(days);
//      mFilter.List().push_back(new MatchDate(daysAgo));
// ------------------------------------------------------------------------------------------------
class Match
{
public:
    Match(bool matchOn = true) :
       m_matchOn(matchOn)
    { }

    virtual bool IsMatch(const MFT_STANDARD& attr, const MFT_FILEINFO& fileInfo, const MatchInfo& matchInfo) const = 0;
    bool m_matchOn;
};

//
// Date matching Test filters:
//
extern bool IsDateModifyGreater(const MFT_STANDARD &, const FILETIME& );
extern bool IsDateModifyEqual(const MFT_STANDARD &, const FILETIME& );
extern bool IsDateModifyLess(const MFT_STANDARD &, const FILETIME& );



// ------------------------------------------------------------------------------------------------
class MatchDate : public Match
{
public:
    typedef bool (*Test)(const MFT_STANDARD &, const FILETIME& );
    MatchDate(const FILETIME& fileTime, Test test = IsDateModifyGreater, bool matchOn = true) :
        Match(matchOn),
        m_fileTime(fileTime), m_test(test)
    { }

    virtual bool IsMatch(const MFT_STANDARD& attr, const MFT_FILEINFO& fileInfo, const MatchInfo& matchInfo) const
    {
        return m_test(attr, m_fileTime) == m_matchOn;
    }

    FILETIME m_fileTime;
    Test     m_test;
};

//
// Name matching Test filters:
//

extern bool IsNameIcase(const MFT_FILEINFO&, const std::wstring& name);    // Ignore case
extern bool IsName(const MFT_FILEINFO&, const std::wstring& name);     // currently not working.

// ------------------------------------------------------------------------------------------------
class MatchName : public Match
{
public:
    typedef bool (*Test)(const MFT_FILEINFO&, const std::wstring& name);

    MatchName(const std::wstring& name, Test test = IsNameIcase, bool matchOn = true) :
        Match(matchOn),
        m_name(name), m_test(test)
    { }

    virtual ~MatchName()
    { }

    virtual bool IsMatch(const MFT_STANDARD&, const MFT_FILEINFO& fileInfo, const MatchInfo& matchInfo) const
    {
        return ((fileInfo.chFileNameLength != 0) && m_test(fileInfo, m_name)) == m_matchOn;
    }

    std::wstring m_name;
    Test         m_test;
};



//
// Size matching Test filters:
//

extern bool IsSizeGreater(const MFT_FILEINFO&, LONGLONG size);
extern bool IsSizeEqual(const MFT_FILEINFO&, LONGLONG size);
extern bool IsSizeLess(const MFT_FILEINFO&, LONGLONG size);

// ------------------------------------------------------------------------------------------------
class MatchSize : public Match
{
public:
    typedef bool (*Test)(const MFT_FILEINFO&, LONGLONG size);

    MatchSize(LONGLONG size, Test test = IsSizeGreater, bool matchOn = true) :
        Match(matchOn),
        m_size(size), m_test(test) 
    { }

    virtual bool IsMatch(const MFT_STANDARD&, const MFT_FILEINFO& fileInfo, const MatchInfo& matchInfo) const
    {
        return m_test(fileInfo, m_size) == m_matchOn;
    }

    LONGLONG     m_size;
    Test         m_test;
};


// ------------------------------------------------------------------------------------------------
class FsFilter : public Match
{
public:
    typedef std::vector<SharePtr<Match>> MatchList;

    FsFilter() : Match(true)  { }
    // virtual bool IsMatch(const MFT_STANDARD& attr, const MFT_FILEINFO& fileInfo, const MatchInfo& pData) const = 0;
    virtual bool IsValid() const = 0;

    void SetMatch(const MatchList& matchList)
    {
        m_testList = matchList;
    }

    MatchList& List()
    {
        return m_testList;
    }
    
protected:
    MatchList m_testList;
};

// ------------------------------------------------------------------------------------------------
class StreamFilter
{
public:
    // TODO  - filter and store stream names.
    virtual bool IsMatch(const wchar_t* pwFilename, 
        const wchar_t* pwStreamName, 
        DWORD streamLength) const
    {
        // std::wcout << pwFilename << pwStreamName << " StreamSize:" << streamLength << std::endl;
        return false;   // TODO - add logic
    }
};

// ------------------------------------------------------------------------------------------------
//  Single filter rule
//  Ex:
//      OneFilter oneFilter(new MatchName("*.txt", IsNameIcase));
//      ...user filter
//      FILETIME today = ...
//      oneFilter.SetMatch(new MatchDate(daysAgo, IsDateModifyGreater));
//
class OneFilter : public FsFilter
{
public:
    OneFilter() 
    { }

    OneFilter(SharePtr<Match>& rMatch) : m_rMatch(rMatch) 
    { }

    virtual ~OneFilter()
    { }

    void SetMatch(SharePtr<Match>& rMatch)
    { m_rMatch = rMatch; }

    virtual bool IsMatch(const MFT_STANDARD& attr, const MFT_FILEINFO& fileInfo, const MatchInfo& matchInfo) const
    {
        return m_rMatch->IsMatch(attr, fileInfo, matchInfo);
    }

    virtual bool IsValid() const
    { return !m_rMatch.IsNull();  }

private:
    SharePtr<Match> m_rMatch;

};

// ------------------------------------------------------------------------------------------------
//  Multiple filter rules and all must be true to pass. 
//  Ex:
//      MultiFilter mFilter;
//      mFilter.List().push_back(new MatchName(L"foo"));
//      mFilter.List().push_back(new MatchName(L"*.txt", IsNameIcase, false));  // reverse match
//      mFilter.List().push_back(new MatchDate(Today, IsDateModifyGreater);
//
class AndFilter : public FsFilter
{
public:

    AndFilter() 
    { }

    AndFilter(const MatchList& matchList)
    {
        m_testList = matchList;
    }

    virtual ~AndFilter()
    { }

    virtual bool IsMatch(const MFT_STANDARD& attr, const MFT_FILEINFO& fileInfo, const MatchInfo& matchInfo) const
    {
        for (unsigned mIdx = 0; mIdx < m_testList.size(); mIdx++)
        {
            if (!m_testList[mIdx]->IsMatch(attr, fileInfo, matchInfo))
                return false;
        }
        return true;
    }

    virtual bool IsValid() const
    { return m_testList.size() != 0; }

};

// ------------------------------------------------------------------------------------------------
//  Multiple filter rules and any true to pass.
class AnyFilter : public FsFilter {
public:

    AnyFilter()
    { }

    AnyFilter(const MatchList& matchList)
    {
        m_testList = matchList;
    }

    virtual ~AnyFilter()
    { }

    virtual bool IsMatch(const MFT_STANDARD& attr, const MFT_FILEINFO& fileInfo, const MatchInfo& matchInfo) const
    {
        for (unsigned mIdx = 0; mIdx < m_testList.size(); mIdx++) {
            if (m_testList[mIdx]->IsMatch(attr, fileInfo, matchInfo))
                return true;
        }
        return false;
    }

    virtual bool IsValid() const
    {  return m_testList.size() != 0;  }
};

