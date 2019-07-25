// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cassert>
#include <cstdio>
#include <Utility.h>
#include <visitstream.h>
#include <vector>

using std::vector;
using std::string;

#define CHECK_WCSM(P,S,R) \
{\
    if (WildcardStringMatch(P,S) != R) \
    {\
        cerr << "Test on line " << __LINE__ << " failed." << endl;\
        cerr << "    When matching pattern \"" << P << "\" against string \"" << S << "\"" << endl;\
        cerr << "    WildcardStringMatch returned " << !R << " instead of " << R << endl;\
        wcsm_error_cnt++;\
    }\
}

int main(int argc, char **argv)
{
    int wcsm_error_cnt = 0;

    // Test WildcardStringMatch function
    CHECK_WCSM("*.exo.##.##",
       "conepulse.exo.16.00",  true);

    CHECK_WCSM("*.exo.##.##",
       "CONEPULSE.exo.16.00",  true);

    CHECK_WCSM("*.EXO.##.##",
       "CONEPULSE.exo.16.00",  true);

    CHECK_WCSM("*.##.##.exo",
       "conepulse.16.00.exo",  true);

    CHECK_WCSM("*.*.*.exo",
     "conepulse.16.00.exo",    true);

    CHECK_WCSM("*.exo.##.##",
       "conepulse.exo.160.00", false);

    CHECK_WCSM("*.exo.###.###",
       "conepulse.exo.16.00",  false);

    CHECK_WCSM("*.exo.#.##",
       "conepulse.exo.16.00",  false);

    CHECK_WCSM("*.exo",
       "conepulse.exo.16.00",  false);

    CHECK_WCSM("*.exo",
 "conepulse.16.00.exo",        true);

    CHECK_WCSM("*.##.##.exo",
       "conepulse.16.00.exo",  true);

    CHECK_WCSM("*.ex*",
 "conepulse.16.00.exo",        true);

    CHECK_WCSM("*.foo.##.##",
       "conepulse.exo.16.00",  false);

    CHECK_WCSM("UCD_#*_#*",
               "UCD_1abc_2def",true);

#if 0
    CHECK_WCSM("UCD_#*_#*",
               "UCD_1 _2",      false);
#endif

    CHECK_WCSM("U_#*.inp",     
               "U_1abc.inp",   true);

    CHECK_WCSM("U_#*.inp",     
               "u_1abc.inp",   true);

    return wcsm_error_cnt;
}
