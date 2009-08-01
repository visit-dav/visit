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
  \file    BOVConverter.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/
#include <cstdarg>
#include <sstream>
#include "BOVConverter.h"
#include "IO/KeyValueFileParser.h"
#include "Controller/Controller.h"
#include "Basics/SysTools.h"

enum DataType {
  UnknownType,
  Float,
  Integer,
  Char
};

static DataType bov_type(const KeyValueFileParser &kvp);
static const KeyValPair *find_header(const KeyValueFileParser &,
                                     const char *s, ...);

BOVConverter::BOVConverter()
{
  m_vConverterDesc = "Brick of Values";
  m_vSupportedExt.push_back("BOV");
}

bool BOVConverter::ConvertToRAW(
                            const std::string& strSourceFilename,
                            const std::string&, bool,
                            UINT64& iHeaderSkip, UINT64& iComponentSize,
                            UINT64& iComponentCount, bool& bConvertEndianness,
                            bool& bSigned, bool& bIsFloat,
                            UINTVECTOR3& vVolumeSize,
                            FLOATVECTOR3& vVolumeAspect, std::string& strTitle,
                            UVFTables::ElementSemanticTable& eType,
                            std::string& strIntermediateFile,
                            bool& bDeleteIntermediateFile)
{
  MESSAGE("Attempting to convert BOV: %s", strSourceFilename.c_str());
  KeyValueFileParser hdr(strSourceFilename.c_str());

  if(!hdr.FileReadable()) {
    T_ERROR("Could not parse %s; could not open.", strSourceFilename.c_str());
    return false;
  }
  KeyValPair *file = hdr.GetData("DATA_FILE");
  const KeyValPair *size = find_header(hdr, "DATA SIZE", "DATA_SIZE", NULL);
  KeyValPair *aspect_x = hdr.GetData("BRICK X_AXIS");
  KeyValPair *aspect_y = hdr.GetData("BRICK Y_AXIS");
  KeyValPair *aspect_z = hdr.GetData("BRICK Z_AXIS");
  strTitle = "BOV Volume";
  eType = UVFTables::ES_UNDEFINED;

  {
    iHeaderSkip = 0;
    strIntermediateFile = SysTools::GetPath(strSourceFilename) + file->strValue;
    MESSAGE("Reading data from %s", strIntermediateFile.c_str());
    bDeleteIntermediateFile = false;
  }
  {
    std::istringstream ss(size->strValue);
    ss >> vVolumeSize[0] >> vVolumeSize[1] >> vVolumeSize[2];
    MESSAGE("Dimensions: %ux%ux%u",
            vVolumeSize[0], vVolumeSize[1], vVolumeSize[2]);
    iHeaderSkip = 0;
  }
  {
    iComponentCount = 1;
    iComponentSize = 8;
    bConvertEndianness = false;
    bIsFloat = false;
    bSigned = false;

    if(bov_type(hdr) == Float) {
      iComponentSize = 32;
      bSigned = true;
      bIsFloat = true;
    }
    MESSAGE("%lu-bit %s, %s data", iComponentSize,
            bSigned ? "signed" : "unsigned",
            bIsFloat ? "floating point" : "integer(?)");
  }
  {
    // e.g. "BRICK X_AXIS 1.000 0.000 0.000".  Might not exist.
    vVolumeAspect[0] = vVolumeAspect[1] = vVolumeAspect[2] = 1.0;
    if(aspect_x && aspect_y && aspect_z) {
      std::stringstream x(aspect_x->strValue);
      std::stringstream y(aspect_y->strValue);
      std::stringstream z(aspect_z->strValue);
      float junk;
      x >> vVolumeAspect[0];
      y >> junk >> vVolumeAspect[1];
      z >> junk >> junk >> vVolumeAspect[2];
    }
    MESSAGE("Aspect: %2.2fx%2.2fx%2.2f",
            vVolumeAspect[0], vVolumeAspect[1], vVolumeAspect[2]);
  }
  return true;
}

bool BOVConverter::ConvertToNative(
                               const std::string&,
                               const std::string&,
                               UINT64, UINT64,
                               UINT64, bool,
                               bool,  UINTVECTOR3,
                               FLOATVECTOR3,
                               bool)
{
  /// \todo implement?
  return false; // unsupported.
}

static DataType
bov_type(const KeyValueFileParser &kvp)
{
  const KeyValPair *format = NULL;
  DataType retval = UnknownType;

  // Search a list of strings until we find one.
  static const char formats[][16] = {
    "DATA FORMAT", "DATA_FORMAT", "FORMAT"
  };
  for(size_t i=0; i < sizeof(formats); ++i) {
    format = kvp.GetData(formats[i]);
    if(format != NULL) { break; }
  }

  // Might not have found any of those.
  if(format == NULL) {
    T_ERROR("Could not determine data format.  "
            "Is this a BOV file?  Potentially corrupt.");
    return UnknownType;
  }

  // Data type is specified by strings, but the exact string isn't quite
  // uniform.  Another search.
  static const char floats[][16] = {
    "FLOAT", "FLOATS"
  };
  for(size_t i=0; i < sizeof(floats); ++i) {
    if(format->strValueUpper == std::string(floats[i])) {
      retval = Float;
      break;
    }
  }

  if(retval != Float) {
    MESSAGE("Developer does not have integer or char datasets to test"
            " with; such a dataset will be reported as unknown type!");
  }
  return retval;
}

// Given a list of strings, returns the first KeyValPair that exists.  Useful
// since some bovs use "_" between words in a key, some don't...
// The argument list must be NULL-terminated.
static const KeyValPair *
find_header(const KeyValueFileParser &kvp, const char *s, ...)
{
  va_list args;
  const KeyValPair *ret;

  // First try 's'.
  ret = kvp.GetData(s);
  if(ret) { return ret; }

  va_start(args, s);
    const char *key = va_arg(args, const char *);
    while(key) {
      ret = kvp.GetData(key);
      if(ret) { return ret; }
    }
  va_end(args);
  return NULL;
}
