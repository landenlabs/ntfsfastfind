// ------------------------------------------------------------------------------------------------
// FileSystem Time class used to manipulate time.
//
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


#include "FsTime.h"

#include <iostream>
#include <iomanip>

//-----------------------------------------------------------------------------
FILETIME FsTime::UnixTimeToFileTime(time_t unixTime)
{
    // UnixTime seconds since midnight January 1, 1970 UTC 
    // FILETIME 100-nanosecond intervals since January 1, 1601 UTC 

    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(unixTime, 10000000) + 116444736000000000;
    FILETIME fileTime;
    fileTime.dwLowDateTime = (DWORD)ll;
    fileTime.dwHighDateTime = ll >> 32;
    return fileTime;
}

//-----------------------------------------------------------------------------
FILETIME FsTime::SecondsToFileTime(time_t unixTime)
{
    // UnixTime seconds since midnight January 1, 1970 UTC 
    // FILETIME 100-nanosecond intervals since January 1, 1601 UTC 

    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(unixTime, 10000000);
    FILETIME fileTime;
    fileTime.dwLowDateTime = (DWORD)ll;
    fileTime.dwHighDateTime = ll >> 32;
    return fileTime;
}

// ---------------------------------------------------------------------------
std::wostream& operator<<(std::wostream& out, const FILETIME& utcFT)
{
    FILETIME   ltzFT;
    SYSTEMTIME sysTime;

    FileTimeToLocalFileTime(&utcFT, &ltzFT);    // convert UTC to local Timezone
    FileTimeToSystemTime(&ltzFT, &sysTime);
 
    wchar_t szLocalDate[255], szLocalTime[255];
    szLocalDate[0] = szLocalTime[0] = '\0';

    // GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_SHORTDATE, &sysTime, NULL, szLocalDate, sizeof(szLocalDate) );
    GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &sysTime, L"MM'/'dd'/'yyyy", szLocalDate, ARRAYSIZE(szLocalDate) );
    GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_NOSECONDS, &sysTime, NULL, szLocalTime, ARRAYSIZE(szLocalTime) );

    out << std::setw(10) << szLocalDate << ' ' << std::setw(9) << szLocalTime;

    return out;
}

