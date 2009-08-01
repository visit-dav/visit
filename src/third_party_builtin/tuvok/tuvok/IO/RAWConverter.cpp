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
  \file    RAWConverter.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    December 2008
*/
#include <cerrno>
#include <cstring>
#include <3rdParty/bzip2/bzlib.h>
#include "boost/cstdint.hpp"

#include "RAWConverter.h"
#include "IOManager.h"  // for the size defines
#include <Basics/SysTools.h>
#include <IO/gzio.h>
#include "UVF/Histogram1DDataBlock.h"
#include "UVF/Histogram2DDataBlock.h"
#include "UVF/MaxMinDataBlock.h"
#include "UVF/RasterDataBlock.h"

using namespace std;
using boost::int64_t;

bool RAWConverter::ConvertRAWDataset(const string& strFilename, const string& strTargetFilename, const string& strTempDir,
                                     UINT64 iHeaderSkip, UINT64 iComponentSize, UINT64 iComponentCount, bool bConvertEndianness, bool bSigned, bool bIsFloat,
                                     UINTVECTOR3 vVolumeSize, FLOATVECTOR3 vVolumeAspect, const string& strDesc, const string& strSource, UVFTables::ElementSemanticTable eType)
{
  if (iComponentCount > 4) {
    T_ERROR("Currently, only up to four component data is supported.");
    return false;
  }

  if (iComponentSize < 16) bConvertEndianness = false; // catch silly user input

  MESSAGE("Converting RAW dataset %s to %s", strFilename.c_str(), strTargetFilename.c_str());

  string strSourceFilename;
  string tmpFilename0 = strTempDir+SysTools::GetFilename(strFilename)+".endianess";
  string tmpFilename1 = strTempDir+SysTools::GetFilename(strFilename)+".quantized";

  if (bConvertEndianness) {
    MESSAGE("Performing endianess conversion ...");

    if (iComponentSize != 16 && iComponentSize != 32 && iComponentSize != 64) {
      T_ERROR("Unable to endian convert anything but 16bit, 32bit, or 64bit values (requested %i)", iComponentSize);
      return false;
    }

    LargeRAWFile WrongEndianData(strFilename, iHeaderSkip);
    WrongEndianData.Open(false);

    if (!WrongEndianData.IsOpen()) {
      T_ERROR("Unable to open source file %s", strFilename.c_str());
      return false;
    }

    LargeRAWFile ConvEndianData(tmpFilename0);
    ConvEndianData.Create();

    if (!ConvEndianData.IsOpen()) {
      T_ERROR("Unable to open temp file %s for endianess conversion", tmpFilename0.c_str());
      WrongEndianData.Close();
      return false;
    }

    UINT64 ulFileLength = WrongEndianData.GetCurrentSize();
    size_t iBufferSize = min<size_t>(size_t(ulFileLength), size_t(BRICKSIZE*BRICKSIZE*BRICKSIZE*iComponentSize/8)); // hint: this must fit into memory otherwise other subsystems would break
    UINT64 ulBufferConverted = 0;

    unsigned char* pBuffer = new unsigned char[iBufferSize];

    while (ulBufferConverted < ulFileLength) {

      size_t iBytesRead = WrongEndianData.ReadRAW(pBuffer, iBufferSize);

      switch (iComponentSize) {
        case 16 : for (size_t i = 0;i<iBytesRead;i+=2)
                    EndianConvert::Swap<unsigned short>((unsigned short*)(pBuffer+i));
                  break;
        case 32 : for (size_t i = 0;i<iBytesRead;i+=4)
                    EndianConvert::Swap<float>((float*)(pBuffer+i));
                  break;
        case 64 : for (size_t i = 0;i<iBytesRead;i+=8)
                    EndianConvert::Swap<double>((double*)(pBuffer+i));
                  break;
      }

      size_t iBytesWritten = ConvEndianData.WriteRAW(pBuffer, iBytesRead);

      if (iBytesRead != iBytesWritten)  {
        T_ERROR("Read/Write error converting endianess from %s to %s", strFilename.c_str(), tmpFilename0.c_str());
        WrongEndianData.Close();
        ConvEndianData.Close();
        Remove(tmpFilename0, Controller::Debug::Out());
        delete [] pBuffer;
        return false;
      }

      ulBufferConverted += UINT64(iBytesWritten);
    }

    delete [] pBuffer;

    WrongEndianData.Close();
    ConvEndianData.Close();
    strSourceFilename = tmpFilename0;
    iHeaderSkip = 0;  // the new file is straigt raw without any header
  } else strSourceFilename = strFilename;

  Histogram1DDataBlock Histogram1D;

	switch (iComponentSize) {
    case 8 :
      // do not run the Process8BitsTo8Bits when we are dealing with unsigned color data, 
      // in that case only the histogram would be computed and we do not use in that case
      /// \todo change this if we want to support non-color multi-component data
      if (iComponentCount != 4 || bSigned) 
        strSourceFilename = Process8BitsTo8Bits(iHeaderSkip, strSourceFilename,
                                                tmpFilename1,
                                                iComponentCount*vVolumeSize.volume(),
                                                bSigned, &Histogram1D);
      break;
    case 16 :
      strSourceFilename = QuantizeShortTo12Bits(iHeaderSkip, strSourceFilename,
                                                tmpFilename1,
                                                iComponentCount*vVolumeSize.volume(),
                                                bSigned, &Histogram1D);
      break;
		case 32 :	
      if (bIsFloat)
        strSourceFilename = QuantizeFloatTo12Bits(iHeaderSkip,
                                                  strSourceFilename,
                                                  tmpFilename1,
                                                  iComponentCount*vVolumeSize.volume(),
                                                  &Histogram1D);
      else
        strSourceFilename = QuantizeIntTo12Bits(iHeaderSkip, strSourceFilename,
                                                tmpFilename1,
                                                iComponentCount*vVolumeSize.volume(),
                                                bSigned, &Histogram1D);
      iComponentSize = 16;
      break;
		case 64 :	
      if (bIsFloat) 
        strSourceFilename = QuantizeDoubleTo12Bits(iHeaderSkip,
                                                   strSourceFilename,
                                                   tmpFilename1,
                                                   iComponentCount*vVolumeSize.volume(),
                                                   &Histogram1D);
      else
        strSourceFilename = QuantizeLongTo12Bits(iHeaderSkip,
                                                 strSourceFilename,
                                                 tmpFilename1,
                                                 iComponentCount*vVolumeSize.volume(),
                                                 bSigned, &Histogram1D);
      iComponentSize = 16;
      break;
  }

  if (strSourceFilename == "")  {
    T_ERROR("Read/Write error quantizing to %s", strFilename.c_str());
    return false;
  }

  bool bQuantized;
  if (strSourceFilename == tmpFilename1) {
    bQuantized = true;

    // if we actually created two temp file so far we can delete the first one
    if (bConvertEndianness) {
      Remove(tmpFilename0, Controller::Debug::Out());
      bConvertEndianness = false;
    }

    iHeaderSkip = 0; // the new file is straigt raw without any header
  } else {
    bQuantized = false;
  }

  LargeRAWFile SourceData(strSourceFilename, iHeaderSkip);
  SourceData.Open(false);

  if (!SourceData.IsOpen()) {
    T_ERROR("Unable to open source file %s", strSourceFilename.c_str());
    return false;
  }

	wstring wstrUVFName(strTargetFilename.begin(), strTargetFilename.end());
	UVF uvfFile(wstrUVFName);

	UINT64 iLodLevelCount = 1;
  UINT32 iMaxVal = vVolumeSize.maxVal();

  while (iMaxVal > BRICKSIZE) {
    iMaxVal /= 2;
    iLodLevelCount++;
  }

	GlobalHeader uvfGlobalHeader;
  uvfGlobalHeader.bIsBigEndian = EndianConvert::IsBigEndian();
	uvfGlobalHeader.ulChecksumSemanticsEntry = UVFTables::CS_MD5;
	uvfFile.SetGlobalHeader(uvfGlobalHeader);

	RasterDataBlock dataVolume;

  if (strSource == "")
    dataVolume.strBlockID = (strDesc!="") ? strDesc + " volume converted by ImageVis3D" : "Volume converted by ImageVis3D";
  else
    dataVolume.strBlockID = (strDesc!="") ? strDesc + " volume converted from " + strSource + " by ImageVis3D" : "Volume converted from " + strSource + " by ImageVis3D";

	dataVolume.ulCompressionScheme = UVFTables::COS_NONE;
	dataVolume.ulDomainSemantics.push_back(UVFTables::DS_X);
	dataVolume.ulDomainSemantics.push_back(UVFTables::DS_Y);
	dataVolume.ulDomainSemantics.push_back(UVFTables::DS_Z);

	dataVolume.ulDomainSize.push_back(vVolumeSize.x);
	dataVolume.ulDomainSize.push_back(vVolumeSize.y);
	dataVolume.ulDomainSize.push_back(vVolumeSize.z);

	dataVolume.ulLODDecFactor.push_back(2);
	dataVolume.ulLODDecFactor.push_back(2);
	dataVolume.ulLODDecFactor.push_back(2);

	dataVolume.ulLODGroups.push_back(0);
	dataVolume.ulLODGroups.push_back(0);
	dataVolume.ulLODGroups.push_back(0);

	dataVolume.ulLODLevelCount.push_back(iLodLevelCount);

	vector<UVFTables::ElementSemanticTable> vSem;

	switch (iComponentCount) {
		case 3 : vSem.push_back(UVFTables::ES_RED);
				 vSem.push_back(UVFTables::ES_GREEN);
				 vSem.push_back(UVFTables::ES_BLUE); break;
		case 4 : vSem.push_back(UVFTables::ES_RED);
				 vSem.push_back(UVFTables::ES_GREEN);
				 vSem.push_back(UVFTables::ES_BLUE);
				 vSem.push_back(UVFTables::ES_ALPHA); break;
		default : for (UINT64 i = 0;i<iComponentCount;i++) vSem.push_back(eType);
	}

	dataVolume.SetTypeToVector(iComponentSize,
							               iComponentSize == 32 ? 23 : iComponentSize,
							               bSigned,
							               vSem);
	
	dataVolume.ulBrickSize.push_back(BRICKSIZE);
	dataVolume.ulBrickSize.push_back(BRICKSIZE);
	dataVolume.ulBrickSize.push_back(BRICKSIZE);

	dataVolume.ulBrickOverlap.push_back(BRICKOVERLAP);
	dataVolume.ulBrickOverlap.push_back(BRICKOVERLAP);
	dataVolume.ulBrickOverlap.push_back(BRICKOVERLAP);

	vector<double> vScale;
	vScale.push_back(vVolumeAspect.x);
	vScale.push_back(vVolumeAspect.y);
	vScale.push_back(vVolumeAspect.z);
	dataVolume.SetScaleOnlyTransformation(vScale);

  MaxMinDataBlock MaxMinData(static_cast<size_t>(iComponentCount));

	switch (iComponentSize) {
		case 8 :	
          switch (iComponentCount) {
            case 1 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<unsigned char,1>, SimpleMaxMin<unsigned char,1>, &MaxMinData, &Controller::Debug::Out()); break;
						case 2 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<unsigned char,2>, SimpleMaxMin<unsigned char, 2>, &MaxMinData, &Controller::Debug::Out()); break;
						case 3 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<unsigned char,3>, SimpleMaxMin<unsigned char, 3>, &MaxMinData, &Controller::Debug::Out()); break;
						case 4 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<unsigned char,4>, SimpleMaxMin<unsigned char, 4>, &MaxMinData, &Controller::Debug::Out()); break;
						default: T_ERROR("Unsupported iComponentCount %i for iComponentSize %i.", int(iComponentCount), int(iComponentSize)); uvfFile.Close(); SourceData.Close(); return false;
					} break;
		case 16 :
          switch (iComponentCount) {
						case 1 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<unsigned short,1>, SimpleMaxMin<unsigned short, 1>, &MaxMinData, &Controller::Debug::Out()); break;
						case 2 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<unsigned short,2>, SimpleMaxMin<unsigned short, 2>, &MaxMinData, &Controller::Debug::Out()); break;
						case 3 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<unsigned short,3>, SimpleMaxMin<unsigned short, 3>, &MaxMinData, &Controller::Debug::Out()); break;
						case 4 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<unsigned short,4>, SimpleMaxMin<unsigned short, 4>, &MaxMinData, &Controller::Debug::Out()); break;
						default: T_ERROR("Unsupported iComponentCount %i for iComponentSize %i.", int(iComponentCount), int(iComponentSize)); uvfFile.Close(); SourceData.Close(); return false;
					} break;
		case 32 :	
          switch (iComponentCount) {
						case 1 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<float,1>, SimpleMaxMin<float, 1>, &MaxMinData, &Controller::Debug::Out()); break;
						case 2 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<float,2>, SimpleMaxMin<float, 2>, &MaxMinData, &Controller::Debug::Out()); break;
						case 3 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<float,3>, SimpleMaxMin<float, 3>, &MaxMinData, &Controller::Debug::Out()); break;
						case 4 : dataVolume.FlatDataToBrickedLOD(&SourceData, strTempDir+"tempFile.tmp", CombineAverage<float,4>, SimpleMaxMin<float, 4>, &MaxMinData, &Controller::Debug::Out()); break;
						default: T_ERROR("Unsupported iComponentCount %i for iComponentSize %i.", int(iComponentCount), int(iComponentSize)); uvfFile.Close(); SourceData.Close(); return false;
					} break;
		default: T_ERROR("Unsupported iComponentSize %i.", int(iComponentSize)); uvfFile.Close(); SourceData.Close(); return false;
	}

	string strProblemDesc;
	if (!dataVolume.Verify(&strProblemDesc)) {
    T_ERROR("Verify failed with the following reason: %s", strProblemDesc.c_str());
    uvfFile.Close();
    SourceData.Close();
    if (bConvertEndianness) {
      Remove(tmpFilename0, Controller::Debug::Out());
    }
    if (bQuantized) {
      Remove(tmpFilename1, Controller::Debug::Out());
    }
		return false;
	}

	if (!uvfFile.AddDataBlock(&dataVolume,dataVolume.ComputeDataSize(), true)) {
    T_ERROR("AddDataBlock failed!");
    uvfFile.Close();
    SourceData.Close();
    if (bConvertEndianness) {
      Remove(tmpFilename0, Controller::Debug::Out());
    }
    if (bQuantized) {
      Remove(tmpFilename1, Controller::Debug::Out());
    }
		return false;
	}

  // do compute histograms when we are dealing with color data
  /// \todo change this if we want to support non color multi component data
  if (iComponentCount != 4) {

    // if no resampling was perfomed above we need to compute the 1d histogram here
    if (Histogram1D.GetHistogram().empty()) {
      MESSAGE("Computing 1D Histogram...");
      if (!Histogram1D.Compute(&dataVolume)) {
        T_ERROR("Computation of 1D Histogram failed!");
        uvfFile.Close();
        SourceData.Close();
        if (bConvertEndianness) {
          Remove(tmpFilename0, Controller::Debug::Out());
        }
        if (bQuantized) {
          Remove(tmpFilename1, Controller::Debug::Out());
        }
		    return false;
      }
    }

    MESSAGE("Computing 2D Histogram...");
    Histogram2DDataBlock Histogram2D;
    if (!Histogram2D.Compute(&dataVolume, Histogram1D.GetHistogram().size())) {
      T_ERROR("Computation of 2D Histogram failed!");
      uvfFile.Close();
      SourceData.Close();
      if (bConvertEndianness) {
        Remove(tmpFilename0, Controller::Debug::Out());
      }
      if (bQuantized) {
        Remove(tmpFilename1, Controller::Debug::Out());
      }
		  return false;
    }
    MESSAGE("Storing histogram data...");
	  uvfFile.AddDataBlock(&Histogram1D,Histogram1D.ComputeDataSize());
	  uvfFile.AddDataBlock(&Histogram2D,Histogram2D.ComputeDataSize());
  }

  MESSAGE("Storing acceleration data...");
  uvfFile.AddDataBlock(&MaxMinData, MaxMinData.ComputeDataSize());

