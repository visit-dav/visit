// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtDebugDumpOptions.C                            //
// ************************************************************************* //

#include <avtDebugDumpOptions.h>
#include <visit-config.h>
#include <FileFunctions.h>
#include <InstallationFunctions.h>
#include <DebugStream.h>
#ifdef _WIN32
  #ifndef _WIN32_LEAN_AND_MEAN
    #define _WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
#endif
using std::string;

// init static members

bool   avtDebugDumpOptions::doDump = false;
bool   avtDebugDumpOptions::doDatasetDump = false;

#ifdef _WIN32
string avtDebugDumpOptions::outputDir = GetUserVisItDirectory();
#else
string avtDebugDumpOptions::outputDir = "";
#endif


// ****************************************************************************
//  Method: avtDebugDumpOptions constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Feburary 13, 2008
//
// ****************************************************************************

avtDebugDumpOptions::avtDebugDumpOptions()
{;}


// ****************************************************************************
//  Method: avtDebugDumpOptions destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Feburary 13, 2008
//
// ****************************************************************************

avtDebugDumpOptions::~avtDebugDumpOptions()
{;}



// ****************************************************************************
//  Method: avtDebugDumpOptions::EnableDump
//
//  Purpose:
//      Turns on all debug dump options.
//
//  Programmer: Cyrus Harrison
//  Creation:   Feburary 13, 2008
//
// ****************************************************************************

void 
avtDebugDumpOptions::EnableDump()
{
    doDump = true;
    doDatasetDump = true;
}


// ****************************************************************************
//  Method: avtDebugDumpOptions::DisableDump
//
//  Purpose:
//      Turns off all debug dump options.
//
//  Programmer: Cyrus Harrison
//  Creation:   Feburary 13, 2008
//
// ****************************************************************************

void 
avtDebugDumpOptions::DisableDump()
{
    doDump = false;
    doDatasetDump = false;
}


// ****************************************************************************
//  Method: avtDebugDumpOptions::EnableDatasetDump
//
//  Purpose:
//      Turns on the dataset dump debug option.
//
//  Programmer: Cyrus Harrison
//  Creation:   Feburary 13, 2008
//
// ****************************************************************************

void 
avtDebugDumpOptions::EnableDatasetDump()
{
   doDatasetDump = true;
}


// ****************************************************************************
//  Method: avtDebugDumpOptions::DisableDatasetDump
//
//  Purpose:
//      Turns off the dataset dump debug option.
//
//  Programmer: Cyrus Harrison
//  Creation:   Feburary 13, 2008
//
// ****************************************************************************

void 
avtDebugDumpOptions::DisableDatasetDump()
{
    doDatasetDump = false;
}

// ****************************************************************************
//  Method: avtDebugDumpOptions::SetDumpDirectory
//
//  Purpose:
//      Sets the directory for debug dump output.
//
//  Arguments:
//      odir       A valid directory.
//
//  Notes:         If the directory does not exist, this method will revert to
//                 the current working directory.
//                 We should probably check for write perms to the directory
//                 in the future. 
//
//  Programmer:    Cyrus Harrison
//  Creation:      Feburary 13, 2008
//
//  Modifications:
//    Cyrus Harrison, Tue Feb 19 08:29:10 PST 2008
//    Support for Windows directories. 
//
//    Kathleen Bonnell, Mon Jan 24 17:38:51 MST 2011 
//    Change how existence of directory is tested on Windows. If invalid
//    directory, set to user's VisIt directory, as "" as the 'launch' directory
//    may not be valid or writeable.
//
// ****************************************************************************

void
avtDebugDumpOptions::SetDumpDirectory(const string &odir)
{
    string res_dir = odir;
    bool invalidDir = false;
    if(res_dir != "")
    {
        // make sure it has a "/" or "\" (windows) suffix
        string slash = string(VISIT_SLASH_STRING);
        if (res_dir[res_dir.length()-1] != slash[0] )
            res_dir += slash;

        // make sure the dir exists!
#ifdef WIN32
        DWORD resAttrs = GetFileAttributes(res_dir.c_str());
        if (resAttrs == INVALID_FILE_ATTRIBUTES)
            invalidDir = true;
        else if (!(resAttrs & FILE_ATTRIBUTE_DIRECTORY))
            invalidDir = true;
#else
        FileFunctions::VisItStat_t s;
        FileFunctions::VisItStat(res_dir.c_str(), &s);
        mode_t mode = s.st_mode;
        invalidDir = (!S_ISDIR(mode));
#endif
        if(invalidDir)
        {
            debug1 << "Debug Dump:: Warning \"" << odir  << "\""
                   << " is not a valid directory." << endl;
#ifdef _WIN32
            res_dir = GetUserVisItDirectory();
            debug1 << "Debug Dump:: Output directory set to user's VisIt"
                   << " directory."<<endl;
#else
            // revert to cwd
            res_dir = "";
#endif
        }
    }
    
    if(res_dir == "")
    {
        debug1 << "Debug Dump:: Output directory set to where VisIt"
               << " was launched."<<endl;
    }
    else
    {
        debug1 << "Debug Dump:: Output directory set to "
               << "\"" << res_dir << "\"" <<endl;
    }
    
    outputDir = res_dir;
}

