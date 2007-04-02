/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                        avtAreaBetweenCurvesQuery.C                        //
// ************************************************************************* //

#include <avtAreaBetweenCurvesQuery.h>

#include <stdio.h>


// ****************************************************************************
//  Method: avtAreaBetweenCurvesQuery::avtAreaBetweenCurvesQuery
//
//  Purpose:
//      Construct an avtAreaBetweenCurvesQuery object.
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
// ****************************************************************************

avtAreaBetweenCurvesQuery::avtAreaBetweenCurvesQuery()
{
}


// ****************************************************************************
//  Method: avtAreaBetweenCurvesQuery::~avtAreaBetweenCurvesQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 2, 2003
//
// ****************************************************************************

avtAreaBetweenCurvesQuery::~avtAreaBetweenCurvesQuery()
{
}


// ****************************************************************************
//  Method: avtAreaBetweenCurvesQuery::CompareCurves
//
//  Purpose:
//      Computes the area between two curves.
//
//  Programmer:   Hank Childs
//  Creation:     October 3, 2003
//
// ****************************************************************************

double
avtAreaBetweenCurvesQuery::CompareCurves(int n1, const float *x1, const float *y1,
                              int n2, const float *x2, const float *y2)
{
    vector<float>  newCurve1Vals;
    vector<float>  newCurve2Vals;
    vector<float>  usedX;
    PutOnSameXIntervals(n1, x1, y1, n2, x2, y2, usedX, newCurve1Vals,
                        newCurve2Vals);

    int total_n_pts = usedX.size();
    double sum = 0;
    for (int i = 0 ; i < total_n_pts-1 ; i++)
    {
        // each pair forms a trapezoid.  sum the areas of the trapezoids.
        // Note: this does not take into account the degenerate case where
        // the trapezoid doubles back on itself.
        double base1 = fabs(newCurve1Vals[i] - newCurve2Vals[i]);
        double base2 = fabs(newCurve1Vals[i+1] - newCurve2Vals[i+1]);
        double height = usedX[i+1] - usedX[i];
        double area = height * (base1 + base2) / 2.;
        sum += area;
    }

    return sum;
}


// ****************************************************************************
//  Method: avtAreaBetweenCurvesQuery::CreateMessage
//
//  Purpose:
//      Creates a message stating the area between curves.
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

std::string
avtAreaBetweenCurvesQuery::CreateMessage(double area)
{
    char msg[1024];
    sprintf(msg, "The area between the curves is %g.", area);
    std::string m = msg;
    return m;
}


