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

#include <iostream>
using namespace std;

// ****************************************************************************
//  File:  genpolycases.cpp
//
//  Purpose:
//    Generates the transforms of a polygon.
//    Points are 0-(npts-1), edges are A-L.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  9, 2012
//
// ****************************************************************************

void
GenCaseFwd(int npts, int shift)
{
    cerr << "{{";
    for (int i=0; i<npts; i++)
    {
        cerr << ((i+shift)%npts) << (i<npts-1?",":"");
    }
    cerr << "},{";
    for (int i=0; i<npts; i++)
    {
        cerr << "'"
             << char(int('a')+((i+shift)%npts))
             << "'" << (i<npts-1?",":"");
    }
    cerr << "},";
    cerr << "false";
    cerr << " }," << endl;
}

void
GenCaseRev(int npts, int shift)
{
    cerr << "{{";
    for (int i=0; i<npts; i++)
    {
        cerr << ((npts-1-i+shift)%npts) << (i<npts-1?",":"");
    }
    cerr << "},{";
    for (int i=0; i<npts; i++)
    {
        cerr << "'"
             << char(int('a')+((npts+npts-2-i+shift)%npts))
             << "'" << (i<npts-1?",":"");
    }
    cerr << "},";
    cerr << "true ";
    cerr << " }," << endl;
}

void
GenCases(int npts)
{
    cerr << "\n-- polygon with "<<npts<<" points --\n";
    for (int i=0; i<npts; i++)
        GenCaseFwd(npts, i);
    for (int i=0; i<npts; i++)
        GenCaseRev(npts, i);
}

int main()
{
    GenCases(3);
    GenCases(4);
    GenCases(5);
    GenCases(6);
    GenCases(7);
    GenCases(8);
}
