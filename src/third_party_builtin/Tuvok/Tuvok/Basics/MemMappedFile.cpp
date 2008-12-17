/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
  \file    MemMappedFile.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.2
  \date    July 2008
*/

#include "MemMappedFile.h"

using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <memory.h>

#ifdef _WIN32
  #include <process.h>
#else
  #include <sys/mman.h>
  #include <unistd.h>
#endif
 

MemMappedFile::MemMappedFile(const string strFilename, const MMFILE_ACCESS eAccesMode, const UINT64& iLengthForNewFile, const UINT64& iOffset, const UINT64& iBytesToMap) :
  m_strFilename(strFilename),
  m_eAccesMode(eAccesMode),
  m_iLengthForNewFile(iLengthForNewFile),
  m_AllocationGranularity(0),
  #ifdef WIN32
    m_hMem(NULL),
    m_dwDesiredAccessMap(0)
  #else
    m_fdes(0),
    m_dwMmmapMode(0)
  #endif
{
  ComputeAllocationGranularity();
  m_bIsOpen = OpenFile(m_strFilename.c_str(), m_eAccesMode, m_iLengthForNewFile, iOffset, iBytesToMap) == 0;
}

MemMappedFile::MemMappedFile(const wstring strFilename, const MMFILE_ACCESS eAccesMode, const UINT64& iLengthForNewFile, const UINT64& iOffset, const UINT64& iBytesToMap):
  m_strFilename( strFilename.begin(), strFilename.end() ),
  m_eAccesMode(eAccesMode),
  m_iLengthForNewFile(iLengthForNewFile),
  m_AllocationGranularity(0),
  #ifdef WIN32
    m_hMem(NULL),
    m_dwDesiredAccessMap(0)
  #else
    m_fdes(0),
    m_dwMmmapMode(0)
  #endif

{
  ComputeAllocationGranularity();
  string st(strFilename.begin(),strFilename.end());
  m_bIsOpen = OpenFile(st.c_str(), m_eAccesMode, m_iLengthForNewFile, iOffset, iBytesToMap) == 0;
}

MemMappedFile::~MemMappedFile(void)
{
  Close();
}

void  MemMappedFile::Flush() {
#ifdef WIN32
  if (m_pData) FlushViewOfFile(m_pData, 0);
#else
  if (m_pData) msync(m_pData, m_dwFileMappingSize, MS_SYNC);
#endif 
}

void  MemMappedFile::Close() {
  if (m_bIsOpen) {
    #ifdef _WIN32
      CloseHandle( m_hMem );
      if (m_pData) UnmapViewOfFile( m_pData);
    #else
      close( m_fdes );
      if (m_pData) munmap( m_pData, m_dwFileMappingSize);
    #endif
    m_bIsOpen = false;
    m_pData = NULL;

  }
}

void  MemMappedFile::Erase() {
  Close();
  remove(m_strFilename.c_str());
}

void* MemMappedFile::ReOpen(const UINT64& iOffset, const UINT64& iBytesToMap) {
  if (m_bIsOpen) Close();

  m_bIsOpen = OpenFile(m_strFilename.c_str(), m_eAccesMode, m_iLengthForNewFile, iOffset, iBytesToMap) == 0;

  return m_pData;
}


