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

#include "Pattern.h"

// Initialize static members
//
bool (*Pattern::ChrCmp)(wchar_t c1, wchar_t c2) = Pattern::NCaseChrCmp;
static wchar_t sDirChr = L'\\';

//-----------------------------------------------------------------------------
// Simple wildcard matcher, used by directory file scanner.
// Patterns supported:
//          ?        ; any single character
//          *        ; zero or more characters

bool Pattern::Compare(const wchar_t* wildStr, int wildOff, const wchar_t* rawStr, int rawOff)
{
    const wchar_t EOS = L'\0';
    while (wildStr[wildOff])
    {
        if (rawStr[rawOff] == EOS)
            return (wildStr[wildOff] == L'*');

        if (wildStr[wildOff] == L'*')
        {
            if (wildStr[wildOff + 1] == EOS)
                return true;

            do
            {
                // Find match with char after '*'
                while (rawStr[rawOff] && 
                    !Pattern::ChrCmp(rawStr[rawOff], wildStr[wildOff+1]))
                    rawOff++;
                if (rawStr[rawOff] &&
                    Compare(wildStr, wildOff + 1, rawStr, rawOff))
                        return true;
                if (rawStr[rawOff])
                    ++rawOff;
            } while (rawStr[rawOff]);

            if (rawStr[rawOff] == EOS)
                return (wildStr[wildOff+1] == L'*');
        }
        else if (wildStr[wildOff] == L'?')
        {
            if (rawStr[rawOff] == EOS)
                return false;
            rawOff++;
        }
        else
        {
            if (!Pattern::ChrCmp(rawStr[rawOff], wildStr[wildOff]))
                return false;
            if (wildStr[wildOff] == EOS)
                return true;
            ++rawOff;
        }

        ++wildOff;
    }

    return (wildStr[wildOff] == rawStr[rawOff]);
}
