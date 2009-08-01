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
  \file    AbstrConverter.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    December 2008
*/

#include <algorithm>
#include "AbstrConverter.h"
#include "IOManager.h"  // for the size defines
#include "Controller/Controller.h"
#include "UVF/Histogram1DDataBlock.h"
#include "Quantize.h"

const std::string
AbstrConverter::Process8BitsTo8Bits(UINT64 iHeaderSkip,
                                    const std::string& strFilename,
                                    const std::string& strTargetFilename,
                                    UINT64 iSize, bool bSigned,
                                    Histogram1DDataBlock* Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  std::vector<UINT64> aHist(256);
  std::fill(aHist.begin(), aHist.end(), 0);

  std::string strSignChangedFile;
  if (bSigned)  {
    MESSAGE("Changing signed to unsigned char and computing 1D histogram...");
    LargeRAWFile OutputData(strTargetFilename);
    OutputData.Create(iSize);

    if (!OutputData.IsOpen()) {
      InputData.Close();
      return "";
    }

    signed char* pInData = new signed char[INCORESIZE];

    UINT64 iPos = 0;
    while (iPos < iSize)  {
      size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE);
      if (iRead == 0) break;

      for (size_t i = 0;i<iRead;i++) {
        pInData[i] += 127;
        if (Histogram1D) aHist[(unsigned char)pInData[i]]++;
      }
      OutputData.WriteRAW((unsigned char*)pInData, iRead);
      iPos += UINT64(iRead);
    }

    if (iPos < iSize) {
      WARNING("Specified size and real datasize mismatch");
    }

    delete [] pInData;
    strSignChangedFile = strTargetFilename;
    OutputData.Close();
  } else {
    if (Histogram1D) {
      MESSAGE("Computing 1D Histogram...");
      unsigned char* pInData = new unsigned char[INCORESIZE];

      UINT64 iPos = 0;
      UINT64 iDivLast = 0;
      while (iPos < iSize)  {
        size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE);
        if (iRead == 0) break;
        for (size_t i = 0;i<iRead;i++) aHist[pInData[i]]++;
        iPos += UINT64(iRead);

        if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
          MESSAGE("Computing 1D Histogram (%i percent complete)",
                  int((100*iPos)/iSize));
          iDivLast = (100*iPos)/iSize;
        }
      }

      if (iPos < iSize) {
        WARNING("Specified size and real datasize mismatch");
      }

      MESSAGE("1D Histogram complete");
      delete [] pInData;
    }
    strSignChangedFile = strFilename;
  }

  InputData.Close();
  if ( Histogram1D ) Histogram1D->SetHistogram(aHist);

  return strSignChangedFile;
}


