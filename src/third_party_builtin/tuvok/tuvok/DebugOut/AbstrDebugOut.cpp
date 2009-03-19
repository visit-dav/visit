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
  \file    AbstrDebugOut.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    January 2009
*/

#include "AbstrDebugOut.h"

void AbstrDebugOut::PrintErrorList() {
  printf( "Printing recorded errors:" );
  for (std::deque< std::string >::const_iterator i = m_strErrorList.begin();
       i < m_strErrorList.end(); i++)
    printf( i->c_str() );
  printf( "end of recorded errors" );
}

void AbstrDebugOut::PrintWarningList() {
  printf( "Printing recorded errors:" );
  for (std::deque< std::string >::const_iterator i = m_strWarningList.begin();
       i < m_strErrorList.end(); i++)
    printf( i->c_str() );
  printf( "end of recorded errors" );
}

void AbstrDebugOut::PrintMessageList() {
  printf( "Printing recorded errors:" );
  for (std::deque< std::string >::const_iterator i = m_strMessageList.begin();
       i < m_strErrorList.end(); i++)
    printf( i->c_str() );
  printf( "end of recorded errors" );
}

void AbstrDebugOut::SetOutput(bool bShowErrors,
                              bool bShowWarnings,
                              bool bShowMessages,
                              bool bShowOther) {
  SetShowMessages(bShowMessages);
  SetShowWarnings(bShowWarnings);
  SetShowErrors(bShowErrors);
  SetShowOther(bShowOther);
}

void AbstrDebugOut::GetOutput(bool& bShowErrors,
                              bool& bShowWarnings,
                              bool& bShowMessages,
                              bool& bShowOther) const {

  bShowMessages = ShowMessages();
  bShowWarnings = ShowWarnings();
  bShowErrors   = ShowErrors();
  bShowOther    = ShowOther();
}


void AbstrDebugOut::SetShowMessages(bool bShowMessages) {
  m_bShowMessages = bShowMessages;
}

void AbstrDebugOut::SetShowWarnings(bool bShowWarnings) {
  m_bShowWarnings = bShowWarnings;
}

void AbstrDebugOut::SetShowErrors(bool bShowErrors) {
  m_bShowErrors = bShowErrors;
}

void AbstrDebugOut::SetShowOther(bool bShowOther) {
  m_bShowOther = bShowOther;
}
