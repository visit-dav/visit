/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <avtDistanceToBestFitLineFilter.h>
#include <vtkDataArray.h>
#include <avtDataTree.h>
#include <avtParallel.h>
#include <math.h>

#define N_SUM   0
#define X_SUM   1
#define Y_SUM   2
#define XY_SUM  3
#define X2_SUM  4

#define N_CALC_VALUES 5

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::avtDistanceToBestFitLineFilter
//
// Purpose: 
//   Constructor for the avtDistanceToBestFitLineFilter class.
//
// Arguments:
//   v : If true then difference vertically - otherwise calculate perpendicular
//       distance.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:15:57 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtDistanceToBestFitLineFilter::avtDistanceToBestFitLineFilter(bool v) :
    avtBinaryMathFilter()
{
    verticalDifference = v;
    pass = 1;
    sums[N_SUM] = 0.;
    sums[X_SUM] = 0.;
    sums[Y_SUM] = 0.;
    sums[XY_SUM] = 0.;
    sums[X2_SUM] = 0.;
}

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::~avtDistanceToBestFitLineFilter
//
// Purpose: 
//   Destructor for the avtDistanceToBestFitLineFilter class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:16:49 PST 2005
//
// Modifications:
//   
// ****************************************************************************

avtDistanceToBestFitLineFilter::~avtDistanceToBestFitLineFilter()
{
}

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::PreExecute
//
// Purpose: 
//   Sets all of the sums to zero.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:17:07 PST 2005
//
// Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//   
// ****************************************************************************

void
avtDistanceToBestFitLineFilter::PreExecute(void)
{
    avtBinaryMathFilter::PreExecute();

    sums[N_SUM] = 0.;
    sums[X_SUM] = 0.;
    sums[Y_SUM] = 0.;
    sums[XY_SUM] = 0.;
    sums[X2_SUM] = 0.;
}

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::Execute
//
// Purpose: 
//   Performs a multi-pass execute. In pass 1 we calculate sums for the best
//   fit line. In pass 2, we calculate the difference between the variables
//   and the best fit line.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:17:24 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtDistanceToBestFitLineFilter::Execute(void)
{
    //
    // Sum the values required to do the best fit line.
    //
    pass = 1;
    avtDataTree_p tree    = GetInputDataTree();
    totalNodes = 2 * tree->GetNumberOfLeaves();
    avtDataTree_p newTree = avtDataTreeStreamer::Execute(tree);
    newTree = 0;

    // Sum the array values over all processors, making sure each processor
    // gets the results
    double d[N_CALC_VALUES];
    SumDoubleArrayAcrossAllProcessors(sums, d, N_CALC_VALUES);
    for(int i = 0; i < N_CALC_VALUES; ++i)
        sums[i] = d[i];

    //
    // Make it perform the expression.
    //
    pass = 2;
    avtDataTree_p newTree2 = avtDataTreeStreamer::Execute(tree);

    SetOutputDataTree(newTree2);
}

// ****************************************************************************
// Method: avtDistanceToBestFitLineFilter::DoOperation
//
// Purpose: 
//   Does the work of calculating the expression.
//
// Arguments:
//   in1 : The data array containing the X coordinate.
//   in2 : The data array containing the Y coordinate.
//   out : The resulting data.
//   ncomps : The number of components.
//   ntuples : The number of tuples.
//
// Note:       Note that we only populate the out data array in pass 2. The 
//             results of pass 1 get thrown away.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 18 16:18:09 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtDistanceToBestFitLineFilter::DoOperation(vtkDataArray *in1, 
    vtkDataArray *in2, vtkDataArray *out, int ncomps, int ntuples)
{
    if(pass == 1)
    {
        // Sum up the values required to calculate the best fit line.
        sums[N_SUM] += double(ntuples);
        for(vtkIdType i = 0; i < ntuples; ++i)
        {
            float x = in1->GetTuple1(i);
            float y = in2->GetTuple1(i);

            sums[X_SUM] += double(x);
            sums[Y_SUM] += double(y);
            sums[XY_SUM] += double(x * y);
            sums[X2_SUM] += double(x * x);
        }
    }
    else if(pass == 2)
    {
        double dY = (sums[N_SUM] * sums[XY_SUM] - sums[X_SUM] * sums[Y_SUM]);
        double dX = (sums[N_SUM] * sums[X2_SUM] - sums[X_SUM] * sums[X_SUM]);
        double m, b;

        if(dX == 0.)
        {
            double xLine = sums[X_SUM] / sums[N_SUM];
            for(vtkIdType i = 0; i < ntuples; ++i)
            {
                float x = in1->GetTuple1(i);
                out->SetTuple1(i, x - xLine);
            }
        }
        else
        {
            m =  dY / dX;
            b = (sums[Y_SUM] - m * sums[X_SUM]) / sums[N_SUM];

            if(verticalDifference)
            {
                for(vtkIdType i = 0; i < ntuples; ++i)
                {
                    float x = in1->GetTuple1(i);
                    float y = in2->GetTuple1(i);
                    float yLine = m * x + b;
                    out->SetTuple1(i, y - yLine);
                }
            }
            else // perpendicular distance.
            {
                for(vtkIdType i = 0; i < ntuples; ++i)
                {
                    float x0 = in1->GetTuple1(i);
                    float y0 = in2->GetTuple1(i);
                    
                    // We know line eq. Y = MX + B
                    // Reorganized: ax + by + c = 0 where a=M, b=-1, c=B
                    //
                    // Dist from point to line:
                    //   |a*x0 + b*y0 + c| / sqrt(a^2 + b^2)
                    //
                    // Subst a,b,c
                    //   |M*x0 -1*y0 + B| / sqrt(M^2 + 1)

                    float d = fabs(m * x0 - y0 + b) / sqrt(m*m + 1);
                    out->SetTuple1(i, d);
                }
            }
        }
    }
}
