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
  \file    SysTools.h
  \brief    Simple routines for filename handling
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.5
  \date    Dec 2008
*/

#pragma once

#ifndef SYSTOOLS_H
#define SYSTOOLS_H

#include <string>
#include <vector>

#ifdef _WIN32
  #include <windows.h>

  #ifdef max
    #undef max
  #endif

  #ifdef min
    #undef min
  #endif

#else
  #include <wchar.h>
  typedef wchar_t WCHAR;
  typedef unsigned char CHAR;
#endif

namespace SysTools {

  std::wstring ToLowerCase(const std::wstring& str);
  std::string ToLowerCase(const std::string& str);
  std::wstring ToUpperCase(const std::wstring& str);
  std::string ToUpperCase(const std::string& str);

  std::string GetFromResourceOnMac(const std::string& fileName);
  std::wstring GetFromResourceOnMac(const std::wstring& fileName);

  bool FileExists(const std::string& fileName);
  bool FileExists(const std::wstring& fileName);

  std::string GetExt(const std::string& fileName);
  std::wstring GetExt(const std::wstring& fileName);

  std::string GetPath(const std::string& fileName);
  std::wstring GetPath(const std::wstring& fileName);

  std::string GetFilename(const std::string& fileName);
  std::wstring GetFilename(const std::wstring& fileName);

  std::string FindPath(const std::string& fileName, const std::string& path);
  std::wstring FindPath(const std::wstring& fileName, const std::wstring& path);

  std::string  RemoveExt(const std::string& fileName);
  std::wstring RemoveExt(const std::wstring& fileName);

  std::string  ChangeExt(const std::string& fileName, const std::string& newext);
  std::wstring ChangeExt(const std::wstring& fileName, const std::wstring& newext);

  std::string  AppendFilename(const std::string& fileName, const std::string& tag);
  std::wstring AppendFilename(const std::wstring& fileName, const std::wstring& tag);

  std::string  FindNextSequenceName(const std::string& fileName, const std::string& ext, const std::string& dir="");
  std::wstring FindNextSequenceName(const std::wstring& fileName, const std::wstring& ext, const std::wstring& dir=L"");

  unsigned int FindNextSequenceIndex(const std::string& fileName, const std::string& ext, const std::string& dir="");
  unsigned int FindNextSequenceIndex(const std::wstring& fileName, const std::wstring& ext, const std::wstring& dir=L"");

#ifdef _WIN32
  std::vector<std::wstring> GetDirContents(const std::wstring& dir, const std::wstring& fileName=L"*", const std::wstring& ext=L"*");
  std::vector<std::string> GetDirContents(const std::string& dir, const std::string& fileName="*", const std::string& ext="*");
#else
  std::vector<std::wstring> GetDirContents(const std::wstring& dir, const std::wstring& fileName=L"*", const std::wstring& ext=L"");
  std::vector<std::string> GetDirContents(const std::string& dir, const std::string& fileName="*", const std::string& ext="");
#endif

  std::vector<std::wstring> GetSubDirList(const std::wstring& dir);
  std::vector<std::string> GetSubDirList(const std::string& dir);
  
  bool GetFileStats(const std::string& strFileName, struct stat& stat_buf);
  bool GetFileStats(const std::wstring& wstrFileName, struct stat& stat_buf);

  void RemoveLeadingWhitespace(std::wstring &str);
  void RemoveLeadingWhitespace(std::string &str);
  void RemoveTailingWhitespace(std::wstring &str);
  void RemoveTailingWhitespace(std::string &str);

#ifdef _WIN32
  bool GetFilenameDialog(const std::string& lpstrTitle, const CHAR* lpstrFilter, std::string &filename, const bool save, HWND owner=NULL, DWORD* nFilterIndex=NULL);
  bool GetFilenameDialog(const std::wstring& lpstrTitle, const WCHAR* lpstrFilter, std::wstring &filename, const bool save, HWND owner=NULL, DWORD* nFilterIndex=NULL);
#endif

  class CmdLineParams {
    public:
      #ifdef _WIN32
        CmdLineParams();
      #endif
      CmdLineParams(int argc, char** argv);

      bool SwitchSet(const std::string& parameter);
      bool SwitchSet(const std::wstring& parameter);

      bool GetValue(const std::string& parameter, double& value);
      bool GetValue(const std::wstring& parameter, double& value);
      bool GetValue(const std::string& parameter, float& value);
      bool GetValue(const std::wstring& parameter, float& value);
      bool GetValue(const std::string& parameter, int& value);
      bool GetValue(const std::wstring& parameter, int& value);
      bool GetValue(const std::string& parameter, std::string& value);
      bool GetValue(const std::wstring& parameter, std::wstring& value);
    
    protected:
      std::vector<std::string> m_strArrayParameters;
      std::vector<std::string> m_strArrayValues;

      std::string m_strFilename;
  };

}

#endif // SYSTOOLS_H
