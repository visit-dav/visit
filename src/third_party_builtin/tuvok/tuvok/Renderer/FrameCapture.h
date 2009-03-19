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
  \file    FrameCapture.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    November 2008
*/

#pragma once

#ifndef FRAMECAPTURE_H
#define FRAMECAPTURE_H

#include "../StdTuvokDefines.h"
#include <string>
#include "../Basics/SysTools.h"
#include "../Basics/Vectors.h"
#include <QtGui/QImage>

class FrameCapture {
  public:
    FrameCapture() {}
    virtual ~FrameCapture() {}

    virtual bool CaptureSingleFrame(const std::string& strFilename, bool bPreserveTransparency) const = 0;
    bool CaptureSequenceFrame(const std::string& strFilename, bool bPreserveTransparency, std::string* strRealFilename=NULL) {
      std::string strSequenceName = SysTools::FindNextSequenceName(strFilename);
      if (strRealFilename) (*strRealFilename) = strSequenceName;
      return CaptureSingleFrame(strSequenceName, bPreserveTransparency);
    }
  protected:
    bool SaveImage(const std::string& strFilename, const UINTVECTOR2& vSize,
                   unsigned char* pData, bool bPreserveTransparency) const {
      QImage qTargetFile(QSize(vSize.x, vSize.y), QImage::Format_ARGB32);

      size_t i = 0;
      if (bPreserveTransparency) {
        for (int y = 0;y<int(vSize.y);y++) {
          for (int x = 0;x<int(vSize.x);x++) {
            qTargetFile.setPixel(x,(vSize.y-1)-y,
                                 qRgba(int(pData[i+0]),
                                       int(pData[i+1]),
                                       int(pData[i+2]),
                                       int(pData[i+3])));
            i+=4;
          }
        }
      } else {
        for (int y = 0;y<int(vSize.y);y++) {
          for (int x = 0;x<int(vSize.x);x++) {
            qTargetFile.setPixel(x,(vSize.y-1)-y,
                                qRgba(int(pData[i+0]),
                                      int(pData[i+1]),
                                      int(pData[i+2]),
                                      255));
            i+=4;
          }
        }
      }

      return qTargetFile.save(strFilename.c_str());
    }
};

#endif // FRAMECAPTURE_H
