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
  \file    MemMappedFile.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.2
  \date    July 2008
*/

#pragma once

#ifndef MEMMAPPEDFILE_H
#define MEMMAPPEDFILE_H

#include "../StdTuvokDefines.h"
#include <string>
#include "../Basics/MathTools.h"

#ifdef _WIN32
  #include <windows.h>
  // undef stupid windows defines to max and min
  #ifdef max
  #undef max
  #endif

  #ifdef min
  #undef min
  #endif
#endif

enum MMFILE_ACCESS {
  MMFILE_ACCESS_READONLY = 0,
  MMFILE_ACCESS_READWRITE,
};


class MemMappedFile
{
public:

  MemMappedFile(const std::wstring strFilename, const MMFILE_ACCESS eAccesMode = MMFILE_ACCESS_READONLY, const UINT64& iLengthForNewFile = 0, const UINT64& iOffset=0, const UINT64& iBytesToMap=0);
  MemMappedFile(const std::string strFilename, const MMFILE_ACCESS eAccesMode = MMFILE_ACCESS_READONLY, const UINT64& iLengthForNewFile = 0, const UINT64& iOffset=0, const UINT64& iBytesToMap=0);
  ~MemMappedFile(void);

  void*  GetDataPointer() const {return m_pData;}
  UINT64  GetFileMappingSize() const {return m_dwFileMappingSize;}
  UINT64  GetFileLength() const {return m_dwFileSize;}
  bool  IsOpen() const {return m_bIsOpen;}

  void  Flush();
  void  Close();
  void  Erase();
  void*  ReOpen(const UINT64& iOffset=0, const UINT64& iBytesToMap=0);
  void*  ReMap(const UINT64& iOffset=0, const UINT64& iBytesToMap=0);
  void  ChangeView(const UINT64& iOffset=0, const UINT64& iBytesToMap=0);

protected:
  bool  m_bIsOpen;
  void*  m_pData;
  UINT64  m_dwFileMappingSize;
  UINT64  m_dwFileSize;
  int    m_id;

  // for reopen
  std::string      m_strFilename;
  MMFILE_ACCESS    m_eAccesMode;
  UINT64        m_iLengthForNewFile;

  int    OpenFile(const char* strPath, const MMFILE_ACCESS eAccesMode, const UINT64& iLengthForNewFile = 0, const UINT64& iOffset=0, const UINT64& iBytesToMap=0);

private:
  void ComputeAllocationGranularity();

  #ifdef WIN32
    DWORD  m_AllocationGranularity;
    HANDLE  m_hMem;
    DWORD  m_dwDesiredAccessMap;
  #else
    long  m_AllocationGranularity;
    int    m_fdes;
    int    m_dwMmmapMode;
  #endif

};

#endif // MEMMAPPEDFILE_H
