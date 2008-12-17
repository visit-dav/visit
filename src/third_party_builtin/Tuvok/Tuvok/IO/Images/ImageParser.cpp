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
  \file    ImageParser.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.2
  \date    September 2008
*/

#include "ImageParser.h"
#include <QtGui/QImage>
#include <QtGui/QColor>
#include "../../Basics/SysTools.h"

using namespace std;

ImageFileInfo::ImageFileInfo() : 
  SimpleImageFileInfo(),
  m_ivSize(0,0),
  m_iAllocated(0),
  m_iComponentCount(1)
{}

ImageFileInfo::ImageFileInfo(const std::string& strFileName) : 
  SimpleImageFileInfo(strFileName),
  m_ivSize(0,0),
  m_iAllocated(0),
  m_iComponentCount(1)
{}


ImageFileInfo::ImageFileInfo(const std::wstring& wstrFileName) : 
  SimpleImageFileInfo(wstrFileName),
  m_ivSize(0,0),
  m_iAllocated(0),
  m_iComponentCount(1)
{}

void ImageFileInfo::ComputeSize() {
  m_iDataSize = m_iComponentCount*m_ivSize.area()*m_iAllocated/8;
}

/*************************************************************************************/

SimpleImageFileInfo::SimpleImageFileInfo() :
  SimpleFileInfo()
{}

SimpleImageFileInfo::SimpleImageFileInfo(const std::string& strFileName) :
  SimpleFileInfo(strFileName)
{}

SimpleImageFileInfo::SimpleImageFileInfo(const std::wstring& wstrFileName) :
  SimpleFileInfo(wstrFileName)
{}

SimpleImageFileInfo::SimpleImageFileInfo(const SimpleImageFileInfo* info) :
  SimpleFileInfo(info)
{}

bool SimpleImageFileInfo::GetData(void* pData, unsigned int iLength, unsigned int iOffset) {
  QImage qImage(m_strFileName.c_str()); 
  if (qImage.isNull()) return false;

  int iCount = 0;
  for (int y = 0;y<qImage.height();y++)
    for (int x = 0;x<qImage.width();x++) {
      if (int(iOffset) > iCount) continue;
      QColor pixel(qImage.pixel(x,y));
      unsigned char cValue = (unsigned char)((pixel.red() + pixel.green() + pixel.blue()) / 3);
      ((unsigned char*)pData)[iCount-iOffset] = cValue;
      if (int(iLength) == iCount-int(iOffset)) break;
      iCount++;
    }

  return true;
}

SimpleFileInfo* SimpleImageFileInfo::clone() {
  SimpleImageFileInfo* pSimpleImageFileInfo = new SimpleImageFileInfo(this);
  return (SimpleFileInfo*)pSimpleImageFileInfo;
}


/*************************************************************************************/

ImageStackInfo::ImageStackInfo() :
  FileStackInfo()
{}

ImageStackInfo::ImageStackInfo(const ImageFileInfo* fileInfo) :
  FileStackInfo(UINTVECTOR3(fileInfo->m_ivSize,1), FLOATVECTOR3(1,1,1), fileInfo->m_iAllocated, fileInfo->m_iAllocated,
                fileInfo->m_iComponentCount, false, "image file", "IMAGE")
{
  m_Elements.push_back(new SimpleImageFileInfo(fileInfo));
}

ImageStackInfo::ImageStackInfo(const ImageStackInfo* other)
{
  m_ivSize          = other->m_ivSize;
  m_fvfAspect       = other->m_fvfAspect;
  m_iAllocated      = other->m_iAllocated;
  m_iStored         = other->m_iStored;
  m_iComponentCount = other->m_iComponentCount;
  m_bIsBigEndian    = other->m_bIsBigEndian;
  m_strDesc         = other->m_strDesc;
  m_strFileType     = other->m_strFileType;

  for (unsigned int i=0;i<other->m_Elements.size();i++) {
    SimpleImageFileInfo* e = new SimpleImageFileInfo((SimpleImageFileInfo*)other->m_Elements[i]);
    m_Elements.push_back(e);
  }
}


bool ImageStackInfo::Match(const ImageFileInfo* info) {
  if (m_ivSize.xy()     == info->m_ivSize &&
      m_iAllocated      == info->m_iAllocated &&
      m_iComponentCount == info->m_iComponentCount) {

    std::vector<SimpleFileInfo*>::iterator iter;

    for (iter = m_Elements.begin(); iter < m_Elements.end(); iter++) {
      if ((*iter)->m_iImageIndex > info->m_iImageIndex) break;
    }

    m_Elements.insert(iter,new SimpleImageFileInfo(info));

    return true;
  } else return false;
}

/*************************************************************************************/

ImageParser::ImageParser(void)
{
}

ImageParser::~ImageParser(void)
{
}

void ImageParser::GetDirInfo(string  strDirectory) {
  vector<string> files = SysTools::GetDirContents(strDirectory);
  vector<ImageFileInfo> fileInfos;
  for (unsigned int i = 0;i<files.size();i++) {
    files[i] = strDirectory+"/"+files[i]; 
  }

  // query directory for image files
  for (unsigned int i = 0;i<files.size();i++) {
    QImage qImage(files[i].c_str()); 
    if (!qImage.isNull()) {
      ImageFileInfo info(files[i]);
      info.m_ivSize          = UINTVECTOR2(qImage.size().width(), qImage.size().height());
      info.m_iAllocated      = 8;  // lets assume all images are 8 bit 
      info.m_iComponentCount = 1;  // as qt converts any image to RGBA we also assume that the images were 1 component

      info.ComputeSize();

      fileInfos.push_back(info);
    }
  }

  // sort results into stacks
  for (unsigned int i = 0; i<m_FileStacks.size(); i++) delete m_FileStacks[i];
  m_FileStacks.clear();

  for (unsigned int i = 0; i<fileInfos.size(); i++) {
    bool bFoundMatch = false;
    for (unsigned int j = 0; j<m_FileStacks.size(); j++) {
      if (((ImageStackInfo*)m_FileStacks[j])->Match(&fileInfos[i])) {
        bFoundMatch = true;
        break;
      }
    }  
    if (!bFoundMatch) {
      ImageStackInfo* newStack = new ImageStackInfo(&fileInfos[i]);
      m_FileStacks.push_back(newStack);
    }
  }
}

void ImageParser::GetDirInfo(wstring wstrDirectory) {
  string strDirectory(wstrDirectory.begin(), wstrDirectory.end());
  GetDirInfo(strDirectory);
}
