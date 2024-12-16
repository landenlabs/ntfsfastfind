// ------------------------------------------------------------------------------------------------
// Handle Locale (Language and Country) specific formatting.
// This was derived from publicly available source code XFormatNumber 
// http://www.codeproject.com/script/Articles/ViewDownloads.aspx?aid=2492
// License:  The Code Project Open License (CPOL) 
//           http://www.codeproject.com/info/cpol10.aspx
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

#include "LocaleFmt.h"

#include <wchar.h>
#include <stdarg.h>
#include <vadefs.h>
#include <assert.h>

#include <string>

// ------------------------------------------------------------------------------------------------
// Derived from publicly available source code XFormatNumber  
// http://www.codeproject.com/script/Articles/ViewDownloads.aspx?aid=2492
// Format a number and insert appropriate commas at each thousand group.
// Does equivalent for non-U.S. Locales
//
//  Ex:
//      wchar_t str[20];    // big enough for final value with commas.
//      LocaleFmt::snprintf(str, ARRAYSIZE(str), "%f", fValue);
//      LocaleFmt::snprintf(str, ARRAYSIZE(str), "%.2f", fValue);
//      LocaleFmt::snprintf(str, ARRAYSIZE(str), "%d", dValue);

wchar_t* LocaleFmt::snprintf(wchar_t* str, unsigned maxChar, wchar_t* fmt, ...)
{
    // Format number into a string.
    va_list args;
    va_start(args, fmt);
    vswprintf_s(str, maxChar, fmt, args);
    va_end(args);

    // Get locale specific format information.
    NUMBERFMT nf = GetNumberFormat();

    // Get number of digits right of decimal point.
    const wchar_t* pDec = wcsstr(str, nf.lpDecimalSep);
    size_t decimalPt = (pDec == NULL) ? 0 : wcslen(pDec) - wcslen(nf.lpDecimalSep);
    nf.NumDigits = (UINT)decimalPt;

    // Copy raw string into temporary buffer.
    std::wstring rawStr = str;
    int nLen = GetNumberFormat(
            LOCALE_USER_DEFAULT, 
            0, 
            rawStr.c_str(), 
            &nf, 
            str,            
            maxChar - 1);

    DWORD err = GetLastError();
    assert(nLen > 0 || err == 0);

    return str;
}

// ------------------------------------------------------------------------------------------------
// Get Locale's numeric format definition.

const NUMBERFMT& LocaleFmt::GetNumberFormat()
{
    static NUMBERFMT sNumberFormat;
    static bool sGotNf = false;
    static wchar_t sDecimalSep[10];
    static wchar_t sThousandsSep[10];

    if (false == sGotNf)
    {
        sGotNf = true;
        ZeroMemory(&sNumberFormat, sizeof(sNumberFormat));
  
        wchar_t buffer[10];
    
        // Get locale decimal separator.
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                sDecimalSep, ARRAYSIZE(sDecimalSep)-1);
                sNumberFormat.lpDecimalSep = sDecimalSep;
  
        // Get locale thousand separator.
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND,
                sThousandsSep, ARRAYSIZE(sThousandsSep)-1);
                sNumberFormat.lpThousandSep = sThousandsSep;
 
        // Get locale leading zero.
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO, buffer, ARRAYSIZE(buffer)-1);
        sNumberFormat.LeadingZero = _wtoi(buffer);
  
        // Get locale group length.
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, buffer, ARRAYSIZE(buffer)-1);
        sNumberFormat.Grouping = _wtoi(buffer);
  
        // Get locale negative number mode.
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGNUMBER, buffer, ARRAYSIZE(buffer)-1);
        sNumberFormat.NegativeOrder = _wtoi(buffer);
    }

    return sNumberFormat;
}
  
    
  