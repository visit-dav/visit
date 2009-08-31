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
// ****************************************************************************
class UTILITY_API Namescheme
{
  public:
    Namescheme(const char *fmt, ...);
   ~Namescheme();
    const char *GetName(int n);

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

    static void FreeTree(DBexprnode *tree);
    static DBexprnode *UpdateTree(DBexprnode *tree, const char t, int v, char *s);
    static DBexprnode *BuildExprTree(const char **porig);
    static int SaveString(Namescheme *ns, const char *sval);
    static int EvalExprTree(Namescheme *ns, DBexprnode *tree, int n);

    char *fmt;            // orig. format string
    const char **fmtptrs; // ptrs into printf part of fmt for each conversion spec.
    int fmtlen;           // len of printf part of fmt
    int ncspecs;          // # of conversion specs in printf part of fmt
    char delim;           // delimiter char used for parts of fmt
    int nembed;           // number of last embedded string encountered
    char **embedstrs;     // ptrs to copies of embedded strings
    int narrefs;          // number of array refs in conversion specs
    char **arrnames;      // array names used by array refs
    const int **arrvals;  // pointer to actual array data assoc. with each name
    char **exprstrs;      // expressions to be evaluated for each conv. spec.
};

#endif