const std::string
AbstrConverter::QuantizeShortTo12Bits(UINT64 iHeaderSkip,
                                      const std::string& strFilename,
                                      const std::string& strTargetFilename,
                                      UINT64 iSize, bool bSigned,
                                      Histogram1DDataBlock* Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  std::vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  // determine max and min
  unsigned short iMax = 0;
  unsigned short iMin = std::numeric_limits<unsigned short>::max();
  short* pInData = new short[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;

  std::pair<unsigned short,unsigned short> minmax;
  minmax = io_minmax(raw_data_src<unsigned short>(strFilename.c_str()),
                     TuvokProgress<UINT64>(iSize));
  iMin = minmax.first;
  iMax = minmax.second;

  std::string strQuantFile;
  // if file uses less or equal than 12 bits quit here
  if (iMax < 4096) {
    MESSAGE("No quantization required (min=%i, max=%i)", iMin, iMax);
    aHist.resize(iMax+1);  // size is the maximum value plus one (the zero value)
    delete [] pInData;
    InputData.Close();
    strQuantFile = strFilename;
  } else {
    if (bSigned) {
      MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)",
              int(iMin)-std::numeric_limits<short>::max(),
              int(iMax)-std::numeric_limits<short>::max());
    } else {
      MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)",
              iMin, iMax);
    }
    std::fill(aHist.begin(), aHist.end(), 0);

    // otherwise quantize
    LargeRAWFile OutputData(strTargetFilename);
    OutputData.Create(iSize*2);

    if (!OutputData.IsOpen()) {
      delete [] pInData;
      InputData.Close();
      return "";
    }

    UINT64 iRange = iMax-iMin;

    InputData.SeekStart();
    iPos = 0;
    iDivLast = 0;
    while (iPos < iSize)  {
      size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*2)/2;
      if(iRead == 0) { break; } // bail out if the read gave us nothing.

      for (size_t i = 0;i<iRead;i++) {
        unsigned short iValue = (bSigned) ? pInData[i] + std::numeric_limits<short>::max() : pInData[i];
        unsigned short iNewVal = std::min<unsigned short>(4095, (unsigned short)((UINT64(iValue-iMin) * 4095)/iRange));
        pInData[i] = iNewVal;
        aHist[iNewVal]++;
      }
      iPos += UINT64(iRead);

      if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
        if (bSigned) {
          MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)"
                  "\n%i percent complete",
                  int(iMin) - std::numeric_limits<short>::max(),
                  int(iMax) - std::numeric_limits<short>::max(),
                  int((100*iPos)/iSize));
        } else {
          MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)"
                  "\n%i percent complete", iMin, iMax, int((100*iPos)/iSize));
        }
        iDivLast = (100*iPos)/iSize;
      }

      OutputData.WriteRAW((unsigned char*)pInData, 2*iRead);
    }

    delete [] pInData;
    OutputData.Close();
    InputData.Close();

    strQuantFile = strTargetFilename;
  }

  if (Histogram1D) Histogram1D->SetHistogram(aHist);

  return strQuantFile;
}

const std::string
AbstrConverter::QuantizeFloatTo12Bits(UINT64 iHeaderSkip,
                                      const std::string& strFilename,
                                      const std::string& strTargetFilename,
                                      UINT64 iSize,
                                      Histogram1DDataBlock* Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  // determine max and min
  float fMax = -std::numeric_limits<float>::max();
  float fMin = std::numeric_limits<float>::max();
  float* pInData = new float[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;

  std::pair<float,float> minmax;
  minmax = io_minmax(raw_data_src<float>(strFilename.c_str()),
                     TuvokProgress<UINT64>(iSize));
  fMin = minmax.first;
  fMax = minmax.second;

  // quantize
  LargeRAWFile OutputData(strTargetFilename);
  OutputData.Create(iSize*2);

  if (!OutputData.IsOpen()) {
    delete [] pInData;
    InputData.Close();
    return "";
  }

  MESSAGE("Quantizing to 12 bit (input data has range from %g to %g)",
          fMin, fMax);

  float fQuantFact = 4095.0f / float(fMax-fMin);
  unsigned short* pOutData = new unsigned short[INCORESIZE];

  std::vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  InputData.SeekStart();
  iPos = 0;
  iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*4)/4;
    if(iRead == 0) { break; } // bail out if the read gave us nothing.

    for (size_t i = 0;i<iRead;i++) {
      unsigned short iNewVal = std::min<unsigned short>(4095,
                                   static_cast<unsigned short>
                                   ((pInData[i]-fMin) * fQuantFact));
      pOutData[i] = iNewVal;
      aHist[iNewVal]++;
    }
    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      MESSAGE("Quantizing to 12 bit (input data has range from %g to %g)\n"
              "%i percent complete", fMin, fMax, int((100*iPos)/iSize));
      iDivLast = (100*iPos)/iSize;
    }

    OutputData.WriteRAW((unsigned char*)pOutData, 2*iRead);
  }

  delete [] pInData;
  delete [] pOutData;
  OutputData.Close();
  InputData.Close();

  if (Histogram1D) Histogram1D->SetHistogram(aHist);

  return strTargetFilename;
}

