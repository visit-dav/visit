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
  \file    SysTools.cpp
  \brief    Simple routines for filename handling
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.5
  \date    Dec 2008
*/

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <functional>
#include <iterator>
#include <sstream>
#include <sys/stat.h>

#include "StdDefines.h"
#ifndef _WIN32
  #include <regex.h>
  #include <dirent.h>
  #include <unistd.h>
#endif

#ifdef DETECTED_OS_APPLE
  #include <CoreFoundation/CoreFoundation.h>
#endif

// define MAX / MIN
#ifndef MAX
  #define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
  #define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include "SysTools.h"

using namespace std;

namespace SysTools {

  vector< string > Tokenize(const string& strInput, bool bQuoteprotect) {
    if (bQuoteprotect) {
      string buf;
      stringstream ss(strInput);
      vector<string> strElements;
      bool bProtected = false;
      while (ss >> buf) {
        string cleanBuf = buf;
        if (cleanBuf[0] == '\"')
          cleanBuf = cleanBuf.substr(1, cleanBuf.length()-1);
        if (cleanBuf[cleanBuf.size()-1] == '\"')
          cleanBuf = cleanBuf.substr(0, cleanBuf.length()-1);

        if (bProtected) {
          size_t end = strElements.size()-1;
          strElements[end] = strElements[end] + " " + cleanBuf;
        }
        else
          strElements.push_back(cleanBuf);

        if (buf[0] == '\"')            bProtected = true;
        if (buf[buf.size()-1] == '\"') bProtected = false;
      }
      return strElements;
    } else {
      string buf;
      stringstream ss(strInput);
      vector<string> strElements;
      while (ss >> buf) strElements.push_back(buf);
      return strElements;
    }
  }

  vector< wstring > Tokenize(const wstring& strInput, bool bQuoteprotect) {
    if (bQuoteprotect) {
      wstring buf;
      wstringstream ss(strInput);
      vector<wstring> strElements;
      bool bProtected = false;
      while (ss >> buf) {
        wstring cleanBuf = buf;
        if (cleanBuf[0] == '\"') cleanBuf = cleanBuf.substr(1, cleanBuf.length()-1);
        if (cleanBuf[cleanBuf.size()-1] == '\"') cleanBuf = cleanBuf.substr(0, cleanBuf.length()-1);

        if (bProtected)
          strElements[strElements.size()-1] = strElements[strElements.size()-1] + L" " + cleanBuf;
        else
          strElements.push_back(cleanBuf);

        if (buf[0] == '\"')            bProtected = true;
        if (buf[buf.size()-1] == '\"') bProtected = false;
      }
      return strElements;
    } else {
      wstring buf;
      wstringstream ss(strInput);
      vector<wstring> strElements;
      while (ss >> buf) strElements.push_back(buf);
      return strElements;
    }
  }

  string GetFromResourceOnMac(const string& strFileName) {
    #ifdef DETECTED_OS_APPLE
      CFStringRef cfFilename = CFStringCreateWithCString(kCFAllocatorDefault, RemoveExt(GetFilename(strFileName)).c_str(), CFStringGetSystemEncoding());
      CFStringRef cfExt = CFStringCreateWithCString(kCFAllocatorDefault, GetExt(GetFilename(strFileName)).c_str(), CFStringGetSystemEncoding());

      CFURLRef    imageURL = CFBundleCopyResourceURL( CFBundleGetMainBundle(), cfFilename, cfExt, NULL );
      if (imageURL == NULL) return "";
      CFStringRef macPath = CFURLCopyFileSystemPath(imageURL, kCFURLPOSIXPathStyle);
      const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
      if (macPath != 0 && pathPtr != 0) {
        string result = pathPtr;
        return result;
      } else return strFileName;
    #else
      return strFileName;
    #endif

  }

  wstring GetFromResourceOnMac(const wstring& wstrFileName) {
    #ifdef DETECTED_OS_APPLE
      // for now just call the string method by converting the unicode string down to an 8bit string
      string strFileName(wstrFileName.begin(), wstrFileName.end());
      string strResult = GetFromResourceOnMac(strFileName);
      wstring wstrResult(strResult.begin(), strResult.end());
      return wstrResult;
    #else
      return wstrFileName;
    #endif

  }