void* MemMappedFile::ReMap(const UINT64& iOffset, const UINT64& iBytesToMap) {
  if (!m_bIsOpen) return ReOpen(iOffset, iBytesToMap);

  UINT64 iPosAdjustment = (iOffset % UINT64(m_AllocationGranularity));

#ifdef _WIN32
  if (m_pData) UnmapViewOfFile( m_pData);

  DWORD dwHigh  = DWORD((((iOffset-iPosAdjustment) >> 32) & 0xFFFFFFFF));
  DWORD dwLow = DWORD(((iOffset-iPosAdjustment) & 0xFFFFFFFF));
  m_pData = MapViewOfFile( m_hMem, m_dwDesiredAccessMap, dwHigh, dwLow, (iBytesToMap == 0) ? 0 : (size_t)(iBytesToMap+iPosAdjustment));
  if (m_pData == NULL) {
#else
  if (m_pData) munmap( m_pData, m_dwFileMappingSize);

  m_pData = mmap( NULL, ((iBytesToMap == 0) ? (m_dwFileSize-iOffset) : iBytesToMap)+iPosAdjustment, m_dwMmmapMode, MAP_SHARED, m_fdes, iOffset-iPosAdjustment);
  if (m_pData == (void *)-1) {
#endif
    m_dwFileMappingSize = 0;
    return NULL;
  }

  m_pData = (char*)m_pData + iPosAdjustment;
  m_dwFileMappingSize = (iBytesToMap != 0) ? iBytesToMap : m_dwFileSize;

  return m_pData;
}


int  MemMappedFile::OpenFile(const char* strPath, const MMFILE_ACCESS eAccesMode, const UINT64& iLengthForNewFile, const UINT64& iOffset, const UINT64& iBytesToMap) {

  m_id = rand();
 
  bool bExists = true, bGrowFile = false;
  char buffer[4096];
  int res;
    
  UINT64 iPosAdjustment = (iOffset % m_AllocationGranularity);

#ifdef _WIN32

  struct _stati64 stat_buf;
  
  HANDLE hFile;
  DWORD sz;

  // check if file already exists and determine its length 
  res = _stat64( strPath, &stat_buf);
  if (res < 0) {
        if (errno == ENOENT) 
    bExists = false;
        else 
    return -1;
  }
  if (bExists) 
    if (UINT64(stat_buf.st_size)>=iLengthForNewFile) 
      m_dwFileSize = stat_buf.st_size;
    else {
      m_dwFileSize = iLengthForNewFile;
      bGrowFile = true;
    }
  else {
    if (iLengthForNewFile == 0) {
      return -1;
    } else {
      m_dwFileSize = iLengthForNewFile;
    }
  }

  DWORD dwDesiredAccess    = (!bExists || eAccesMode == MMFILE_ACCESS_READWRITE) ? GENERIC_READ    | GENERIC_WRITE     : GENERIC_READ;
  DWORD dwShareMode      = (!bExists || eAccesMode == MMFILE_ACCESS_READWRITE) ? FILE_SHARE_READ | FILE_SHARE_WRITE  : FILE_SHARE_READ;
  DWORD flProtect        = (!bExists || eAccesMode == MMFILE_ACCESS_READWRITE) ? PAGE_READWRITE : PAGE_READONLY;
  m_dwDesiredAccessMap    = (!bExists || eAccesMode == MMFILE_ACCESS_READWRITE) ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ;

    // create security descriptor (needed for Windows NT) 
    PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR) malloc( SECURITY_DESCRIPTOR_MIN_LENGTH );
    if( pSD == NULL ) return -2;

    InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE);

  SECURITY_ATTRIBUTES sa;

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = TRUE;

    // create or open file 
  hFile = CreateFileA ( strPath, dwDesiredAccess, dwShareMode, &sa, bExists ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
        free( pSD);
        return -3;
    }
    if (! bExists || bGrowFile) {
    
    UINT64 dwWriteSize;
    if (bGrowFile) {
      SetFilePointer(hFile, 0, NULL, FILE_END);
      dwWriteSize = m_dwFileSize - stat_buf.st_size;
    } else dwWriteSize = m_dwFileSize;

        // ensure that file is long enough and filled with zero 
        memset( buffer, 0, sizeof(buffer));
        for (UINT64 i = 0; i < dwWriteSize/sizeof(buffer); ++i) {
            if (! WriteFile( hFile, buffer, sizeof(buffer), &sz, NULL)) {
                return -3;
            }
        }
        if (! WriteFile( hFile, buffer, static_cast<DWORD>(dwWriteSize%sizeof(buffer)), &sz, NULL)) {
            return -3;
        }
    }
        
    // create file mapping 
    sprintf_s( buffer, sizeof(buffer), "%d", m_id);
    m_hMem = CreateFileMappingA( hFile, &sa, flProtect, static_cast<DWORD>((m_dwFileSize & 0xFFFFFFFF00000000) >> 32), static_cast<DWORD>(m_dwFileSize & 0xFFFFFFFF), buffer);
    free( pSD);
    if (NULL == m_hMem) return -3;


  DWORD dwHigh  = DWORD((((iOffset-iPosAdjustment) >> 32) & 0xFFFFFFFF));
  DWORD dwLow = DWORD(((iOffset-iPosAdjustment) & 0xFFFFFFFF));

  // map the file to memory 
  m_pData = MapViewOfFile( m_hMem, m_dwDesiredAccessMap, dwHigh, dwLow, (iBytesToMap == 0) ? 0 : (size_t)(iBytesToMap+iPosAdjustment));

    CloseHandle( hFile );

  if (NULL == m_pData) {
    CloseHandle( m_hMem );
    return -3;
  }