const std::string
AbstrConverter::QuantizeDoubleTo12Bits(UINT64 iHeaderSkip,
                                       const std::string& strFilename,
                                       const std::string& strTargetFilename,
                                       UINT64 iSize,
                                       Histogram1DDataBlock* Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  // determine max and min
  double fMax = -std::numeric_limits<double>::max();
  double fMin = std::numeric_limits<double>::max();
  double* pInData = new double[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;

  std::pair<double,double> minmax;
  minmax = io_minmax(raw_data_src<double>(strFilename.c_str()),
                     TuvokProgress<UINT64>(iSize));
  fMin = minmax.first;
  fMax = minmax.second;

  // quantize
  LargeRAWFile OutputData(strTargetFilename);
  OutputData.Create(iSize*2);

  if (!OutputData.IsOpen()) {
    delete [] pInData;
    InputData.Close();
    return "";
  }

  MESSAGE("Quantizing to 12 bit (input data has range from %g to %g)",
          fMin, fMax);

  double fQuantFact = 4095 / (fMax-fMin);
  unsigned short* pOutData = new unsigned short[INCORESIZE];

  std::vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  InputData.SeekStart();
  iPos = 0;
  iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*8)/8;
    if(iRead == 0) { break; } // bail out if the read gave us nothing.

    for (size_t i = 0;i<iRead;i++) {
      unsigned short iNewVal = std::min<unsigned short>(4095,
                                   static_cast<unsigned short>
                                   ((pInData[i]-fMin) * fQuantFact));
      pOutData[i] = iNewVal;
      aHist[iNewVal]++;
    }
    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      MESSAGE("Quantizing to 12 bit (input data has range from %g to %g)\n"
              "%i percent complete", fMin, fMax, int((100*iPos)/iSize));
      iDivLast = (100*iPos)/iSize;
    }

    OutputData.WriteRAW((unsigned char*)pOutData, 2*iRead);
  }

  delete [] pInData;
  delete [] pOutData;
  OutputData.Close();
  InputData.Close();

  if (Histogram1D) Histogram1D->SetHistogram(aHist);

  return strTargetFilename;
}