  void ReplaceAll(string& input, const string& search, const string& replace) {
    size_t pos=0;
    while(pos<input.size())
    {
      pos=input.find(search,pos);
      if(pos==std::string::npos) break;
      input.replace(pos,search.size(),replace);
      pos +=replace.size();
    }
  }

  void ReplaceAll(wstring& input, const wstring& search, const wstring& replace) {
    size_t pos=0;
    while(pos<input.size())
    {
      pos=input.find(search,pos);
      if(pos==std::string::npos) break;
      input.replace(pos,search.size(),replace);
      pos +=replace.size();
    }
  }

  inline int myTolower(int c) {return tolower(static_cast<unsigned char>(c));}
  inline int myToupper(int c) {return toupper(static_cast<unsigned char>(c));}

  void RemoveLeadingWhitespace(wstring &str) {
    while (int(str.find_first_not_of(L" ")) > 0 || int(str.find_first_not_of(L"\t")) > 0
           || int(str.find_first_not_of(L"\r")) > 0 || int(str.find_first_not_of(L"\n")) > 0)  {
      str = str.substr(str.find_first_not_of(L" "), str.length());
      str = str.substr(str.find_first_not_of(L"\t"), str.length());
      str = str.substr(str.find_first_not_of(L"\n"), str.length());
      str = str.substr(str.find_first_not_of(L"\r"), str.length());
    }
  }

  void RemoveLeadingWhitespace(string &str) {
    while (int(str.find_first_not_of(" ")) > 0 || int(str.find_first_not_of("\t")) > 0
           || int(str.find_first_not_of("\r")) > 0 || int(str.find_first_not_of("\n")) > 0)  {
      str = str.substr(str.find_first_not_of(" "), str.length());
      str = str.substr(str.find_first_not_of("\t"), str.length());
      str = str.substr(str.find_first_not_of("\n"), str.length());
      str = str.substr(str.find_first_not_of("\r"), str.length());
    }
  }


  void RemoveTailingWhitespace(wstring &str) {
    reverse(str.begin(), str.end());
    RemoveLeadingWhitespace(str);
    reverse(str.begin(), str.end());
  }

  void RemoveTailingWhitespace(string &str) {
    reverse(str.begin(), str.end());
    RemoveLeadingWhitespace(str);
    reverse(str.begin(), str.end());
  }

  bool GetFileStats(const string& strFileName, struct ::stat& stat_buf) {
    return (::stat( strFileName.c_str(), &stat_buf) >= 0);
  }

  bool GetFileStats(const wstring& wstrFileName, struct ::stat& stat_buf) {
    string strFileName(wstrFileName.begin(), wstrFileName.end());
    return (::stat( strFileName.c_str(), &stat_buf) >= 0);
  }

  bool FileExists(const wstring& wstrFileName) {
    struct ::stat stat_buf;
    return GetFileStats(wstrFileName, stat_buf);
  }

  bool FileExists(const string& strFileName) {
    struct ::stat stat_buf;
    return GetFileStats(strFileName, stat_buf);
  }

  string GetExt(const string& fileName) {
    size_t indexDot = fileName.find_last_of(".");
    size_t indexSlash = MAX(int(fileName.find_last_of("\\")),int(fileName.find_last_of("/")));
    if (indexDot == string::npos || (indexSlash != string::npos && indexDot < indexSlash)) return "";

    string ext = fileName.substr(indexDot+1);
    return ext;
  }

  wstring GetExt(const wstring& fileName) {
    size_t indexDot = fileName.find_last_of(L".");
    size_t indexSlash = MAX(int(fileName.find_last_of(L"\\")),int(fileName.find_last_of(L"/")));
    if (indexDot == wstring::npos || (indexSlash != wstring::npos && indexDot < indexSlash)) return L"";

    wstring ext = fileName.substr(indexDot+1);
    return ext;
  }

  wstring ToLowerCase(const wstring& str) {
    wstring result(str);
    transform(str.begin(), str.end(), result.begin(), myTolower);
    return result;
  }

  string ToLowerCase(const string& str) {
    string result(str);
    transform(str.begin(), str.end(), result.begin(), myTolower);
    return result;
  }

  wstring ToUpperCase(const wstring& str) {
    wstring result(str);
    transform(str.begin(), str.end(), result.begin(), myToupper);
    return result;
  }

  string ToUpperCase(const string& str) {
    string result(str);
    transform(str.begin(), str.end(), result.begin(), myToupper);
    return result;
  }

