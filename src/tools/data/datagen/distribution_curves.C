// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            distribution_curves.C                          //
// ************************************************************************* //

#include <stdio.h>
#include <math.h>

// Cyrus Harrison, Tue Feb  2 09:21:03 PST 2010
// changed to use fstream directry instead of of visitstream.h

#include <fstream>
using namespace std;


// ****************************************************************************
//  Function: main
//
//  Purpose:
//      Creates Ultra files for well known distributions (useful for testing
//      queries like kurtosis and skewness).
//
//  Programmer: Hank Childs
//  Creation:   August 5, 2005
//
//  Modification:
//    Kathleen Bonnell, Fri Oct 28 14:27:12 PDT 2005
//    Add TIME to end of file.
//
// ****************************************************************************

#ifndef M_E
#define M_E 2.7182818284590452354
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main()
{
    int  i;
    int npts = 50000;

    ofstream ofile("distribution.ultra");

    ofile << "# Laplace Distribution" << endl;
    double mean = 0.;
    double b = 1.;
    for (i = 0 ; i < npts ; i++)
    {
        double X = 20*((i-(npts/2.)) / (npts/2.));
        double LP = 1/(2.*b) * pow(M_E, -fabs(X-mean)/b);
        ofile << X << " " << LP << endl;
    }

    ofile << "# Log Normal Distribution" << endl;
    double S = 1.;
    double M = 0.;
    for (i = 0 ; i < npts ; i++)
    {
        double X = 20*((i+1.) / npts);
        double denom = 1./(S*sqrt(2*M_PI)*X);
        double exp = -log(X-M)*log(X-M) / (2*S*S);
        double log_normal = denom * pow(M_E, exp);
        ofile << X << "\t" << log_normal << endl;
    }

    ofile << "# Exponential Distribution" << endl;
    double lambda = 1;
    for (i = 0 ; i < npts ; i++)
    {
        double X = 20*(((float)i) / npts);
        ofile << X << "\t" << pow(M_E, -lambda*X) << endl;
    }
    ofile << "# TIME 56.789" << endl;
}


