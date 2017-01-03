/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#ifndef FILE_FUNCTIONS_H
#define FILE_FUNCTIONS_H
#include <misc_exports.h>
#include <string>

#include <visit-config.h>
#include <sys/types.h>
#include <sys/stat.h>

// ****************************************************************************
//   Modifications:
//    Kathleen Biagas, Fri Jun 26 12:10:08 PDT 2015
//    Added FileMatchesPatternCB, a callback that can be used with
//    ReadAndProcessDirectory. (Moved from NetworkManager).
//
//    Kathleen Biagas, Wed Nov 24 16:26:11 MST 2015
//    Use _stat64 for VisItStat_t and __int64 for VisItOff_t if running a
//    64 bit version on windows.
//
// ****************************************************************************

namespace FileFunctions
{
//
// Type definitions
//
typedef void (ProcessDirectoryCallback)(void *, const std::string &, bool,
                                        bool, long);

#if defined(_WIN32)
  #if defined(_WIN64)
    typedef struct _stat64 VisItStat_t;
    typedef __int64 VisItOff_t;
  #else
    typedef struct _stat VisItStat_t;
    typedef off_t VisItOff_t;
  #endif
  typedef unsigned short mode_t;
  #ifndef S_ISDIR
    #define S_ISDIR(m) (((m) &S_IFMT) == S_IFDIR)
  #endif
#else
  #if SIZEOF_OFF64_T > 4
    typedef struct stat64 VisItStat_t;
    typedef off64_t VisItOff_t;
  #else
    typedef struct stat VisItStat_t;
    typedef off_t VisItOff_t;
  #endif
#endif

typedef enum
{
    PERMISSION_RESULT_READABLE,
    PERMISSION_RESULT_NONREADABLE,
    PERMISSION_RESULT_NOFILE
} PermissionsResult;

PermissionsResult MISC_API CheckPermissions(const std::string &filename);

int         MISC_API VisItStat(const std::string &filename, VisItStat_t *buf);
int         MISC_API VisItFstat(int fd, VisItStat_t *buf);
bool        MISC_API ReadAndProcessDirectory(const std::string &,
                                      ProcessDirectoryCallback *,
                                      void * = 0,
                                      bool = false);
std::string MISC_API GetCurrentWorkingDirectory();
std::string MISC_API ExpandPath(const std::string &path, 
                                const std::string &workingDir = std::string());
std::string MISC_API FilteredPath(const std::string &path);

// Filename manipulation
const char  MISC_API *Basename(const char *path, const char *suffix=0);
std::string MISC_API  Basename(const std::string &path, const std::string &suffix="");
const char  MISC_API *Dirname(const char *path);
std::string MISC_API  Dirname(const std::string &path);
const char  MISC_API *Absname(const char *cwd_context, 
                              const char *path,
                              const char *pathSep = VISIT_SLASH_STRING);
std::string MISC_API  Absname(const std::string &cwd_context, 
                              const std::string &path,
                              const std::string &pathSep = std::string(VISIT_SLASH_STRING));
const char  MISC_API *Normalize(const char *path,
                                const char *pathSep = VISIT_SLASH_STRING);
std::string MISC_API  Normalize(const std::string &path,
                                const std::string &pathSep = std::string(VISIT_SLASH_STRING));

void MISC_API         SplitHostDatabase(const std::string &hostDB, 
                                        std::string &host, std::string &db);
std::string MISC_API  ComposeDatabaseName(const std::string &host,
                                          const std::string &db);

void MISC_API  FileMatchesPatternCB(void *, const std::string &, bool, bool, long);
};

#endif