/*
  /// \todo maybe add information from the source file to the UVF, like DICOM desc etc.

  KeyValuePairDataBlock testPairs;
	testPairs.AddPair("SOURCE","DICOM");
	testPairs.AddPair("CONVERTED BY","DICOM2UVF V1.0");
	UINT64 iDataSize = testPairs.ComputeDataSize();
	uvfFile.AddDataBlock(testPairs,iDataSize);
*/

  MESSAGE("Computing checksum and writing file...");

	uvfFile.Create();
	SourceData.Close();
	uvfFile.Close();

  MESSAGE("Removing temporary files...");

  if (bConvertEndianness) {
    Remove(tmpFilename0, Controller::Debug::Out());
  }
  if (bQuantized) {
    Remove(tmpFilename1, Controller::Debug::Out());
  }

  MESSAGE("Done!");
  return true;
}


#ifdef WIN32
  #pragma warning( disable : 4996 ) // disable deprecated warning
#endif

/** Converts a gzip-compressed chunk of a file to a raw file.
 * @param strFilename the input (compressed) file
 * @param strTargetFilename the target uvf file
 * @param iHeaderSkip number of bytes to skip off of strFilename */
bool RAWConverter::ExtractGZIPDataset(const string& strFilename,
                                      const string& strUncompressedFile,
                                      UINT64 iHeaderSkip)
{
  FILE *f_compressed;
  FILE *f_inflated;
  int ret;

  MESSAGE("Deflating GZIP data ...");

  f_compressed = fopen(strFilename.c_str(), "rb");
  f_inflated = fopen(strUncompressedFile.c_str(), "wb");

  if(f_compressed == NULL) {
    T_ERROR("Could not open %s", strFilename.c_str());
    fclose(f_inflated);
    return false;
  }
  if(f_inflated == NULL) {
    T_ERROR("Could not open %s", strUncompressedFile.c_str());
    fclose(f_compressed);
    return false;
  }

  if(fseek(f_compressed, static_cast<long>(iHeaderSkip), SEEK_SET) != 0) {
    /// \todo use strerror(errno) and actually report the damn error.
    T_ERROR("Seek failed");
    fclose(f_compressed);
    return false;
  }

  gz_skip_header(f_compressed); // always needed?

  ret = gz_inflate(f_compressed, f_inflated);

  fclose(f_compressed);
  fclose(f_inflated);

  switch(ret) {
    case Z_OK:
      MESSAGE("Decompression successful.");
      break;
    case Z_MEM_ERROR:
      T_ERROR("Not enough memory decompress %s",
                 strFilename.c_str());
      return false;
      break;
    case Z_DATA_ERROR:
      T_ERROR("Deflation invalid or incomplete");
      return false;
      break;
    case Z_VERSION_ERROR:
      T_ERROR("Zlib library versioning error!");
      return false;
      break;
    default:
      WARNING("Unknown / unhandled case %d", ret);
      return false;
      break;
  }

  return true;
}