const std::string
AbstrConverter::QuantizeLongTo12Bits(UINT64 iHeaderSkip,
                                     const std::string& strFilename,
                                     const std::string& strTargetFilename,
                                     UINT64 iSize, bool bSigned,
                                     Histogram1DDataBlock* Histogram1D) {
  AbstrDebugOut &dbg = Controller::Debug::Out();
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  std::vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  // determine max and min
  UINT64 iMax = 0;
  UINT64 iMin = std::numeric_limits<UINT64>::max();
  UINT64* pInData = new UINT64[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*8)/8;
    if (iRead == 0) break;

    for (size_t i = 0;i<iRead;i++) {
      UINT64 iValue = (bSigned) ? pInData[i] + std::numeric_limits<int>::max()
                                : pInData[i];
      if (iMax < iValue)  iMax = iValue;
      if (iMin > iValue)  iMin = iValue;
	  if (iMax < 4096) {
		aHist[static_cast<size_t>(iValue)]++;
	  }
    }

    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      dbg.Message(_func_, "Computing value range (%i percent complete)",
                  int((100*iPos)/iSize));
      iDivLast = (100*iPos)/iSize;
    }

    if (iMin == 0 && iMax == std::numeric_limits<UINT64>::max()) break;
  }

  if (iPos < iSize) {
    dbg.Warning(_func_, "Specified size and real datasize mismatch");
    iSize = iPos;
  }

  std::string strQuantFile;

  if (bSigned)
    dbg.Message(_func_, "Quantizing to 12 bit (input data has range from "
                        "%i to %i)", int(iMin) - std::numeric_limits<int>::max(),
                                     int(iMax) - std::numeric_limits<int>::max());
  else
    dbg.Message(_func_, "Quantizing to 12 bit (input data has range from "
                        "%i to %i)", iMin, iMax);

  std::fill(aHist.begin(), aHist.end(), 0);

  // otherwise quantize
  LargeRAWFile OutputData(strTargetFilename);
  OutputData.Create(iSize*2);

  if (!OutputData.IsOpen()) {
    delete [] pInData;
    InputData.Close();
    return "";
  }

  UINT64 iRange = iMax-iMin;

  InputData.SeekStart();
  iPos = 0;
  iDivLast = 0;
  while (iPos < iSize) {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*8)/8;
    if(iRead == 0) { break; } // bail out if the read gave us nothing.

    for (size_t i = 0; i < iRead; i++) {
      UINT64 iValue = (bSigned) ? pInData[i] + std::numeric_limits<int>::max()
                                : pInData[i];
      // if the range fits into 12 bits do only bias not rescale
      UINT64 iNewVal = (iRange < 4096) ?
                        iValue-iMin :
                        std::min<UINT64>(4095, (UINT32)((UINT64(iValue-iMin) * 4095)/iRange));
      ((unsigned short*)pInData)[i] = (unsigned short)iNewVal;
      aHist[static_cast<size_t>(iNewVal)]++;
    }
    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      if (bSigned)
        dbg.Message(_func_, "Quantizing to 12 bit (input data has range "
                    "from %i to %i)\n%i percent complete",
                    int(iMin) - std::numeric_limits<int>::max(),
                    int(iMax) - std::numeric_limits<int>::max(),
                    int((100*iPos)/iSize));
      else
        dbg.Message(_func_, "Quantizing to 12 bit (input data has range "
                    "from %i to %i)\n%i percent complete", iMin, iMax,
                    int((100*iPos)/iSize));
      iDivLast = (100*iPos)/iSize;
    }

    OutputData.WriteRAW((unsigned char*)pInData, 2*iRead);
  }

  delete [] pInData;
  OutputData.Close();
  InputData.Close();

  strQuantFile = strTargetFilename;

  if (Histogram1D) Histogram1D->SetHistogram(aHist);

  return strQuantFile;
}

const std::string
AbstrConverter::QuantizeIntTo12Bits(UINT64 iHeaderSkip,
                                    const std::string& strFilename,
                                    const std::string& strTargetFilename,
                                    UINT64 iSize, bool bSigned,
                                    Histogram1DDataBlock* Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  std::vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  // determine max and min
  UINT32 iMax = 0;
  UINT32 iMin = std::numeric_limits<UINT32>::max();
  UINT32* pInData = new UINT32[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*4)/4;
    if (iRead == 0) break;

    for (size_t i = 0;i<iRead;i++) {
      UINT32 iValue = (bSigned) ? pInData[i] + std::numeric_limits<int>::max() : pInData[i];
      if (iMax < iValue)  iMax = iValue;
      if (iMin > iValue)  iMin = iValue;
      if (iMax < 4096)    aHist[iValue]++;
    }

    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      MESSAGE("Computing value range (%i percent complete)",
              int((100*iPos)/iSize));
      iDivLast = (100*iPos)/iSize;
    }

    if (iMin == 0 && iMax == std::numeric_limits<UINT32>::max()) break;
  }

  if (iPos < iSize) {
    WARNING("Specified size and real datasize mismatch");
    iSize = iPos;
  }

  std::string strQuantFile;
  if (bSigned) {
    MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)",
            int(iMin) - std::numeric_limits<int>::max(),
            int(iMax) - std::numeric_limits<int>::max());
  } else {
    MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)",
            iMin, iMax);
  }
  std::fill(aHist.begin(), aHist.end(), 0);

  // otherwise quantize
  LargeRAWFile OutputData(strTargetFilename);
  OutputData.Create(iSize*2);

  if (!OutputData.IsOpen()) {
    delete [] pInData;
    InputData.Close();
    return "";
  }

  UINT64 iRange = iMax-iMin;

  InputData.SeekStart();
  iPos = 0;
  iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*4)/4;
    if(iRead == 0) { break; } // bail out if the read gave us nothing.

    for (size_t i = 0;i<iRead;i++) {
      UINT32 iValue = (bSigned) ? pInData[i] + std::numeric_limits<int>::max() : pInData[i];
      // if the range fits into 12 bits do only bias not rescale
      UINT32 iNewVal = (iRange < 4096) ? iValue-iMin : std::min<UINT32>(4095, (UINT32)((UINT64(iValue-iMin) * 4095)/iRange));
      ((unsigned short*)pInData)[i] = (unsigned short)iNewVal;
      aHist[iNewVal]++;
    }
    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      if (bSigned) {
        MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)"
                "\n%i percent complete",
                int(iMin) - std::numeric_limits<int>::max(),
                int(iMax) - std::numeric_limits<int>::max(),
                int((100*iPos)/iSize));
      } else {
        MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)"
                "\n%i percent complete", iMin, iMax, int((100*iPos)/iSize));
      }
      iDivLast = (100*iPos)/iSize;
    }

    OutputData.WriteRAW((unsigned char*)pInData, 2*iRead);
  }

  delete [] pInData;
  OutputData.Close();
  InputData.Close();

  strQuantFile = strTargetFilename;

  if (Histogram1D) Histogram1D->SetHistogram(aHist);

  return strQuantFile;
}

