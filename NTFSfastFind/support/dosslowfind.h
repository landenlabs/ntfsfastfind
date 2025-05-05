// ------------------------------------------------------------------------------------------------
// Slow scan dos/windows file system.
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
#include "ntfsutil.h"

#include <string>
#include <iostream>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Scan file system using Directory API, which is slower than using NTFS but can be
// faster if limited to a subdirectory.
class DirSlowFind {
public:
    DirSlowFind(NtfsUtil::ReportCfg& reportCfg, std::wostream& wout) :
        m_reportCfg(reportCfg),
        m_wout(wout),
        m_error(0) { }

    void ScanFiles();
    void ScanFiles(const wchar_t* path) {
        wcscpy_s(m_path, ARRAYSIZE(m_path), path);
        wchar_t* pSlash = wcsrchr(m_path, '\\');
        if (pSlash)
            *pSlash = '\0';
        ScanFiles();
    }

    NtfsUtil::ReportCfg& m_reportCfg;
    std::wostream& m_wout;
    int                  m_error;

    // Dummy objects so we can call filters in ReportCfg.
    MFTRecord            m_mftRecord;
    NtfsUtil::FileInfo   m_fileInfo;

    wchar_t              m_path[MAX_PATH];
};
