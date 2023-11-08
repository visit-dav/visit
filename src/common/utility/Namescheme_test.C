// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <Namescheme.h>
#include <string.h>
#include <iostream>
#include <stdio.h>

int main()
{
    int i;
    int P[100], U[4], PFS[4] = {0,1,2,3};
    const char *N[3] = { "red", "green", "blue" };
    const char *FileNumbers[] = {"1","2","3"};

    // Test a somewhat complex expression 
    Namescheme *ns = new Namescheme("@foo_%+03d@3-((n % 3)*(4+1)+1/2)+1");
    if (strcmp(ns->GetName(25), "foo_+01") != 0)
        return 1;
    delete ns;

    // Test a constant namescheme
    ns = new Namescheme("gorfo");
    if (strcmp(ns->GetName(0), "gorfo") != 0)
        return 1;
    if (strcmp(ns->GetName(151), "gorfo") != 0)
        return 1;
    if (strcmp(ns->GetName(20861), "gorfo") != 0)
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
        "@(n/2)?((n/9)?((n/26)?3:2:):1:):0:"        // level part (1rst %d) 
        "@(n/2)?((n/9)?((n/26)?n-26:n-9:):n-2:):n:" // patch part (2nd %d)
    );
    if (strcmp(ns->GetName( 1), "level0,patch1")  != 0) return 1; // A
    if (strcmp(ns->GetName( 3), "level1,patch1")  != 0) return 1; // B
    if (strcmp(ns->GetName( 8), "level1,patch6")  != 0) return 1; // C
    if (strcmp(ns->GetName( 9), "level2,patch0")  != 0) return 1; // D
    if (strcmp(ns->GetName(20), "level2,patch11") != 0) return 1; // E
    if (strcmp(ns->GetName(25), "level2,patch16") != 0) return 1; // F
    if (strcmp(ns->GetName(26), "level3,patch0")  != 0) return 1; // G
    if (strcmp(ns->GetName(30), "level3,patch4")  != 0) return 1; // H
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
    ns = new Namescheme("#foo_%s#(n-5)?'manager':'worker':");
    if (strcmp(ns->GetName(5), "foo_worker") != 0)
        return 1;
    if (strcmp(ns->GetName(6), "foo_manager") != 0)
        return 1;
    delete ns;

    // Test array-based references in a name scheme
    for (i = 0; i < 100; i++)
        P[i] = i*5;
    for (i = 0; i < 4; i++)
        U[i] = i*i;
    ns = new Namescheme("@foo_%03dx%03d@#P[n]@#U[n%4]", P, U);
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

    // Test array-based references to char* valued array
    ns = new Namescheme("Hfoo_%sH$N[n%3]", N);
    if (strcmp(ns->GetName(17), "foo_blue") != 0) return 1;
    if (strcmp(ns->GetName(6), "foo_red") != 0) return 1;
    delete ns;

    // Test McCandless' example (new way)
    ns = new Namescheme("@%s@(n/4)?'&myfilename.%d&n/4':'':@");
    if (strcmp(ns->GetName(0), "") != 0) return 1;
    if (strcmp(ns->GetName(1), "") != 0) return 1;
    if (strcmp(ns->GetName(4), "myfilename.1") != 0) return 1;
    if (strcmp(ns->GetName(15), "myfilename.3") != 0) return 1;
    delete ns;

    // Test McCandless' example (old way)
    ns = new Namescheme("@%s%s@(n/4)?'myfilename.':'':@(n/4)?$/arr_dir/FileNumbers[n/4-1]:'':",FileNumbers);
    if (strcmp(ns->GetName(0), "") != 0) return 1;
    if (strcmp(ns->GetName(1), "") != 0) return 1;
    if (strcmp(ns->GetName(4), "myfilename.1") != 0) return 1;
    if (strcmp(ns->GetName(15), "myfilename.3") != 0) return 1;
    delete ns;

    // Text Exodus material volume fraction variable convention
    ns = new Namescheme("@%s@n?'&VOLFRC_%d&n':'VOID_FRC':@");
    if (strcmp(ns->GetName(0), "VOID_FRC") != 0) return 1;
    if (strcmp(ns->GetName(1), "VOLFRC_1") != 0) return 1;
    if (strcmp(ns->GetName(2), "VOLFRC_2") != 0) return 1;
    if (strcmp(ns->GetName(10), "VOLFRC_10") != 0) return 1;
    if (strcmp(ns->GetName(2746), "VOLFRC_2746") != 0) return 1;
    delete ns;

    ns = new Namescheme("|chemA_016_00000%s%.0d|#PFS[(n/4) % 4]?'.':'':|#PFS[(n/4) % 4]", PFS);
    if (strcmp(ns->GetName(0), "chemA_016_00000") != 0) return 1;
    if (strcmp(ns->GetName(1), "chemA_016_00000") != 0) return 1;
    if (strcmp(ns->GetName(2), "chemA_016_00000") != 0) return 1;
    if (strcmp(ns->GetName(3), "chemA_016_00000") != 0) return 1;
    if (strcmp(ns->GetName(4), "chemA_016_00000.1") != 0) return 1;
    if (strcmp(ns->GetName(5), "chemA_016_00000.1") != 0) return 1;
    if (strcmp(ns->GetName(8), "chemA_016_00000.2") != 0) return 1;
    if (strcmp(ns->GetName(11), "chemA_016_00000.2") != 0) return 1;
    if (strcmp(ns->GetName(15), "chemA_016_00000.3") != 0) return 1;
    delete ns;

    ns = new Namescheme("|chemA_%04X|n%3");
    if (ns->GetIndex(0) != 0) return 1;
    if (ns->GetIndex(1) != 1) return 1;
    if (ns->GetIndex(2) != 2) return 1;
    if (ns->GetIndex(3) != 0) return 1;
    if (ns->GetIndex(4) != 1) return 1;
    delete ns;

    // This is only necessary if you wanna run valgrind and confirm no leaks
    Namescheme::FreeClassStaticResources();

    return 0;
}