/** Tests a bzip return code for errors, and translates it to a string for the
 * debug logs.
 * @param bz_err the error code (given by the bzip2 library)
 * @return true if an error occurred */
static bool
bz_err_test(int bz_err)
{
#ifdef TUVOK_NO_IO
  T_ERROR("bzip2 library not available!");
  return true;
#else
  bool error_occurred = true;
  switch(bz_err) {
        case BZ_OK:        /* FALL THROUGH */
        case BZ_RUN_OK:    /* FALL THROUGH */
        case BZ_FLUSH_OK:  /* FALL THROUGH */
        case BZ_FINISH_OK:
            MESSAGE("Bzip operation successful.");
            error_occurred = false;
            break;
        case BZ_STREAM_END:
            MESSAGE("End of bzip stream.");
            break;
        case BZ_CONFIG_ERROR:
            T_ERROR("Bzip configuration error");
            break;
        case BZ_SEQUENCE_ERROR:
            T_ERROR("Bzip sequencing error");
            break;
        case BZ_PARAM_ERROR:
            T_ERROR("Bzip parameter error");
            break;
        case BZ_MEM_ERROR:
            T_ERROR("Bzip memory allocation failed.");
            break;
        case BZ_DATA_ERROR_MAGIC:
            WARNING("Bzip stream does not have correct magic bytes!");
            /* FALL THROUGH */
        case BZ_DATA_ERROR:
            T_ERROR("Bzip data integrity error; this usually means the "
                    "compressed file is corrupt.");
            break;
        case BZ_IO_ERROR: {
            const char *err_msg = strerror(errno);
            T_ERROR("Bzip IO error: %s", err_msg);
            break;
        }
        case BZ_UNEXPECTED_EOF:
            WARNING("EOF before end of Bzip stream.");
            break;
        case BZ_OUTBUFF_FULL:
            T_ERROR("Bzip output buffer is not large enough");
            break;
    }
    return error_occurred;
#endif
}

