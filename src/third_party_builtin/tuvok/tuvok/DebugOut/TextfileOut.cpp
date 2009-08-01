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
  \file    TextfileOut.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#include <fstream>
#include <stdarg.h>
#ifdef WIN32
  #include <windows.h>
#endif
#include "TextfileOut.h"
#include <ctime>

using namespace std;

TextfileOut::TextfileOut(std::string strFilename) :
  m_strFilename(strFilename)
{
  this->printf("MESSAGE (TextfileOut::TextfileOut:): Starting up");
}

TextfileOut::~TextfileOut() {
  this->printf("MESSAGE (TextfileOut::~TextfileOut:): Shutting down\n\n\n");
}

void TextfileOut::printf(const char* format, ...) const
{
  if (!m_bShowOther) return;

  char buff[16384];

  va_list args;
  va_start(args, format);
#ifdef WIN32
  _vsnprintf_s( buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf( buff, sizeof(buff), format, args);
#endif
  va_end(args);

  time_t epoch_time;
  time(&epoch_time);

#ifdef DETECTED_OS_WINDOWS
  struct tm now;
  localtime_s(&now, &epoch_time);
#define ADDR_NOW (&now)
#else
  struct tm* now;
  now = localtime(&epoch_time);
#define ADDR_NOW (now)
#endif
  char datetime[64];

  ofstream fs;
  fs.open(m_strFilename.c_str(),  ios_base::app);
  if (fs.fail()) return;

  if(strftime(datetime, 64, "(%d.%m.%Y %H:%M:%S)", ADDR_NOW) > 0) {
    fs << datetime << " " << buff << std::endl;
  } else {
    fs << buff << std::endl;
  }

  fs.flush();
  fs.close();
}

void TextfileOut::_printf(const char* format, ...) const
{
  char buff[16384];

  va_list args;
  va_start(args, format);

#ifdef WIN32
  _vsnprintf_s( buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf( buff, sizeof(buff), format, args);
#endif
  va_end(args);

  time_t epoch_time;
  time(&epoch_time);

#ifdef DETECTED_OS_WINDOWS
  struct tm now;
  localtime_s(&now, &epoch_time);
#undef ADDR_NOW
#define ADDR_NOW (&now)
#else
  struct tm* now;
  now = localtime(&epoch_time);
#define ADDR_NOW (now)
#endif
  char datetime[64];

  ofstream fs;
  fs.open(m_strFilename.c_str(),  ios_base::app);
  if (fs.fail()) return;
  if(strftime(datetime, 64, "(%d.%m.%Y %H:%M:%S)", ADDR_NOW) > 0) {
    fs << datetime << " " << buff << std::endl;
  } else {
    fs << buff << std::endl;
  }
  fs.flush();
  fs.close();
}

void TextfileOut::Message(const char* source, const char* format, ...) {
  if (!m_bShowMessages) return;
  char buff[16384];
  va_list args;
  va_start(args, format);
#ifdef WIN32
  _vsnprintf_s( buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf( buff, sizeof(buff), format, args);
#endif
  va_end(args);
  this->_printf("MESSAGE (%s): %s", source, buff);
}

void TextfileOut::Warning(const char* source, const char* format, ...) {
  if (!m_bShowWarnings) return;
  char buff[16384];
  va_list args;
  va_start(args, format);
#ifdef WIN32
  _vsnprintf_s( buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf( buff, sizeof(buff), format, args);
#endif
  va_end(args);
  this->_printf("WARNING (%s): %s", source, buff);
}

void TextfileOut::Error(const char* source, const char* format, ...) {
  if (!m_bShowErrors) return;
  char buff[16384];
  va_list args;
  va_start(args, format);
#ifdef WIN32
  _vsnprintf_s( buff, 16384, sizeof(buff), format, args);
#else
  vsnprintf( buff, sizeof(buff), format, args);
#endif
  va_end(args);
  this->_printf("ERROR (%s): %s", source, buff);
}
