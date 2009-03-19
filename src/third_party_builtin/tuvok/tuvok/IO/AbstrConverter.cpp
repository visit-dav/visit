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
#include <Controller/Controller.h>
#include <Basics/SysTools.h>

using namespace std;

const string AbstrConverter::Process8BitsTo8Bits(UINT64 iHeaderSkip, const string& strFilename, const string& strTargetFilename, UINT64 iSize, bool bSigned, Histogram1DDataBlock& Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  vector<UINT64> aHist(256);
  std::fill(aHist.begin(), aHist.end(), 0);

  string strSignChangedFile;
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
        aHist[(unsigned char)pInData[i]]++;
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

    delete [] pInData;
    strSignChangedFile = strFilename;
  }
  MESSAGE("1D Histogram complete");

  InputData.Close();
  Histogram1D.SetHistogram(aHist);

  return strSignChangedFile;
}


const string AbstrConverter::QuantizeShortTo12Bits(UINT64 iHeaderSkip, const string& strFilename, const string& strTargetFilename, UINT64 iSize, bool bSigned, Histogram1DDataBlock& Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  // determine max and min
  unsigned short iMax = 0;
  unsigned short iMin = numeric_limits<unsigned short>::max();
  short* pInData = new short[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*2)/2;
    if (iRead == 0) break;

    for (size_t i = 0;i<iRead;i++) {
      unsigned short iValue = (bSigned) ? pInData[i] + numeric_limits<short>::max() : pInData[i];
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

    if (iMin == 0 && iMax == 65535) break;
  }

  if (iPos < iSize) {
    WARNING("Specified size and real datasize mismatch");
    iSize = iPos;
  }

  string strQuantFile;
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
              int(iMin)-numeric_limits<short>::max(),
              int(iMax)-numeric_limits<short>::max());
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
        unsigned short iValue = (bSigned) ? pInData[i] + numeric_limits<short>::max() : pInData[i];
        unsigned short iNewVal = min<unsigned short>(4095, (unsigned short)((UINT64(iValue-iMin) * 4095)/iRange));
        pInData[i] = iNewVal;
        aHist[iNewVal]++;
      }
      iPos += UINT64(iRead);

      if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
        if (bSigned) {
          MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)"
                  "\n%i percent complete",
                  int(iMin) - numeric_limits<short>::max(),
                  int(iMax) - numeric_limits<short>::max(),
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

  Histogram1D.SetHistogram(aHist);

  return strQuantFile;
}

const string AbstrConverter::QuantizeFloatTo12Bits(UINT64 iHeaderSkip, const string& strFilename, const string& strTargetFilename, UINT64 iSize, Histogram1DDataBlock& Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  // determine max and min
  float fMax = -numeric_limits<float>::max();
  float fMin = numeric_limits<float>::max();
  float* pInData = new float[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*4)/4;
    if (iRead == 0) break;

    for (size_t i = 0;i<iRead;i++) {
      if (fMax < pInData[i]) fMax = pInData[i];
      if (fMin > pInData[i]) fMin = pInData[i];
    }

    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      MESSAGE("Computing value range (%i percent complete)",
              int((100*iPos)/iSize));
      iDivLast = (100*iPos)/iSize;
    }

  }

  if (iPos < iSize) {
    WARNING("Specified size and real datasize mismatch");
    iSize = iPos;
  }

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

  vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  InputData.SeekStart();
  iPos = 0;
  iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*4)/4;
    if(iRead == 0) { break; } // bail out if the read gave us nothing.

    for (size_t i = 0;i<iRead;i++) {
      unsigned short iNewVal = min<unsigned short>(4095,
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

  Histogram1D.SetHistogram(aHist);

  return strTargetFilename;
}

const string AbstrConverter::QuantizeDoubleTo12Bits(UINT64 iHeaderSkip, const string& strFilename, const string& strTargetFilename, UINT64 iSize, Histogram1DDataBlock& Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  // determine max and min
  double fMax = -numeric_limits<double>::max();
  double fMin = numeric_limits<double>::max();
  double* pInData = new double[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*8)/8;
    if (iRead == 0) break;

    for (size_t i = 0;i<iRead;i++) {
      if (fMax < pInData[i]) fMax = pInData[i];
      if (fMin > pInData[i]) fMin = pInData[i];
    }

    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      MESSAGE("Computing value range (%i percent complete)",
              int((100*iPos)/iSize));
      iDivLast = (100*iPos)/iSize;
    }

  }

  if (iPos < iSize) {
    WARNING("Specified size and real datasize mismatch");
    iSize = iPos;
  }

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

  vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  InputData.SeekStart();
  iPos = 0;
  iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*8)/8;
    if(iRead == 0) { break; } // bail out if the read gave us nothing.

    for (size_t i = 0;i<iRead;i++) {
      unsigned short iNewVal = min<unsigned short>(4095,
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

  Histogram1D.SetHistogram(aHist);

  return strTargetFilename;
}


const string
AbstrConverter::QuantizeLongTo12Bits(UINT64 iHeaderSkip,
                                     const string& strFilename,
                                     const string& strTargetFilename,
                                     UINT64 iSize, bool bSigned,
                                     Histogram1DDataBlock& Histogram1D) {
  AbstrDebugOut &dbg = Controller::Debug::Out();
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  // determine max and min
  UINT64 iMax = 0;
  UINT64 iMin = numeric_limits<UINT64>::max();
  UINT64* pInData = new UINT64[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*8)/8;
    if (iRead == 0) break;

    for (size_t i = 0;i<iRead;i++) {
      UINT64 iValue = (bSigned) ? pInData[i] + numeric_limits<int>::max()
                                : pInData[i];
      if (iMax < iValue)  iMax = iValue;
      if (iMin > iValue)  iMin = iValue;
      if (iMax < 4096)    aHist[iValue]++;
    }

    iPos += UINT64(iRead);

    if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
      dbg.Message(_func_, "Computing value range (%i percent complete)",
                  int((100*iPos)/iSize));
      iDivLast = (100*iPos)/iSize;
    }

    if (iMin == 0 && iMax == numeric_limits<UINT64>::max()) break;
  }

  if (iPos < iSize) {
    dbg.Warning(_func_, "Specified size and real datasize mismatch");
    iSize = iPos;
  }

  string strQuantFile;
  // if file uses less than or equal to 12 bits, we're already done.
  if (iMax < 4096) {
    dbg.Message(_func_, "No quantization required (min=%i, max=%i)",
                iMin, iMax);
    // reduce the size to the filled size (the maximum value plus one (the zero
    // value))
    aHist.resize(iMax+1);
    delete [] pInData;
    InputData.Close();
    strQuantFile = strFilename;
  } else {
    if (bSigned)
      dbg.Message(_func_, "Quantizing to 12 bit (input data has range from "
                          "%i to %i)", int(iMin) - numeric_limits<int>::max(),
                                       int(iMax) - numeric_limits<int>::max());
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
        UINT64 iValue = (bSigned) ? pInData[i] + numeric_limits<int>::max()
                                  : pInData[i];
        UINT64 iNewVal = min<UINT64>(4095,
                                     (UINT64)((UINT64(iValue-iMin) * 4095)/iRange));
        pInData[i] = iNewVal;
        aHist[iNewVal]++;
      }
      iPos += UINT64(iRead);

      if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
        if (bSigned)
          dbg.Message(_func_, "Quantizing to 12 bit (input data has range "
                      "from %i to %i)\n%i percent complete",
                      int(iMin) - numeric_limits<int>::max(),
                      int(iMax) - numeric_limits<int>::max(),
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
  }

  Histogram1D.SetHistogram(aHist);

  return strQuantFile;
}

