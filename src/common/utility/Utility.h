// ************************************************************************* //
//                                Utility.h                                  //
// ************************************************************************* //

#ifndef UTILITY_H
#define UTILITY_H
#include <utility_exports.h>
#include <string>

//
// Function Prototypes
//
char UTILITY_API *CreateMessageStrings(char **, int *, int);
int  UTILITY_API  LongestCommonPrefixLength(const char * const *, int);
int  UTILITY_API  LongestCommonSuffixLength(const char * const *, int);
void UTILITY_API  WaitUntilFile(const char *);
bool UTILITY_API  WildcardStringMatch(const char *p, const char *s);
bool UTILITY_API  WildcardStringMatch(const std::string &p, const std::string &s);

inline char *C_strdup(char const * const);
inline char *CXX_strdup(char const * const);
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

char *
CXX_strdup(char const * const c)
{
    char *p = new char[strlen(c)+1];
    strcpy(p, c);
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


