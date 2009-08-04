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

#include <Utility.h>
#include <stdlib.h>

// ****************************************************************************
//  Function: PointSorter
//
//  Purpose:
//      Used to sort points using the qsort routine.
//
//  Programmer: Hank Childs
//  Creation:   October 3, 2003
//
// ****************************************************************************

static int
PointSorter(const void *p1, const void *p2)
{
    const float *f1 = (const float *) p1;
    const float *f2 = (const float *) p2;

    if (*f1 > *f2)
        return 1;
    if (*f1 < *f2)
        return -1;

    return 0;
}

// ****************************************************************************
//  Method: PutOnSameXIntervals
//
//  Purpose:
//      Curves are defined by a series of line segments.  The endpoints of
//      these line segments may not correspond on the two input curves.
//      The output of this function will be a new series of x-intervals that
//      will allow easier comparison between two curves.
//
//  Programmer:   Hank Childs
//  Creation:     October 4, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Oct 14 17:19:01 PDT 2004
//    This method assumes that there are no duplicate x-values in the
//    passed array -- so call AverageYValsForDuplicateX to ensure that is
//    the case.
//
// ****************************************************************************

void 
PutOnSameXIntervals(int on1, const float *ox1, const float *oy1, int on2, 
        const float *ox2, const float *oy2, floatVector &usedX, 
        floatVector &newCurve1Vals, floatVector &newCurve2Vals)
{
    int  i;
    floatVector x1, y1, x2, y2;

    AverageYValsForDuplicateX(on1, ox1, oy1, x1, y1);
    AverageYValsForDuplicateX(on2, ox2, oy2, x2, y2);
   
    int n1 = x1.size();
    int n2 = x2.size();
    int  total_n_pts = n1 + n2;

    //
    // We want to put the line segments along the same x-intervals.  So we
    // want to determine what those intervals are.  Start by identifying all
    // of the unique x-points.
    //
    float *all_xs = new float[total_n_pts];
    int index = 0;
    for (i = 0 ; i < n1 ; i++)
        all_xs[index++] = x1[i];
    for (i = 0 ; i < n2 ; i++)
        all_xs[index++] = x2[i];
    qsort(all_xs, total_n_pts, sizeof(float), PointSorter);

    //
    // Repeats will through the algorithm off, so sort those out now.
    //
    floatVector unique_x;
    for (i = 0 ; i < total_n_pts ; i++)
    {
         bool uniquePoint = true;
         if ((i > 0) && (all_xs[i] == all_xs[i-1]))
             uniquePoint = false;
         if (uniquePoint)
             unique_x.push_back(all_xs[i]);
    }
    delete [] all_xs;
    total_n_pts = unique_x.size();

    int nextIndForCurve1 = 0;
    int nextIndForCurve2 = 0;
    for (i = 0 ; i < total_n_pts ; i++)
    {
        // We don't want to consider points that are not valid for both curves.
        if ((unique_x[i] < x1[0]) || (unique_x[i] > x1[n1-1]) ||
            (unique_x[i] < x2[0]) || (unique_x[i] > x2[n2-1]))
        {
            if (unique_x[i] == x1[nextIndForCurve1])
                nextIndForCurve1++;
            if (unique_x[i] == x2[nextIndForCurve2])
                nextIndForCurve2++;
            continue;
        }

        if (unique_x[i] == x1[nextIndForCurve1])
        {
            // The point to consider is from curve 1.  Simply push back the
            // Y-value and indicate that we are now focused on the next point.
            newCurve1Vals.push_back(y1[nextIndForCurve1]);
            nextIndForCurve1++;
        }
        else
        {
            // We haven't seen x1[nextIndForCurve] yet, so we know
            // that unique_x[i] must be less than it.  In addition, we know
            // that x1[nextIndForCurve-1] must be valid, since otherwise
            // we would have skipped unique_x[i] as "out of range".
            float x_begin = x1[nextIndForCurve1-1];
            float x_end  = x1[nextIndForCurve1];
            float percent = (unique_x[i] - x_begin) / (x_end - x_begin);
            float slope = y1[nextIndForCurve1] - y1[nextIndForCurve1-1];
            float y = percent * slope + y1[nextIndForCurve1-1];
            newCurve1Vals.push_back(y);
        }

        if (unique_x[i] == x2[nextIndForCurve2])
        {
            // The point to consider is from curve 2.  Simply push back the
            // Y-value and indicate that we are now focused on the next point.
            newCurve2Vals.push_back(y2[nextIndForCurve2]);
            nextIndForCurve2++;
        }
        else
        {
            // We haven't seen x2[nextIndForCurve] yet, so we know
            // that unique_x[i] must be less than it.  In addition, we know
            // that x2[nextIndForCurve-1] must be valid, since otherwise
            // we would have skipped unique_x[i] as "out of range".
            float x_begin = x2[nextIndForCurve2-1];
            float x_end  = x2[nextIndForCurve2];
            float percent = (unique_x[i] - x_begin) / (x_end - x_begin);
            float slope = y2[nextIndForCurve2] - y2[nextIndForCurve2-1];
            float y = percent * slope + y2[nextIndForCurve2-1];
            newCurve2Vals.push_back(y);
        }

        usedX.push_back(unique_x[i]);
    }
}

// ****************************************************************************
//  Method: AverageYValsForDuplicateX
//
//  Purpose:
//    If there are duplicate x-values, then average the y-values for all
//    duplicates to create a unique x-values list with appropriate y-values.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 14, 2004
//
//  Modifications:
//
// ****************************************************************************

void
AverageYValsForDuplicateX(int n, const float *x, const float *y, 
                          floatVector &X, floatVector &Y)
{
    int i, j, nDups = 1;
    float sum;
    for (i = 0; i < n ; i+= nDups) 
    {
        if (i < n-1)
        {
            if (x[i] != x[i+1])
            {
                X.push_back(x[i]);
                Y.push_back(y[i]);
                nDups = 1;
            }
            else 
            {
                sum = y[i];
                nDups = 1;
                for (j = i+1; j < n; j++)
                {
                    if (x[j] != x[i])
                        break;
                    sum += y[j];
                    nDups++;
                }
                X.push_back(x[i]);
                Y.push_back(sum/nDups);
            }
        }
        else if (i == (n-1) && (x[i] != x[i-1]))
        {
            X.push_back(x[i]);
            Y.push_back(y[i]);
            nDups = 1;
        }
    }
}

