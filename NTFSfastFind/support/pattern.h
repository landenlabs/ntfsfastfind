// ------------------------------------------------------------------------------------------------
// Simple pattern matching class.
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


#pragma once

#include <windows.h>
#include <vector>
#include <ctype.h>

class Pattern
{
public:
    /// Compare simple pattern against string.
    /// Patterns supported:
    ///          ?        ; any single character
    ///          *        ; zero or more characters
    static bool CompareCase(const wchar_t* pattern, const wchar_t* str)
    {
        // ToDo - make case comparison an argument to Compare.
        return Compare(pattern, 0, str, 0);
    }

    /// Compare simple pattern against string.
    /// Patterns supported:
    ///          ?        ; any single character
    ///          *        ; zero or more characters
    static bool CompareNoCase(const wchar_t* pattern, const wchar_t* str)
    {
        // ToDo - make case comparison an argument to Compare.
        return Compare(pattern, 0, str, 0);
    }

    // Text comparison functions.
    static bool YCaseChrCmp(wchar_t c1, wchar_t c2) { return c1 == c2; }
    static bool NCaseChrCmp(wchar_t c1, wchar_t c2) { return tolower(c1) == tolower(c2); }

private:
    static bool (*ChrCmp)(wchar_t c1, wchar_t c2);
    static bool Compare(const wchar_t* wildStr, int wildOff, const wchar_t* rawStr, int rawOff);
};

