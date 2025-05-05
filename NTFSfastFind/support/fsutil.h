// ------------------------------------------------------------------------------------------------
// FileSystem utility class used to access file system information.
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


#pragma pack(push, curAlignment)
#pragma pack(1)

struct DiskInfo
{
    WORD	wCylinder;
    WORD	wHead;
    WORD	wSector;
    DWORD	dwNumSectors;
    WORD	wType;
    DWORD	dwRelativeSector;
    DWORD	dwNTRelativeSector;
    DWORD	dwBytesPerSector;
};

struct Partition
{
    BYTE	chBootInd;
    BYTE	chHead;
    BYTE	chSector;
    BYTE	chCylinder;
    BYTE	chType;
    BYTE	chLastHead;
    BYTE	chLastSector;
    BYTE	chLastCylinder;
    DWORD	dwRelativeSector;
    DWORD	dwNumberSectors;
};
#pragma pack(pop, curAlignment)

const WORD PART_TABLE = 0;
const WORD BOOT_RECORD = 1;
const WORD EXTENDED_PART = 2;

const BYTE PART_UNKNOWN = 0x00;     // Unknown.  
const BYTE PART_DOS2_FAT = 0x01;	// 12-bit FAT.  
const BYTE PART_DOS3_FAT = 0x04;	// 16-bit FAT. Partition smaller than 32MB.  
const BYTE PART_EXTENDED = 0x05;	// Extended MS-DOS Partition.  
const BYTE PART_DOS4_FAT = 0x06;	// 16-bit FAT. Partition larger than or equal to 32MB.  
const BYTE PART_NTFS = 0x07;	    // NTFS
const BYTE PART_DOS32 = 0x0B;		// 32-bit FAT. Partition up to 2047GB.  
const BYTE PART_DOS32X = 0x0C;		// Same as PART_DOS32(0Bh), but uses Logical Block Address Int 13h extensions.  
const BYTE PART_DOSX13 = 0x0E;		// Same as PART_DOS4_FAT(06h), but uses Logical Block Address Int 13h extensions.  
const BYTE PART_DOSX13X = 0x0F;		// Same as PART_EXTENDED(05h), but uses Logical Block Address Int 13h extensions.  

const WORD SECTOR_SIZE = 512;

namespace FsUtil
{
    // Return drive letter of file path or letter of current path.
    wchar_t GetDriveLetter(const wchar_t* path);

    // Get disk informatino list of available partitions.
    enum FsBits { eFsNone = 0, eFsDOS12 = 1, eFsDOS16 = 2, eFsDOS32 = 4, eFsNTFS = 8, eFsALL=15 };
    typedef std::vector<DiskInfo> DiskInfoList;
    DWORD GetLogicalDrives(const wchar_t* phyDrv, DiskInfoList& diskInfoList, FsBits whichFs);

    // Get physical disk number and partition number for volume.
    // Pass volume name  "\\\\.\\C:"  which is \\.\C:
    DWORD GetDriveAndPartitionNumber(const wchar_t* volumeName, unsigned& phyDrvNum, unsigned& partitionNum);

 
    // Pass volume name  "\\\\.\\C:"  which is \\.\C:
    DWORD GetNtfsDiskNumber(const wchar_t* volumeName, int& diskNumber, LONGLONG& offset);
 
    DWORD GetDriveStartSector(const wchar_t* phyDrv, DiskInfoList& diskInfoList);
};