  string GetPath(const string& fileName) {
    string path = fileName.substr(0,MAX(int(fileName.find_last_of("\\")),int(fileName.find_last_of("/")))+1);
    return path;
  }

  wstring GetPath(const wstring& fileName) {
    wstring path = fileName.substr(0,MAX(int(fileName.find_last_of(L"\\")),int(fileName.find_last_of(L"/")))+1);
    return path;
  }

  std::string GetFilename(const std::string& fileName) {
    size_t index = MAX(int(fileName.find_last_of("\\")),int(fileName.find_last_of("/")))+1;
    string name = fileName.substr(index,fileName.length()-index);
    return name;
  }

  std::wstring GetFilename(const std::wstring& fileName) {
    size_t index = MAX(int(fileName.find_last_of(L"\\")),int(fileName.find_last_of(L"/")))+1;
    wstring name = fileName.substr(index,fileName.length()-index);
    return name;
  }

  string FindPath(const string& fileName, const string& path) {
    string searchFile;
    string slash = "";

    if (fileName[0] != '/' && fileName[0] != '\\' && path[path.length()-1] != '/' && path[path.length()-1] != '\\') {
      slash = "/";
    }


    // search in the given path
    searchFile = path + slash + fileName;

    // search in the given path
    searchFile = path + fileName;
    if (FileExists(searchFile))  return searchFile;

    // search in the current directory
    searchFile = "./" + fileName;
    if (FileExists(searchFile)) return searchFile;

    // search in the parent directory
    searchFile = "../" + fileName;
    if (FileExists(searchFile)) return searchFile;

    return "";
  }

  wstring FindPath(const wstring& fileName, const wstring& path) {
    wstring searchFile;
    wstring slash = L"";

    if (fileName[0]         != '/' && fileName[0]         != '\\' &&
      path[path.length()-1] != '/' && path[path.length()-1] != '\\') {

      slash = L"/";
    }


    // search in the given path
    searchFile = path + slash + fileName;
    if (FileExists(searchFile))  return searchFile;

    // search in the current directory
    searchFile = L".\\" + fileName;
    if (FileExists(searchFile)) return searchFile;

    // search in the parent directory
    searchFile = L"..\\" + fileName;
    if (FileExists(searchFile)) return searchFile;

    return L"";
  }

  string  RemoveExt(const string& fileName) {
    return fileName.substr(0,fileName.find_last_of("."));
  }

  wstring RemoveExt(const wstring& fileName) {
    return fileName.substr(0,fileName.find_last_of(L"."));
  }

  string  ChangeExt(const string& fileName, const std::string& newext) {
    return RemoveExt(fileName)+ "." + newext;
  }

  wstring ChangeExt(const std::wstring& fileName, const std::wstring& newext) {
    return RemoveExt(fileName)+ L"." + newext;
  }

  string  CheckExt(const string& fileName, const std::string& newext) {
    string currentExt = GetExt(fileName);
#ifdef _WIN32  // do a case insensitive check on windows systems
    if (ToLowerCase(currentExt) != ToLowerCase(newext))
#else
    if (currentExt != newext)
#endif
      return fileName + "." + newext;
    else
      return fileName;
  }

  wstring CheckExt(const std::wstring& fileName, const std::wstring& newext) {
    wstring currentExt = GetExt(fileName);
#ifdef _WIN32  // do a case insensitive check on windows systems
    if (ToLowerCase(currentExt) != ToLowerCase(newext))
#else
    if (currentExt != newext)
#endif
      return fileName + L"." + newext;
    else
      return fileName;
  }

  string  AppendFilename(const string& fileName, const int iTag) {
    return AppendFilename(fileName, ToString(iTag));
  }

  wstring AppendFilename(const wstring& fileName, const int iTag) {
    return AppendFilename(fileName, ToWString(iTag));
  }

  string  AppendFilename(const string& fileName, const string& tag) {
    return RemoveExt(fileName) + tag + "." + GetExt(fileName);
  }

  wstring AppendFilename(const wstring& fileName, const wstring& tag) {
    return RemoveExt(fileName) + tag + L"." + GetExt(fileName);
  }

