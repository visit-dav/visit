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
  \file    DICOMParser.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.2
  \date    September 2008
*/


#pragma once

#ifndef DICOMPARSER_H
#define DICOMPARSER_H

#include "../../StdTuvokDefines.h"
#include <IO/DirectoryParser.h>

// if the following define is set, the DICOM parser putputs detailed parsing information
// be carfull with this option it may create a huge amount of output
#ifndef DEBUG
  //#define DEBUG_DICOM
#endif

#include <string>
#include <vector>
#include <fstream>
#include "../../Basics/Vectors.h"
#include "../../Basics/EndianConvert.h"

class SimpleDICOMFileInfo : public SimpleFileInfo {
public:
  SimpleDICOMFileInfo();
  SimpleDICOMFileInfo(const std::string& strFileName);
  SimpleDICOMFileInfo(const std::wstring& wstrFileName);
  SimpleDICOMFileInfo(const SimpleDICOMFileInfo* info);
  virtual ~SimpleDICOMFileInfo() {}

  FLOATVECTOR3 m_fvPatientPosition; // this data is needed to fix aspect ratio which is brocken in many DICOM files ... idiots

  virtual bool GetData(void* pData, UINT32 iLength, UINT32 iOffset);
  virtual SimpleFileInfo* clone();

protected:
  UINT32 m_iOffsetToData;
};

class DICOMFileInfo : public SimpleDICOMFileInfo {
public:

  DICOMFileInfo();
  DICOMFileInfo(const std::string& strFileName);
  DICOMFileInfo(const std::wstring& wstrFileName);
  virtual ~DICOMFileInfo() {}

  UINT32       m_iSeries;
  UINTVECTOR3  m_ivSize;
  FLOATVECTOR3 m_fvfAspect;
  UINT32       m_iAllocated;
  UINT32       m_iStored;
  UINT32       m_iComponentCount;
  bool         m_bIsBigEndian;
  bool         m_bIsJPEGEncoded;
  std::string  m_strAcquDate;
  std::string  m_strAcquTime;
  std::string  m_strModality;
  std::string  m_strDesc;

  void SetOffsetToData(const UINT32 iOffset);
};


class DICOMStackInfo : public FileStackInfo {
public:

  DICOMStackInfo();
  DICOMStackInfo(const DICOMFileInfo* info);
  DICOMStackInfo(const DICOMStackInfo* other);
  virtual ~DICOMStackInfo() {}
  bool Match(const DICOMFileInfo* info);

  UINT32       m_iSeries;
  std::string  m_strAcquDate;
  std::string  m_strAcquTime;
  std::string  m_strModality;
};

enum DICOM_eType {
  TYPE_AE, // Application Entity string 16 bytes maximum
  TYPE_AS, // Age String string 4 bytes fixed
  TYPE_AT, // Attribute Tag string 4 bytes fixed
  TYPE_CS, // Code String string 16 bytes maximum
  TYPE_DA, // Date string 8 bytes fixed
  TYPE_DS, // Decimal String string 16 bytes maximum
  TYPE_DT, // Date Time string 26 bytes maximum
  TYPE_FL, // Floating Point Single binary 4 bytes fixed
  TYPE_FD, // Floating Point Double binary 8 bytes fixed
  TYPE_IS, // Integer String string 12 bytes maximum
  TYPE_LO, // Long String string 64 chars maximum
  TYPE_LT, // Long Text string 1024 chars maximum
  TYPE_OB, // Other Byte
  TYPE_OW, // Other Word
  TYPE_OF, // Other Float
  TYPE_PN, // Person Name string 64 chars maximum
  TYPE_SH, // Short String string 16 chars maximum
  TYPE_SL, // Signed Long binary 4 bytes fixed
  TYPE_SQ, // Sequence of Items - -
  TYPE_SS, // Signed Short binary 2 bytes fixed
  TYPE_ST, // Short Text string 1024 chars maximum
  TYPE_TM, // Time string 16 bytes maximum
  TYPE_UI, // Unique Identifier (UID) string 64 bytes maximum
  TYPE_UL, // Unsigned Long binary 4 bytes fixed
  TYPE_US, // Unsigned Short binary 2 bytes fixed
  TYPE_UT, // Unlimited Text string 232-2
  TYPE_UN, // Unknown
  TYPE_Implicit // Implict File no type
};


class DICOMParser : public DirectoryParser
{
public:
  DICOMParser(void);
  ~DICOMParser(void);

  virtual void GetDirInfo(std::string  strDirectory);
  virtual void GetDirInfo(std::wstring wstrDirectory);

  static bool GetDICOMFileInfo(const std::string& fileName, DICOMFileInfo& info);
protected:

  static void SkipUnusedElement(std::ifstream& fileDICOM, std::string& value, const UINT32 iElemLength);
  static void ReadHeaderElemStart(std::ifstream& fileDICOM, short& iGroupID, short& iElementID, DICOM_eType& eElementType, UINT32& iElemLength, bool bImplicit, bool bNeedsEndianConversion);
  static UINT32 GetUInt(std::ifstream& fileDICOM, const DICOM_eType eElementType, const UINT32 iElemLength, const bool bNeedsEndianConversion);

  #ifdef DEBUG_DICOM
  static void ParseUndefLengthSequence(std::ifstream& fileDICOM, short& iSeqGroupID, short& iSeqElementID, DICOMFileInfo& info, const bool bImplicit, const bool bNeedsEndianConversion, UINT32 iDepth);
  #else
  static void ParseUndefLengthSequence(std::ifstream& fileDICOM, short& iSeqGroupID, short& iSeqElementID, DICOMFileInfo& info, const bool bImplicit, const bool bNeedsEndianConversion);
  #endif

};

#endif // DICOMPARSER_H
