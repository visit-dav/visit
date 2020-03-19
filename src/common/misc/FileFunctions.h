// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef FILE_FUNCTIONS_H
#define FILE_FUNCTIONS_H
#include <misc_exports.h>
#include <string>

#include <visit-config.h>
#include <sys/types.h>
#include <sys/stat.h>

struct dirent;

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
//    Mark C. Miller, Thu Mar 15 14:18:43 PDT 2018
//    Added FileType enum and GetFileType methods.
//
//    Mark C. Miller, Fri Mar 23 12:10:59 PDT 2018
//    Added LINK to FileTypes.
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
  #ifndef S_ISREG
    #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
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

typedef enum
{
    FILE_TYPE_REG,
    FILE_TYPE_DIR,
    FILE_TYPE_LINK,
    FILE_TYPE_OTHER,
    FILE_TYPE_NOT_KNOWN
} FileType;

VisItStat_t* const FILE_TYPE_DONT_STAT = ((VisItStat_t*)0x1);

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
FileType MISC_API GetFileType(char const *filename, struct dirent const *dent = 0,
         VisItStat_t *statbuf = 0);
FileType MISC_API GetFileType(std::string const &filename, struct dirent const *dent = 0,
         VisItStat_t *statbuf = 0);

bool MISC_API ReadTextFile(const std::string &filename, std::string &fileContents);
};

#endif
