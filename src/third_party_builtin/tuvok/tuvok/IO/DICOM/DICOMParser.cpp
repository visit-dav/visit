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
  \file    DICOMParser.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.2
  \date    September 2008
*/

#include "DICOMParser.h"

#ifdef DEBUG_DICOM
  #include <Basics/Console.h>
  #include <sstream>
#endif

#include <sys/stat.h>
#include <Basics/SysTools.h>
#include <algorithm>

std::string DICOM_TypeStrings[28] = {
  "AE", // Application Entity string 16 bytes maximum
  "AS", // Age String string 4 bytes fixed
  "AT", // Attribute Tag string 4 bytes fixed
  "CS", // Code String string 16 bytes maximum
  "DA", // Date string 8 bytes fixed
  "DS", // Decimal String string 16 bytes maximum
  "DT", // Date Time string 26 bytes maximum
  "FL", // Floating Point Single binary 4 bytes fixed
  "FD", // Floating Point Double binary 8 bytes fixed
  "IS", // Integer String string 12 bytes maximum
  "LO", // Long String string 64 chars maximum
  "LT", // Long Text string 1024 chars maximum
  "OB", // Other Byte
  "OW", // Other Word
  "OF", // Other Float
  "PN", // Person Name string 64 chars maximum
  "SH", // Short String string 16 chars maximum
  "SL", // Signed Long binary 4 bytes fixed
  "SQ", // Sequence of Items - -
  "SS", // Signed Short binary 2 bytes fixed
  "ST", // Short Text string 1024 chars maximum
  "TM", // Time string 16 bytes maximum
  "UI", // Unique Identifier (UID) string 64 bytes maximum
  "UL", // Unsigned Long binary 4 bytes fixed
  "US", // Unsigned Short binary 2 bytes fixed
  "UT", // Unlimited Text string 232-2
  "UN", // Unknown 
  "Implicit"
};

using namespace std;

DICOMParser::DICOMParser(void)
{
}

DICOMParser::~DICOMParser(void)
{
}

bool StacksSmaller ( FileStackInfo* pElem1, FileStackInfo* pElem2 )
{
  DICOMStackInfo* elem1 = (DICOMStackInfo*)pElem1;
  DICOMStackInfo* elem2 = (DICOMStackInfo*)pElem2;

  return elem1->m_iSeries < elem2->m_iSeries;
}


void DICOMParser::GetDirInfo(string  strDirectory) {
  vector<string> files = SysTools::GetDirContents(strDirectory);
  vector<DICOMFileInfo> fileInfos;

  // query directory for DICOM files
  for (size_t i = 0;i<files.size();i++) {
    DICOMFileInfo info;
    if (GetDICOMFileInfo(strDirectory+"/"+files[i], info)) fileInfos.push_back(info);
  }

  // sort results into stacks
  for (size_t i = 0; i<m_FileStacks.size(); i++) delete m_FileStacks[i];
  m_FileStacks.clear();


  for (size_t i = 0; i<fileInfos.size(); i++) {
    bool bFoundMatch = false;
    for (size_t j = 0; j<m_FileStacks.size(); j++) {
      if (((DICOMStackInfo*)m_FileStacks[j])->Match(&fileInfos[i])) {
        bFoundMatch = true;
        break;
      }
    }  
    if (!bFoundMatch) {
      DICOMStackInfo* newStack = new DICOMStackInfo(&fileInfos[i]);
      m_FileStacks.push_back(newStack);
    }
  }

  // sort stacks by sequence number
  sort( m_FileStacks.begin( ), m_FileStacks.end( ), StacksSmaller );

  // fix Z aspect ratio - which is broken in many DICOMs - using the patient position
  for (size_t i = 0; i<m_FileStacks.size(); i++) {
    if (m_FileStacks[i]->m_Elements.size() < 2) continue;    
    float fZDistance = fabs(((SimpleDICOMFileInfo*)m_FileStacks[i]->m_Elements[1])->m_fvPatientPosition.z - 
                            ((SimpleDICOMFileInfo*)m_FileStacks[i]->m_Elements[0])->m_fvPatientPosition.z);
    if (fZDistance != 0) m_FileStacks[i]->m_fvfAspect.z = fZDistance;
  }
}

