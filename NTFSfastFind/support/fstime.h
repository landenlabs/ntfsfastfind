// ------------------------------------------------------------------------------------------------
// FileSystem Time class used to manipulate time.
//
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

#include <Windows.h>

#include <iostream>
#include <time.h>
#include <assert.h>

class FsTime
{
public:
    FsTime(void) {}

    static FILETIME TodayUTC()
    {
        return UnixTimeToFileTime(time(0));
    }

    /// Convert Unix time to FILETIME.
    ///   UnixTime seconds since midnight January 1, 1970 UTC 
    ///   FILETIME  100-nanosecond intervals since January 1, 1601 UTC 
    static FILETIME UnixTimeToFileTime(time_t t);

    /// Convert seconds to nanoseconds.
    static FILETIME SecondsToFileTime(time_t t);

    class TimeSpan
    {
    public:
        TimeSpan(double seconds) : m_seconds(seconds)
        {  }

        operator FILETIME () const
        {
            return SecondsToFileTime(time_t(m_seconds));   
        }

#if 0
        operator LARGE_INTEGER () const
        {
            return *(LARGE_INTEGER*)& SecondsToFileTime(time_t(m_seconds));  
        }
#endif

        static TimeSpan Days(double days)
        { return TimeSpan(days * sSecondsPerDay); }

        static const size_t sSecondsPerDay = 24 * 60 * 60;

    private:
        double m_seconds;
    };
};

inline LONGLONG Quad(const LARGE_INTEGER& li)
{ return li.QuadPart; }
inline LONGLONG Quad(const FILETIME& li)
{ return ((LARGE_INTEGER*)&li)->QuadPart; }

//
// Math operators
//
inline FILETIME operator+(const FILETIME& left, const FsTime::TimeSpan& right)
{
    assert(sizeof(LARGE_INTEGER) == sizeof(FILETIME));

    LARGE_INTEGER result;
    result.QuadPart= Quad(left) + Quad(right);
    return *((FILETIME*)&result);
}

inline FILETIME operator-(const FILETIME& left, const FsTime::TimeSpan& right)
{
    assert(sizeof(LARGE_INTEGER) == sizeof(FILETIME));

    LARGE_INTEGER result;
    result.QuadPart= Quad(left) - Quad(right);
    return *((FILETIME*)&result);
}

//
//  Pretty print time to stream.
//
extern std::wostream& operator<<(std::wostream& out, const FILETIME& utcFT);