/********************************************************************/

const std::string
AbstrConverter::QuantizeShortTo8Bits(UINT64 iHeaderSkip,
                                     const std::string& strFilename,
                                     const std::string& strTargetFilename,
                                     UINT64 iSize, bool bSigned,
                                     Histogram1DDataBlock* Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  std::vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  // determine max and min
  unsigned short iMax = 0;
  unsigned short iMin = std::numeric_limits<unsigned short>::max();
  short* pInData = new short[INCORESIZE];
  unsigned char* pOutData = new unsigned char[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*2)/2;
    if (iRead == 0) break;

    for (size_t i = 0;i<iRead;i++) {
      unsigned short iValue = (bSigned) ? pInData[i] + std::numeric_limits<short>::max() : pInData[i];
      if (iMax < iValue)  iMax = iValue;
      if (iMin > iValue)  iMin = iValue;
    }

    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      MESSAGE("Computing value range (%i percent complete)",
              int((100*iPos)/iSize));
      iDivLast = (100*iPos)/iSize;
    }

    if (iMin == 0 && iMax == 65535) break;
  }

  if (iPos < iSize) {
    WARNING("Specified size and real datasize mismatch");
    iSize = iPos;
  }

  if (bSigned) {
    MESSAGE("Quantizing to 8 bit (input data has range from %i to %i)",
            int(iMin)-std::numeric_limits<short>::max(),
            int(iMax)-std::numeric_limits<short>::max());
  } else {
    MESSAGE("Quantizing to 8 bit (input data has range from %i to %i)",
            iMin, iMax);
  }
  std::fill(aHist.begin(), aHist.end(), 0);

  // quantize
  LargeRAWFile OutputData(strTargetFilename);
  OutputData.Create(iSize);

  if (!OutputData.IsOpen()) {
    delete [] pInData;
    delete [] pOutData;
    InputData.Close();
    return "";
  }

  UINT64 iRange = iMax-iMin;

  InputData.SeekStart();
  iPos = 0;
  iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*2)/2;
    if(iRead == 0) { break; } // bail out if the read gave us nothing.

    for (size_t i = 0;i<iRead;i++) {
      unsigned short iValue = (bSigned) ? pInData[i] + std::numeric_limits<short>::max() : pInData[i];
      unsigned char iNewVal = std::min<unsigned char>(255,
                                      (unsigned char)((UINT64(iValue-iMin) * 255)/iRange));
      pOutData[i] = iNewVal;
      aHist[iNewVal]++;
    }
    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      if (bSigned) {
        MESSAGE("Quantizing to 8 bit (input data has range from %i to %i)"
                "\n%i percent complete",
                int(iMin) - std::numeric_limits<short>::max(),
                int(iMax) - std::numeric_limits<short>::max(),
                int((100*iPos)/iSize));
      } else {
        MESSAGE("Quantizing to 8 bit (input data has range from %i to %i)"
                "\n%i percent complete", iMin, iMax, int((100*iPos)/iSize));
      }
      iDivLast = (100*iPos)/iSize;
    }

    OutputData.WriteRAW((unsigned char*)pOutData, iRead);
  }

  delete [] pInData;
  delete [] pOutData;

  OutputData.Close();
  InputData.Close();


  if (Histogram1D) Histogram1D->SetHistogram(aHist);

  return strTargetFilename;
}

