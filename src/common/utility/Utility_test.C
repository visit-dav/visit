/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
