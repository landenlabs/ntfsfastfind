// ------------------------------------------------------------------------------------------------
// FileSystem utility class used to access file system information.
//
// Project: NTFSfastFind
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

#include "FsUtil.h"
#include "BaseTypes.h"
#include <sstream>
#ifdef _DEBUG
#include <iostream>
#include <iomanip>      // std::setw
#endif

#define IOCTL_VOLUME_LOGICAL_TO_PHYSICAL \
        CTL_CODE( IOCTL_VOLUME_BASE, 8, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_VOLUME_PHYSICAL_TO_LOGICAL \
        CTL_CODE( IOCTL_VOLUME_BASE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS )

// ------------------------------------------------------------------------------------------------
wchar_t FsUtil::GetDriveLetter(const wchar_t* path)
{
    if (wcscspn(path, L":") == 1)
        return path[0];

    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectory(ARRAYSIZE(currentDir), currentDir);
    return currentDir[0];
}

// ------------------------------------------------------------------------------------------------
DWORD FsUtil::GetDriveAndPartitionNumber(const wchar_t* volumeName, unsigned& phyDrvNum, unsigned& partitionNum)
{
    Hnd volumeHandle = CreateFile(
        volumeName,                     // "\\\\.\\C:";
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (!volumeHandle.IsValid())
        return GetLastError();

    struct STORAGE_DEVICE_NUMBER 
    {
        DEVICE_TYPE DeviceType;
        ULONG       DeviceNumber;
        ULONG       PartitionNumber;
    };

    STORAGE_DEVICE_NUMBER storage_device_number;
    DWORD dwBytesReturned;

    if (!DeviceIoControl(
            volumeHandle,
            IOCTL_STORAGE_GET_DEVICE_NUMBER,
            NULL,
            0,
            &storage_device_number,
            sizeof(storage_device_number),
            &dwBytesReturned,
            NULL))
    {
        DWORD err = GetLastError();
#ifdef _DEBUG
        std::wcerr << err << " Failed GetDriveAndPartitionNumber CreateFile " << volumeName << std::endl;
#endif
        return err;
    }

    phyDrvNum = storage_device_number.DeviceNumber;
    partitionNum = storage_device_number.PartitionNumber - 1;   // appears to one based, so shift down one.

    return ERROR_SUCCESS;
}

 
// ------------------------------------------------------------------------------------------------

DWORD FsUtil::GetNtfsDiskNumber(const wchar_t* volumeName, int& diskNumber, LONGLONG& offset)
{
    Hnd volumeHandle = CreateFile(
        volumeName,                     // "\\\\.\\C:";
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (!volumeHandle.IsValid())
        return GetLastError();

    // if (strcmp(szFileSystemName, "NTFS") == 0)
    {
        // Volume logical offset  
        struct VOLUME_LOGICAL_OFFSET 
        {
            LONGLONG    LogicalOffset;
        };

        // Volume physical offset 
        struct VOLUME_PHYSICAL_OFFSET 
        {
            ULONG       DiskNumber;
            LONGLONG    Offset;
        };

        // Volume physical offsets 
        struct VOLUME_PHYSICAL_OFFSETS 
        {
            ULONG                   NumberOfPhysicalOffsets;
            VOLUME_PHYSICAL_OFFSET  PhysicalOffset[10];  // ANYSIZE_ARRAY];
        };

        VOLUME_LOGICAL_OFFSET   volumeLogicalOffset;
        VOLUME_PHYSICAL_OFFSETS volumePhysicalOffsets;
        LONGLONG logicalOffset = 0; // lpRetrievalPointersBuffer->Extents [0].Lcn.QuadPart * dwClusterSizeInBytes;
        DWORD dwBytesReturned;
        ZeroMemory(&volumePhysicalOffsets, sizeof(volumePhysicalOffsets));
        volumePhysicalOffsets.PhysicalOffset[0].DiskNumber = 123;

        volumeLogicalOffset.LogicalOffset = logicalOffset;
        if (!DeviceIoControl(
            volumeHandle,
            IOCTL_VOLUME_LOGICAL_TO_PHYSICAL,
            &volumeLogicalOffset,
            sizeof(VOLUME_LOGICAL_OFFSET),
            &volumePhysicalOffsets,
            sizeof(volumePhysicalOffsets),
            &dwBytesReturned,
            NULL))
        {
            return GetLastError();
        }

        if (volumePhysicalOffsets.NumberOfPhysicalOffsets > 0)
        {
            diskNumber = volumePhysicalOffsets.PhysicalOffset[0].DiskNumber;
            offset = volumePhysicalOffsets.PhysicalOffset[0].Offset;
            return ERROR_SUCCESS;
        }
    }

    return ERROR_BAD_UNIT;  // not NTFS file system.
}

// ------------------------------------------------------------------------------------------------
///  

DWORD FsUtil::GetDriveStartSector(const wchar_t* volumeName, DiskInfoList& diskInfoList)
{
    int patIdx, nRet;

    Hnd hDrive = CreateFile(
        volumeName,                         // "\\\\.\\C:";
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hDrive == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
#ifdef _DEBUG
        std::wcerr << err << " Failed GetDriveStartSector CreateFile " << volumeName << std::endl;
#endif
        return err;
    }

    VOLUME_DISK_EXTENTS volumeDiskExtents;
    DWORD dwBytesReturned = 0;
    BOOL bResult = DeviceIoControl(hDrive
        , IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS
        , NULL
        , 0
        , &volumeDiskExtents
        , sizeof(volumeDiskExtents)
        , &dwBytesReturned
        , NULL);
    if (!bResult) {
        return GetLastError();
    }


    if (volumeDiskExtents.NumberOfDiskExtents > 0)
    {
        DiskInfo diskInfo;
        ZeroMemory(&diskInfo, sizeof(diskInfo));
        diskInfo.dwNTRelativeSector = volumeDiskExtents.Extents[0].StartingOffset.QuadPart / SECTOR_SIZE;
        diskInfo.dwNumSectors = volumeDiskExtents.Extents[0].ExtentLength.QuadPart / SECTOR_SIZE;  
        diskInfoList.push_back(diskInfo);
    }
    return ERROR_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
/// This function is from vinoj kumar's article forensic in codeguru

DWORD FsUtil::GetLogicalDrives(const wchar_t* phyDrv, DiskInfoList& diskInfoList, FsBits whichFs)
{
    int patIdx, nRet;

    Hnd hDrive = CreateFile(phyDrv, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (hDrive == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
#ifdef _DEBUG
        std::wcerr << err << " Failed GetLogicalDrives CreateFile " << phyDrv << std::endl;
#endif
        return err;
    }

    DWORD dwBytes;
    const unsigned sSectorSize = SECTOR_SIZE;
    BYTE szSector[sSectorSize];
    nRet = ReadFile(hDrive, szSector, sSectorSize, &dwBytes, 0);
    if (!nRet)
        return GetLastError();

    DWORD dwMainPrevRelSector = 0;
    DWORD dwPrevRelSector     = 0;

    Partition*  pPartition = (Partition*)(szSector + 0x1BE);
    // int partSize = sizeof(Partition);
    DiskInfo diskInfo;
    int partCount = (dwBytes - 0x1BE) / sizeof(Partition);
#ifdef _DEBUG
    std::wcerr << "----" << phyDrv << " Partitions ----\n";

    std::wcerr << " #"
        << " Cyl"
        << " Heads"
        << " Type" 
        << " ChType"
        << " Sector"
        << "    #Sectors" 
        << "   RelSector" 
        << std::endl;
#endif
    for (patIdx = 0; patIdx < partCount; patIdx++) /// scanning partitions in the physical disk
    {
        ZeroMemory(&diskInfo, sizeof(diskInfo));
        diskInfo.wCylinder    = pPartition->chCylinder;
        diskInfo.wHead        = pPartition->chHead;
        diskInfo.wSector      = pPartition->chSector;
        diskInfo.dwNumSectors = pPartition->dwNumberSectors;
        diskInfo.wType        = 
                ((pPartition->chType == PART_EXTENDED) || (pPartition->chType == PART_DOSX13X)) ? 
                EXTENDED_PART:BOOT_RECORD;

        if ((pPartition->chType == PART_EXTENDED) || (pPartition->chType == PART_DOSX13X))
        {
            dwMainPrevRelSector		    = pPartition->dwRelativeSector;
            diskInfo.dwNTRelativeSector	= dwMainPrevRelSector;
        }
        else
        {
            diskInfo.dwNTRelativeSector = dwMainPrevRelSector + pPartition->dwRelativeSector;
        }



        if (diskInfo.wType == EXTENDED_PART)
            break;

        if (pPartition->chType == 0)
            break;

#ifdef _DEBUG
        std::wcerr << std::setw(2) << patIdx
            << std::setw(4) << diskInfo.wCylinder
            << std::setw(6) << diskInfo.wHead
            << std::setw(5) << diskInfo.wType
            << std::setw(7) << pPartition->chType
            << std::setw(7) << diskInfo.wSector
            << std::setw(12) << diskInfo.dwNumSectors
            << std::setw(12) << diskInfo.dwNTRelativeSector
            << std::endl;
#endif

        switch (pPartition->chType)
        {
        case PART_DOS2_FAT: // FAT12
            if ((whichFs & eFsDOS12) != 0)
                diskInfoList.push_back(diskInfo);
            break;
        case PART_DOSX13:
        case PART_DOS4_FAT:
        case PART_DOS3_FAT:
            if ((whichFs & eFsDOS16) != 0)
                diskInfoList.push_back(diskInfo);
            break;
        case PART_DOS32X:
        case PART_DOS32:
            if ((whichFs & eFsDOS32) != 0)
                diskInfoList.push_back(diskInfo);
            break;
        case PART_NTFS:  
            if ((whichFs & eFsNTFS) != 0)
                diskInfoList.push_back(diskInfo);
            break;
        default: // Unknown
            if (whichFs == eFsALL)
                diskInfoList.push_back(diskInfo);
            break;
        }

        pPartition++;
    }

    if (patIdx == partCount)
        return ERROR_SUCCESS;

    for (int LogiHard = 0; LogiHard < 50; LogiHard++) // scanning extended partitions
    {
        if (diskInfo.wType == EXTENDED_PART)
        {
            LARGE_INTEGER n64Pos;

            n64Pos.QuadPart = ((LONGLONG) diskInfo.dwNTRelativeSector) * SECTOR_SIZE;

            nRet = SetFilePointer(hDrive, n64Pos.LowPart, &n64Pos.HighPart, FILE_BEGIN);
            if (nRet == 0xffffffff)
            {
                DWORD err = GetLastError();
#ifdef _DEBUG
                std::wcerr << err << " Failed GetLogicalDrives SetFilePointer " << n64Pos.QuadPart << std::endl;
#endif
                return err;
            }

            dwBytes = 0;

            nRet = ReadFile(hDrive, szSector, SECTOR_SIZE, (DWORD *) &dwBytes, NULL);
            if (!nRet)
            {
                DWORD err = GetLastError();
#ifdef _DEBUG
                std::wcerr << err << " Failed GetLogicalDrives Read Logical Table " << LogiHard << std::endl;
#endif
                return err;
            }
 

            if (dwBytes != SECTOR_SIZE) {
#ifdef _DEBUG
                std::wcerr << " Failed GetLogicalDrives Read Logical Table size=" << dwBytes << std::endl;
#endif
                return ERROR_READ_FAULT;
            }

            pPartition = (Partition*) (szSector+0x1BE);
#ifdef _DEBUG
            std::wcerr << "Logical table size=" << dwBytes << " ElemSize=" << sizeof(Partition) << " count=" << partCount << std::endl;
#endif

            for (patIdx = 0; patIdx < 4; patIdx++)
            {
                diskInfo.wCylinder = pPartition->chCylinder;
                diskInfo.wHead = pPartition->chHead;
                diskInfo.dwNumSectors = pPartition->dwNumberSectors;
                diskInfo.wSector = pPartition->chSector;
                diskInfo.dwRelativeSector = 0;
                diskInfo.wType = ((pPartition->chType == PART_EXTENDED) || (pPartition->chType == PART_DOSX13X)) ? EXTENDED_PART:BOOT_RECORD;

                if ((pPartition->chType == PART_EXTENDED) || (pPartition->chType == PART_DOSX13X))
                {
                    dwPrevRelSector = pPartition->dwRelativeSector;
                    diskInfo.dwNTRelativeSector = dwPrevRelSector + dwMainPrevRelSector;
                }
                else
                {
                    diskInfo.dwNTRelativeSector = dwMainPrevRelSector + dwPrevRelSector + pPartition->dwRelativeSector;
                }

                if (diskInfo.wType == EXTENDED_PART)
                    break;

                if (pPartition->chType == 0)
                    break;

#ifdef _DEBUG
                std::wcerr << std::setw(2) << patIdx
                    << std::setw(4) << diskInfo.wCylinder
                    << std::setw(6) << diskInfo.wHead
                    << std::setw(5) << diskInfo.wType
                    << std::setw(7) << pPartition->chType
                    << std::setw(7) << diskInfo.wSector
                    << std::setw(12) << diskInfo.dwNumSectors
                    << std::setw(12) << diskInfo.dwNTRelativeSector
                    << std::endl;
#endif

                switch(pPartition->chType)
                {
                case PART_DOS2_FAT: // FAT12
                    if ((whichFs & eFsDOS12) != 0)
                        diskInfoList.push_back(diskInfo);
                    break;
                case PART_DOSX13:
                case PART_DOS4_FAT:
                case PART_DOS3_FAT:
                    if ((whichFs & eFsDOS16) != 0)
                        diskInfoList.push_back(diskInfo);
                    break;
                case PART_DOS32X:
                case PART_DOS32:
                    if ((whichFs & eFsDOS32) != 0)
                        diskInfoList.push_back(diskInfo);
                    break;
                case 7: // NTFS
                    if ((whichFs & eFsNTFS) != 0)
                        diskInfoList.push_back(diskInfo);
                    break;
                default: // Unknown
#ifdef _DEBUG
                    std::wcerr << "Unknown logical drive type=" << pPartition->chType << std::endl;
#endif
                    break;
                }

                pPartition++;
            }

            if (patIdx == 4)
                break;
        }
    }


    return ERROR_SUCCESS;
}
