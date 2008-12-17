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
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    September 2008
*/

#include "KeyValueFileParser.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "../Basics/SysTools.h"

using namespace std;


KeyValPair::KeyValPair() : 
	strKey(""),
	wstrKey(L""),
	strKeyUpper(""),
	wstrKeyUpper(L""),

	strValue(""),
	wstrValue(L""),
	strValueUpper(""),
	wstrValueUpper(L""),
	uiValue(0),
	iValue(0),
	fValue(0),
	viValue(0,0,0),
	vfValue(0,0,0)

{}

KeyValPair::KeyValPair(const string& key, const string& value) :
	strKey(key),
	wstrKey(key.begin(), key.end()),

	strValue(value),
	wstrValue(value.begin(), value.end())
{
	istringstream ss1( value ), ss2( value ), ss3( value ), ss4( value ), ss5( value ), ss6( value );
	ss1 >> uiValue;
	ss2 >> iValue;
	ss3 >> fValue;
	ss4 >> viValue.x >> viValue.y >> viValue.z;
	ss5 >> vuiValue.x >> vuiValue.y >> vuiValue.z;
	ss6 >> vfValue.x >> vfValue.y >> vfValue.z;

	strKeyUpper  = strKey; transform(strKeyUpper.begin(), strKeyUpper.end(), strKeyUpper.begin(), ::toupper);
	wstrKeyUpper = wstrKey; transform(wstrKeyUpper.begin(), wstrKeyUpper.end(), wstrKeyUpper.begin(), ::toupper);
	strValueUpper  = strValue; transform(strValueUpper.begin(), strValueUpper.end(), strValueUpper.begin(), ::toupper);
	wstrValueUpper  = wstrValue; transform(wstrValueUpper.begin(), wstrValueUpper.end(), wstrValueUpper.begin(), ::toupper);
}

KeyValPair::KeyValPair(const wstring& key, const wstring& value) :
	strKey(key.begin(), key.end()),
	wstrKey(key),

	strValue(value.begin(), value.end()),
	wstrValue(value)
{
	wistringstream ss1( value ), ss2( value ), ss3( value ), ss4( value ), ss5( value ), ss6( value );
	ss1 >> uiValue;
	ss2 >> iValue;
	ss3 >> fValue;
	ss4 >> viValue.x >> viValue.y >> viValue.z;
	ss5 >> vuiValue.x >> vuiValue.y >> vuiValue.z;
	ss6 >> vfValue.x >> vfValue.y >> vfValue.z;

	strKeyUpper  = strKey; transform(strKeyUpper.begin(), strKeyUpper.end(), strKeyUpper.begin(), ::toupper);
	wstrKeyUpper = wstrKey; transform(wstrKeyUpper.begin(), wstrKeyUpper.end(), wstrKeyUpper.begin(), ::toupper);
	strValueUpper  = strValue; transform(strValueUpper.begin(), strValueUpper.end(), strValueUpper.begin(), ::toupper);
	wstrValueUpper  = wstrValue; transform(wstrValueUpper.begin(), wstrValueUpper.end(), wstrValueUpper.begin(), ::toupper);
}


KeyValueFileParser::KeyValueFileParser(const string& strFilename, char cToken)
{
	wstring wstrFilename(strFilename.begin(), strFilename.end());
	m_bFileReadable = ParseFile(wstrFilename,wchar_t(cToken));
}

KeyValueFileParser::KeyValueFileParser(const wstring& wstrFilename, wchar_t cToken)
{
	m_bFileReadable = ParseFile(wstrFilename,cToken);
}

KeyValueFileParser::~KeyValueFileParser()
{
}

KeyValPair* KeyValueFileParser::GetData(const string&  strKey, const bool bCaseSensitive) {
	if (!bCaseSensitive) {
		string upperKey(strKey);
		transform(upperKey.begin(), upperKey.end(), upperKey.begin(), ::toupper);
		for (uint i = 0;i<m_vecTokens.size();i++) if (m_vecTokens[i].strKeyUpper == upperKey) return &m_vecTokens[i];
	} else {
		for (uint i = 0;i<m_vecTokens.size();i++) if (m_vecTokens[i].strKey == strKey) return &m_vecTokens[i];
	}
	return NULL;
}

KeyValPair* KeyValueFileParser::GetData(const wstring& wstrKey, const bool bCaseSensitive) {
	if (!bCaseSensitive) {
		wstring wupperKey(wstrKey);
		transform(wupperKey.begin(), wupperKey.end(), wupperKey.begin(), ::toupper);
		for (uint i = 0;i<m_vecTokens.size();i++) if (m_vecTokens[i].wstrKeyUpper == wupperKey) return &m_vecTokens[i];
	} else {
		for (uint i = 0;i<m_vecTokens.size();i++) if (m_vecTokens[i].wstrKey == wstrKey) return &m_vecTokens[i];
	}
	return NULL;
}


bool KeyValueFileParser::ParseFile(wstring wstrFilename, wchar_t cToken) {
	wstring line;

	string strFilename(wstrFilename.begin(), wstrFilename.end());
	wifstream fileData(strFilename.c_str());	

	if (fileData.is_open())
	{
		while (! fileData.eof() )
		{
			getline (fileData,line);
			SysTools::RemoveLeadingWhitespace(line);

			if (line[0] == '#') continue;				// skip comments
			if (line.find_first_of(cToken) == string::npos) continue;  // skip invalid lines

			wstring strKey = line.substr(0, line.find_first_of(cToken));
			SysTools::RemoveTailingWhitespace(strKey);

			line = line.substr(line.find_first_of(cToken)+1, line.length());
			SysTools::RemoveLeadingWhitespace(line);
			SysTools::RemoveTailingWhitespace(line);

			if (strKey.length() == 0 || line.length() == 0) continue;

			KeyValPair newKey(strKey, line);
			m_vecTokens.push_back(newKey);
		}
		fileData.close();
	} else return false;

	return true;
}
