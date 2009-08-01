/*
   The MIT License

   Copyright (c) 2009 Institut of Mechanics and Fluid Dynamics,
   TU Bergakademie Freiberg.


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
  \file    REKConverter.h
  \author  Andre Liebscher
           Institut of Mechanics and Fluid Dynamics 
           TU Bergakademie Freiberg
  \version 1.0
  \date    March 2009
*/


#pragma once

#ifndef REKCONVERTER_H
#define REKCONVERTER_H

#include "../StdTuvokDefines.h"
#include "RAWConverter.h"

/** A converter for REK volumes used by Frauenhofer EZRT.  */
class REKConverter : public RAWConverter {
public:
  REKConverter();
  virtual ~REKConverter() {}

  virtual bool ConvertToRAW(const std::string& strSourceFilename, 
                            const std::string& strTempDir, bool bNoUserInteraction,
                            UINT64& iHeaderSkip, UINT64& iComponentSize, UINT64& iComponentCount, 
                            bool& bConvertEndianess, bool& bSigned, bool& bIsFloat, UINTVECTOR3& vVolumeSize,
                            FLOATVECTOR3& vVolumeAspect, std::string& strTitle,
                            UVFTables::ElementSemanticTable& eType, std::string& strIntermediateFile,
                            bool& bDeleteIntermediateFile);

  /// unimplemented!
  virtual bool ConvertToNative(const std::string& strRawFilename, const std::string& strTargetFilename, UINT64 iHeaderSkip,
                               UINT64 iComponentSize, UINT64 iComponentCount, bool bSigned, bool bFloatingPoint, 
                               UINTVECTOR3 vVolumeSize,FLOATVECTOR3 vVolumeAspect, bool bNoUserInteraction);

  virtual bool CanExportData() const {return false;}

private:
  template<typename T, size_t N>
  T Parse( char data[], bool convertEndianess = false ) {
    char buffer[N];

    for( size_t i = 0; i < N; i++ ) {
      buffer[i] = convertEndianess ? data[N-i+1] : data[i];
    }
    
    return *reinterpret_cast<T*>(buffer);
  }
};

#endif // REKCONVERTER_H
