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
  \file    MultiplexOut.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    September 2008
*/

#include "MultiplexOut.h"

#ifdef WIN32
  #include <windows.h>
#endif

#include <stdarg.h>

#include <fstream>
using namespace std;

MultiplexOut::~MultiplexOut() {
  for (size_t i = 0;i<m_vpDebugger.size();i++) {
    if (m_vbDeleteOnExit[i]) {
      m_vpDebugger[i]->printf("MESSAGE (MultiplexOut::~MultiplexOut:): Shutting down");
      delete m_vpDebugger[i];
    }
  }
}

void MultiplexOut::AddDebugOut(AbstrDebugOut* pDebugger, bool bDeleteOnExit) {
  m_vpDebugger.push_back(pDebugger);
  m_vbDeleteOnExit.push_back(bDeleteOnExit);
  pDebugger->Message("MultiplexOut::AddDebugOut","Operating as part of a multiplexed debug out now.");
}

void MultiplexOut::RemoveDebugOut(AbstrDebugOut* pDebugger) {
  size_t iFound = size_t(-1);
  for (size_t i = 0;i<m_vpDebugger.size();i++) {
    if (m_vpDebugger[i] == pDebugger) {
      iFound = i;
      break;     
    } else {
      // recursivley check for occurences of pDebugger
      MultiplexOut* p = dynamic_cast<MultiplexOut*>(m_vpDebugger[i]);
      if (p != NULL) p->RemoveDebugOut(pDebugger);
    }
  }

  if (iFound != size_t(-1)) {
    if (m_vbDeleteOnExit[iFound]) delete m_vpDebugger[iFound];

    m_vpDebugger.erase(m_vpDebugger.begin()+iFound);
    m_vbDeleteOnExit.erase(m_vbDeleteOnExit.begin()+iFound);
  }

}


void MultiplexOut::printf(const char* format, ...)
{
  char buff[16384];
  va_list args;
  va_start(args, format);
#ifdef WIN32
  _vsnprintf_s( buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf( buff, sizeof(buff), format, args);
#endif

  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->printf(buff);
}

void MultiplexOut::Message(const char* source, const char* format, ...) {
  char buff[16384];
  va_list args;
  va_start(args, format);
#ifdef WIN32
  _vsnprintf_s( buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf( buff, sizeof(buff), format, args);
#endif
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->Message(source,buff);
}

void MultiplexOut::Warning(const char* source, const char* format, ...) {
  char buff[16384];
  va_list args;
  va_start(args, format);
#ifdef WIN32
  _vsnprintf_s( buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf( buff, sizeof(buff), format, args);
#endif
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->Warning(source,buff);
}

void MultiplexOut::Error(const char* source, const char* format, ...) {
  char buff[16384];
  va_list args;
  va_start(args, format);
#ifdef WIN32
  _vsnprintf_s( buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf( buff, sizeof(buff), format, args);
#endif
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->Error(source,buff);
}