/** Converts a bzip2-compressed file chunk to a raw file.
 * @param strFilename the input (compressed) file
 * @param strTargetFilename the target uvf file
 * @param iHeaderSkip number of bytes to skip of strFilename's header*/
bool RAWConverter::ExtractBZIP2Dataset(const string& strFilename,
                                       const string& strUncompressedFile,
                                       UINT64 iHeaderSkip)
{
#ifdef TUVOK_NO_IO
  T_ERROR("Tuvok built without IO routines; bzip2 not available!");
  return false;
#else
  BZFILE *bzf;
  int bz_err;
  std::vector<char> buffer(INCORESIZE);

  FILE *f_compressed = fopen(strFilename.c_str(), "rb");
  FILE *f_inflated = fopen(strUncompressedFile.c_str(), "wb");

  if(f_compressed == NULL) {
    T_ERROR("Could not open %s", strFilename.c_str());
    fclose(f_inflated);
    return false;
  }
  if(f_inflated == NULL) {
    T_ERROR("Could not open %s", strUncompressedFile.c_str());
    fclose(f_compressed);
    return false;
  }

  if(fseek(f_compressed, static_cast<long>(iHeaderSkip), SEEK_SET) != 0) {
    /// \todo use strerror(errno) and actually report the damn error.
    T_ERROR("Seek failed");
    fclose(f_inflated);
    fclose(f_compressed);
    return false;
  }

  bzf = BZ2_bzReadOpen(&bz_err, f_compressed, 0, 0, NULL, 0);
  if(bz_err_test(bz_err)) {
    T_ERROR("Bzip library error occurred; bailing.");
    fclose(f_inflated);
    fclose(f_compressed);
    return false;
  }

  do {
    int nbytes = BZ2_bzRead(&bz_err, bzf, &buffer[0], INCORESIZE);
    if(bz_err != BZ_STREAM_END && bz_err_test(bz_err)) {
      T_ERROR("Bzip library error occurred; bailing.");
      fclose(f_inflated);
      fclose(f_compressed);
      return false;
    }
    if(1 != fwrite(&buffer[0], nbytes, 1, f_inflated)) {
      WARNING("%d-byte write of decompressed file failed.",
                  nbytes);
      fclose(f_inflated);
      fclose(f_compressed);
      return false;
    }
  } while(bz_err == BZ_OK);

  fclose(f_inflated);
  fclose(f_compressed);

  return true;
#endif
}

