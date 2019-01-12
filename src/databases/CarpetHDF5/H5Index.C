
#include <string>
#include <FileFunctions.h>
#include "H5Index.h"

using     std::string;

// ****************************************************************************
//
//  Modifications:
//    Kathleen Biagas, Wed Nov 24 16:34:51 MST 2015
//    Use VisItStat.
//
// ****************************************************************************

static bool fileExists(const string &file)
{ 
  FileFunctions::VisItStat_t stFileInfo; 
  return (FileFunctions::VisItStat(file.c_str(),&stFileInfo) == 0);
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
