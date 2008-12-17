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
  \file    SystemInfo.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/

#pragma once

#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include "../Basics/MathTools.h"

class SystemInfo
{
public:
  SystemInfo();

  unsigned int GetProgrammBitWith() {return m_iProgrammBitWith;}
  UINT64 GetCPUMemSize() {return m_iCPUMemSize;}
  UINT64 GetGPUMemSize() {return m_iGPUMemSize;}
  UINT64 GetMaxUsableCPUMem() {return m_iUseMaxCPUMem;}
  UINT64 GetMaxUsableGPUMem() {return m_iUseMaxGPUMem;}
  void SetMaxUsableCPUMem(UINT64 iUseMaxCPUMem) {m_iUseMaxCPUMem = iUseMaxCPUMem;}
  void SetMaxUsableGPUMem(UINT64 iUseMaxGPUMem) {m_iUseMaxGPUMem = iUseMaxGPUMem;}
  unsigned int GetNumberOfCPUs() {return m_iNumberOfCPUs;}

private:
  unsigned ComputeNumCPUs();
  UINT64 ComputeCPUMemSize();
  UINT64 ComputeGPUMemory();

  unsigned int  m_iProgrammBitWith;
  UINT64        m_iCPUMemSize;
  UINT64        m_iUseMaxCPUMem;
  UINT64        m_iGPUMemSize;
  UINT64        m_iUseMaxGPUMem;
  unsigned int  m_iNumberOfCPUs;
};

#endif // SYSTEMINFO_H