  vector<wstring> GetSubDirList(const wstring& dir) {
    vector<wstring> subDirs;
    wstring rootdir;

#ifdef _WIN32
    wstringstream s;
    if (dir == L"") {
      WCHAR path[4096];
      GetCurrentDirectoryW(4096, path);
      s << path << L"/";
    } else {
      s << dir << L"/";
    }

    rootdir = s.str();

    WIN32_FIND_DATAW FindFileData;
    HANDLE hFind;

    hFind=FindFirstFileW((rootdir + L"*.*").c_str(), &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        wstring wstrFilename = FindFileData.cFileName;
        if( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && wstrFilename != L"." && wstrFilename != L"..") {
          subDirs.push_back(wstrFilename);
        }
      }while ( FindNextFileW(hFind, &FindFileData) );
    }

    FindClose(hFind);
#else
  if (dir == L"") {
    rootdir = L"./";
  } else {
    rootdir = dir + L"/";
  }

  string strDir(rootdir.begin(), rootdir.end());

  DIR* dirData=opendir(strDir.c_str());

  if (dirData != NULL) {

    struct dirent *inode;

    while ((inode=readdir(dirData)) != NULL) {
      string strFilenameLocal = inode->d_name;
      wstring wstrFilename(strFilenameLocal.begin(), strFilenameLocal.end());
      string strFilename = strDir + strFilenameLocal;

      struct ::stat st;
      if (::stat(strFilename.c_str(), &st) != -1)
        if (S_ISDIR(st.st_mode) && strFilenameLocal != "." && strFilenameLocal != "..") {
          subDirs.push_back(wstrFilename);
        }
    }
  }
  closedir(dirData);
#endif

    vector<wstring> completeSubDirs;
    for (size_t i = 0;i<subDirs.size();i++) {
      completeSubDirs.push_back(rootdir+subDirs[i]);
      vector<wstring> subSubDirs = GetSubDirList(rootdir+subDirs[i]);
      for (size_t j = 0;j<subSubDirs.size();j++) {
        completeSubDirs.push_back(subSubDirs[j]);
      }
    }

    return completeSubDirs;
  }


  vector<string> GetSubDirList(const string& dir) {
    vector<string> subDirs;
    string rootdir;


#ifdef _WIN32
    stringstream s;
    if (dir == "") {
      CHAR path[4096];
      GetCurrentDirectoryA(4096, path);
      s << path << "/";
    } else {
      s << dir << "/";
    }

    rootdir = s.str();

    WIN32_FIND_DATAA FindFileData;
    HANDLE hFind;

    hFind=FindFirstFileA((rootdir + "*.*").c_str(), &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        string strFilename = FindFileData.cFileName;
        if( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strFilename != "." && strFilename != "..") {
          subDirs.push_back(strFilename);
        }
      }while ( FindNextFileA(hFind, &FindFileData) );
    }

    FindClose(hFind);
#else
  if (dir == "") {
    rootdir = "./";
  } else {
    rootdir = dir + "/";
  }

  DIR* dirData=opendir(rootdir.c_str());

  if (dirData != NULL) {

    struct dirent *inode;

    while ((inode=readdir(dirData)) != NULL) {
      string strFilenameLocal = inode->d_name;
      string strFilename = rootdir + strFilenameLocal;

      struct ::stat st;
      if (::stat(strFilename.c_str(), &st) != -1)
        if (S_ISDIR(st.st_mode) && strFilenameLocal != "." && strFilenameLocal != "..") {
          subDirs.push_back(strFilenameLocal);
        }
    }
  }
  closedir(dirData);
