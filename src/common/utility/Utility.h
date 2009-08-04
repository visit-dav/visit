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

// ************************************************************************* //
//                                Utility.h                                  //
// ************************************************************************* //

#ifndef UTILITY_H
#define UTILITY_H
#include <utility_exports.h>
#include <visit-config.h>
#include <vectortypes.h>
#include <map>

#include <limits.h>
#include <float.h>

//
// Function Prototypes
//
char UTILITY_API *CreateMessageStrings(char **, int *, int);
int  UTILITY_API  LongestCommonPrefixLength(const char * const *, int);
int  UTILITY_API  LongestCommonSuffixLength(const char * const *, int);

// LINUX ONLY
void UTILITY_API  GetMemorySize(unsigned int &, unsigned int &);

void UTILITY_API  WaitUntilFile(const char *);
bool UTILITY_API  WildcardStringMatch(const char *p, const char *s);
bool UTILITY_API  WildcardStringMatch(const std::string &p, const std::string &s);
bool UTILITY_API  NumericStringCompare(const std::string &str1, const std::string &str2);

std::vector<std::string> UTILITY_API SplitValues(const std::string &buff,
                                                 char delim);

bool        UTILITY_API VisItInitExtentsToLimits(double *exts, int n);
bool        UTILITY_API VisItInitExtentsToLimits(float *exts, int n);
bool        UTILITY_API VisItInitExtentsToLimits(int *exts, int n);

// NOTE: Put these in their own header.
void UTILITY_API PutOnSameXIntervals(int on1, const float *ox1, 
        const float *oy1, int on2, const float *ox2, const float *oy2,
        floatVector &usedX, floatVector &newCurve1Vals,
        floatVector &newCurve2Vals);
void UTILITY_API AverageYValsForDuplicateX(int n, const float *x, 
        const float *y, floatVector &X, floatVector &Y);


#if defined(PANTHERHACK)
// Broken on Panther
#else
bool        UTILITY_API ConvertArgsToTunneledValues(const std::map<int,int>&,
                                                    std::vector<std::string>&);
#endif

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

// ****************************************************************************
//  Function: VisItInitExtentsToLimits
//
//  Purpose: Initialize an extents array to numerical limits of platform
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2007 
//
// ****************************************************************************

inline bool 
VisItInitExtentsToLimits(double *exts, int n)
{
    if (!(n == 2 || n == 4 || n == 6))
        return false;

    for (int i = 0; i < n; i++)
        exts[i] = n % 2 ? -DBL_MAX : DBL_MAX;

    return true;
}

inline bool 
VisItInitExtentsToLimits(float *exts, int n)
{
    if (!(n == 2 || n == 4 || n == 6))
        return false;

    for (int i = 0; i < n; i++)
        exts[i] = n % 2 ? -FLT_MAX : FLT_MAX;

    return true;
}

inline bool 
VisItInitExtentsToLimits(int *exts, int n)
{
    if (!(n == 2 || n == 4 || n == 6))
        return false;

    for (int i = 0; i < n; i++)
        exts[i] = n % 2 ? -INT_MAX : INT_MAX;

    return true;
}
#endif