const string AbstrConverter::QuantizeIntTo12Bits(UINT64 iHeaderSkip, const string& strFilename, const string& strTargetFilename, UINT64 iSize, bool bSigned, Histogram1DDataBlock& Histogram1D) {
  LargeRAWFile InputData(strFilename, iHeaderSkip);
  InputData.Open(false);
  UINT64 iPercent = iSize / 100;

  if (!InputData.IsOpen()) return "";

  vector<UINT64> aHist(4096);
  std::fill(aHist.begin(), aHist.end(), 0);

  // determine max and min
  UINT32 iMax = 0;
  UINT32 iMin = numeric_limits<UINT32>::max();
  UINT32* pInData = new UINT32[INCORESIZE];
  UINT64 iPos = 0;
  UINT64 iDivLast = 0;
  while (iPos < iSize)  {
    size_t iRead = InputData.ReadRAW((unsigned char*)pInData, INCORESIZE*4)/4;
    if (iRead == 0) break;

    for (size_t i = 0;i<iRead;i++) {
      UINT32 iValue = (bSigned) ? pInData[i] + numeric_limits<int>::max() : pInData[i];
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

    if (iMin == 0 && iMax == numeric_limits<UINT32>::max()) break;
  }

  if (iPos < iSize) {
    WARNING("Specified size and real datasize mismatch");
    iSize = iPos;
  }

  string strQuantFile;
  // if file uses less or equal than 12 bits quit here
  if (iMax < 4096) {
    MESSAGE("No quantization required (min=%i, max=%i)", iMin, iMax);
    aHist.resize(iMax+1);  // reduce the size to the filled size (the maximum value plus one (the zero value))
    delete [] pInData;
    InputData.Close();
    strQuantFile = strFilename;
  } else {
    if (bSigned) {
      MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)",
              int(iMin) - numeric_limits<int>::max(),
              int(iMax) - numeric_limits<int>::max());
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
        UINT32 iValue = (bSigned) ? pInData[i] + numeric_limits<int>::max() : pInData[i];
        UINT32 iNewVal = min<UINT32>(4095, (UINT32)((UINT64(iValue-iMin) * 4095)/iRange));
        pInData[i] = iNewVal;
        aHist[iNewVal]++;
      }
      iPos += UINT64(iRead);

      if (iPercent > 1 && (100*iPos)/iSize > iDivLast) {
        if (bSigned) {
          MESSAGE("Quantizing to 12 bit (input data has range from %i to %i)"
                  "\n%i percent complete",
                  int(iMin) - numeric_limits<int>::max(),
                  int(iMax) - numeric_limits<int>::max(),
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

  Histogram1D.SetHistogram(aHist);

  return strQuantFile;
}
