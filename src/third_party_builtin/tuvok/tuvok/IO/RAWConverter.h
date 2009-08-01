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
  \file    RAWConverter.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    December 2008
*/


#pragma once

#ifndef RAWCONVERTER_H
#define RAWCONVERTER_H

#include "../StdTuvokDefines.h"
#include "AbstrConverter.h"
#include <Controller/Controller.h>

template<class T> class MinMaxScanner {
public:
  MinMaxScanner(LargeRAWFile* file, T& minValue, T& maxValue, UINT64 iElemCount) {
    size_t iMaxElemCount = size_t(std::min<UINT64>(BLOCK_COPY_SIZE, iElemCount) / sizeof(T));
    T* pInData = new T[iMaxElemCount];

    UINT64 iPos = 0;
    while (iPos < iElemCount)  {
      size_t iRead = file->ReadRAW((unsigned char*)pInData, iMaxElemCount*sizeof(T))/sizeof(T);
      if (iRead == 0) break;

      for (size_t i = 0;i<iRead;i++) {
        if (minValue > pInData[i]) minValue = pInData[i];
        if (maxValue < pInData[i]) maxValue = pInData[i];
      }
      iPos += UINT64(iRead);
    }

    delete [] pInData;
  }
};

class RAWConverter : public AbstrConverter {
public:
  virtual ~RAWConverter() {}

  static bool ConvertRAWDataset(const std::string& strFilename,
                                const std::string& strTargetFilename,
                                const std::string& strTempDir,
                                UINT64 iHeaderSkip, UINT64 iComponentSize,
                                UINT64 iComponentCount,
                                bool bConvertEndianness, bool bSigned,
                                bool bIsFloat, UINTVECTOR3 vVolumeSize,
                                FLOATVECTOR3 vVolumeAspect,
                                const std::string& strDesc,
                                const std::string& strSource,
                                UVFTables::ElementSemanticTable
                                  eType=UVFTables::ES_UNDEFINED);

  static bool ExtractGZIPDataset(const std::string& strFilename,
                                 const std::string& strUncompressedFile,
                                 UINT64 iHeaderSkip);

  static bool ExtractBZIP2Dataset(const std::string& strFilename,
                                  const std::string& strUncompressedFile,
                                  UINT64 iHeaderSkip);

  static bool ParseTXTDataset(const std::string& strFilename,
                              const std::string& strBinaryFile,
                              UINT64 iHeaderSkip, UINT64 iComponentSize,
                              UINT64 iComponentCount, bool bSigned,
                              bool bIsFloat, UINTVECTOR3 vVolumeSize);

  static bool AppendRAW(const std::string& strRawFilename, UINT64 iHeaderSkip,
                        const std::string& strTargetFilename,
                        UINT64 iComponentSize,
                        bool bChangeEndianess=false,
                        bool bToSigned=false);

  virtual bool ConvertToNative(const std::string& strRawFilename,
                               const std::string& strTargetFilename,
                               UINT64 iHeaderSkip, UINT64 iComponentSize,
                               UINT64 iComponentCount, bool bSigned,
                               bool bFloatingPoint, UINTVECTOR3 vVolumeSize,
                               FLOATVECTOR3 vVolumeAspect,
                               bool bNoUserInteraction);

  virtual bool ConvertToUVF(const std::string& strSourceFilename,
                            const std::string& strTargetFilename,
                            const std::string& strTempDir,
                            bool bNoUserInteraction);

  virtual bool Analyze(const std::string& strSourceFilename,
                       const std::string& strTempDir,
                       bool bNoUserInteraction, RangeInfo& info);

  static bool Analyze(const std::string& strSourceFilename, UINT64 iHeaderSkip,
                      UINT64 iComponentSize, UINT64 iComponentCount,
                      bool bSigned, bool bFloatingPoint,
                      UINTVECTOR3 vVolumeSize,
                      RangeInfo& info);

  virtual bool CanExportData() const {return true;}

  /// Removes the given file or directory.  Warns if the file could not be
  /// deleted.
  /// @return true if the remove succeeded.
  static bool Remove(const std::string &, AbstrDebugOut &);

};

#endif // RAWCONVERTER_H
