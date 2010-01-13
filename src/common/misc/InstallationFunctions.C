/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <InstallationFunctions.h>

#include <visit-config.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <snprintf.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

#include <DebugStream.h>
#include <Environment.h>
#include <FileFunctions.h>

//
// Static vars.
//
static bool isDevelopmentVersion = false;

// ****************************************************************************
// Method: GetDefaultConfigFile
//
// Purpose: 
//   Returns the name and path of the default configuration file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 18:26:30 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Feb 16 09:29:44 PDT 2005
//   Moved from ConfigManager class, deleted old modification comments.
//
//   Hank Childs, Wed Aug 24 16:38:55 PDT 2005
//   Allow for files with absolute paths.
//
//   Kathleen Bonnell, Fri Jul 20 10:48:21 PDT 2007 
//   On Windows, 'realhome' is now VISITUSERHOME. 
//
//   Kathleen Bonnell, Tue Jul 24 15:19:23 PDT 2007 
//   On Windows, don't prepend if filename begins with quotes, or 'C:'
//
//   Kathleen Bonnell, Thu Jul 26 11:27:32 PDT 2007 
//   Fix WIN32 code to use 'home' if not null, otherwise VISITUSERHOME. 
//
//   Jeremy Meredith, Thu Aug  7 16:20:02 EDT 2008
//   Use const char* for string literals.
//
//   Tom Fogal, Sun Apr 19 11:39:50 MST 2009
//   Use `Environment' to simplify and fix a compilation error.
//
// ****************************************************************************

char *
GetDefaultConfigFile(const char *filename, const char *home)
{
    char *retval;
    const char *configFileName;
    int  filenameLength;

#ifdef WIN32
    // If the filename is enclosed in quotes, do no prepend the home directory.
    if (filename != NULL && (filename[0] == '\'' || filename[0] == '\"'))
    {
        retval = new char[strlen(filename)+1];
        strcpy(retval, filename);
        return retval;
    }
    // If the filename has an absolute path, do not prepend the home directory.
    if (filename != NULL && (filename[0] == 'C' && filename[1] == ':'))
    {
        retval = new char[strlen(filename)+1];
        strcpy(retval, filename);
        return retval;
    }
#endif

    // If the filename has an absolute path, do not prepend the home
    // directory.
    if (filename != NULL && filename[0] == VISIT_SLASH_CHAR)
    {
        // Must make a copy because the caller will delete this.
        retval = new char[strlen(filename)+1];
        strcpy(retval, filename);
        return retval;
    }

    // Figure out the proper filename to use. If no filename was given, use
    // "config" as the default filename.
    if(filename == 0)
    {
        filenameLength = 7;
        configFileName = "config";
    }
    else
    {
        filenameLength = strlen(filename);
        configFileName = (char *)filename;
    }

#if defined(_WIN32)
    const char *homevar = (home == NULL) ? "VISITUSERHOME" : home;
    std::string realhome = Environment::get(homevar);

    if(!realhome.empty())
    {
        if(home == NULL)
        {
            // User config. Get the username so we can append it to
            // the filename.
            DWORD namelen = 100;
            char username[100];
            GetUserName(username, &namelen);

            retval = new char[realhome.length() + namelen + 5 + filenameLength + 2 + 7];
            sprintf(retval, "%s\\%s for %s.ini", realhome.c_str(),
                    configFileName, username);
        }
        else
        {
            // System config.
            retval = new char[realhome.length() + filenameLength + 2 + 7];
            sprintf(retval, "%s\\%s.ini", realhome.c_str(), configFileName);
        }
    }
    else
    {
        retval = new char[filenameLength + 1 + 4];
        sprintf(retval, "%s.ini", configFileName);
    }
#else
    // The file it is assumed to be in the home directory unless the home
    // directrory doesn't exist, in which case we will say it is
    // in the current directory.
    const char *homevar = (home == NULL) ? "HOME" : home;
    std::string realhome = Environment::get(homevar);
    if(Environment::exists(homevar))
    {
        retval = new char[realhome.length() + filenameLength + 2 + 7];
        sprintf(retval, "%s/.visit/%s", realhome.c_str(), configFileName);
    }
    else
    {
        retval = new char[filenameLength + 1];
        strcpy(retval, configFileName);
    }
#endif

    return retval;
}

