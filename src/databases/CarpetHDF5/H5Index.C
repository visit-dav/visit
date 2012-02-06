
#include <string>
#include <sys/stat.h> 

#include "H5Index.h"

using     std::string;

static bool fileExists(const string &file)
{ 
  struct stat stFileInfo; 
  return (stat(file.c_str(),&stFileInfo) == 0);
}

string H5Index::indexFilename(const string &file)
{
  const string h5ext(".h5");
  const string iext(".idx.h5");
  return file.substr(0, file.length() - h5ext.length()) + iext;
}

bool H5Index::haveIndex(const string &file)
{
  return fileExists(indexFilename(file));
}
