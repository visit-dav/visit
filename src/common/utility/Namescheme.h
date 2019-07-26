// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               Namescheme.h                                //
// ************************************************************************* //

#ifndef NAMESCHEME_H
#define NAMESCHEME_H
#include <utility_exports.h>

// ****************************************************************************
//  Class: Namescheme 
//
//  Purpose: On-the-fly mapping of the natural numbers (0,1,2,3...) to strings
//      using printf-style name-expression generation. A common example is
//      using the string 'domain_%03d' to generate strings of the form...
//          domain_001
//          domain_023
//          domain_148
//
//  This code was adapted from silo library's namescheme function, where it
//  originated as C code hence prolific use of char* instead of std::string.
//
//  For examples of use, see the test client, Namescheme_test.C.
//
//  Programmer: Mark C. Miller
//  Creation:   Wed Aug 26 15:34:45 PDT 2009
//
//  Modifications
//
//    Mark C. Miller, Thu Dec 18 13:05:54 PST 2014
//    Changed embedded strings to be embedded nameschemes.
// ****************************************************************************
class UTILITY_API Namescheme
{
  public:
    Namescheme(const char *fmt, ...);
   ~Namescheme();
    const char *GetName(int n);
    int GetIndex(int n);
    static void FreeClassStaticResources(void);

  private:

    typedef struct _DBexprnode {
        char type;
        int val;
        char sval[128];
        struct _DBexprnode *left;
        struct _DBexprnode *right;
    } DBexprnode;

    Namescheme() {};

    static const int max_expstrs;
    static const int max_fmtlen;
    static const int max_retstrs = 32;
    static char *retstrbuf[max_retstrs];

    static void FreeTree(DBexprnode *tree);
    static DBexprnode *UpdateTree(DBexprnode *tree, const char t, int v, char *s);
    static DBexprnode *BuildExprTree(const char **porig);
    static int SaveInternalString(Namescheme *ns, char const * const sval);
    static char *SaveReturnedString(char const * const sval);
    static int EvalExprTree(Namescheme *ns, DBexprnode *tree, int n);

    char *fmt;            // orig. format string
    const char **fmtptrs; // ptrs into printf part of fmt for each conversion spec.
    int fmtlen;           // len of printf part of fmt
    int ncspecs;          // # of conversion specs in printf part of fmt
    char delim;           // delimiter char used for parts of fmt
    int nembed;           // number of last embedded string encountered
    Namescheme **embedns; // ptrs to copies of embedded nameschemes
    int narrefs;          // number of array refs in conversion specs
    char **arrnames;      // array names used by array refs
    const int **arrvals;  // pointer to actual array data assoc. with each name
    char **exprstrs;      // expressions to be evaluated for each conv. spec.
    DBexprnode **exprtrees; // Built expression trees for each conv. spec.
};

#endif
