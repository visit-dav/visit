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
  \file    AbstrDebugOut.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    August 2008
*/


#pragma once

#ifndef ABSTRDEBUGOUT_H
#define ABSTRDEBUGOUT_H

class AbstrDebugOut {
  public:
#ifdef _DEBUG
    AbstrDebugOut() : m_bShowMessages(true), m_bShowWarnings(true), m_bShowErrors(true), m_bShowOther(true) {}
#else
    AbstrDebugOut() : m_bShowMessages(false), m_bShowWarnings(false), m_bShowErrors(true), m_bShowOther(true) {}
#endif
    virtual ~AbstrDebugOut() {}
    virtual void printf(const char* format, ...) = 0;
    virtual void Message(const char* source, const char* format, ...) = 0;
    virtual void Warning(const char* source, const char* format, ...) = 0;
    virtual void Error(const char* source, const char* format, ...) = 0;

    bool m_bShowMessages;
    bool m_bShowWarnings;
    bool m_bShowErrors;
    bool m_bShowOther;
};

#endif // ABSTRDEBUGOUT_H
