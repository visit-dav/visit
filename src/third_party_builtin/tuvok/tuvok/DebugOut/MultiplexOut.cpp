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
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    September 2008
*/

#include "MultiplexOut.h"

#ifdef WIN32
  #include <windows.h>
  // undef stupid windows defines to max and min
  #ifdef max
  #undef max
  #endif

  #ifdef min
  #undef min
  #endif
#endif

#include <stdarg.h>
#include <algorithm>

#include <fstream>
using namespace std;

MultiplexOut::~MultiplexOut() {
  for (size_t i = 0;i<m_vpDebugger.size();i++) {
    m_vpDebugger[i]->Message(_func_, "(MultiplexOut::~MultiplexOut): Shutting down");
    delete m_vpDebugger[i];
  }
}

void MultiplexOut::AddDebugOut(AbstrDebugOut* pDebugger) {
  m_vpDebugger.push_back(pDebugger);
  pDebugger->Message(_func_,"Operating as part of a multiplexed debug out now.");
}

void MultiplexOut::RemoveDebugOut(AbstrDebugOut* pDebugger) {
  std::vector<AbstrDebugOut*>::iterator del;

  del = std::find(m_vpDebugger.begin(), m_vpDebugger.end(), pDebugger);

  if(del != m_vpDebugger.end()) {
    delete *del;
    m_vpDebugger.erase(del);
  }
}


void MultiplexOut::printf(const char* format, ...) const
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

void MultiplexOut::SetShowMessages(bool bShowMessages) {
  AbstrDebugOut::SetShowMessages(bShowMessages);
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->SetShowMessages(bShowMessages);
}

void MultiplexOut::SetShowWarnings(bool bShowWarnings) {
  AbstrDebugOut::SetShowWarnings(bShowWarnings);
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->SetShowWarnings(bShowWarnings);
}

void MultiplexOut::SetShowErrors(bool bShowErrors) {
  AbstrDebugOut::SetShowErrors(bShowErrors);
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->SetShowErrors(bShowErrors);
}

void MultiplexOut::SetShowOther(bool bShowOther) {
  AbstrDebugOut::SetShowOther(bShowOther);
  for (size_t i = 0;i<m_vpDebugger.size();i++) m_vpDebugger[i]->SetShowOther(bShowOther);
}