#else
  struct stat stat_buf;

 
  // check if file already exists and determine its length
  res = stat( strPath, &stat_buf);
  if (res < 0) {
    if (errno == ENOENT) 
      bExists = false;
    else 
      return -1;
  }
  if (bExists) {
    if (UINT64(stat_buf.st_size)>iLengthForNewFile) 
      m_dwFileSize = stat_buf.st_size;
    else {
      m_dwFileSize = iLengthForNewFile;
      bGrowFile = true;
    }
  } else {
    if (iLengthForNewFile == 0)
      return -1;
    else
      m_dwFileSize = iLengthForNewFile;
  }
 
  int dwDesiredAccess = (!bExists || eAccesMode == MMFILE_ACCESS_READWRITE) ? O_RDWR : O_RDONLY;
  int dwShareMode      = (!bExists || eAccesMode == MMFILE_ACCESS_READWRITE) ? S_IRUSR | S_IWUSR  : S_IRUSR;
  m_dwMmmapMode    = (!bExists || eAccesMode == MMFILE_ACCESS_READWRITE) ? PROT_READ | PROT_WRITE : PROT_READ;
 
  // open / create mapped file 
  m_fdes = open( strPath, (bExists) ? dwDesiredAccess : dwDesiredAccess | O_CREAT, dwShareMode);
  if (m_fdes < -1) return -1;
 
  // ensure that file is long enough and filled with zero 
  if (! bExists || bGrowFile) {      
    UINT64 dwWriteSize;
    if (bGrowFile) {
      lseek(m_fdes, 0, SEEK_END);
      dwWriteSize = m_dwFileSize - stat_buf.st_size;
    } else dwWriteSize = m_dwFileSize;

    memset( buffer, 0, sizeof(buffer));
    for (size_t i = 0; i < dwWriteSize/sizeof(buffer); ++i) {
      if (write( m_fdes, buffer, sizeof(buffer)) != sizeof(buffer)) {
        return -1;
      }
    }
    if ((size_t)(write( m_fdes, buffer, dwWriteSize%sizeof(buffer))) != dwWriteSize%sizeof(buffer)) {
      return -1;
    }
  }
  // map the file to memory 
  m_pData = mmap( NULL, ((iBytesToMap == 0) ? (m_dwFileSize-iOffset) : iBytesToMap)+iPosAdjustment, m_dwMmmapMode, MAP_SHARED, m_fdes, iOffset-iPosAdjustment);
  if (m_pData == (void *)-1) return -1;
#endif

  m_pData = (char*)m_pData + iPosAdjustment;
  m_dwFileMappingSize = (iBytesToMap != 0) ? iBytesToMap : m_dwFileSize;

  return 0;
}


void MemMappedFile::ComputeAllocationGranularity() {
#ifdef _WIN32
  SYSTEM_INFO siSysInfo;
  GetSystemInfo(&siSysInfo); 
  m_AllocationGranularity = siSysInfo.dwAllocationGranularity;
#else
  m_AllocationGranularity = sysconf(_SC_PAGE_SIZE);
#endif
}
