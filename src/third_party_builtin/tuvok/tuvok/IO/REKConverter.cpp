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
  \file    REKConverter.cpp
  \author  Andre Liebscher
           Institut of Mechanics and Fluid Dynamics 
           TU Bergakademie Freiberg
  \date    March 2009
*/

#include <fstream>
#include <sstream>
#include <cstring>
#include "REKConverter.h"
#include "Controller/Controller.h"
#include "boost/cstdint.hpp"

using namespace std;


REKConverter::REKConverter()
{
  m_vConverterDesc = "Fraunhofer EZRT Volume";
  m_vSupportedExt.push_back("REK");
}

bool
REKConverter::ConvertToRAW(const std::string& strSourceFilename,
                                  const std::string&,
                                  bool, UINT64& iHeaderSkip,
                                  UINT64& iComponentSize,
                                  UINT64& iComponentCount,
                                  bool& bConvertEndianess, bool& bSigned,
                                  bool& bIsFloat, UINTVECTOR3& vVolumeSize,
                                  FLOATVECTOR3& vVolumeAspect,
                                  std::string& strTitle,
                                  UVFTables::ElementSemanticTable& eType,
                                  std::string& strIntermediateFile,
                                  bool& bDeleteIntermediateFile)
{
  MESSAGE("Attempting to convert REK dataset %s", strSourceFilename.c_str());

  // Read header an check for "magic" values of the REK file
  ifstream fileData(strSourceFilename.c_str(), ifstream::in | ifstream::binary);
  char buffer[2048];

  if(fileData.is_open()) {
    fileData.read( buffer, sizeof(buffer) );
    unsigned char ff[] = { 255, 255, 255, 255 };
    if( memcmp(&buffer[116], &ff, 4) != 0 ) {
      WARNING("The file %s is not a REK file", strSourceFilename.c_str());
      fileData.close();
      return false;
    }
  } else {
    WARNING("Could not open REK file %s", strSourceFilename.c_str());
    return false;
  }
  fileData.close();
  
  // standard values which are always true (I guess)
  strTitle = "Fraunhofer EZRT";
  eType = UVFTables::ES_UNDEFINED;
  vVolumeAspect = FLOATVECTOR3(1,1,1);
  bSigned = false;
  bIsFloat = false;
  iComponentCount = 1;
  strIntermediateFile = strSourceFilename;
  bDeleteIntermediateFile = false;
  
  // read file format from header - first try to guess endieness from offset 4-5 (bits per pixel)
  // Anyway, I do not think that anyone would ever encounter such a file stored in big endian encoding.
  bConvertEndianess = Parse<boost::uint16_t, 2>( &buffer[4] ) > 32;
  
  vVolumeSize[0] = Parse<boost::uint16_t, 2>( &buffer[0], bConvertEndianess );
  vVolumeSize[1] = Parse<boost::uint16_t, 2>( &buffer[2], bConvertEndianess );
  vVolumeSize[2] = Parse<boost::uint16_t, 2>( &buffer[6], bConvertEndianess );
  iComponentSize = Parse<boost::uint16_t, 2>( &buffer[4], bConvertEndianess );
  iHeaderSkip = Parse<boost::uint16_t, 2>( &buffer[8], bConvertEndianess );

  return true;
}

// unimplemented!
bool
REKConverter::ConvertToNative(const std::string&,
                                     const std::string&,
                                     UINT64, UINT64, 
                                     UINT64, bool,
                                     bool,
                                     UINTVECTOR3,
                                     FLOATVECTOR3,
                                     bool)
{
  return false;
}