// ****************************************************************************
// Method: GetSystemConfigFile
//
// Purpose: 
//   Returns the system config file name.
//
// Arguments:
//   filename : The base name of the system filename.
//
// Returns:    The system config file name.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 19 12:33:06 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Feb 23 15:53:34 PST 2004
//   I added logic to try and determine the name of the appropriate config
//   file.
//
//   Brad Whitlock, Wed Feb 16 09:29:44 PDT 2005
//   Moved from ConfigManager class.
//
// ****************************************************************************

char *
GetSystemConfigFile(const char *filename)
{
    const char *sysConfigName = filename;

    //
    // If no system config file name was given, check the VISITSYSTEMCONFIG
    // environment variable if we're on Windows. Otherwise, just use the
    // name "config".
    //
    if(sysConfigName == 0)
    {
#if defined(_WIN32)
        // Try and get the system config filename from the environment settings.
        sysConfigName = getenv("VISITSYSTEMCONFIG");
#endif

        // If we still don't have the name of a system config file, use 
        // the name "config".
        if(sysConfigName == 0)
            sysConfigName = "config";
    }

    return GetDefaultConfigFile(sysConfigName, "VISITHOME");
}

// ****************************************************************************
// Method: GetUserVisItDirectory
//
// Purpose: 
//   Returns the user's .visit directory or equivalent.
//
// Returns:    The directory where VisIt likes to put stuff.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 17:44:59 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Feb 16 09:29:44 PDT 2005
//   Moved from ConfigManager class.
//
//   Kathleen Bonnell, Fri Jul 20 10:48:21 PDT 2007 
//   User directory on windows is now defined by VISITUSERHOME env var. 
//
//   Tom Fogal, Sun Apr 19 12:44:06 MST 2009
//   Use `Environment' to simplify and fix a compilation error.
//
// ****************************************************************************

std::string
GetUserVisItDirectory()
{
#if defined(_WIN32)
    const std::string home = Environment::get("VISITUSERHOME");
#else
    const std::string home = Environment::get("HOME");
#endif

    std::string homedir;

    if(!home.empty())
    {
#if defined(_WIN32)
        homedir = home;
#else
        homedir = home + "/.visit";
#endif

        if(homedir[homedir.size() - 1] != VISIT_SLASH_CHAR)
            homedir += VISIT_SLASH_STRING;
    }

    return homedir;
}

// ****************************************************************************
// Method: GetUserVisItRCFile
//
// Purpose: 
//   Returns the name of the VisIt RC file in the user's .visit directory.
//
// Returns:    The name of the RC file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 17:44:04 PST 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
GetUserVisItRCFile()
{
    return GetUserVisItDirectory() + "visitrc";
}

// ****************************************************************************
// Method: GetSystemVisItRCFile
//
// Purpose: 
//   Returns the name of the VisIt RC file in the VisIt installation directory.
//
// Returns:    The name of the RC file.
//
// Note:       
//
// Programmer: Gunther H. Weber
// Creation:   Thu Jan 17 15:10:30 PST 2008
//
// Modifications:
//   Kathleen Bonnell, Tue Sep  2 08:17:22 PDT 2008
//   Value returned from GetDefaultConfigFile has been new'd, and should
//   be deleted here.
//   
// ****************************************************************************

std::string
GetSystemVisItRCFile()
{
    const char *defConfig = GetDefaultConfigFile("visitrc", "VISITHOME");
    std::string retVal(defConfig);
    delete [] defConfig;
    return retVal;
}