bool RAWConverter::ParseTXTDataset(const string& strFilename,
                                     const string& strBinaryFile,
                                     UINT64 iHeaderSkip,
                                     UINT64 iComponentSize,
                                     UINT64 iComponentCount,
                                     bool bSigned,
                                     bool bIsFloat,
                                     UINTVECTOR3 vVolumeSize)
{
  ifstream sourceFile(strFilename.c_str(),ios::binary);
  if (!sourceFile.is_open()) {
    T_ERROR("Unable to open source file %s.", strFilename.c_str());
    return false;
  }

  LargeRAWFile binaryFile(strBinaryFile);
  binaryFile.Create(iComponentSize/8 * iComponentCount * vVolumeSize.volume());
  if (!binaryFile.IsOpen()) {
    T_ERROR("Unable to open temp file %s.", strBinaryFile.c_str());
    sourceFile.close();
    return false;
  }

  sourceFile.seekg(static_cast<std::streamoff>(iHeaderSkip));
  if (bIsFloat) {
    if (!bSigned) {
      T_ERROR("Unsupported data type "
                                                  "(unsigned float)");
      sourceFile.close();
      binaryFile.Delete();
      return false;
    }
    switch (iComponentSize) {
      case 32 : {
                  while (! sourceFile.eof() )
                  {
                    float tmp;
                    sourceFile >> tmp;
                    binaryFile.WriteRAW((unsigned char*)&tmp,4);
                  }
                 break;
               }
      case 64 : {
                  while (! sourceFile.eof() )
                  {
                    double tmp;
                    sourceFile >> tmp;
                    binaryFile.WriteRAW((unsigned char*)&tmp,8);
                  }
                 break;
               }
      default : {
                  T_ERROR("Unable unsupported data type. (float)");
                  sourceFile.close();
                  binaryFile.Delete();
                  return false;
                }
    }
  } else {
    switch (iComponentSize) {
      case 8 : {
                  if (bSigned) {
                    while (! sourceFile.eof() )
                    {
                      int tmp;
                      sourceFile >> tmp;
                      signed char tmp2 = static_cast<signed char>(tmp);
                      binaryFile.WriteRAW((unsigned char*)&tmp2,1);
                    }
                  } else {
                    while (! sourceFile.eof() )
                    {
                      int tmp;
                      sourceFile >> tmp;
                      unsigned char tmp2 = static_cast<unsigned char>(tmp);
                      binaryFile.WriteRAW((unsigned char*)&tmp2,1);
                    }
                  }
                 break;
               }
      case 16 : {
                  if (bSigned) {
                    while (! sourceFile.eof() )
                    {
                      signed short tmp;
                      sourceFile >> tmp;
                      binaryFile.WriteRAW((unsigned char*)&tmp,2);
                    }
                  } else {
                    while (! sourceFile.eof() )
                    {
                      unsigned short tmp;
                      sourceFile >> tmp;
                      binaryFile.WriteRAW((unsigned char*)&tmp,2);
                    }
                  }
                 break;
               }
      case 32 : {
                  if (bSigned) {
                    while (! sourceFile.eof() )
                    {
                      signed int tmp;
                      sourceFile >> tmp;
                      binaryFile.WriteRAW((unsigned char*)&tmp,4);
                    }
                  } else {
                    while (! sourceFile.eof() )
                    {
                      UINT32 tmp;
                      sourceFile >> tmp;
                      binaryFile.WriteRAW((unsigned char*)&tmp,4);
                    }
                  }
                 break;
               }
      default : {
                  T_ERROR("Unable unsupported data type. (int)");
                  sourceFile.close();
                  binaryFile.Delete();
                  return false;
                }
    }
  }
  binaryFile.Close();
  sourceFile.close();

  return true;
}

