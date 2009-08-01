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
  \file    KeyValueFileParser.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    September 2008
*/

#include "KeyValueFileParser.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <Basics/SysTools.h>

using namespace std;
using namespace SysTools;

KeyValPair::KeyValPair() :
  strKey(""),
  wstrKey(L""),
  strKeyUpper(""),
  wstrKeyUpper(L""),
  strValue(""),
  wstrValue(L""),
  strValueUpper(""),
  wstrValueUpper(L"")
{}

KeyValPair::KeyValPair(const string& key, const string& value) :
  strKey(key),
  wstrKey(key.begin(), key.end()),

  strValue(value),
  wstrValue(value.begin(), value.end())
{
  vstrValue = Tokenize(value);
  for (size_t i = 0;i<vstrValue.size();i++) {
    vwstrValue.push_back(wstring(vstrValue[i].begin(), vstrValue[i].end()));
  }
  FillDerivedData();
}

KeyValPair::KeyValPair(const wstring& key, const wstring& value) :
  strKey(key.begin(), key.end()),
  wstrKey(key),

  strValue(value.begin(), value.end()),
  wstrValue(value)
{
  vwstrValue = Tokenize(value);
  for (size_t i = 0;i<vwstrValue.size();i++) {
    vstrValue.push_back(string(vwstrValue[i].begin(), vwstrValue[i].end()));
  }
  FillDerivedData();
}

void KeyValPair::FillDerivedData() {
  int    _iValue;
  UINT32 _uiValue;
  float  _fValue;


  for (size_t i = 0;i<vwstrValue.size();i++) {
    if (FromString(_iValue, vstrValue[i]))
      viValue.push_back(_iValue);
    else
      viValue.push_back(0);

    if (FromString(_uiValue, vstrValue[i]))
      vuiValue.push_back(_uiValue);
    else
      vuiValue.push_back(0);

    if (FromString(_fValue, vstrValue[i]))
      vfValue.push_back(_fValue);
    else
      vfValue.push_back(0.0f);
  }

  if (vwstrValue.size() > 0) {
    iValue  = viValue[0];
    uiValue = vuiValue[0];
    fValue  = vfValue[0];
  } else {
    iValue  = 0;
    uiValue = 0;
    fValue  = 0.0f;
  }

  strKeyUpper  = ToUpperCase(strKey);
  wstrKeyUpper = ToUpperCase(wstrKey);
  strValueUpper  = ToUpperCase(strValue);
  wstrValueUpper  = ToUpperCase(wstrValue);
}


KeyValueFileParser::KeyValueFileParser(const string& strFilename, bool bStopOnEmptyLine, const string& strToken, const std::string& strEndToken)
{
  m_bFileReadable = ParseFile(strFilename, bStopOnEmptyLine, strToken, strEndToken);
}

KeyValueFileParser::KeyValueFileParser(const wstring& wstrFilename, bool bStopOnEmptyLine, const wstring& wstrToken, const std::wstring& wstrEndToken)
{
  string strFilename(wstrFilename.begin(), wstrFilename.end());
  string strToken(wstrToken.begin(), wstrToken.end());
  string strEndToken(wstrEndToken.begin(), wstrEndToken.end());

  m_bFileReadable = ParseFile(strFilename, bStopOnEmptyLine, strToken, strEndToken);
}

KeyValueFileParser::~KeyValueFileParser()
{
}

KeyValPair* KeyValueFileParser::GetData(const string&  strKey, const bool bCaseSensitive) {
  if (!bCaseSensitive) {
    string upperKey(strKey);
    transform(upperKey.begin(), upperKey.end(), upperKey.begin(), ::toupper);
    for (UINT32 i = 0;i<m_vecTokens.size();i++) if (m_vecTokens[i].strKeyUpper == upperKey) return &m_vecTokens[i];
  } else {
    for (UINT32 i = 0;i<m_vecTokens.size();i++) if (m_vecTokens[i].strKey == strKey) return &m_vecTokens[i];
  }
  return NULL;
}

KeyValPair* KeyValueFileParser::GetData(const wstring& wstrKey, const bool bCaseSensitive) {
  if (!bCaseSensitive) {
    wstring wupperKey(wstrKey);
    transform(wupperKey.begin(), wupperKey.end(), wupperKey.begin(), ::toupper);
    for (UINT32 i = 0;i<m_vecTokens.size();i++) if (m_vecTokens[i].wstrKeyUpper == wupperKey) return &m_vecTokens[i];
  } else {
    for (UINT32 i = 0;i<m_vecTokens.size();i++) if (m_vecTokens[i].wstrKey == wstrKey) return &m_vecTokens[i];
  }
  return NULL;
}

struct matching_keys : public std::binary_function<std::string,
                                                   KeyValPair,
                                                   bool> {
  bool operator()(const std::string &key, const KeyValPair &kv) const {
    return key == kv.strKey;
  }
};

const KeyValPair* KeyValueFileParser::GetData(const std::string& strKey,
                                              const bool bCaseSensitive) const
{
  std::string key(strKey);
  if(!bCaseSensitive) {
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
  }

  std::vector<KeyValPair>::const_iterator iter;
  iter = std::find_if(m_vecTokens.begin(), m_vecTokens.end(),
                      std::bind1st(matching_keys(), key));

  if(iter == m_vecTokens.end()) {
    return NULL;
  }
  return &(*iter);
}


bool KeyValueFileParser::ParseFile(const std::string& strFilename, bool bStopOnEmptyLine, const std::string& strToken, const std::string& strEndToken) {
  string line;
  ifstream fileData(strFilename.c_str(),ios::binary);

  m_iStopPos = 0;
  if (fileData.is_open())
  {
    while (! fileData.eof() )
    {
      getline (fileData,line);
      RemoveLeadingWhitespace(line);

      // remove windows line endings
      if (line.length() > 0 && line[line.length()-1] == 13)
        line = line.substr(0,line.length()-1);

      if ((strEndToken != "" && strEndToken == line) ||
          (bStopOnEmptyLine && line.empty()))  {
        m_iStopPos = fileData.tellg();
        break;
      }

      if (line[0] == '#') continue;        // skip comments
      if (line.find_first_of(strToken) == string::npos) continue;  // skip invalid lines

      string strKey = line.substr(0, line.find_first_of(strToken));
      RemoveTailingWhitespace(strKey);

      line = line.substr(line.find_first_of(strToken)+strToken.length(), line.length());
      RemoveLeadingWhitespace(line);
      RemoveTailingWhitespace(line);

      if (strKey.length() == 0 || line.length() == 0) continue;

      KeyValPair newKey(strKey, line);
      m_vecTokens.push_back(newKey);
    }
    fileData.close();
  } else return false;

  return true;
}
