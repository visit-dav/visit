// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                Utility.h                                  //
// ************************************************************************* //

#ifndef UTILITY_H
#define UTILITY_H
#include <utility_exports.h>
#include <visitstream.h>
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
void UTILITY_API  PrintCallStack(ostream &out, const char *file, int line);

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

bool UTILITY_API CheckHostValidity(const std::string &host);
bool UTILITY_API ConvertArgsToTunneledValues(const std::map<int,int>&,
                                             std::vector<std::string>&);
bool UTILITY_API GetSSHClient(std::string &sshClient);

inline char *C_strdup(char const * const);
inline char *C_strndup(char const * const, size_t);
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

inline char *
C_strndup(char const * const c, size_t n)
{
    size_t len = (strlen(c) < n) ? strlen(c) : n;
    char *p = (char *) malloc(len+1);
    memcpy(p, c, len);
    p[len] = '\0';
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
    size_t len = (strlen(c) < n) ? strlen(c) : n;
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


