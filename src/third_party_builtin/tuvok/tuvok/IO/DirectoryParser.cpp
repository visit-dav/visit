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
  \file    DirectoryParser.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.2
  \date    September 2008
*/

#include "DirectoryParser.h"

#include <algorithm>
#include <sys/stat.h>
#include <memory.h>

using namespace std;

DirectoryParser::DirectoryParser(void)
{
}

DirectoryParser::~DirectoryParser(void)
{
  for (size_t i = 0; i<m_FileStacks.size(); i++) delete m_FileStacks[i];
  m_FileStacks.clear();
}

/*************************************************************************************/

SimpleFileInfo::SimpleFileInfo(const std::string& strFileName) :   
  m_strFileName(strFileName),
  m_iImageIndex(0),
  m_iDataSize(0)
{
  m_wstrFileName = wstring(strFileName.begin(),strFileName.end());  
}

SimpleFileInfo::SimpleFileInfo(const std::wstring& wstrFileName) :   
  m_wstrFileName(wstrFileName),
  m_iImageIndex(0),
  m_iDataSize(0)
{
  m_strFileName = string(wstrFileName.begin(),wstrFileName.end());  
}

SimpleFileInfo::SimpleFileInfo() :   
  m_strFileName(""),
  m_wstrFileName(L""),
  m_iImageIndex(0),
  m_iDataSize(0)
{}

SimpleFileInfo::SimpleFileInfo(const SimpleFileInfo* other) :
  m_strFileName(other->m_strFileName),
  m_wstrFileName(other->m_wstrFileName),
  m_iImageIndex(other->m_iImageIndex),
  m_iDataSize(other->m_iDataSize)
{
}

bool SimpleFileInfo::GetData(void** pData) {
  if (*pData == NULL) *pData = (void*)new char[GetDataSize()];

  memset(*pData, 0, GetDataSize());
  return GetData(*pData, GetDataSize(), 0);
}


/*************************************************************************************/

FileStackInfo::FileStackInfo() :
  m_ivSize(0,0,0),
  m_fvfAspect(1,1,1),
  m_iAllocated(0),
  m_iStored(0),
  m_iComponentCount(1),
  m_bIsBigEndian(false),
  m_bIsJPEGEncoded(false),
  m_strDesc(""),
  m_strFileType("")
{}

FileStackInfo::FileStackInfo(const FileStackInfo* other) :
  m_ivSize(other->m_ivSize),
  m_fvfAspect(other->m_fvfAspect),
  m_iAllocated(other->m_iAllocated),
  m_iStored(other->m_iStored),
  m_iComponentCount(other->m_iComponentCount),
  m_bIsBigEndian(other->m_bIsBigEndian),
  m_bIsJPEGEncoded(other->m_bIsJPEGEncoded),
  m_strDesc(other->m_strDesc),
  m_strFileType(other->m_strFileType)
{
  for (size_t i=0;i<other->m_Elements.size();i++) {
    SimpleFileInfo* e = other->m_Elements[i]->clone();
    m_Elements.push_back(e);
  }
}

FileStackInfo::~FileStackInfo() {
  for (size_t i=0;i<m_Elements.size();i++) delete m_Elements[i];
}

FileStackInfo::FileStackInfo( UINTVECTOR3  ivSize, FLOATVECTOR3 fvfAspect, UINT32 iAllocated, UINT32 iStored,
                              UINT32 iComponentCount, bool bIsBigEndian, bool bIsJPEGEncoded, const std::string& strDesc, const std::string& strFileType) :
  m_ivSize(ivSize),
  m_fvfAspect(fvfAspect),
  m_iAllocated(iAllocated),
  m_iStored(iStored),
  m_iComponentCount(iComponentCount),
  m_bIsBigEndian(bIsBigEndian),
  m_bIsJPEGEncoded(bIsJPEGEncoded),
  m_strDesc(strDesc),
  m_strFileType(strFileType)
{
}
