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
  \file    StkConverter.cpp
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/
#include "StkConverter.h"
#include "../Basics/SysTools.h"
#include "../3rdParty/boost/cstdint.hpp"
#include "../3rdParty/tiff/tiffio.h"
#include "../Controller/Controller.h"

struct stk {
  boost::uint32_t x,y,z;      ///< dimensions
  boost::uint16_t bpp;        ///< bits per pixel
  boost::uint16_t samples;    ///< number of components per pixel
};

static bool stk_read_metadata(TIFF *, struct stk &);
static void stk_read_write_strips(TIFF *, LargeRAWFile &);

StkConverter::StkConverter()
{
  m_vConverterDesc = "Stk Volume (Metamorph)";
  m_vSupportedExt.push_back("STK");
}

bool
StkConverter::ConvertToRAW(const std::string& strSourceFilename,
                           const std::string& strTempDir,
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
  AbstrDebugOut& dbg = Controller::Debug::Out();
  dbg.Message(_func_, "Attempting to convert stk file: %s",
              strSourceFilename.c_str());

  TIFF *tif = TIFFOpen(strSourceFilename.c_str(), "r");
  if(tif == NULL) {
    dbg.Error(_func_, "Could not open %s", strSourceFilename.c_str());
    return false;
  }
  struct stk metadata;
  stk_read_metadata(tif, metadata);
  dbg.Message(_func_, "%ux%ux%u %s", metadata.x, metadata.y, metadata.z,
                                     m_vConverterDesc.c_str());
  dbg.Message(_func_, "%hu bits per component.", metadata.bpp);
  dbg.Message(_func_, "%hu component%s.", metadata.samples,
              (metadata.samples == 1) ? "" : "s");
  // copy that metadata into Tuvok variables.
  iComponentSize = metadata.bpp;
  iComponentCount = metadata.samples;
  vVolumeSize[0] = metadata.x;
  vVolumeSize[1] = metadata.y;
  vVolumeSize[2] = metadata.z;

  // IIRC libtiff handles all the endian issues for us.
  bConvertEndianess = false;

  // One might consider setting the values below explicitly as bugs, but we're
  // not quite sure where to read these from.  In any case, we don't have any
  // data for which these settings are invalid.
  bSigned = false;
  bIsFloat = false;
  vVolumeAspect[0] = 1;
  vVolumeAspect[1] = 1;
  vVolumeAspect[2] = 1;

  strTitle = "STK Volume";
  eType = UVFTables::ES_UNDEFINED;

  // Create an intermediate file to hold the data.
  iHeaderSkip = 0;
  strIntermediateFile = strTempDir +
                        SysTools::GetFilename(strSourceFilename) + ".binary";
  LargeRAWFile binary(strIntermediateFile);
  binary.Create(iComponentSize/8 * iComponentCount * vVolumeSize.volume());
  if(!binary.IsOpen()) {
    dbg.Error(_func_, "Could not create binary file %s",
              strIntermediateFile.c_str());
    TIFFClose(tif);
    return false;
  }
  // Populate the intermediate file.  We just run through each strip and write
  // it out; technically, this is not kosher for Tuvok, since a single strip
  // might exceed INCORESIZE.  That said, I've never seen a strip which is
  // larger than 8192 bytes.
  stk_read_write_strips(tif, binary);
  bDeleteIntermediateFile = true;

  binary.Close();
  TIFFClose(tif);

  return true;
}

// unimplemented!
bool
StkConverter::ConvertToNative(const std::string&, const std::string&,
                              UINT64, UINT64, UINT64, bool, bool, UINTVECTOR3,
                              FLOATVECTOR3, bool)
{
  return false;
}

static bool
stk_read_metadata(TIFF *tif, struct stk &metadata)
{
  // read the number of bits per component from the tiff tag.
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &metadata.bpp);
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &metadata.x);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &metadata.y);
  // It's common for Stk files not to easily give the depth.
  if(TIFFGetField(tif, TIFFTAG_IMAGEDEPTH, &metadata.z) == 0) {
    // Depth not available as a tag; have to read it from the stk metadata.
    // In particular, we'll look up the UIC3Tag and count the number of values
    // in there.
    const ttag_t uic3tag = (ttag_t) 33630; // the tag is private.
    boost::uint16_t count;
    void *data;
    if(TIFFGetField(tif, uic3tag, &count, &data) == 0) {
      return false;  // UIC3 tag does not exist; this is not a stk.
    }
    // The data actually gives the per-slice spacing .. but we just ignore
    // that.  All we care about is how many slices there are.
    metadata.z = count;
  }
  metadata.samples = 1;
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &metadata.samples);
  return true;
}

static void
stk_read_write_strips(TIFF *tif, LargeRAWFile &raw)
{
  const tstrip_t n_strips = TIFFNumberOfStrips(tif);
  tdata_t buf = static_cast<tdata_t>(_TIFFmalloc(TIFFStripSize(tif)));
  for(tstrip_t s=0; s < n_strips; ++s) {
    /// @todo FIXME: don't assume the strip is raw; could be encoded.
    /// There's a `compression scheme' tag which probably details this.
    tsize_t n_bytes = TIFFReadRawStrip(tif, s, buf, static_cast<tsize_t>(-1));
    raw.WriteRAW(static_cast<unsigned char*>(buf), n_bytes);
  }
  _TIFFfree(buf);
}
