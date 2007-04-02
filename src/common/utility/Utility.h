/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                Utility.h                                  //
// ************************************************************************* //

#ifndef UTILITY_H
#define UTILITY_H
#include <utility_exports.h>
#include <visit-config.h>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>

//
// Type definitions
//
typedef void (ProcessDirectoryCallback)(void *, const std::string &, bool,
                                        bool, long);
typedef enum {CONFIGSTATE_IOERROR,
              CONFIGSTATE_FIRSTTIME,
              CONFIGSTATE_SUCCESS} ConfigStateEnum;

#if defined(_WIN32)
typedef struct _stat VisItStat_t;
typedef off_t VisItOff_t;
#else

#if SIZEOF_OFF64_T > 4
typedef struct stat64 VisItStat_t;
typedef off64_t VisItOff_t;
#else
typedef struct stat VisItStat_t;
typedef off_t VisItOff_t;
#endif

#endif

//
// Function Prototypes
//
char UTILITY_API *CreateMessageStrings(char **, int *, int);
int  UTILITY_API  LongestCommonPrefixLength(const char * const *, int);
int  UTILITY_API  LongestCommonSuffixLength(const char * const *, int);
bool UTILITY_API  ReadAndProcessDirectory(const std::string &,
                                          ProcessDirectoryCallback *,
                                          void * = 0,
                                          bool = false);
std::string UTILITY_API ExpandUserPath(const std::string &);

void UTILITY_API  WaitUntilFile(const char *);
bool UTILITY_API  WildcardStringMatch(const char *p, const char *s);
bool UTILITY_API  WildcardStringMatch(const std::string &p, const std::string &s);
bool UTILITY_API  NumericStringCompare(const std::string &str1, const std::string &str2);

std::vector<std::string> UTILITY_API SplitValues(const std::string &buff,
                                                 char delim);

std::string UTILITY_API GetVisItInstallationDirectory();
std::string UTILITY_API GetVisItInstallationDirectory(const char *version);
std::string UTILITY_API GetVisItArchitectureDirectory();
std::string UTILITY_API GetVisItArchitectureDirectory(const char *version);
std::string UTILITY_API GetVisItLauncher();
void        UTILITY_API SetIsDevelopmentVersion(bool val);
bool        UTILITY_API GetIsDevelopmentVersion();

std::string UTILITY_API GetUserVisItDirectory();
UTILITY_API char *      GetDefaultConfigFile(const char *filename = 0, const char *home = 0);
UTILITY_API char *      GetSystemConfigFile(const char *filename = 0);

int         UTILITY_API ConfigStateGetRunCount(ConfigStateEnum &code);
void        UTILITY_API ConfigStateIncrementRunCount(ConfigStateEnum &code);

int         UTILITY_API VisItStat(const char *filename, VisItStat_t *buf);
int         UTILITY_API VisItFstat(int fd, VisItStat_t *buf);

inline char *C_strdup(char const * const);
inline char *CXX_strdup(char const * const);
inline char *CXX_strndup(char const * const, size_t);
inline void  InlineCopy(char *&, const char * const &, const int &);
inline void  InlineExtract(char * const &, const char *&, const int &);

//
// Includes for inlined functions.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// ****************************************************************************
//  Method: C_strdup
//
//  Purpose:
//      Acts like a strdup, which is not used because it is apparently unsafe.
//      Makes a copy of the string using calloc.
//
//  Returns:    A copy of the argument string.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
// ****************************************************************************

inline char *
C_strdup(char const * const c)
{
    void *v = calloc(strlen(c)+1, sizeof(char));
    char *p = static_cast< char * >(v);
    strcpy(p, c);
    return p;
}


// ****************************************************************************
//  Method: CXX_strdup
//
//  Purpose:
//      Acts like a strdup, which is not used because it is apparently unsafe.
//      Makes a copy of the string using new.
//
//  Returns:    A copy of the argument string.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
// ****************************************************************************

inline char *
CXX_strdup(char const * const c)
{
    char *p = new char[strlen(c)+1];
    strcpy(p, c);
    return p;
}


// ****************************************************************************
//  Method: CXX_strndup
//
//  Purpose:
//      Acts like a strndup, which is not used because it is apparently unsafe.
//      Makes a copy of the string using new.
//
//  Returns:    A copy of the argument string.
//
//  Programmer: Eric Brugger
//  Creation:   December 11, 2006
//
// ****************************************************************************

inline char *
CXX_strndup(char const * const c, size_t n)
{
    int len = (strlen(c) < n) ? strlen(c) : n;
    char *p = new char[len+1];
    memcpy(p, c, len);
    p[len] = '\0';
    return p;
}


// ****************************************************************************
//  Function: InlineCopy
//
//  Purpose:
//      Copies a character array to another character array and offsets the
//      target list by the amount copied.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2001
//
// ****************************************************************************

inline void 
InlineCopy(char *&target, const char * const &src, const int &amount)
{
    for (int i = 0 ; i < amount ; i++)
    {
        target[i] = src[i];
    }
    target += amount;
}


// ****************************************************************************
//  Function: InlineExtract
//
//  Purpose:
//      Copies a character array to another character array and offsets the
//      source list by the amount copied.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2001
//
// ****************************************************************************

inline void
InlineExtract(char * const &target, const char *&src, const int &amount)
{
    for (int i = 0 ; i < amount ; i++)
    {
        target[i] = src[i];
    }
    src += amount;
}


#endif