void DICOMParser::GetDirInfo(wstring wstrDirectory) {
  string strDirectory(wstrDirectory.begin(), wstrDirectory.end());
  GetDirInfo(strDirectory);
}

void DICOMParser::ReadHeaderElemStart(ifstream& fileDICOM, short& iGroupID, short& iElementID, DICOM_eType& eElementType, UINT32& iElemLength, bool bImplicit, bool bNeedsEndianConversion) {
  string typeString = "  ";

  fileDICOM.read((char*)&iGroupID,2);
  fileDICOM.read((char*)&iElementID,2);

  if (iGroupID == 0x2) {  // ignore input for meta block
    bImplicit = false;
    bNeedsEndianConversion = false;
  }

  if (bNeedsEndianConversion) {
    iGroupID = EndianConvert::Swap<short>(iGroupID);
    iElementID = EndianConvert::Swap<short>(iElementID);
  }

  if (bImplicit) {
    eElementType = TYPE_Implicit;
    fileDICOM.read((char*)&iElemLength,4);
    if (bNeedsEndianConversion) iElemLength = EndianConvert::Swap<UINT32>(iElemLength);
  } else {
    fileDICOM.read(&typeString[0],2);    
    short tmp;
    fileDICOM.read((char*)&tmp,2);
    if (bNeedsEndianConversion) tmp = EndianConvert::Swap<short>(tmp);
    iElemLength = tmp;
    eElementType = TYPE_UN;
    for (UINT32 i = 0;i<27;i++) {
      if (typeString == DICOM_TypeStrings[i]) eElementType = DICOM_eType(i);
    }
  }
}


UINT32 DICOMParser::GetUInt(ifstream& fileDICOM, const DICOM_eType eElementType, const UINT32 iElemLength, const bool bNeedsEndianConversion) {
  string value;
  UINT32 result;
  switch (eElementType) {
    case TYPE_Implicit :
    case TYPE_IS  : {
              value.resize(iElemLength);
              fileDICOM.read(&value[0],iElemLength);
              result = atoi(value.c_str());
              break;
            }
    case TYPE_UL  : {
              fileDICOM.read((char*)&result,4);
              if (bNeedsEndianConversion) result = EndianConvert::Swap<UINT32>(result);
              break;
            }
    case TYPE_US  : {
              short tmp;
              fileDICOM.read((char*)&tmp,2);
              if (bNeedsEndianConversion) tmp = EndianConvert::Swap<short>(tmp);
              result = tmp;
              break;
            }
    default : result = 0; break;
  }
  return result;
}