bool RAWConverter::ConvertToNative(const std::string& strRawFilename, const std::string& strTargetFilename, UINT64 iHeaderSkip,
                                   UINT64 iComponentSize, UINT64 , bool , bool,
                                   UINTVECTOR3, FLOATVECTOR3, bool) {
  // convert raw to raw is easy :-), just copy the file and ignore the metadata

  // if the file exists, delete it first
  if (SysTools::FileExists(strTargetFilename))
    Remove(strTargetFilename, Controller::Debug::Out());
  if (SysTools::FileExists(strTargetFilename)) {
    T_ERROR("Unable to remove existing target file %s.", strTargetFilename.c_str());
    return false;
  }

  return AppendRAW(strRawFilename, iHeaderSkip, strTargetFilename, iComponentSize, EndianConvert::IsBigEndian());
}

bool RAWConverter::AppendRAW(const std::string& strRawFilename, UINT64 iHeaderSkip, const std::string& strTargetFilename,
                             UINT64 iComponentSize, bool bChangeEndianess, bool bToSigned) {
  // open source file
  LargeRAWFile fSource(strRawFilename, iHeaderSkip);
  fSource.Open(false);
  if (!fSource.IsOpen()) {
    T_ERROR("Unable to open source file %s.", strRawFilename.c_str());
    return false;
  }
  // append to target file
  LargeRAWFile fTarget(strTargetFilename);
  fTarget.Append();
  if (!fTarget.IsOpen()) {
    fSource.Close();
    T_ERROR("Unable to open target file %s.", strTargetFilename.c_str());
    return false;
  }

  UINT64 iSourceSize = fSource.GetCurrentSize();
  UINT64 iCopySize = min(iSourceSize,BLOCK_COPY_SIZE);
  unsigned char* pBuffer = new unsigned char[size_t(iCopySize)];
  UINT64 iCopiedSize = 0;

  do {
    MESSAGE("Writing output data\n%g percent completed", 100.0f*float(iCopiedSize)/float(iSourceSize));

    iCopySize = fSource.ReadRAW(pBuffer, iCopySize);

    if (bToSigned) {

      switch (iComponentSize) {
        case 8  : // char to uchar
                  for (size_t i = 0;i<iCopySize;i++)
                    (*(char*)(pBuffer+i)) = char(*(unsigned char*)(pBuffer+i)) - std::numeric_limits<char>::max();
                  break;
        case 16 : // short to ushort
                  for (size_t i = 0;i<iCopySize;i+=2)
                    (*(short*)(pBuffer+i)) = short(*(unsigned short*)(pBuffer+i)) - std::numeric_limits<short>::max();
                  break;
        case 32 : // int to uint
                  for (size_t i = 0;i<iCopySize;i+=4)
                    (*(int*)(pBuffer+i)) = int(*(unsigned int*)(pBuffer+i)) - std::numeric_limits<int>::max();
                  break;
        case 64 : // ulonglong to longlong
                  for (size_t i = 0;i<iCopySize;i+=8)
                    (*(int64_t*)(pBuffer+i)) = int64_t(*(UINT64*)(pBuffer+i)) - std::numeric_limits<int64_t>::max();
                  break;
        default : T_ERROR("Unsuported data type for vff files.");
                  return false;
      }
    }

    if (bChangeEndianess) {
      switch (iComponentSize) {
        case 16 : for (size_t i = 0;i<iCopySize;i+=2)
                    EndianConvert::Swap<unsigned short>((unsigned short*)(pBuffer+i));
                  break;
        case 32 : for (size_t i = 0;i<iCopySize;i+=4)
                    EndianConvert::Swap<float>((float*)(pBuffer+i));
                  break;
        case 64 : for (size_t i = 0;i<iCopySize;i+=8)
                    EndianConvert::Swap<double>((double*)(pBuffer+i));
                  break;
      }
    }

    fTarget.WriteRAW(pBuffer, iCopySize);
    iCopiedSize += iCopySize;
  } while (iCopySize > 0);

  fSource.Close();
  fTarget.Close();
  delete [] pBuffer;

  return true;
}


