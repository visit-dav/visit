// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
