// ------------------------------------------------------------------------------------------------
// Memory Block object used like a vector with bounds checking.
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

#include <exception>
#include <vector>
#include <assert.h>


struct BlockException : public std::exception
{
    const char* what() const throw() { return "Block"; }
};

// Readonly memory region.
class Block
{
public:
    Block(void) :
        m_ptr(0), m_len(0)
    { }

    Block(const void* ptr, size_t len) :
        m_ptr((char*)ptr), m_len(len)
    { }

    template <typename T>
    Block(const std::vector<T>& buffer) :
        m_ptr((const char*)&buffer[0]), m_len(buffer.size())
    {
    }

    void Set(const void* ptr, size_t len)
    { 
        m_ptr = (char*)ptr; 
        m_len = len; 
    }

    void* Copy(void* pDst, size_t offset, size_t len) const
    {
        if (offset + len > m_len)
            throw BlockException();

        memcpy(pDst, m_ptr + offset, len);
        return pDst;
    }

    template <typename T>
    const T& OutRef(size_t offset, size_t len) const
    {
        if (offset + len > m_len)
            throw BlockException();
        return *(T*)(m_ptr + offset);
    }

    template <typename T>
    const T* OutPtr(size_t offset, size_t len = sizeof(T)) const
    {
        if (offset + len > m_len)
            throw BlockException();
        return (T*)(m_ptr + offset);
    }

    const void* OutVPtr(size_t offset) const
    {
        return (m_ptr + offset);
    }

    size_t size() const
    { return m_len; }

private:
    const char* m_ptr;
    size_t      m_len;
};