#endif

    vector<string> completeSubDirs;
    for (size_t i = 0;i<subDirs.size();i++) {
      completeSubDirs.push_back(rootdir+subDirs[i]);
      vector<string> subSubDirs = GetSubDirList(rootdir+subDirs[i]);
      for (size_t j = 0;j<subSubDirs.size();j++) {
        completeSubDirs.push_back(subSubDirs[j]);
      }
    }
    return completeSubDirs;
  }

  vector<wstring> GetDirContents(const wstring& dir,
                                 const wstring& fileName,
                                 const wstring& ext) {
    vector<wstring> files;
    wstringstream s;

#ifdef _WIN32
    wstring wstrDir;
    if (dir == L"") {
      WCHAR path[4096];
      GetCurrentDirectoryW(4096, path);
      s << path << L"/" << fileName << L"." << ext;
      wstrDir = wstring(path);
    } else {
      s << dir << L"/" << fileName << L"." << ext;
      wstrDir = dir;
    }

    WIN32_FIND_DATAW FindFileData;
    HANDLE hFind;


    hFind=FindFirstFileW(s.str().c_str(), &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        if( 0 == (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
          files.push_back(wstrDir + L"/" + wstring(FindFileData.cFileName));
        }
      }while ( FindNextFileW(hFind, &FindFileData) );
    }

    FindClose(hFind);
#else
  wstring wstrDir;
  if (dir == L"") {
    wstrDir = L"./";
  } else {
    wstrDir = dir + L"/";
  }
  string strDir(wstrDir.begin(), wstrDir.end());


  // filter files via regexpr
  string regExpr = "^";
  regex_t preg;
  if (fileName != L"")  {
    string strFileName(fileName.begin(), fileName.end());
    regExpr = regExpr + strFileName;
    // replace * by .* and ? by .
    ReplaceAll(regExpr, "*", ".*");
    ReplaceAll(regExpr, "?", ".");
  }
  if (ext != L"")  {
    if (fileName == L"") regExpr = regExpr + ".*";

    string tmpext(ext.begin(), ext.end());
    // replace * by .* and ? by .
    ReplaceAll(tmpext, "*", ".*");
    ReplaceAll(tmpext, "?", ".");

    // append dot and extension to regular expression
    regExpr = regExpr + "\\." + tmpext + "$";
  }
  if (regcomp(&preg, regExpr.c_str(), REG_EXTENDED | REG_NOSUB) != 0) return files;


  DIR* dirData=opendir(strDir.c_str());

  if (dirData != NULL) {
    struct dirent *finfo;

    while ((finfo=readdir(dirData)) != NULL) {
      string strFilename = finfo->d_name;
      wstring wstrFilename(strFilename.begin(), strFilename.end());
      strFilename = strDir + strFilename;

      struct ::stat st;
      if (::stat(strFilename.c_str(), &st) != -1)
        if (!S_ISDIR(st.st_mode) && !regexec(&preg, finfo->d_name, size_t(0), NULL, 0)) {
          files.push_back(wstrFilename);
        }
    }
  }
  regfree(&preg);
  closedir(dirData);
#endif

    return files;
  }

  vector<string> GetDirContents(const string& dir,
                                const string& fileName,
                                const string& ext) {
    vector<string> files;

    stringstream s;


#ifdef _WIN32
    WIN32_FIND_DATAA FindFileData;
    HANDLE hFind;

    string strDir;
    if (dir == "") {
      char path[4096];
      GetCurrentDirectoryA(4096, path);
      s << path << "/" << fileName << "." << ext;
      strDir = string(path);
    } else {
      s << dir << "/" << fileName << "." << ext;
      strDir = dir;
    }

    hFind=FindFirstFileA(s.str().c_str(), &FindFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        if( 0 == (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
          files.push_back(strDir + "/" + string(FindFileData.cFileName));
        }
      }while ( FindNextFileA(hFind, &FindFileData) );
    }

    FindClose(hFind);
#else
  string strDir;
  if (dir == "") {
    strDir = "./";
  } else {
    strDir = dir + "/";
  }

  // filter files via regexpr
  string regExpr = "^";
  regex_t preg;
  if (fileName != "")  {
    regExpr = regExpr + fileName;
    // replace * by .* and ? by .
    ReplaceAll(regExpr, "*", ".*");
    ReplaceAll(regExpr, "?", ".");
  }
  if (ext != "")  {
    if (fileName == "") regExpr = regExpr + ".*";

    string tmpext = ext;
    // replace * by .* and ? by .
    ReplaceAll(tmpext, "*", ".*");
    ReplaceAll(tmpext, "?", ".");

    // append dot and extension to regular expression
    regExpr = regExpr + "\\." + tmpext + "$";
  }
  if (regcomp(&preg, regExpr.c_str(), REG_EXTENDED | REG_NOSUB) != 0) return files;

  DIR* dirData=opendir(strDir.c_str());

  if (dirData != NULL) {
    struct dirent *finfo;

    while ((finfo=readdir(dirData)) != NULL) {
      string strFilename = finfo->d_name;
      strFilename = strDir + strFilename;

      struct ::stat st;
      if (::stat(strFilename.c_str(), &st) != -1) {
        if (!S_ISDIR(st.st_mode) &&
            !regexec(&preg, finfo->d_name, size_t(0), NULL, 0)) {
          files.push_back(std::string(strFilename.c_str()));
        }
      }
    }
    closedir(dirData);
  }
  regfree(&preg);
#endif

    return files;
  }

  std::string  FindNextSequenceName(const std::string& strFilename) {
    std::string dir = SysTools::GetPath(strFilename);
    std::string fileName = SysTools::RemoveExt(SysTools::GetFilename(strFilename));
    std::string ext = SysTools::GetExt(strFilename);

    return FindNextSequenceName(fileName, ext, dir);
  }

  std::wstring FindNextSequenceName(const std::wstring& wStrFilename) {
    std::wstring dir = SysTools::GetPath(wStrFilename);
    std::wstring fileName = SysTools::RemoveExt(SysTools::GetFilename(wStrFilename));
    std::wstring ext = SysTools::GetExt(wStrFilename);

    return FindNextSequenceName(fileName, ext, dir);
  }

  // Functor to identify the numeric ID appended to a given filename.
  template <typename T> struct fileNumber : public std::unary_function<T, size_t> {
    size_t operator()(const T& filename) const {
      // get just the filename itself, without extension or path information.
      T fn = RemoveExt(GetFilename(filename));

      // Find where the numbers start.
      typename T::const_iterator numerals = fn.end()-1;
	  while (numerals != fn.begin() && ::isdigit(*(numerals-1))) --numerals;

      // convert it to a size_t and return that.
      size_t retval = 0;
      FromString(retval, T(&*numerals));
      return retval;
    }
  };

  // Given a filename model and a directory, identify the next filename in the
  // sequence.  Sequences start at 0 and increment.
  string FindNextSequenceName(const string& fileName, const string& ext,
                              const string& dir) {
    stringstream out;
    vector<string> files = GetDirContents(dir, fileName+"*", ext);

    // Get a list of all the trailing numeric values.
    std::vector<size_t> values;
    values.reserve(files.size());
    std::transform(files.begin(), files.end(), std::back_inserter(values),
                   fileNumber<std::string>());

    // No files in the dir?  Default to 0.
    if(values.empty()) {
      out << dir << fileName << 0 << "." << ext;
    } else {
      // Otherwise, the next number is the current max + 1.
      size_t max_val = *(std::max_element(values.begin(), values.end()));
      out << dir << fileName << max_val+1 << "." << ext;
    }

    return out.str();
  }

  wstring  FindNextSequenceName(const wstring& fileName, const wstring& ext, const wstring& dir) {
    wstringstream out;
    vector<wstring> files = GetDirContents(dir, fileName+L"*", ext);

    // Get a list of all the trailing numeric values.
    std::vector<size_t> values;
    values.reserve(files.size());
    std::transform(files.begin(), files.end(), std::back_inserter(values),
                   fileNumber<std::wstring>());

    // No files in the dir?  Default to 0.
    if(values.empty()) {
      out << dir << fileName << 0 << L"." << ext;
    } else {
      // Otherwise, the next number is the current max + 1.
      size_t max_val = *(std::max_element(values.begin(), values.end()));
      out << dir << fileName << max_val+1 << L"." << ext;
    }

    return out.str();
  }


  UINT32 FindNextSequenceIndex(const string& fileName, const string& ext, const string& dir) {
    vector<string> files = GetDirContents(dir, fileName+"*", ext);

    UINT32 iMaxIndex = 0;
    for (size_t i = 0; i<files.size();i++) {
      string curFilename = RemoveExt(files[i]);
      UINT32 iCurrIndex = UINT32(atoi(curFilename.substr(fileName.length()).c_str()));
      iMaxIndex = (iMaxIndex <= iCurrIndex) ? iCurrIndex+1 : iMaxIndex;
    }

    return iMaxIndex;
  }

  UINT32 FindNextSequenceIndex(const wstring& fileName, const wstring& ext, const wstring& dir) {
    vector<wstring> files = GetDirContents(dir, fileName+L"*", ext);

    UINT32 iMaxIndex = 0;
    for (size_t i = 0; i<files.size();i++) {
      wstring wcurFilename = RemoveExt(files[i]);
      string curFilename(wcurFilename.begin(), wcurFilename.end());
      UINT32 iCurrIndex = UINT32(atoi(curFilename.substr(fileName.length()).c_str()));
      iMaxIndex = (iMaxIndex <= iCurrIndex) ? iCurrIndex+1 : iMaxIndex;
    }

    return iMaxIndex;
  }

  bool GetTempDirectory(std::string& path) {
  #ifdef DETECTED_OS_WINDOWS
    DWORD result = ::GetTempPathA(0, "");
    if(result == 0) return false;
    std::vector<char> tempPath(result + 1);
    result = ::GetTempPathA(static_cast<DWORD>(tempPath.size()), &tempPath[0]);
    if((result == 0) || (result >= tempPath.size())) return false;
    path = std::string( tempPath.begin(), tempPath.begin() + static_cast<std::size_t>(result)  );
    return true;
  #else
    char * pointer;
    pointer = tmpnam(NULL);
    path = GetPath(std::string( pointer ));
    return true;
  #endif
  }


  bool GetTempDirectory(std::wstring& path) {
  #ifdef DETECTED_OS_WINDOWS
      DWORD result = ::GetTempPathW(0, L"");
      if(result == 0) return false;
      std::vector<WCHAR> tempPath(result + 1);
      result = ::GetTempPathW(static_cast<DWORD>(tempPath.size()), &tempPath[0]);
      if((result == 0) || (result >= tempPath.size())) return false;
      path = std::wstring( tempPath.begin(), tempPath.begin() + static_cast<std::size_t>(result)  );
      return true;
  #else 
      // too lazy to find the unicode version for linux and mac
      std::string astrPath;
      if (!GetTempDirectory(astrPath)) return false;    
      path = std::wstring( astrPath.begin(), astrPath.end());
      return true;
  #endif
  }