#if defined(_WIN32)
// ***************************************************************************
//
// Functions to get at VisIt data stored in the Windows registry.
//
//  Modifications:
//    Kathleen Bonnell, Wed May 21 08:12:16 PDT 2008
//    Only malloc keyval if it hasn't already been done.
// ***************************************************************************
int
ReadKeyFromRoot(HKEY which_root, const char *ver, const char *key,
    char **keyval)
{
    int  readSuccess = 0;
    char regkey[100];
    HKEY hkey;

    /* Try and read the key from the system registry. */
    sprintf(regkey, "VISIT%s", ver);
    if (*keyval == 0)
        *keyval = (char *)malloc(500);
    if(RegOpenKeyEx(which_root, regkey, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        DWORD keyType, strSize = 500;
        if(RegQueryValueEx(hkey, key, NULL, &keyType,
           (unsigned char *)*keyval, &strSize) == ERROR_SUCCESS)
        {
            readSuccess = 1;
        }

        RegCloseKey(hkey);
    }

    return readSuccess;
}

int
ReadKey(const char *ver, const char *key, char **keyval)
{
    int retval = 0;

    if((retval = ReadKeyFromRoot(HKEY_CLASSES_ROOT, ver, key, keyval)) == 0)
        retval = ReadKeyFromRoot(HKEY_CURRENT_USER, ver, key, keyval);
    
    return retval;     
}

int
WriteKeyToRoot(HKEY which_root, const char *ver, const char *key,
    const char *keyval)
{
    int  writeSuccess = 0;
    char regkey[100];
    HKEY hkey;

    /* Try and read the key from the system registry. */
    sprintf(regkey, "VISIT%s", ver);
    if(RegOpenKeyEx(which_root, regkey, 0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS)
    {
        DWORD strSize = strlen(keyval);
        if(RegSetValueEx(hkey, key, NULL, REG_SZ,
           (const unsigned char *)keyval, strSize) == ERROR_SUCCESS)
        {
            writeSuccess = 1;
        }

        RegCloseKey(hkey);
    }

    return writeSuccess;
}

int
WriteKey(const char *ver, const char *key, const char *keyval)
{
    int retval = 0;

    if((retval = WriteKeyToRoot(HKEY_CLASSES_ROOT, ver, key, keyval)) == 0)
        retval = WriteKeyToRoot(HKEY_CURRENT_USER, ver, key, keyval);

    return retval;
}
#endif

// ****************************************************************************
// Method: SetIsDevelopmentVersion
//
// Purpose: 
//   Sets whether the version of VisIt is a development version.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 14 10:51:30 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
SetIsDevelopmentVersion(bool val)
{
    isDevelopmentVersion = val;
}

// ****************************************************************************
// Method: GetIsDevelopmentVersion
//
// Purpose: 
//   Returns whether VisIt is a development version of VisIt.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 14 10:50:35 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
GetIsDevelopmentVersion()
{
    return isDevelopmentVersion;
}

// ****************************************************************************
// Method: GetVisItInstallationDirectory
//
// Purpose: 
//   Returns the directory where VisIt was installed.
//
// Arguments:
//   version : The version for which we want information. Only used on Windows.
//
// Returns:    The directory where VisIt is installed.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 2 12:08:57 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Mar 7 14:23:59 PST 2005
//   I fixed a bug that made it return the wrong part of the string.
//
//   Brad Whitlock, Mon May 9 16:26:43 PST 2005
//   I prevented it from stripping the last directory off if we're in a
//   development directory.
//
//   Brad Whitlock, Thu Jul 14 10:49:42 PDT 2005
//   I made it use the isDevelopmentDir library variable.
//
//   Brad Whitlock, Thu Dec 21 18:55:46 PST 2006
//   Added missing code for win32 development.
//
//   Kathleen Bonnell, Fri Jul 20 10:48:21 PDT 2007 
//   Removed MSVC6 specific code. 
//
//   Kathleen Bonnell, Tue Sep  2 08:20:15 PDT 2008 
//   Regardless of return value from ReadKey, visithome may have been
//   malloc'd and thus requires free. 
//
//   Tom Fogal, Sun Apr 19 12:47:22 MST 2009
//   Use `Environment' to simplify and fix a compilation error.
//
// ****************************************************************************

std::string
GetVisItInstallationDirectory()
{
    return GetVisItInstallationDirectory(VISIT_VERSION);
}

#if _MSC_VER <= 1310
#define _VISIT_MSVC_VER "MSVC7.Net"
#elif _MSC_VER <= 1400
#define _VISIT_MSVC_VER "MSVC8.Net"
#else
#define _VISIT_MSVC_VER ""
#endif

std::string
GetVisItInstallationDirectory(const char *version)
{
#if defined(_WIN32)
    // Get the installation dir for the specified from the registry.
    char *visitHome = 0;
    std::string installDir("C:\\");
    if(ReadKey(version, "VISITHOME", &visitHome) == 1)
    {
        installDir = visitHome;
    }
    else
    {
        // Use the VISITDEVDIR environment var.
        std::string visitdev;
        char *devdir = getenv("VISITDEVDIR");
        if(devdir == 0)
            visitdev = std::string("C:\\VisItDev") + std::string(version);
        else
            visitdev = std::string(devdir);
        installDir = visitdev + "\\bin\\" + _VISIT_MSVC_VER + "\\Release";
    }
    if (visitHome != 0)
        free(visitHome);
    return installDir;
#else
    // Get the installation dir for the version that's running. They all use
    // the same "visit" script so it's okay to do this.
    std::string installDir("/usr/local/visit");
    const std::string idir = Environment::get("VISITHOME");
    if(!idir.empty())
    {
        // The directory often has a "/bin" on the end. Strip it off.
        std::string home(idir);
        if(isDevelopmentVersion)
            installDir = idir;
        else
        {
            int lastSlash = home.rfind("/");
            if(lastSlash != -1)
                installDir = home.substr(0, lastSlash);
            else
                installDir = idir;
        }
    }
    return installDir;
#endif
}

// ****************************************************************************
// Method: GetVisItArchitectureDirectory
//
// Purpose: 
//   Gets the name of the directory where VisIt's current binary is installed.
//   This directory typically contains the bin, lib, plugin, etc directories
//   on UNIX installations.
//
// Arguments:
//   version : The version number for which we want the archtecture dir.
//
// Returns:    The architecture dir.
//
// Note:       On Windows, this function returns the same as
//             GetVisItInstallationDirectory.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 24 11:33:07 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul 11 10:08:30 PDT 2005
//   Made it work for the non-installed case on Windows.
//
//   Kathleen Bonnell, Fri Jul 20 10:48:21 PDT 2007 
//   Removed MSVC6 specific code. 
//
//   Kathleen Bonnell, Tue Sep  2 08:20:15 PDT 2008 
//   Regardless of return value from ReadKey, visithome may have been
//   malloc'd and thus requires free. 
//
//   Tom Fogal, Sun Apr 19 12:47:42 MST 2009
//   Use `Environment' to simplify and fix a compilation error.
//
//   Tom Fogal, Mon Jan 11 22:09:23 MST 2010
//   Give a warning when relying on the compile-time default.
//
// ****************************************************************************

std::string
GetVisItArchitectureDirectory()
{
    return GetVisItArchitectureDirectory(VISIT_VERSION);
}

std::string
GetVisItArchitectureDirectory(const char *version)
{
#if defined(_WIN32)
    // Get the installation dir for the specified from the registry.
    char *visitHome = 0;
    std::string archDir("C:\\");
    if(ReadKey(version, "VISITHOME", &visitHome) == 1)
    {
        archDir = visitHome;
    }
    else
    {
        // Use the VISITDEVDIR environment var.
        std::string visitdev;
        char *devdir = getenv("VISITDEVDIR");
        if(devdir == 0)
            visitdev = std::string("C:\\VisItDev") + std::string(version);
        else
            visitdev = std::string(devdir);
        archDir = visitdev + "\\bin\\" + _VISIT_MSVC_VER + "\\Release";
    }
    if (visitHome != 0)
        free(visitHome);
    return archDir;
#else
    // Get the installation dir for the version that's running. They all use
    // the same "visit" script so it's okay to do this.
    std::string archDir(std::string("/usr/local/visit/") +
                        std::string(VISIT_VERSION));
    const std::string adir = Environment::get("VISITARCHHOME");
    if(!adir.empty())
        archDir = adir;
    else
    {
        debug1 << "WARNING: Using default '/usr/local/visit' architecture "
                  "directory.  This is unlikely to be correct -- set your "
                  "'VISITARCHHOME' environment variable!" << std::endl;
    }
    return archDir;
#endif
}

// ****************************************************************************
// Method: GetVisItLauncher
//
// Purpose: 
//   Returns the name of the VisIt launch program, wherever it is found.
//
// Returns:    The full name of the VisIt launcher.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 14 10:54:33 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
GetVisItLauncher()
{
#if defined(_WIN32)
    return std::string(GetVisItInstallationDirectory() + "\\visit.exe");
#else
    return std::string(GetVisItInstallationDirectory() + "/bin/visit");
#endif
}

// ****************************************************************************
// Function: ReadInstallationInfo
//
// Purpose: 
//   Reads VisIt's .visitinstall file, which is a file that visit-install creates
//   that tells us the arguments used to install VisIt. Mostly, we want it so
//   we know which distribution file was used during installation so we can
//   download that same file.
//
// Arguments:
//   distName   : Return argument for the distribution name.
//   configName : Return argument for the config name.
//   bankName   : Return argument for the bank that was used.
//
// Returns:    True if the distribution can be determined; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 11:16:08 PDT 2008
//
// Modifications:
//
//   Tom Fogal, Sun Apr 19 12:48:38 MST 2009
//   Use `Environment' to simplify and fix a compilation error.
//   
// ****************************************************************************

bool
ReadInstallationInfo(std::string &distName, std::string &configName, std::string &bankName)
{
    // There is a 1:1 match between archNames and distNames.

    //
    // Architecture names that match VisIt's architecture directories. These are the
    // directory names that you'd find in /usr/local/apps/visit/<ver>
    //
    static const char *archNames[] = {
    "linux-intel",
    "linux-intel",
    "linux-x86_64",
    "linux-x86_64",
    "linux-ia64",
    "linux-ia64",

    "darwin-i386",
    "darwin-ppc",

    "sun4-sunos5-sparc",

    "ibm-aix-pwr",
    "ibm-aix-pwr64",

    "sgi-irix6-mips2",

    // Deprecated
    "dec-osf1-alpha",
    };

#define NARCH (sizeof(archNames) / sizeof(const char *))

    //
    // Names that match VisIt's distribution names. These are used in the filenames
    // that contain a VisIt distribution: e.g. visit1_10_1.darwin-i386.tar.gz
    //
    static const char *distNames[] = {
    "linux_rhel3",
    "linux-ellipse",
    "linux-x86_64",
    "linux-x86_64-fedora4",
    "linux-ia64",
    "linux-altix",

    "darwin-i386",
    "darwin-ppc",

    "sunos5",

    "aix",
    "aix64",

    "irix6",

    // Deprecated
    "osf1",
    };

    //
    // Try and determine the platform that should be downloaded.
    //
    std::string archHome = Environment::get("VISITARCHHOME");
    bool platformDetermined = false;
    if(!archHome.empty())
    {
        std::string arch(archHome);

        // Try and read the .installinfo file that tells us just how VisIt
        // was installed. That way we can be sure that we pick up the right
        // Linux or AIX installation.
        std::string installinfo(archHome);
        if(installinfo[installinfo.length()-1] != '/')
            installinfo += "/";
        installinfo += ".installinfo";

        FILE *fp = fopen(installinfo.c_str(), "rt");
        if(fp != NULL)
        {
            int fver = 0;
            if(fscanf(fp, "%d;", &fver) == 1)
            {
                char str[200];
                for(int i = 0; i < 3; ++i)
                {
                    int j = 0;
                    for(; j < 200-1; ++j)
                    {                        
                        str[j] = (char)fgetc(fp);
                        if(str[j] == ';' || str[j] < ' ')
                            break;
                    }
                    str[j] = '\0';

                    if(j >= 1)
                    {
                        if(i == 0)
                            configName = str;
                        else if(i == 1)
                            bankName = str;
                        else
                        {
                            distName = str;
                            platformDetermined = true;
                        }                          
                    }
                    else
                    {
                        // error
                        break;
                    }
                }
            }

            fclose(fp);
        }

        // We could not open the .installinfo file so let's try and
        // determine the distName based on the archNames.
        if(!platformDetermined)
        {
            int lastSlash = arch.rfind("/");
            if(lastSlash != -1)
            {
                arch = arch.substr(lastSlash+1, arch.length() - lastSlash - 1);
                for(int i = 0; i < NARCH; ++i)
                {
                    if(arch == archNames[i])
                    {
                        platformDetermined = true;
                        distName = distNames[i];
                        break;
                    }
                }
            }
        }
    }

    return platformDetermined;
}

// ****************************************************************************
// Function: ConfigStateGetRunCount
//
// Purpose: 
//   Returns the number of times the current version of VisIt has been run.
//
// Arguments:
//    code : Returns the success/error code for the operation.
//
// Note:       The number of times the current version of VisIt has been run.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 16 09:55:53 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:59:15 PDT 2005
//   Changed interface to ReadKey.
//
//   Kathleen Bonnell, Thu Jan 10 14:01:59 PST 2008 
//   Removed Windows-specific code as it didn't work right on diskless boxes. 
//
// ****************************************************************************

int
ConfigStateGetRunCount(ConfigStateEnum &code)
{
    int nStartups = 1;
    std::string rcFile(GetUserVisItDirectory());
    rcFile += "state";
    rcFile += VISIT_VERSION;
    rcFile += ".txt";

    FILE *f = 0;
    if((f = fopen(rcFile.c_str(), "r")) != 0)
    {
        if(fscanf(f, "%d", &nStartups) == 1)
        { 
            if(nStartups < 0)
                nStartups = 1;
        }
        fclose(f);
        code = CONFIGSTATE_SUCCESS;
    }
    else
        code = CONFIGSTATE_IOERROR;

    return nStartups;
}

// ****************************************************************************
// Function: ConfigStateIncrementRunCount
//
// Purpose: 
//   Increments the number of times the current version of VisIt has been run.
//
// Arguments:
//    code : Returns the success/error code for the operation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 16 09:56:54 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:58:53 PDT 2005
//   Changed interface to WriteKey.
//
//   Brad Whitlock, Mon Mar 7 11:31:35 PDT 2005
//   Changed the Win32 logic.
//
//   Brad Whitlock, Mon Jul 11 09:58:13 PDT 2005
//   I fixed a win32 coding problem.
//
//   Kathleen Bonnell, Thu Jan 10 14:01:59 PST 2008 
//   Removed Windows-specific code as it didn't work right on diskless boxes. 
//
//   Kathleen Bonnell, Fri Jan 18 18:11:12 PST 2008 
//   code2 was being used before set, causing a crash on Windows, I changed
//   the if-statement so code2 is referenced only after it has been set
//   (when !firstTime).
//
// ****************************************************************************

void
ConfigStateIncrementRunCount(ConfigStateEnum &code)
{
    std::string rcFile(GetUserVisItDirectory());
    rcFile += "state";
    rcFile += VISIT_VERSION;
    rcFile += ".txt";

    // Does the file exist?
    bool firstTime = false;
    VisItStat_t s;
    if(VisItStat(rcFile.c_str(), &s) == -1)
        firstTime = true;

    ConfigStateEnum code2;
    int nStartups = firstTime ? 0 : ConfigStateGetRunCount(code2);
    if(!firstTime && code2 == CONFIGSTATE_IOERROR)
        nStartups = 0;
    FILE *f = 0;
    if((f = fopen(rcFile.c_str(), "w")) != 0)
    {
        fprintf(f, "%d\n", nStartups + 1);
        fclose(f);
        code = firstTime ? CONFIGSTATE_FIRSTTIME : CONFIGSTATE_SUCCESS;
    }
    else
        code = CONFIGSTATE_IOERROR;
}

// ****************************************************************************
//  Function:  GetVisItVersionFromString
//
//  Purpose:
//    Parse out a numerical major.minor.patch version number.
//
//  Arguments:
//    v0         the version std::string to parse
//    major      the first portion of the version string
//    minor      the second portion of the version string
//    patch      the third portion of the version string
//
//  Returns:
//    The magnitude of the return value is the number of numeric values
//    found at the beginning of the version string.
//    A negative return code means that there was extra text after the
//    numeric values had been parsed.
//    E.g. "1.4.5" has a return value of 3.
//         "1.4b5" has a return value of -2.
//         "b1.4"  has a return value of 0.
//
//  Programmer:  Brad Whitlock
//  Creation:    October 8, 2007
//
//  Modifications:
//    Jeremy Meredith, Mon Dec 10 13:32:10 EST 2007
//    Changed it a bit to be more careful about extra stuff at the end of the
//    string; a negative return value indicates that abs(returnvalue) numeric
//    values were found first, but that there's extra text after those numeric
//    numeric values.  This lets the caller distinguish between "1.4.5" (ret=3)
//    and "1.4.5b1"/"1.4.5b" (ret=-3), for example, as well as "1.4" (ret=2)
//    and "1.4b1"/"1.4b" (ret=-2).
//
//    Jeremy Meredith, Thu Aug  7 16:20:19 EDT 2008
//    Removed extraneous & from %s argument to scanf.
//
// ****************************************************************************
int
GetVisItVersionFromString(const char *v0, int &major, int &minor, int &patch)
{
    int ret = 0;
    char extra[1000] = "";
    
    if (ret==0 && sscanf(v0, "%d.%d.%d%s", &major, &minor, &patch, extra) == 4)
    {
        ret = -3;
    }
    if (ret==0 && sscanf(v0, "%d.%d.%d", &major, &minor, &patch) == 3)
    {
        ret = 3;
    }
    if (ret==0 && sscanf(v0, "%d.%d%s", &major, &minor, extra) == 3)
    {
        ret = -2;
    }
    if (ret==0 && sscanf(v0, "%d.%d", &major, &minor) == 2)
    {
        ret = 2;
    }
    if (ret==0 && sscanf(v0, "%d%s", &major, extra) == 2)
    {
        ret = -1;
    }
    if (ret==0 && sscanf(v0, "%d", &major) == 1)
    {
        ret = 1;
    }

    return ret;
}

// ****************************************************************************
//  Function:  VisItVersionsCompatible
//
//  Purpose:
//    See if two version strings have the same X and Y (in X.Y.Z).
//
//  Arguments:
//    v0         one version string to compare
//    v0         the other version string to compare
//
//  Programmer:  Brad Whitlock
//  Creation:    October 8, 2007
//
//  Modifications:
//    Jeremy Meredith, Mon Dec 10 13:18:16 EST 2007
//    Ensure both version strings have at least the first two parts numerical.
//    Used absolute value of the return value from GetVisItVersionFromString
//    because we're going to allow betas to remain compatible with non-betas.
//
// ****************************************************************************
bool
VisItVersionsCompatible(const char *v0, const char *v1)
{
    bool ret = false;
    int major0=0, minor0=0, patch0=0;
    int major1=0, minor1=0, patch1=0;

    if ((abs(GetVisItVersionFromString(v0, major0, minor0, patch0)) >= 2) &&
        (abs(GetVisItVersionFromString(v1, major1, minor1, patch1)) >= 2))
    {
        ret = (major0 == major1) && (minor0 == minor1);
    }

    return ret;
}

// ****************************************************************************
// Function: VersionGreaterThan
//
// Purpose: 
//   Returns true when v1 > v2 compared as VisIt versions.
//
// Arguments:
//   v1 : The first version
//   v2 : The second version
//
// Returns:    True when v1>v2
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 11:12:39 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

static int
VersionToInt(const std::string &version)
{
    int v[3];
    GetVisItVersionFromString(version.c_str(), v[0], v[1], v[2]);
    return v[0]*10000 + v[1]*100 + v[2];
}

bool
VersionGreaterThan(const std::string &v1, const std::string &v2)
{
    return VersionToInt(v1) > VersionToInt(v2);
}
