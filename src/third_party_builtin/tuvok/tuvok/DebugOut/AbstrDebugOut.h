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
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    August 2008
*/


#pragma once

#ifndef ABSTRDEBUGOUT_H
#define ABSTRDEBUGOUT_H

#include <deque>
#include <string>
#include "../StdTuvokDefines.h"

class AbstrDebugOut {
  public:
#ifdef _DEBUG
    AbstrDebugOut() :
        m_bShowMessages(false),
        m_bShowWarnings(true),
        m_bShowErrors(true),
        m_bShowOther(true)
    {
        m_bRecordLists[0] = false;
        m_bRecordLists[1] = false;
        m_bRecordLists[2] = false;
    }
#else
    AbstrDebugOut() :
        m_bShowMessages(false),
        m_bShowWarnings(false),
        m_bShowErrors(true),
        m_bShowOther(true)
    {
        m_bRecordLists[0] = false;
        m_bRecordLists[1] = false;
        m_bRecordLists[2] = false;
    }
#endif
    virtual ~AbstrDebugOut() {
      m_bRecordLists[0] = false;
      m_bRecordLists[1] = false;
      m_bRecordLists[2] = false;
    }
    virtual void printf(const char* format, ...) const = 0;
    virtual void Message(const char* source, const char* format, ...) = 0;
    virtual void Warning(const char* source, const char* format, ...) = 0;
    virtual void Error(const char* source, const char* format, ...) = 0;

    void PrintErrorList();
    void PrintWarningList();
    void PrintMessageList();

    virtual void ClearErrorList()   {m_strErrorList.clear();}
    virtual void ClearWarningList() {m_strWarningList.clear();}
    virtual void ClearMessageList() {m_strMessageList.clear();}

    virtual void SetListRecordingErrors(bool bRecord)   {m_bRecordLists[0] = bRecord;}
    virtual void SetListRecordingWarnings(bool bRecord) {m_bRecordLists[1] = bRecord;}
    virtual void SetListRecordingMessages(bool bRecord) {m_bRecordLists[2] = bRecord;}
    virtual bool GetListRecordingErrors()   {return m_bRecordLists[0];}
    virtual bool GetListRecordingWarnings() {return m_bRecordLists[1];}
    virtual bool GetListRecordingMessages() {return m_bRecordLists[2];}

    void SetOutput(bool bShowErrors, bool bShowWarnings, bool bShowMessages,
                   bool bShowOther);
    void GetOutput(bool& bShowErrors, bool& bShowWarnings, bool& bShowMessages,
                   bool& bShowOther) const;

    bool ShowMessages() const {return m_bShowMessages;}
    bool ShowWarnings() const {return m_bShowWarnings;}
    bool ShowErrors() const {return m_bShowErrors;}
    bool ShowOther() const {return m_bShowOther;}

    virtual void SetShowMessages(bool bShowMessages);
    virtual void SetShowWarnings(bool bShowWarnings);
    virtual void SetShowErrors(bool bShowErrors);
    virtual void SetShowOther(bool bShowOther);

protected:
    bool                      m_bShowMessages;
    bool                      m_bShowWarnings;
    bool                      m_bShowErrors;
    bool                      m_bShowOther;

    bool                      m_bRecordLists[3];

    std::deque< std::string > m_strErrorList;
    std::deque< std::string > m_strWarningList;
    std::deque< std::string > m_strMessageList;
};

#endif // ABSTRDEBUGOUT_H