#ifdef _WIN32
  bool GetFilenameDialog(const string& lpstrTitle, const CHAR* lpstrFilter, string &filename, const bool save, HWND owner, DWORD* nFilterIndex) {
    BOOL result;
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn,sizeof(OPENFILENAMEA));

    static CHAR szFile[MAX_PATH];

    char szDir[MAX_PATH];
    if (filename.length()>0) {
      errno_t err=strcpy_s(szDir,MAX_PATH,filename.c_str());
      filename.clear();
      if (err) return false;
    } else szDir[0]=0;
    ofn.lpstrInitialDir = szDir;


    szFile[0] = 0;

    //====== Dialog parameters
    ofn.lStructSize   = sizeof(OPENFILENAMEA);
    ofn.lpstrFilter   = lpstrFilter;
    ofn.nFilterIndex  = 1;
    ofn.lpstrFile     = szFile;
    ofn.nMaxFile      = sizeof(szFile);
    ofn.lpstrTitle    = lpstrTitle.c_str();
    ofn.nMaxFileTitle = sizeof (lpstrTitle.c_str());
    ofn.hwndOwner     = owner;

    if (save) {
      ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT;
      result = GetSaveFileNameA(&ofn);
    } else {
      ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_FILEMUSTEXIST;
      result = GetOpenFileNameA(&ofn);
    }
    if (result)  {
      filename = szFile;
      if (nFilterIndex != NULL) *nFilterIndex = ofn.nFilterIndex;
      return true;
    } else {
      filename = "";
      return false;
    }

  }

  bool GetFilenameDialog(const wstring& lpstrTitle, const WCHAR* lpstrFilter, wstring &filename, const bool save, HWND owner, DWORD* nFilterIndex) {
    BOOL result;
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn,sizeof(OPENFILENAMEW));

    static WCHAR szFile[MAX_PATH];
    szFile[0] = 0;


    WCHAR szDir[MAX_PATH];
    if (filename.length()>0) {
      errno_t err=wcscpy_s(szDir,MAX_PATH,filename.c_str());
      filename.clear();
      if (err) return false;
    } else szDir[0]=0;
    ofn.lpstrInitialDir = szDir;

    //====== Dialog parameters
    ofn.lStructSize   = sizeof(OPENFILENAMEW);
    ofn.lpstrFilter   = lpstrFilter;
    ofn.nFilterIndex  = 1;
    ofn.lpstrFile     = szFile;
    ofn.nMaxFile      = sizeof(szFile);
    ofn.lpstrTitle    = lpstrTitle.c_str();
    ofn.nMaxFileTitle = sizeof (lpstrTitle.c_str());
    ofn.hwndOwner     = owner;

    if (save) {
      ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT;
      result = GetSaveFileNameW(&ofn);
    } else {
      ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_FILEMUSTEXIST;
      result = GetOpenFileNameW(&ofn);
    }
    if (result)  {
      filename = szFile;
      if (nFilterIndex != NULL) *nFilterIndex = ofn.nFilterIndex;
      return true;
    } else {
      filename = L"";
      return false;
    }

  }


  CmdLineParams::CmdLineParams() {
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    wstring wTmp = SysTools::GetFilename(argv[0]);
    string Tmp;
    m_strFilename = string(wTmp.begin(),wTmp.end());

    for(int a=1 ; a<argc ; ++a)
    {
      if(argv[a][0] == '-')
      {
        wTmp = argv[a]+1; Tmp = string(wTmp.begin(),wTmp.end());
        m_strArrayParameters.push_back(Tmp);

        if (a+1<argc && argv[a+1][0] != '-') {
          wTmp = argv[a+1]; Tmp = string(wTmp.begin(),wTmp.end());
          m_strArrayValues.push_back(Tmp);
          ++a;
        } else m_strArrayValues.push_back("");
      }
    }
  }