const std::string
AbstrConverter::QuantizeFloatTo8Bits(UINT64 iHeaderSkip,
                                     const std::string& strFilename,
                                     const std::string& strTargetFilename,
                                     UINT64 iSize,
                                     Histogram1DDataBlock* Histogram1D) {
  /// \todo doing 2 quantizations is neither the most efficient nor the numerically best way
  ///       but it will do the trick until we templatize these methods

  std::string intermFile = QuantizeFloatTo12Bits(iHeaderSkip,
                                                 strFilename,
                                                 strTargetFilename+".temp",
                                                 iSize);

  return QuantizeShortTo8Bits(0,intermFile,strTargetFilename,iSize,false,Histogram1D);
}

const std::string
AbstrConverter::QuantizeDoubleTo8Bits(UINT64 iHeaderSkip, 
                                      const std::string& strFilename, 
                                      const std::string& strTargetFilename,
                                      UINT64 iSize, 
                                      Histogram1DDataBlock* Histogram1D) {
  /// \todo doing 2 quantizations is neither the most efficient nor the numerically best way
  ///       but it will do the trick until we templatize these methods

  std::string intermFile = QuantizeDoubleTo12Bits(iHeaderSkip,
                                                  strFilename,
                                                  strTargetFilename+".temp",
                                                  iSize);

  return QuantizeShortTo8Bits(0,intermFile,strTargetFilename,iSize,false,Histogram1D);
}


const std::string
AbstrConverter::QuantizeLongTo8Bits(UINT64 iHeaderSkip,
                                    const std::string& strFilename,
                                    const std::string& strTargetFilename,
                                    UINT64 iSize, bool bSigned,
                                    Histogram1DDataBlock* Histogram1D) {
  /// \todo doing 2 quantizations is neither the most efficient nor the numerically best way
  ///       but it will do the trick until we templatize these methods

  std::string intermFile = QuantizeLongTo12Bits(iHeaderSkip,
                                                strFilename,
                                                strTargetFilename+".temp",
                                                iSize,
                                                bSigned);

  return QuantizeShortTo8Bits(0,intermFile,strTargetFilename,iSize,false,Histogram1D);
}

const std::string
AbstrConverter::QuantizeIntTo8Bits(UINT64 iHeaderSkip,
                                   const std::string& strFilename,
                                   const std::string& strTargetFilename,
                                   UINT64 iSize, bool bSigned,
                                   Histogram1DDataBlock* Histogram1D) {
  /// \todo doing 2 quantizations is neither the most efficient nor the numerically best way
  ///       but it will do the trick until we templatize these methods

  std::string intermFile = QuantizeIntTo12Bits(iHeaderSkip,
                                               strFilename,
                                               strTargetFilename+".temp",
                                               iSize,
                                               bSigned);

  return QuantizeShortTo8Bits(0,intermFile,strTargetFilename,iSize,false,Histogram1D);
}

