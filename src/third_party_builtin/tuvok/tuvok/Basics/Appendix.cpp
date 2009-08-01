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
  \file    Appendix.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    January 2009
*/

#include "Appendix.h"
#include "SysTools.h"
#include "LargeRAWFile.h"

using namespace std;

Appendix::Appendix(string strTarget, const vector<string>& vstrSource) :
  m_strAPXFile(strTarget),
  m_bOK(false)
{
  UINT64 iOffset=0;
  for (size_t i = 0;i<vstrSource.size();i++) {
    LargeRAWFile inFile(vstrSource[i]);
    inFile.Open(false);
    if (inFile.IsOpen()) {
      UINT64 iSize = inFile.GetCurrentSize();
      inFile.Close();
      InternalFileInfo f(vstrSource[i], iSize, iOffset);
      m_vHeaderData.push_back(f);
      iOffset += iSize;
    } else {
      return;
    }
  }

  // create the file (if the file exists, override)
  LargeRAWFile target(strTarget);
  target.Create();
  if (!target.IsOpen()) {
    return;
  }

  // write the header
  string fileMagic = "APPENDIX";
  target.WriteRAW((unsigned char*)&(fileMagic[0]),8);
  target.WriteData<UINT64>(m_vHeaderData.size(), false);
  UINT64 iMaxSize = 0;
  for (size_t i = 0;i<m_vHeaderData.size();i++) {
    string filename = SysTools::GetFilename(m_vHeaderData[i].m_strName);
    target.WriteData<UINT64>(filename.length(), false);
    target.WriteRAW((unsigned char*)&(filename[0]), filename.length());
    target.WriteData<UINT64>(m_vHeaderData[i].m_iSize, false);
  
    iMaxSize = max(iMaxSize, m_vHeaderData[i].m_iSize);
  }
  m_iHeaderLength = target.GetPos();

  // append the files  
  UINT64 iCopySize = min(iMaxSize,BLOCK_COPY_SIZE);
  unsigned char* pBuffer = new unsigned char[size_t(iCopySize)];

  for (size_t i = 0;i<m_vHeaderData.size();i++) {
    UINT64 iCurrentCopySize = iCopySize;
    LargeRAWFile fInput(m_vHeaderData[i].m_strName);
    fInput.Open(false);

    if (!fInput.IsOpen()) {
      target.Close();
      return;
    }

    do {
      iCurrentCopySize = fInput.ReadRAW(pBuffer, iCurrentCopySize);
      target.WriteRAW(pBuffer, iCurrentCopySize);
    } while (iCurrentCopySize > 0); 

    fInput.Close();

    // remove path from file and leave only filename
    m_vHeaderData[i].m_strName = SysTools::GetFilename(m_vHeaderData[i].m_strName);
  }
  delete [] pBuffer;    

  // done
  target.Close();
  m_bOK = true;
}

Appendix::Appendix(string strAPXFile) :
  m_strAPXFile(strAPXFile),
  m_bOK(false)
{  
  LargeRAWFile fInput(m_strAPXFile);
  fInput.Open(false);
  if (!fInput.IsOpen()) {
    return;
  }

  string fileMagic = "12345678";
  fInput.ReadRAW((unsigned char*)&(fileMagic[0]),8);

  if (fileMagic != "APPENDIX") {
    fInput.Close();
    return;
  }

  UINT64 iFileCount = 0;
  UINT64 iOffset=0;
  fInput.ReadData<UINT64>(iFileCount, false);
  for (size_t i = 0;i<iFileCount;i++) {
    string strName;
    UINT64 iSize;

    UINT64 iNameLength = 0;
    fInput.ReadData<UINT64>(iNameLength, false);
    
    strName.resize(size_t(iNameLength));
    fInput.ReadRAW((unsigned char*)&(strName[0]), iNameLength);
    fInput.ReadData<UINT64>(iSize, false);

    m_vHeaderData.push_back(InternalFileInfo(strName, iSize, iOffset));
    iOffset += iSize;
  }
  m_iHeaderLength = fInput.GetPos();

  fInput.Close();
  m_bOK = true;
}


std::vector<FileInfo> Appendix::ListFiles() {
  std::vector<FileInfo> list;
  for (size_t i = 0;i<m_vHeaderData.size();i++) {
    list.push_back(FileInfo(m_vHeaderData[i].m_strName, m_vHeaderData[i].m_iSize)); 
  }
  return list;
}

bool Appendix::ExtractFile(size_t i, string strTarget) {
  
  if (i >= m_vHeaderData.size()) return false;

  LargeRAWFile fInput(m_strAPXFile);
  fInput.Open(false);
  if (!fInput.IsOpen()) {
    return false;
  }
  fInput.SeekPos(m_iHeaderLength+m_vHeaderData[i].m_iOffset);

  LargeRAWFile fOutput(strTarget);
  fOutput.Create();

  if (!fOutput.IsOpen()) {
    fInput.Close();
    return false;
  }

  UINT64 iCopySize = min(m_vHeaderData[i].m_iSize, BLOCK_COPY_SIZE);
  UINT64 iWritten = 0;
  unsigned char* pBuffer = new unsigned char[size_t(iCopySize)];
  do {
    iCopySize = fInput.ReadRAW(pBuffer, iCopySize);
    fOutput.WriteRAW(pBuffer, iCopySize);
    iWritten += iCopySize;
    if (m_vHeaderData[i].m_iSize - iWritten < iCopySize) iCopySize = m_vHeaderData[i].m_iSize - iWritten;
  } while (iWritten < m_vHeaderData[i].m_iSize); 
  delete [] pBuffer;

  fInput.Close();
  fOutput.Close();

  return true;
}
