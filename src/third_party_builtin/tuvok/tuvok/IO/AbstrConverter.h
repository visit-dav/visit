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
  \file    AbstrConverter.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    December 2008
*/


#pragma once

#ifndef ABSTRCONVERTER_H
#define ABSTRCONVERTER_H

#include "../StdTuvokDefines.h"
#include "../IO/UVF/UVF.h"
#include "../Basics/Vectors.h"

class MasterController;

class RangeInfo {
public:
  UINTVECTOR3               m_vDomainSize;
  FLOATVECTOR3              m_vAspect;
  UINT64                    m_iComponentSize;
  int                       m_iValueType;
  std::pair<double, double> m_fRange;
  std::pair<INT64, INT64>   m_iRange;
  std::pair<UINT64, UINT64> m_uiRange;  
};

class AbstrConverter {
public:
  virtual ~AbstrConverter() {}

  virtual bool ConvertToUVF(const std::string& strSourceFilename, const std::string& strTargetFilename, 
                            const std::string& strTempDir,
                            bool bNoUserInteraction) = 0;

  virtual bool ConvertToRAW(const std::string& strSourceFilename, 
                            const std::string& strTempDir, bool bNoUserInteraction,                                                       
                            UINT64& iHeaderSkip, UINT64& iComponentSize, UINT64& iComponentCount, 
                            bool& bConvertEndianess, bool& bSigned, bool& bIsFloat, UINTVECTOR3& vVolumeSize,
                            FLOATVECTOR3& vVolumeAspect, std::string& strTitle,
                            UVFTables::ElementSemanticTable& eType, std::string& strIntermediateFile,
                            bool& bDeleteIntermediateFile)  = 0;

  virtual bool ConvertToNative(const std::string& strRawFilename, const std::string& strTargetFilename, UINT64 iHeaderSkip,
                               UINT64 iComponentSize, UINT64 iComponentCount, bool bSigned, bool bFloatingPoint,
                               UINTVECTOR3 vVolumeSize, FLOATVECTOR3 vVolumeAspect,
                               bool bNoUserInteraction) = 0;

  virtual bool Analyze(const std::string& strSourceFilename, const std::string& strTempDir, 
                       bool bNoUserInteraction, RangeInfo& info) = 0;
  
  const std::vector<std::string>& SupportedExt() {return m_vSupportedExt;}
  virtual const std::string& GetDesc() {return m_vConverterDesc;}

protected:
  std::string               m_vConverterDesc;
  std::vector<std::string>  m_vSupportedExt;

  static const std::string Process8BitsTo8Bits(UINT64 iHeaderSkip, const std::string& strFilename, const std::string& strTargetFilename, UINT64 iSize, bool bSigned, Histogram1DDataBlock& Histogram1D);
  static const std::string QuantizeShortTo12Bits(UINT64 iHeaderSkip, const std::string& strFilename, const std::string& strTargetFilename, UINT64 iSize, bool bSigned, Histogram1DDataBlock& Histogram1D);
  static const std::string QuantizeIntTo12Bits(UINT64 iHeaderSkip, const std::string& strFilename, const std::string& strTargetFilename, UINT64 iSize, bool bSigned, Histogram1DDataBlock& Histogram1D);
  static const std::string QuantizeLongTo12Bits(UINT64 iHeaderSkip, const std::string& strFilename, const std::string& strTargetFilename, UINT64 iSize, bool bSigned, Histogram1DDataBlock& Histogram1D);
  static const std::string QuantizeFloatTo12Bits(UINT64 iHeaderSkip, const std::string& strFilename, const std::string& strTargetFilename, UINT64 iSize, Histogram1DDataBlock& Histogram1D);
  static const std::string QuantizeDoubleTo12Bits(UINT64 iHeaderSkip, const std::string& strFilename, const std::string& strTargetFilename, UINT64 iSize, Histogram1DDataBlock& Histogram1D);
};

#endif // ABSTRCONVERTER_H
