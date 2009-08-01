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
  \file    ImageParser.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.2
  \date    September 2008
*/
#pragma once

#ifndef IMAGEPARSER_H
#define IMAGEPARSER_H

#include <string>
#include "../../StdTuvokDefines.h"
#include <IO/DirectoryParser.h>
#include "../../Basics/Vectors.h"

// if the following define is set, the DICOM parser putputs detailed parsing information
// be carfull with this option it may create a huge amount of output
#ifndef DEBUG
  //#define DEBUG_DICOM
#endif

class SimpleImageFileInfo : public SimpleFileInfo {
public:
  SimpleImageFileInfo();
  SimpleImageFileInfo(const std::string& strFileName);
  SimpleImageFileInfo(const std::wstring& wstrFileName);
  SimpleImageFileInfo(const SimpleImageFileInfo* info);
  virtual ~SimpleImageFileInfo() {}

  virtual bool GetData(void* pData, UINT32 iLength, UINT32 iOffset);
  virtual SimpleFileInfo* clone();
};

class ImageFileInfo : public SimpleImageFileInfo {
public:
  ImageFileInfo();
  ImageFileInfo(const std::string& strFileName);
  ImageFileInfo(const std::wstring& wstrFileName);
  virtual ~ImageFileInfo() {}

  UINTVECTOR2 m_ivSize;
  UINT32      m_iAllocated;
  UINT32      m_iComponentCount;

  void ComputeSize();

};

class ImageStackInfo : public FileStackInfo {
public:
  ImageStackInfo();
  ImageStackInfo(const ImageFileInfo* info);
  ImageStackInfo(const ImageStackInfo* other);
  virtual ~ImageStackInfo() {}
  bool Match(const ImageFileInfo* info);

};


class ImageParser : public DirectoryParser
{
public:
  ImageParser(void);
  ~ImageParser(void);

  virtual void GetDirInfo(std::string  strDirectory);
  virtual void GetDirInfo(std::wstring wstrDirectory);

protected:

};

#endif // IMAGEPARSER_H