#else
  #include <wchar.h>
  typedef wchar_t WCHAR;
  typedef unsigned char CHAR;
#endif

  CmdLineParams::CmdLineParams(int argc, char** argv) {
    m_strFilename = SysTools::GetFilename(argv[0]);

    for(int a=1 ; a<argc ; ++a)
    {
      if(argv[a][0] == '-')
      {
        m_strArrayParameters.push_back(argv[a]+1);

        if (a+1<argc) {
          m_strArrayValues.push_back(argv[a+1]);
          ++a;
        } else m_strArrayValues.push_back("");
      }
    }
  }


  bool CmdLineParams::SwitchSet(const std::string& parameter) {
    string dummy;
    return GetValue(parameter, dummy);
  }

  bool CmdLineParams::SwitchSet(const std::wstring& parameter) {
    string Parameter(parameter.begin(), parameter.end());
    return SwitchSet(Parameter);
  }

  bool CmdLineParams::GetValue(const wstring& parameter, double& value) {
    string sParameter(parameter.begin(), parameter.end());
    return GetValue(sParameter, value);
  }

  bool CmdLineParams::GetValue(const string& parameter, double& value) {
    string strValue;
    if (GetValue(parameter, strValue)) {
      value = atof(strValue.c_str());
      return true;
    } else return false;
  }

  bool CmdLineParams::GetValue(const wstring& parameter, float& value) {
    string sParameter(parameter.begin(), parameter.end());
    return GetValue(sParameter, value);
  }

  bool CmdLineParams::GetValue(const string& parameter, float& value) {
    string strValue;
    if (GetValue(parameter, strValue)) {
      value = float(atof(strValue.c_str()));
      return true;
    } else return false;
  }

  bool CmdLineParams::GetValue(const wstring& parameter, int& value) {
    string sParameter(parameter.begin(), parameter.end());
    return GetValue(sParameter, value);
  }

  bool CmdLineParams::GetValue(const string& parameter, int& value) {
    string strValue;
    if (GetValue(parameter, strValue)) {
      value = atoi(strValue.c_str());
      return true;
    } else return false;
  }

  bool CmdLineParams::GetValue(const std::string& parameter, std::string& value) {
    string lowerParameter(parameter), lowerListParameter;

    transform(parameter.begin(), parameter.end(), lowerParameter.begin(), myTolower);
    for (size_t i = 0;i<m_strArrayParameters.size();i++) {

      lowerListParameter.resize(m_strArrayParameters[i].length());

      transform(m_strArrayParameters[i].begin(), m_strArrayParameters[i].end(), lowerListParameter.begin(), myTolower);

      if (lowerListParameter == lowerParameter) {
        value = m_strArrayValues[i];
        return true;
      }
    }

    value = "";
    return false;
  }

  bool CmdLineParams::GetValue(const std::wstring& parameter, std::wstring& value) {
    string sParameter(parameter.begin(), parameter.end()),
         sValue(value.begin(), value.end());

    if (GetValue(sParameter, sValue)) {
      value = wstring(sValue.begin(), sValue.end());
      return true;
    } else return false;
  }
}
