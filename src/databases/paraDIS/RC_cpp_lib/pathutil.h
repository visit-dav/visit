#ifndef RDC_PATHUTIL_H
#define RDC_PATHUTIL_H
#include "stringutil.h"

#include <limits.h>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#if defined(PATH_MAX)
#define PARADIS_PATH_MAX PATH_MAX
#elif defined(MAXPATHLEN)
#define PARADIS_PATH_MAX MAXPATHLEN
#elif defined(MAX_PATH)
#define PARADIS_PATH_MAX MAX_PATH
#else
#define PARADIS_PATH_MAX 16384
#endif

//===============================================================
/*!
  Returns the full path to the directory containing the given file or directory, without the trailing "/", unless the result is "/" itself.  
  Examples: 
  Dirname("/usr/local") == "/usr"
  Dirname("/usr/local/") == "/usr"
  Dirname("relative/path") == "/full/path/to/relative"
  Dirname("my/directory/") == "/full/path/to/my"
  Dirname("/") == "/"

*/ 
inline string Dirname(string filename) {
  string dirname = StripBack(filename, "/"); 

  if (!dirname.length()) return string("/");
 
  if (dirname[0] != '/')  {
    // need to switch to absolute path
    char wd[PARADIS_PATH_MAX]; // this is why I hate C programming -- there is no way to be sure about buffer overflows with this kind of crap lingering around
    char* res = getcwd(wd, PARADIS_PATH_MAX); (void) res;

    string::size_type loc = dirname.find("/");
    if (loc == string::npos) {
      return string(wd); 
    }
    // prepend current working directory to mDirname
    dirname  = string(wd) + "/" + dirname; 
  } 
  string::size_type loc = dirname.rfind("/");
  if (loc == string::npos) { // reality check 
    throw string("Error in Dirname(): Logic error: there are no '/' chars in supposedly absolute path: ") + dirname; 
  }
  return  StripBack(dirname.substr(0,loc),"/"); 
}

//===============================================================
/*!
  Returns the last link of the filename, ala the "basename" shell command.  
*/ 
inline string Basename(string filename) {
  filename = StripBack(filename, "/"); 
  string::size_type loc = filename.rfind("/");
  if (loc == string::npos) {
    return filename; 
  } 
  string filename2 = filename.substr(loc+1); 
  
  return filename2; 
}
#endif
