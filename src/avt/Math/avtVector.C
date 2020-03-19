// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "avtVector.h"
#include <vtkMath.h>
#include <stdio.h>

// ****************************************************************************
//  Method:  avtVector::perpendiculars
//
//  Purpose:
//    Given a vector x, find two vectors perpendicular to x.
//
//  Programmer:  Dave Pugmire
//  Creation:    February 15, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtVector::perpendiculars(const avtVector &x, avtVector &y, avtVector &z, double theta)
{
    double X[3] = {x.x, x.y, x.z}, Y[3], Z[3];
    vtkMath::Perpendiculars(X, Y, Z, theta);

    y.x = Y[0];
    y.y = Y[1];
    y.z = Y[2];

    z.x = Z[0];
    z.y = Z[1];
    z.z = Z[2];
}




ostream &operator<<(ostream& out, const avtVector &r)
{
    out << "<" << r.x << "," << r.y << "," << r.z << ">";
    return out;
}

