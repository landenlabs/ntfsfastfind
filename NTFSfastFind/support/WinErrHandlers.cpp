// ------------------------------------------------------------------------------------------------
// Windows error handling
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


#include "WinErrHandlers.h"

#include <Windows.h>
#include "stackwalker.h"

namespace WinErrHandlers {
    // Convert error number to semi-readable string.
    std::wstring ErrorMsg(unsigned int error) {
        wchar_t* lpMsgBuf;

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0,
            NULL);

        std::wstring msg(lpMsgBuf);
        LocalFree(lpMsgBuf);
        return msg;
    }
}


// Specialized stackwalker-output classes
// Console (printf):
class StackWalkerToConsole : public StackWalker {
protected:
    virtual void OnOutput(LPCSTR szText) {
        printf("%s", szText);
    }
};

static wchar_t s_szExceptionLogFileName[_MAX_PATH] = L"\\exceptions.log"; // default
static bool  s_bUnhandledExeptionFilterSet = FALSE;
static long __stdcall CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pExPtrs) {
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

namespace WinErrHandlers {
    void InitUnhandledExceptionFilter() {
        wchar_t szModName[_MAX_PATH];
        if (GetModuleFileName(NULL, szModName, sizeof(szModName) / sizeof(TCHAR)) != 0) {
            wcscpy_s(s_szExceptionLogFileName, szModName);
            wcscat_s(s_szExceptionLogFileName, L".exp.log");
        }
        if (s_bUnhandledExeptionFilterSet == FALSE) {
            // set global exception handler (for handling all unhandled exceptions)
            SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
            s_bUnhandledExeptionFilterSet = TRUE;
        }
    }
}