bool RAWConverter::ConvertToUVF(const std::string& strSourceFilename, const std::string& strTargetFilename,
                                const std::string& strTempDir,
                                bool bNoUserInteraction) {

  UINT64        iHeaderSkip;
  UINT64        iComponentSize;
  UINT64        iComponentCount;
  bool          bConvertEndianess;
  bool          bSigned;
  bool          bIsFloat;
  UINTVECTOR3   vVolumeSize;
  FLOATVECTOR3  vVolumeAspect;
  string        strTitle;
  string        strSource;
  UVFTables::ElementSemanticTable eType;
  string        strIntermediateFile;
  bool          bDeleteIntermediateFile;

  bool bRAWCreated = ConvertToRAW(strSourceFilename, strTempDir,
                                  bNoUserInteraction,
                                  iHeaderSkip, iComponentSize, iComponentCount,
                                  bConvertEndianess, bSigned, bIsFloat,
                                  vVolumeSize, vVolumeAspect, strTitle,
                                  eType, strIntermediateFile,
                                  bDeleteIntermediateFile);
  strSource = SysTools::GetFilename(strSourceFilename);

  if (!bRAWCreated) {
    T_ERROR("Convert to RAW step failed, aborting.");
    return false;
  }

  bool bUVFCreated = ConvertRAWDataset(strIntermediateFile, strTargetFilename, strTempDir, iHeaderSkip, iComponentSize, iComponentCount, bConvertEndianess, bSigned,
                                       bIsFloat, vVolumeSize, vVolumeAspect, strTitle, SysTools::GetFilename(strSourceFilename));

  if (bDeleteIntermediateFile) {
    Remove(strIntermediateFile, Controller::Debug::Out());
  }

  return bUVFCreated;
}

bool RAWConverter::Analyze(const std::string& strSourceFilename,
                           const std::string& strTempDir,
                           bool bNoUserInteraction, RangeInfo& info) {
  UINT64        iHeaderSkip=0;
  UINT64        iComponentSize=0;
  UINT64        iComponentCount=0;
  bool          bConvertEndianess=false;
  bool          bSigned=false;
  bool          bIsFloat=false;
  UINTVECTOR3   vVolumeSize(0,0,0);
  FLOATVECTOR3  vVolumeAspect(0,0,0);
  string        strTitle = "";
  string        strSource = "";
  UVFTables::ElementSemanticTable eType = UVFTables::ES_UNDEFINED;

  string        strRAWFilename = "";
  bool          bRAWDelete = false;


  bool bConverted = ConvertToRAW(strSourceFilename, strTempDir,
                                 bNoUserInteraction,
                                 iHeaderSkip, iComponentSize, iComponentCount,
                                 bConvertEndianess, bSigned, bIsFloat,
                                 vVolumeSize, vVolumeAspect, strTitle,
                                 eType, strRAWFilename,
                                 bRAWDelete);
  strSource = SysTools::GetFilename(strSourceFilename);

  if (!bConverted) return false;

  info.m_vAspect = vVolumeAspect;
  info.m_vDomainSize = vVolumeSize;
  // ConvertToRAW either creates a 16 or 8 bit unsigned int, so checking
  // the iComponentSize is sufficient to make sure the types are the same
  info.m_iComponentSize = iComponentSize;

  bool bAnalyzed = Analyze(strRAWFilename, iHeaderSkip, iComponentSize, iComponentCount,
                           bSigned, bIsFloat, vVolumeSize, info);

  if (bRAWDelete) {
    Remove(strRAWFilename, Controller::Debug::Out());
  }

  return bAnalyzed;
}

