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
  \file    KeyValueFileParser.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    September 2008
*/

#pragma once

#ifndef KEYVALUEFILEPARSER_H
#define KEYVALUEFILEPARSER_H

#ifdef _WIN32
  #pragma warning (disable : 4995)
#endif
#include <string>
#include <vector>
#ifdef _WIN32
  #pragma warning (default : 4995)
#endif

#include "../StdTuvokDefines.h"

class KeyValPair {
public:

  KeyValPair();
  KeyValPair(const std::string& key, const std::string& value);
  KeyValPair(const std::wstring& key, const std::wstring& value);

  // keys
  std::string   strKey;
  std::wstring  wstrKey;
  std::string   strKeyUpper;
  std::wstring  wstrKeyUpper;

  // values
  std::string               strValue;
  std::wstring              wstrValue;
  std::string               strValueUpper;
  std::wstring              wstrValueUpper;
  int                       iValue;
  UINT32                    uiValue;
  float                     fValue;
  std::vector<int>          viValue;
  std::vector<UINT32>       vuiValue;
  std::vector<float>        vfValue;
  std::vector<std::string>  vstrValue;
  std::vector<std::wstring> vwstrValue;

private:
  void FillDerivedData();
};


class KeyValueFileParser
{
public:
  KeyValueFileParser(const std::string& strFilename, bool bStopOnEmptyLine=false, const std::string& strToken = ":", const std::string& strEndToken = "");
  KeyValueFileParser(const std::wstring& wstrFilename, bool bStopOnEmptyLine=false, const std::wstring& wstrToken = L":", const std::wstring& wstrEndToken = L"");

  ~KeyValueFileParser(void);

  KeyValPair* GetData(const std::string&  strKey, const bool bCaseSensitive=false);
  KeyValPair* GetData(const std::wstring& wstrKey, const bool bCaseSensitive=false);
  const KeyValPair* GetData(const std::string&  strKey,
                            const bool bCaseSensitive=false) const;

  bool FileReadable() const {return m_bFileReadable;}

  size_t GetStopPos() {return m_iStopPos;}

protected:
  std::vector<KeyValPair> m_vecTokens;
  bool m_bFileReadable;
  size_t m_iStopPos;

  bool ParseFile(const std::string& strFilename, bool bStopOnEmptyLine, const std::string& strToken, const std::string& strEndToken);
};

#endif // KEYVALUEFILEPARSER_H
