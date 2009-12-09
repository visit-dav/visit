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
#include <Namescheme.h>
#include <string.h>
//#include <stdio.h>

int main()
{
    int i;
    int P[100], U[4];

    // Test a somewhat complex expression 
    Namescheme *ns = new Namescheme("@foo_%+03d@3-((n % 3)*(4+1)+1/2)+1");
    if (strcmp(ns->GetName(25), "foo_+01") != 0)
        return 1;
    delete ns;

    // Test ?:: operator
    ns = new Namescheme("@foo_%d@(n-5)?14:77:");
    if (strcmp(ns->GetName(6), "foo_14") != 0)
        return 1;
    delete ns;

    // Example of AMR-like naming convention where we have the following
    // assignment of pathces to levels starting with the patches on
    // level 0 and ending with the patches on level 3...
    //
    // level: 0 0 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 3 3 3 3 3
    //           |             |                                 |
    //        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 0 0 0 0 0 
    // patch: 0 1 0 1 2 3 4 5 6 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 0 1 2 3 4
    //           |             |                                 |
    //        0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3
    //     n: 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
    //          A   B         C D                     E         F G       H
    //
    //  2 patches at level 0
    //  7 patches at level 1
    // 17 patches at level 2
    //  5 patches at level 3
    ns = new Namescheme("@level%d,patch%d"
        "@(n/2)?((n/9)?((n/26)?3:2:):1:):0:"
        "@(n/2)?((n/9)?((n/26)?n-26:n-9:):n-2:):n:");
    if (strcmp(ns->GetName( 1), "level0,patch1")  != 0) return 1; // A
    if (strcmp(ns->GetName( 3), "level1,patch1")  != 0) return 1; // B
    if (strcmp(ns->GetName( 8), "level1,patch6")  != 0) return 1; // C
    if (strcmp(ns->GetName( 9), "level2,patch0")  != 0) return 1; // D
    if (strcmp(ns->GetName(20), "level2,patch11") != 0) return 1; // E
    if (strcmp(ns->GetName(25), "level2,patch16") != 0) return 1; // F
    if (strcmp(ns->GetName(26), "level3,patch0")  != 0) return 1; // G
    if (strcmp(ns->GetName(30), "level3,patch4")  != 0) return 1; // H
#if 0
    printf("\"%s\"\n", ns->GetName( 1));
    printf("\"%s\"\n", ns->GetName( 3));
    printf("\"%s\"\n", ns->GetName( 8));
    printf("\"%s\"\n", ns->GetName( 9));
    printf("\"%s\"\n", ns->GetName(20));
    printf("\"%s\"\n", ns->GetName(25));
    printf("\"%s\"\n", ns->GetName(26));
#endif
    delete ns;

    // Test multiple conversion specifiers
    ns = new Namescheme("|foo_%03dx%03d|n/5|n%5");
    if (strcmp(ns->GetName(17), "foo_003x002") != 0)
       return 1;
    if (strcmp(ns->GetName(20), "foo_004x000") != 0)
       return 1;
    if (strcmp(ns->GetName(3), "foo_000x003") != 0)
       return 1;
    delete ns;

    // Test embedded string value results
    ns = new Namescheme("#foo_%s#(n-5)?'master':'slave':");
    if (strcmp(ns->GetName(6), "foo_master") != 0)
        return 1;
    delete ns;

    // Test array-based references in a name scheme
    for (i = 0; i < 100; i++)
        P[i] = i*5;
    for (i = 0; i < 4; i++)
        U[i] = i*i;
    ns = new Namescheme("#foo_%03dx%03d#$P[n]#$U[n%4]", P, U);
    if (strcmp(ns->GetName(17), "foo_085x001") != 0)
        return 1;
    if (strcmp(ns->GetName(18), "foo_090x004") != 0)
        return 1;
    if (strcmp(ns->GetName(19), "foo_095x009") != 0)
        return 1;
    if (strcmp(ns->GetName(20), "foo_100x000") != 0)
        return 1;
    if (strcmp(ns->GetName(21), "foo_105x001") != 0)
        return 1;
    delete ns;

    return 0;
}