bool RAWConverter::Analyze(const std::string& strSourceFilename,
                           UINT64 iHeaderSkip, UINT64 iComponentSize,
                           UINT64 iComponentCount, bool bSigned,
                           bool bFloatingPoint, UINTVECTOR3 vVolumeSize,
                           RangeInfo& info) {
  // open source file
  LargeRAWFile fSource(strSourceFilename, iHeaderSkip);
  fSource.Open(false);
  if (!fSource.IsOpen()) {
    T_ERROR("Unable to open source file %s.", strSourceFilename.c_str());
    return false;
  }

  UINT64 iElemCount = vVolumeSize.volume()*iComponentCount;

  if (bFloatingPoint) {
    if (!bSigned) {
      T_ERROR( "Unable unsupported data type. (unsiged float)");
      fSource.Close();
      return false;
    }
    info.m_iValueType = 0;
    switch (iComponentSize) {
      case 32 : {
                  float fMin = numeric_limits<float>::max();
                  float fMax = -numeric_limits<float>::max();
                  MinMaxScanner<float> scanner(&fSource, fMin, fMax, iElemCount);
                  info.m_fRange.first  = fMin;
                  info.m_fRange.second = fMax;
                  break;
                }
      case 64 : {
                  double fMin = numeric_limits<double>::max();
                  double fMax = -numeric_limits<double>::max();
                  MinMaxScanner<double> scanner(&fSource, fMin, fMax, iElemCount);
                  info.m_fRange.first  = fMin;
                  info.m_fRange.second = fMax;
                  break;
               }
      default : {
                  T_ERROR("Unable unsupported data type. (float)");
                  fSource.Close();
                  return false;
                }
    }
  } else {
    if (bSigned)
      info.m_iValueType = 1;
    else
      info.m_iValueType = 2;

    switch (iComponentSize) {
      case 8 : {
                 if (bSigned) {
                   char iMin = numeric_limits<char>::max();
                   char iMax = -numeric_limits<char>::max();
                   MinMaxScanner<char> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_iRange.first  = iMin;
                   info.m_iRange.second = iMax;
                   break;
                 } else {
                   unsigned char iMin = numeric_limits<unsigned char>::max();
                   unsigned char iMax = numeric_limits<unsigned char>::min();
                   MinMaxScanner<unsigned char> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_uiRange.first  = iMin;
                   info.m_uiRange.second = iMax;
                 }
                 break;
               }
      case 16 : {
                 if (bSigned) {
                   short iMin = numeric_limits<short>::max();
                   short iMax = -numeric_limits<short>::max();
                   MinMaxScanner<short> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_iRange.first  = iMin;
                   info.m_iRange.second = iMax;
                   break;
                 } else {
                   unsigned short iMin = numeric_limits<unsigned short>::max();
                   unsigned short iMax = numeric_limits<unsigned short>::min();
                   MinMaxScanner<unsigned short> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_uiRange.first  = iMin;
                   info.m_uiRange.second = iMax;
                 }
                 break;
               }
      case 32 : {
                 if (bSigned) {
                   int iMin = numeric_limits<int>::max();
                   int iMax = -numeric_limits<int>::max();
                   MinMaxScanner<int> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_iRange.first  = iMin;
                   info.m_iRange.second = iMax;
                   break;
                 } else {
                   unsigned int iMin = numeric_limits<unsigned int>::max();
                   unsigned int iMax = numeric_limits<unsigned int>::min();
                   MinMaxScanner<unsigned int> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_uiRange.first  = iMin;
                   info.m_uiRange.second = iMax;
                 }
                 break;
               }
      case 64 : {
                 if (bSigned) {
                   int64_t iMin = numeric_limits<int64_t>::max();
                   int64_t iMax = -numeric_limits<int64_t>::max();
                   MinMaxScanner<int64_t> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_iRange.first  = iMin;
                   info.m_iRange.second = iMax;
                   break;
                 } else {
                   UINT64 iMin = numeric_limits<UINT64>::max();
                   UINT64 iMax = numeric_limits<UINT64>::min();
                   MinMaxScanner<UINT64> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_uiRange.first  = iMin;
                   info.m_uiRange.second = iMax;
                 }
                 break;
               }
      default : {
                  T_ERROR("Unable unsupported data type. (int)");
                  fSource.Close();
                  return false;
                }
    }
  }

  fSource.Close();
  return true;
}


  /// Uses remove(3) to remove the file.
  /// @return true if the remove succeeded.
bool RAWConverter::Remove(const std::string &path, AbstrDebugOut &dbg)
{
  if(std::remove(path.c_str()) == -1) {
#ifdef _WIN32
      char buffer[200];
      strerror_s(buffer, 200, errno);
      dbg.Warning(_func_, "Could not remove `%s': %s", path.c_str(), buffer);
#else
      dbg.Warning(_func_, "Could not remove `%s': %s", path.c_str(),
                  strerror(errno));
#endif
      return false;
  }
  return true;
}
