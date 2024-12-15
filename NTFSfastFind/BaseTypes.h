// ------------------------------------------------------------------------------------------------
// Base types used by NTFSfastFind project.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
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

#include "Hnd.h"
#include "Block.h"
#include "SharePtr.h"

#include <vector>

class Buffer : public std::vector<BYTE>
{
public:
    // data() is part of new STL available in VS2010.
    // Emulate with Data() method.
    // Return inernal pointer to beginning of active region or reserved memory.
    BYTE* Data() 
    {  return data();  }

    // Return subregion of buffer. 
    // Note - this is expensive, it creates a copy of the region.
    Buffer Region(size_t off, size_t len)
    {
        if (off + len > size())
            throw off;

        Buffer region;
        region.resize(len);
        memcpy(&region[0], Data() + off, len);
        return region;
    }
};