#ifdef DEBUG_DICOM
void DICOMParser::ParseUndefLengthSequence(ifstream& fileDICOM, short& iSeqGroupID, short& iSeqElementID, DICOMFileInfo& info, const bool bImplicit, const bool bNeedsEndianConversion, UINT32 iDepth) {
  for (int i = 0;i<int(iDepth)-1;i++) Console::printf("  ");
  Console::printf("iGroupID=%x iElementID=%x elementType=SEQUENCE (undef length)\n", iSeqGroupID, iSeqElementID);
#else
void DICOMParser::ParseUndefLengthSequence(ifstream& fileDICOM, short& , short& , DICOMFileInfo& info, const bool bImplicit, const bool bNeedsEndianConversion) {
#endif
  int iItemCount = 0;
  UINT32 iData;

  string value;
  short iGroupID, iElementID;
  DICOM_eType elementType;

  do {
    fileDICOM.read((char*)&iData,4);

    if (iData == 0xE000FFFE) {
      iItemCount++;
      fileDICOM.read((char*)&iData,4);
      #ifdef DEBUG_DICOM
        for (UINT32 i = 0;i<iDepth;i++) Console::printf("  ");
        Console::printf("START ITEM\n");
      #endif
    } else if (iData == 0xE00DFFFE) {
      iItemCount--;
      fileDICOM.read((char*)&iData,4);
      #ifdef DEBUG_DICOM
        for (UINT32 i = 0;i<iDepth;i++) Console::printf("  ");
        Console::printf("END ITEM\n");
      #endif
    } else if (iData != 0xE0DDFFFE) fileDICOM.seekg(-4, ios_base::cur);


    if (iItemCount > 0) {
      ReadHeaderElemStart(fileDICOM, iGroupID, iElementID, elementType, iData, bImplicit, bNeedsEndianConversion);

      if (elementType == TYPE_SQ) {
        fileDICOM.read((char*)&iData,4);
        if (iData == 0xFFFFFFFF) {
          #ifdef DEBUG_DICOM
          ParseUndefLengthSequence(fileDICOM, iGroupID, iElementID, info, bImplicit, bNeedsEndianConversion, 1);
          #else
          ParseUndefLengthSequence(fileDICOM, iGroupID, iElementID, info, bImplicit, bNeedsEndianConversion);
          #endif
        } else {  
          // HACK: here we simply skip over the entire sequence
          value.resize(iData); 
          fileDICOM.read(&value[0],iData);
          value = "SKIPPED EXPLICIT SEQUENCE";
        }
      } else {

        if (iData == 0xFFFFFFFF) {
          #ifdef DEBUG_DICOM
            ParseUndefLengthSequence(fileDICOM, iGroupID, iElementID, info, bImplicit, bNeedsEndianConversion, iDepth+1); 
          #else
            ParseUndefLengthSequence(fileDICOM, iGroupID, iElementID, info, bImplicit, bNeedsEndianConversion); 
          #endif
        } else {
          value.resize(iData);
          fileDICOM.read(&value[0],iData);
          #ifdef DEBUG_DICOM
            for (UINT32 i = 0;i<iDepth;i++) Console::printf("  ");
            Console::printf("iGroupID=%x iElementID=%x elementType=%s value=%s\n", iGroupID, iElementID, DICOM_TypeStrings[int(elementType)].c_str(), value.c_str());
          #endif
        }
      }
    }

  } while (iData != 0xE0DDFFFE);
  fileDICOM.read((char*)&iData,4);

#ifdef DEBUG_DICOM
  for (UINT32 i = 0;i<iDepth;i++) Console::printf("  ");
  Console::printf("END SEQUENCE\n");
#endif

}

void DICOMParser::SkipUnusedElement(ifstream& fileDICOM, string& value, const UINT32 iElemLength) {
  value.resize(iElemLength);
  fileDICOM.read(&value[0],iElemLength);
}

bool DICOMParser::GetDICOMFileInfo(const string& strFilename, DICOMFileInfo& info) {

  #ifdef DEBUG_DICOM
    Console::printf("Processing file %s\n",strFilename.c_str());
  #endif

  struct stat stat_buf;
  
  bool bImplicit    = false;
  info.m_bIsJPEGEncoded = false;
  bool bNeedsEndianConversion = EndianConvert::IsBigEndian();

  info.m_strFileName = strFilename;
  info.m_wstrFileName = wstring(strFilename.begin(), strFilename.end());
  info.m_ivSize.z = 1; // default if lsices does not apear in the dicom

  // check for basic properties
  if (!SysTools::GetFileStats(strFilename, stat_buf)) return false;  // file exits ?
  if (stat_buf.st_size < 128+4) return false;                // file has minimum length ?

  // open file
  ifstream fileDICOM(strFilename.c_str(), ios::in | ios::binary);
  
  fileDICOM.seekg(128);  // skip first 128 bytes

  char DICM[4];
  fileDICOM.read(DICM,4);
  if (DICM[0] != 'D' || DICM[1] != 'I' || DICM[2] != 'C' || DICM[3] != 'M') return false;  // Look for 'D', 'I', 'C', 'M'

  // Ok, at this point we are very sure that we are dealing with a DICOM File, lets find out the dimensions, the sequence numbers
  
  string value;
  float fSliceSpacing = 0;
  short iGroupID, iElementID;
  UINT32 iElemLength;
  DICOM_eType elementType;

  // read metadata block
  ReadHeaderElemStart(fileDICOM, iGroupID, iElementID, elementType, iElemLength, bImplicit, info.m_bIsBigEndian);
  while (iGroupID == 0x2) {
    switch (iElementID) {
      case 0x0 : {  // File Meta Elements Group Len
            int iMetaLength;            // unused at this time
            fileDICOM.read((char*)&iMetaLength,4);
           } break;
      case 0x1 : {  // Version
            iElemLength = 6;
            value.resize(iElemLength);
            fileDICOM.read(&value[0],iElemLength);
           } break;
      case 0x10 : {  // Parse Type to find out endianess
            value.resize(iElemLength);
            fileDICOM.read(&value[0],iElemLength);
            if (value[iElemLength-1] == 0) value.resize(iElemLength-1);

            if (value == "1.2.840.10008.1.2") {   // Implicit VR Little Endian
              bImplicit = true;
              bNeedsEndianConversion = EndianConvert::IsBigEndian();
              info.m_bIsBigEndian = false;
              #ifdef DEBUG_DICOM
                Console::printf("DICOM file is Implicit VR Little Endian\n");
              #endif
            } else if (value == "1.2.840.10008.1.2.1") { // Explicit VR Little Endian
              bImplicit = false;
              bNeedsEndianConversion = EndianConvert::IsBigEndian();
              info.m_bIsBigEndian = false;
              #ifdef DEBUG_DICOM
                Console::printf("DICOM file is Explicit VR Little Endian\n");
              #endif
            } else if (value == "1.2.840.10008.1.2.2") { // Explicit VR Big Endian
              bImplicit = false;
              bNeedsEndianConversion = EndianConvert::IsLittleEndian();
              info.m_bIsBigEndian = true;
              #ifdef DEBUG_DICOM
                Console::printf("DICOM file is Explicit VR Big Endian\n");
              #endif
            } else if (value == "1.2.840.10008.1.2.4.50" ||   // JPEG Baseline            ( untested due to lack of example DICOMS)
                       value == "1.2.840.10008.1.2.4.51" ||   // JPEG Extended            ( untested due to lack of example DICOMS)
                       value == "1.2.840.10008.1.2.4.55" ||   // JPEG Progressive         ( untested due to lack of example DICOMS)
                       value == "1.2.840.10008.1.2.4.57" ||   // JPEG Lossless            ( untested due to lack of example DICOMS)
                       value == "1.2.840.10008.1.2.4.58" ||   // JPEG Lossless            ( untested due to lack of example DICOMS)
                       value == "1.2.840.10008.1.2.4.70" ||   // JPEG Lossless            ( untested due to lack of example DICOMS)
                       value == "1.2.840.10008.1.2.4.80" ||   // JPEG-LS Lossless         ( untested due to lack of example DICOMS)
                       value == "1.2.840.10008.1.2.4.81" ||   // JPEG-LS Near-lossless    ( untested due to lack of example DICOMS)
                       value == "1.2.840.10008.1.2.4.90" ||   // JPEG 2000 Lossless       ( untested due to lack of example DICOMS)
                       value == "1.2.840.10008.1.2.4.90" ) {  // JPEG 2000                ( untested due to lack of example DICOMS)
              info.m_bIsJPEGEncoded = true;
              bImplicit = false;
              bNeedsEndianConversion = EndianConvert::IsBigEndian();
              info.m_bIsBigEndian = false;
              #ifdef DEBUG_DICOM
                Console::printf("DICOM file is Explicit VR Big Endian\n");
              #endif
            } else {
              return false; // unsupported file format
            }
           } break;
      default : {
        value.resize(iElemLength);
        fileDICOM.read(&value[0],iElemLength);
      } break;
    }
    ReadHeaderElemStart(fileDICOM, iGroupID, iElementID, elementType, iElemLength, bImplicit, bNeedsEndianConversion);
  }
  
  do {
    
    if (elementType == TYPE_SQ) { // skip sequences (explicit)
      fileDICOM.read((char*)&iElemLength,4);
      if (iElemLength == 0xFFFFFFFF) {
        #ifdef DEBUG_DICOM
        ParseUndefLengthSequence(fileDICOM, iGroupID, iElementID, info, false, bNeedsEndianConversion, 1);
        #else
        ParseUndefLengthSequence(fileDICOM, iGroupID, iElementID, info, false, bNeedsEndianConversion);
        #endif
        value = "SEQUENCE";
      } else {  
        // HACK: here we simply skip over the entire sequence
        value.resize(iElemLength);
        fileDICOM.read(&value[0],iElemLength);
        value = "SKIPPED EXPLICIT SEQUENCE";
      }
    } else if (elementType == TYPE_Implicit && iElemLength == 0xFFFFFFFF) { // skip sequences (implicit)
        #ifdef DEBUG_DICOM
        ParseUndefLengthSequence(fileDICOM, iGroupID, iElementID, info, true, bNeedsEndianConversion, 1);
        #else
        ParseUndefLengthSequence(fileDICOM, iGroupID, iElementID, info, true, bNeedsEndianConversion);
        #endif
      value = "SEQUENCE";
    } else {
      switch (iGroupID) {
        case 0x8 : switch (iElementID) {
              case 0x22 : { // Acquisition Date
                    info.m_strAcquDate.resize(iElemLength);
                    fileDICOM.read(&info.m_strAcquDate[0],iElemLength);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_strAcquDate << " (Acquisition Date: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    } break;
              case 0x32 : { // Acquisition Time
                    info.m_strAcquTime.resize(iElemLength);
                    fileDICOM.read(&info.m_strAcquTime[0],iElemLength);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_strAcquTime << " (Acquisition Time: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    } break;
              case 0x60 : { // Modality
                    info.m_strModality.resize(iElemLength);
                    fileDICOM.read(&info.m_strModality[0],iElemLength);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_strModality << " (Modality: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    } break;
              case 0x1030 : { // Study Description
                    info.m_strDesc.resize(iElemLength);
                    fileDICOM.read(&info.m_strDesc[0],iElemLength);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_strDesc << " (Study Description: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    } break;
              default : {
                SkipUnusedElement(fileDICOM, value, iElemLength);
              } break;
             } break;
        case 0x18 : switch (iElementID) {
              case 0x50 : { // Slice Thinkness
                    value.resize(iElemLength);
                    fileDICOM.read(&value[0],iElemLength);
                    info.m_fvfAspect.z = float(atof(value.c_str()));
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_fvfAspect.z << " (Slice Thinkness: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    } break;
              case 0x88 : { // Spacing
                    value.resize(iElemLength);
                    fileDICOM.read(&value[0],iElemLength);
                    fSliceSpacing = float(atof(value.c_str()));
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_fvfAspect.z << " (Slice Spacing: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    } break;
               default : {
                value.resize(iElemLength);
                fileDICOM.read(&value[0],iElemLength);
              } break;
            }  break;
        case 0x20 : switch (iElementID) {
              case 0x11 : { // Series Number
                    info.m_iSeries = GetUInt(fileDICOM, elementType, iElemLength, bNeedsEndianConversion);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_iSeries << " (Series Number: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    } break;
              case 0x13 : { // Image Number
                    info.m_iImageIndex = GetUInt(fileDICOM, elementType, iElemLength, bNeedsEndianConversion);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_iImageIndex << " (Image Number: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    } break;
              case 0x32 : // patient position
                {
                    value.resize(iElemLength);
                    fileDICOM.read(&value[0],iElemLength);

                    size_t iDelimiter = value.find_first_of("\\");

                    info.m_fvPatientPosition.x = float(atof(value.substr(0,iDelimiter).c_str()));

                    value = value.substr(iDelimiter+1, value.length());
                    iDelimiter = value.find_first_of("\\");
                    
                    info.m_fvPatientPosition.y = float(atof(value.substr(0,iDelimiter).c_str()));
                    info.m_fvPatientPosition.z = float(atof(value.substr(iDelimiter+1, value.length()).c_str()));
                    
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_fvfAspect.x << ", " << info.m_fvfAspect.y << " (x,y,z Patient Position: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                }  break;
              default : {
                value.resize(iElemLength);
                fileDICOM.read(&value[0],iElemLength);
              } break;
             } break;
        case 0x28 : switch (iElementID) {
              case  0x2 : // component count
                    if (elementType == TYPE_Implicit) elementType = TYPE_US;
                    info.m_iComponentCount = GetUInt(fileDICOM, elementType, iElemLength, bNeedsEndianConversion);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_iComponentCount << " (samples per pixel: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    break;
              case  0x8 : // Slices
                    if (elementType == TYPE_Implicit) elementType = TYPE_IS;
                    info.m_ivSize.z = GetUInt(fileDICOM, elementType, iElemLength, bNeedsEndianConversion);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_ivSize.z << " (Slices: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    break;
              case 0x10 : // Rows
                    if (elementType == TYPE_Implicit) elementType = TYPE_US;
                    info.m_ivSize.y = GetUInt(fileDICOM, elementType, iElemLength, bNeedsEndianConversion);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_ivSize.y << " (Rows: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                      break;
              case 0x11 : // Columns
                    if (elementType == TYPE_Implicit) elementType = TYPE_US;
                    info.m_ivSize.x = GetUInt(fileDICOM, elementType, iElemLength, bNeedsEndianConversion);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_ivSize.x << " (Columns: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                      break;
              case 0x30 : // x,y spacing
                {
                    value.resize(iElemLength);
                    fileDICOM.read(&value[0],iElemLength);

                    size_t iDelimiter = value.find_first_of("\\");

                    info.m_fvfAspect.x = float(atof(value.substr(0,iDelimiter).c_str()));
                    info.m_fvfAspect.y = float(atof(value.substr(iDelimiter+1, value.length()).c_str()));
                    
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_fvfAspect.x << ", " << info.m_fvfAspect.y << " (x,y spacing: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                }  break;
              case 0x100 : // Allocated
                    if (elementType == TYPE_Implicit) elementType = TYPE_US;
                    info.m_iAllocated = GetUInt(fileDICOM, elementType, iElemLength, bNeedsEndianConversion);
                    #ifdef DEBUG_DICOM
                    {
                        stringstream ss;
                      ss << info.m_iAllocated << " (Allocated bits: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                      break;
              case 0x101 : // Stored
                    if (elementType == TYPE_Implicit) elementType = TYPE_US;
                    info.m_iStored = GetUInt(fileDICOM, elementType, iElemLength, bNeedsEndianConversion);
                    #ifdef DEBUG_DICOM
                    {
                      stringstream ss;
                      ss << info.m_iStored << " (Stored bits: recognized and stored)";
                      value = ss.str();
                    }
                    #endif
                    break;
              default : {
                value.resize(iElemLength);
                fileDICOM.read(&value[0],iElemLength);
              } break;
             } break;
        default : {
          value.resize(iElemLength);
          fileDICOM.read(&value[0],iElemLength);
        } break;

      }
    }
    #ifdef DEBUG_DICOM
    if (value != "SEQUENCE") Console::printf("iGroupID=%x iElementID=%x elementType=%s value=%s\n", iGroupID, iElementID, DICOM_TypeStrings[int(elementType)].c_str(), value.c_str());
    #endif
          
    ReadHeaderElemStart(fileDICOM, iGroupID, iElementID, elementType, iElemLength, bImplicit, info.m_bIsBigEndian);
  } while (iGroupID != 0x7fe0 && elementType != TYPE_UN);


  if (elementType != TYPE_UN) {
    
    if (!bImplicit) {
      // for an explicit file we can actually check if we found the pixel data block (and not some color table)
      UINT32 iPixelDataSize = info.m_ivSize.volume() * info.m_iAllocated / 8;
      UINT32 iDataSizeInFile;
      fileDICOM.read((char*)&iDataSizeInFile,4);

      if (info.m_bIsJPEGEncoded) {
        unsigned char iJPEGID[2];
        while (!fileDICOM.eof()) {
          fileDICOM.read((char*)iJPEGID,2);
          if (iJPEGID[0] == 0xFF && iJPEGID[1] == 0xE0 ) break;
        }
        info.SetOffsetToData(UINT32(int(fileDICOM.tellg())-4));
      } else {
        if (iPixelDataSize != iDataSizeInFile) {
          elementType = TYPE_UN;
        } else info.SetOffsetToData(UINT32(fileDICOM.tellg()));
      }
    } else info.SetOffsetToData(UINT32(fileDICOM.tellg()));  // otherwise just believe we have found the right data block
  } 

  if (elementType == TYPE_UN) {
    // ok we encoutered some strange DICOM file (must likely that additional SIEMENS header) and found an unknown tag, 
    // so lets just march througth the rest of the file and search the magic 0x7fe0, then use the last one found

    size_t iPosition   = fileDICOM.tellg();
    fileDICOM.seekg(0,ios::end);
    size_t iFileLength = fileDICOM.tellg();
    fileDICOM.seekg(iPosition,ios::beg);

    UINT32 iPixelDataSize = info.m_iComponentCount * info.m_ivSize.volume() * info.m_iAllocated / 8;

    bool bOK = false;
    do {
      iGroupID = 0;
      iPosition = fileDICOM.tellg();

      while (!fileDICOM.eof() && iGroupID != 0x7fe0 && iPosition+iPixelDataSize < iFileLength) {
        iPosition++;
        fileDICOM.read((char*)&iGroupID,2);
      }

      // check if this 0x7fe0 is really a group ID
      if (iGroupID == 0x7fe0) {
        fileDICOM.seekg(-2, ios_base::cur);
        ReadHeaderElemStart(fileDICOM, iGroupID, iElementID, elementType, iElemLength, bImplicit, info.m_bIsBigEndian);
        bOK = (elementType == TYPE_OW || elementType == TYPE_OB || elementType == TYPE_OF );

        if (bOK) {
          if (!bImplicit) {
            UINT32 iPixelDataSize = info.m_ivSize.volume() * info.m_iAllocated / 8;
            UINT32 iDataSizeInFile;
            fileDICOM.read((char*)&iDataSizeInFile,4);

            if (iPixelDataSize != iDataSizeInFile) bOK = false;
          }

          info.SetOffsetToData(int(fileDICOM.tellg())); 
        }
        else 
          fileDICOM.seekg(-8, ios_base::cur);
      }


    } while(iGroupID == 0x7fe0);

    if (!bOK) { // ok everthing failed than let's just use the data we have so far, and let's hope that the file ends with the data
      info.SetOffsetToData(UINT32(iFileLength - size_t(iPixelDataSize)));
    }

  }


  fileDICOM.close();

  return info.m_ivSize.volume() != 0;
}

/*************************************************************************************/

SimpleDICOMFileInfo::SimpleDICOMFileInfo(const std::string& strFileName) :
  SimpleFileInfo(strFileName),
  m_fvPatientPosition(0,0,0),
  m_iOffsetToData(0)
{
}

SimpleDICOMFileInfo::SimpleDICOMFileInfo(const std::wstring& wstrFileName) :   
  SimpleFileInfo(wstrFileName),
  m_fvPatientPosition(0,0,0),
  m_iOffsetToData(0)
{
}

SimpleDICOMFileInfo::SimpleDICOMFileInfo() :   
  SimpleFileInfo(),
  m_fvPatientPosition(0,0,0),
  m_iOffsetToData(0)
{
}

SimpleDICOMFileInfo::SimpleDICOMFileInfo(const SimpleDICOMFileInfo* other) :
  SimpleFileInfo(other), 
  m_fvPatientPosition(other->m_fvPatientPosition),
  m_iOffsetToData(other->m_iOffsetToData)
{
}

bool SimpleDICOMFileInfo::GetData(void* pData, UINT32 iLength, UINT32 iOffset) {
  ifstream fs;
  fs.open(m_strFileName.c_str(),fstream::binary);
  if (fs.fail()) return false;

  fs.seekg(m_iOffsetToData+iOffset, ios_base::cur);
  fs.read((char*)pData, iLength);

  fs.close();
  return true;
}

SimpleFileInfo* SimpleDICOMFileInfo::clone() {
  SimpleDICOMFileInfo* pSimpleDICOMFileInfo = new SimpleDICOMFileInfo(this);
  return (SimpleFileInfo*)pSimpleDICOMFileInfo;
}


/*************************************************************************************/

DICOMFileInfo::DICOMFileInfo() : 
  SimpleDICOMFileInfo(),
  m_iSeries(0),
  m_ivSize(0,0,1),
  m_fvfAspect(1,1,1),
  m_iAllocated(0),
  m_iStored(0),
  m_iComponentCount(1),
  m_bIsBigEndian(false),
  m_bIsJPEGEncoded(false),
  m_strAcquDate(""),
  m_strAcquTime(""),
  m_strModality(""),
  m_strDesc("")
{}

DICOMFileInfo::DICOMFileInfo(const std::string& strFileName) : 
  SimpleDICOMFileInfo(strFileName),
  m_iSeries(0),
  m_ivSize(0,0,1),
  m_fvfAspect(1,1,1),
  m_iAllocated(0),
  m_iStored(0),
  m_iComponentCount(1),
  m_bIsBigEndian(false),
  m_bIsJPEGEncoded(false),
  m_strAcquDate(""),
  m_strAcquTime(""),
  m_strModality(""),
  m_strDesc("")
{}


DICOMFileInfo::DICOMFileInfo(const std::wstring& wstrFileName) : 
  SimpleDICOMFileInfo(wstrFileName),
  m_iSeries(0),
  m_ivSize(0,0,1),
  m_fvfAspect(1,1,1),
  m_iAllocated(0),
  m_iStored(0),
  m_iComponentCount(1),
  m_bIsBigEndian(false),
  m_bIsJPEGEncoded(false),
  m_strAcquDate(""),
  m_strAcquTime(""),
  m_strModality(""),
  m_strDesc("")
{}

void DICOMFileInfo::SetOffsetToData(const UINT32 iOffset) {
  m_iOffsetToData = iOffset;
  m_iDataSize = m_iComponentCount*m_ivSize.volume()*m_iAllocated/8;
}

/*************************************************************************************/

DICOMStackInfo::DICOMStackInfo() :
  FileStackInfo(),
  m_iSeries(0),
  m_strAcquDate(""),
  m_strAcquTime(""),
  m_strModality("")
{}

DICOMStackInfo::DICOMStackInfo(const DICOMFileInfo* fileInfo) :
  FileStackInfo(fileInfo->m_ivSize, fileInfo->m_fvfAspect, fileInfo->m_iAllocated, fileInfo->m_iStored,
                fileInfo->m_iComponentCount, fileInfo->m_bIsBigEndian, fileInfo->m_bIsJPEGEncoded, fileInfo->m_strDesc, "DICOM"),
  m_iSeries(fileInfo->m_iSeries),
  m_strAcquDate(fileInfo->m_strAcquDate),
  m_strAcquTime(fileInfo->m_strAcquTime),
  m_strModality(fileInfo->m_strModality)
{
  m_Elements.push_back(new SimpleDICOMFileInfo(fileInfo));
}

DICOMStackInfo::DICOMStackInfo(const DICOMStackInfo* other) :
  m_iSeries(other->m_iSeries),
  m_strAcquDate(other->m_strAcquDate),
  m_strAcquTime(other->m_strAcquTime),
  m_strModality(other->m_strModality)
{
  m_ivSize          = other->m_ivSize;
  m_fvfAspect       = other->m_fvfAspect;
  m_iAllocated      = other->m_iAllocated;
  m_iStored         = other->m_iStored;
  m_iComponentCount = other->m_iComponentCount;
  m_bIsBigEndian    = other->m_bIsBigEndian;
  m_bIsJPEGEncoded  = other->m_bIsJPEGEncoded;
  m_strDesc         = other->m_strDesc;
  m_strFileType     = other->m_strFileType;

  for (size_t i=0;i<other->m_Elements.size();i++) {
    SimpleDICOMFileInfo* e = new SimpleDICOMFileInfo((SimpleDICOMFileInfo*)other->m_Elements[i]);
    m_Elements.push_back(e);
  }
}


bool DICOMStackInfo::Match(const DICOMFileInfo* info) {
  if (m_iSeries       == info->m_iSeries &&
    m_ivSize          == info->m_ivSize &&
    m_iAllocated      == info->m_iAllocated &&
    m_iStored         == info->m_iStored &&
    m_iComponentCount == info->m_iComponentCount &&
    m_fvfAspect       == info->m_fvfAspect &&
    m_bIsBigEndian    == info->m_bIsBigEndian &&
    m_bIsJPEGEncoded  == info->m_bIsJPEGEncoded &&
    m_strAcquDate     == info->m_strAcquDate &&
    //m_strAcquTime   == info->m_strAcquTime &&
    m_strModality     == info->m_strModality &&
    m_strDesc         == info->m_strDesc) {

    std::vector<SimpleFileInfo*>::iterator iter;

    for (iter = m_Elements.begin(); iter < m_Elements.end(); iter++) {
      if ((*iter)->m_iImageIndex > info->m_iImageIndex) break;
    }

    m_Elements.insert(iter,new SimpleDICOMFileInfo(info));

    return true;
  } else return false;
}
