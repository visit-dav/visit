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

// ************************************************************************* //
//                          avtCurveCMFEExpression.C                         //
// ************************************************************************* //

#include <avtCurveCMFEExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>

#include <avtSILRestrictionTraverser.h>

#include <DebugStream.h>
#include <InvalidMergeException.h>
#include <Utility.h>
#include <snprintf.h>


// ****************************************************************************
//  Method: avtCurveCMFEExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 18, 2008 
//
// ****************************************************************************

avtCurveCMFEExpression::avtCurveCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurveCMFEExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 18, 2008 
//
// ****************************************************************************

avtCurveCMFEExpression::~avtCurveCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurveCMFEExpression::PerformCMFE
//
//  Purpose:
//      Performs a cross-mesh field evaluation based on placing curves
//      in the same domain.
//
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 18, 2008 
// 
// ****************************************************************************

avtDataTree_p
avtCurveCMFEExpression::PerformCMFE(avtDataTree_p in1, avtDataTree_p in2,
                                   const std::string &invar,
                                   const std::string &outvar)
{
    avtDataTree_p outtree = ExecuteTree(in1, in2, invar, outvar);
    return outtree;
}


// ****************************************************************************
//  Method: avtCurveCMFEExpression::ExecuteTree
// 
//  Purpose:
//      Executes a data tree for the CurveCMFE expression.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 18, 2008
//
//  Modifications:
//
// ****************************************************************************

avtDataTree_p
avtCurveCMFEExpression::ExecuteTree(avtDataTree_p tree1, avtDataTree_p tree2,
                                   const std::string &invar,
                                   const std::string &outvar)
{
   if (tree1->GetNumberOfLeaves() == 0)
        return tree1;
    else if (tree1->GetNumberOfLeaves() != 1)
        EXCEPTION0(ImproperUseException);

    if (tree2->GetNumberOfLeaves() == 0)
        return tree1;
    else if (tree2->GetNumberOfLeaves() != 1)
        EXCEPTION0(ImproperUseException);

    //
    // We know that there is only one leaf node.  It is the curve.
    //
    vtkRectilinearGrid *curve1 = 
        vtkRectilinearGrid::SafeDownCast(tree1->GetSingleLeaf());
    if (curve1 == NULL)
        EXCEPTION0(ImproperUseException);

    vtkRectilinearGrid *curve2 = 
        vtkRectilinearGrid::SafeDownCast(tree2->GetSingleLeaf());
    if (curve2 == NULL)
        EXCEPTION0(ImproperUseException);

    vtkRectilinearGrid *rg = 
        MergeCurvesToSameXIntervals(curve1, curve2, outvar);
    avtDataTree_p output = new avtDataTree(rg, 1);
    rg->Delete();
    return output;
}


// ****************************************************************************
//  Method: avtCurveCMFEExpression::HasDupX
// 
//  Purpose:
//    Determines if the passed rectilinear grid has duplicated x coordinates.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 24, 2009 
//
//  Modifications:
//
// ****************************************************************************

bool 
avtCurveCMFEExpression::HasDupX(vtkRectilinearGrid *rgrid)
{
    vtkDataArray *x = rgrid->GetXCoordinates();
    bool rv = false;
    for (int i = 1; i < x->GetNumberOfTuples() && !rv; ++i)
    {
        rv = (x->GetTuple1(i-1) == x->GetTuple1(i));
    }
    return rv;
}

// ***************************************************************************
//  The following methods are based on the CoordUtils (/src/common/utility)
//  methods: AverageYValsForDuplicateX and PutOnSameXIntervals.  
//
//  These new methods take into account multiple variables (yvalues) per rgrid
//   
// ***************************************************************************


// ****************************************************************************
//  Method: avtCurveCMFEExpression::RemoveDuplicateX
// 
//  Purpose:
//    Finds duplicate x coordinates in the passed rectilinear grid, removes
//    them and averages the variable values for the duplicate x.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 24, 2009 
//
//  Modifications:
//
// ****************************************************************************

vtkRectilinearGrid *
avtCurveCMFEExpression::RemoveDuplicateX(vtkRectilinearGrid *rgrid)
{
    size_t i, j, k, nDups = 1;
    size_t ny = rgrid->GetPointData()->GetNumberOfArrays();
    double *sum = new double[ny];
    vtkDataArray **y = new vtkDataArray*[ny];
    vtkDataArray **Y = new vtkDataArray*[ny];

    vtkDataArray *x = rgrid->GetXCoordinates();
    for (i = 0; i < ny; i++)
    {
        y[i] = rgrid->GetPointData()->GetArray(i);

        Y[i] = y[i]->NewInstance();
        Y[i]->SetNumberOfComponents(1);
        Y[i]->Allocate(y[i]->GetNumberOfTuples());
        Y[i]->SetName(y[i]->GetName());
    }

    vtkDataArray *X = x->NewInstance();
    X->SetNumberOfComponents(1);
    X->Allocate(x->GetNumberOfTuples());

    size_t n = x->GetNumberOfTuples();
    for (i = 0; i < n; i+= nDups)
    {
        if (i < n-1)
        {
            if (x->GetTuple1(i) != x->GetTuple1(i+1))
            {
                X->InsertNextTuple1(x->GetTuple1(i));
                for (j = 0; j < ny; ++j)
                    Y[j]->InsertNextTuple1(y[j]->GetTuple1(i));
                nDups = 1;
            }
            else 
            {
                 X->InsertNextTuple1(x->GetTuple1(i));

                 for (j = 0; j < ny; j++)
                 {
                     sum[j] = y[j]->GetTuple1(i);
                     nDups = 1;
                     for (k = i+1; k < n; k++)
                     {
                         if (x->GetTuple1(k) != x->GetTuple1(i))
                             break;
                         sum[j] += y[j]->GetTuple1(k);
                         nDups++;
                     }
                     Y[j]->InsertNextTuple1(sum[j]/nDups);
                 }
            }
        }
        else if (i == (n-1) && (x->GetTuple1(i) != x->GetTuple1(i-1)))
        {
            X->InsertNextTuple1(x->GetTuple1(i));
            for (j = 0; j < ny; j++)
            {
                Y[j]->InsertNextTuple1(y[j]->GetTuple1(i));
            }
            nDups = 1;
        }
    }

    
    X->Squeeze();
    vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(
        X->GetNumberOfTuples(), X->GetDataType());
    rg->SetXCoordinates(X);

    for (i = 0; i < ny; ++i)
    {
        Y[i]->Squeeze();
        rg->GetPointData()->AddArray(Y[i]);
        Y[i]->Delete();
    }
    

    delete [] Y;
    delete [] sum;
    return rg;
}


// ****************************************************************************
//  Method: DoubleSorter
// 
//  Purpose:
//    Used to sort values using the qsort routine.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 24, 2009 
//
//  Modifications:
//
// ****************************************************************************

static int
DoubleSorter(const void *p1, const void *p2)
{
    const double *d1 = (const double *) p1;
    const double *d2 = (const double *) p2;

    if (*d1 > *d2)
        return 1;
    if (*d1 < *d2)
        return -1;

    return 0;
}

// ****************************************************************************
//  Method: MergeCurvesToSameXIntervals
// 
//  Purpose:
//    Creates a new curve composed of the intersected domains of the
//    two passed curves.
//
//  Notes:
//    Each curve first removes duplicated x values, averaging variables where
//    duplicate x coords are encountered.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 24, 2009 
//
//  Modifications:
//
// ****************************************************************************

vtkRectilinearGrid *
avtCurveCMFEExpression::MergeCurvesToSameXIntervals(vtkRectilinearGrid *in_r1, 
                                     vtkRectilinearGrid *in_r2,
                                     const std::string &outVar)
{
    vtkRectilinearGrid *r1 = in_r1;
    vtkRectilinearGrid *r2 = in_r2;
    bool deleteR1 = false;
    bool deleteR2 = false;
    if (HasDupX(in_r1))
    {
        r1 = RemoveDuplicateX(in_r1);
        deleteR1 = true;
    }
    if (HasDupX(in_r2))
    {
        r2 = RemoveDuplicateX(in_r2);
        deleteR2 = true;
    }

    vtkDataArray *x1 = r1->GetXCoordinates();
    vtkDataArray *x2 = r2->GetXCoordinates();
  
    int n1 = x1->GetNumberOfTuples();
    int n2 = x2->GetNumberOfTuples();
    int total_n_pts = n1 + n2;

    // store the new x values in the largest data type
    double *all_xs = new double[total_n_pts];
    int index = 0;
    for (int i = 0 ; i < n1 ; ++i)
        all_xs[index++] = x1->GetTuple1(i);
    for (int i = 0 ; i < n2 ; ++i)
        all_xs[index++] = x2->GetTuple1(i);
    qsort(all_xs, total_n_pts, sizeof(double), DoubleSorter);

    //
    // Repeats will throw the algorithm off, so sort those out now.
    //
    doubleVector unique_x;
    for (int i = 0 ; i < total_n_pts ; ++i)
    {
         if (!((i > 0) && (all_xs[i] == all_xs[i-1])))
             unique_x.push_back(all_xs[i]);
    }
    delete [] all_xs;
    total_n_pts = unique_x.size();

    // Grab current arrays and set up new arrays for curve1
    int ny1 = r1->GetPointData()->GetNumberOfArrays();
    vtkDataArray **y1     = new vtkDataArray *[ny1];
    vtkDataArray **new_y1 = new vtkDataArray *[ny1];
    for (int j = 0; j < ny1; ++j)
    {
        y1[j] = r1->GetPointData()->GetArray(j);
        new_y1[j] = y1[j]->NewInstance();
        new_y1[j]->SetNumberOfComponents(y1[j]->GetNumberOfComponents());
        new_y1[j]->Allocate(y1[j]->GetNumberOfTuples());
        new_y1[j]->SetName(y1[j]->GetName());
    }

    // Grab current arrays and set up new arrays for curve2
    int ny2 = r2->GetPointData()->GetNumberOfArrays();
    vtkDataArray **y2     = new vtkDataArray *[ny2];
    vtkDataArray **new_y2 = new vtkDataArray *[ny2];
    for (int j = 0; j < ny2; ++j)
    {
        y2[j] = r2->GetPointData()->GetArray(j);
        new_y2[j] = y2[j]->NewInstance();
        new_y2[j]->SetNumberOfComponents(y2[j]->GetNumberOfComponents());
        new_y2[j]->Allocate(y2[j]->GetNumberOfTuples());
        new_y2[j]->SetName(y2[j]->GetName());
    }
    new_y2[0]->SetName(outVar.c_str());

    int nextIndForCurve1 = 0;
    int nextIndForCurve2 = 0;
    vtkDataArray *usedX = x1->NewInstance();
    usedX->SetNumberOfComponents(1);
    usedX->Allocate(total_n_pts);
    for (int i = 0 ; i < total_n_pts; ++i)
    {
        // We don't want to consider points that are not valid for both curves.
        if ((unique_x[i] < x1->GetTuple1(0)) || 
            (unique_x[i] < x2->GetTuple1(0)) || 
            (unique_x[i] > x1->GetTuple1(n1-1)) ||
            (unique_x[i] > x2->GetTuple1(n2-1)))
        {
            if (unique_x[i] == x1->GetTuple1(nextIndForCurve1))
                nextIndForCurve1++;
            if (unique_x[i] == x2->GetTuple1(nextIndForCurve2))
                nextIndForCurve2++;
            continue;
        }

        if (unique_x[i] == x1->GetTuple1(nextIndForCurve1))
        {
            // The point to consider is from curve 1.  Simply push back the
            // Y-value and indicate that we are now focused on the next point.
            for (int j = 0; j < ny1; ++j)
                new_y1[j]->InsertNextTuple1(y1[j]->GetTuple1(nextIndForCurve1));
            nextIndForCurve1++;
        }
        else
        {
            // We haven't seen x1[nextIndForCurve] yet, so we know
            // that unique_x[i] must be less than it.  In addition, we know
            // that x1[nextIndForCurve-1] must be valid, since otherwise
            // we would have skipped unique_x[i] as "out of range".
            double x_begin = x1->GetTuple1(nextIndForCurve1-1);
            double x_end   = x1->GetTuple1(nextIndForCurve1);
            double percent = (unique_x[i] - x_begin) / (x_end - x_begin);
            for (int j = 0; j < ny1; ++j)
            {
                double slope = y1[j]->GetTuple1(nextIndForCurve1) - 
                               y1[j]->GetTuple1(nextIndForCurve1-1);
                double y = percent * slope + 
                           y1[j]->GetTuple1(nextIndForCurve1-1);
                new_y1[j]->InsertNextTuple1(y);
            }
        }

        if (unique_x[i] == x2->GetTuple1(nextIndForCurve2))
        {
            // The point to consider is from curve 2.  Simply push back the
            // Y-value and indicate that we are now focused on the next point.
            for (int j = 0; j < ny2; ++j)
               new_y2[j]->InsertNextTuple1(y2[j]->GetTuple1(nextIndForCurve2));
            nextIndForCurve2++;
        }
        else
        {
            // We haven't seen x2[nextIndForCurve] yet, so we know
            // that unique_x[i] must be less than it.  In addition, we know
            // that x2[nextIndForCurve-1] must be valid, since otherwise
            // we would have skipped unique_x[i] as "out of range".
            double x_begin = x2->GetTuple1(nextIndForCurve2-1);
            double x_end   = x2->GetTuple1(nextIndForCurve2);
            double percent = (unique_x[i] - x_begin) / (x_end - x_begin);
            for (int j = 0; j < ny2; ++j)
            {
                double slope = y2[j]->GetTuple1(nextIndForCurve2) - 
                               y2[j]->GetTuple1(nextIndForCurve2-1);
                double y = percent * slope + 
                           y2[j]->GetTuple1(nextIndForCurve2-1);
                new_y2[j]->InsertNextTuple1(y);
            }
        }

        usedX->InsertNextTuple1(unique_x[i]);
    }
    usedX->Squeeze();

    vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(
        usedX->GetNumberOfTuples(), usedX->GetDataType());

    rg->SetXCoordinates(usedX);
    usedX->Delete();

    for (int j = 0; j< ny1; ++j)
    {
        new_y1[j]->Squeeze();
        rg->GetPointData()->AddArray(new_y1[j]);
        new_y1[j]->Delete();
    }
    for (int j = 0; j< ny2; ++j)
    {
        new_y2[j]->Squeeze();
        rg->GetPointData()->AddArray(new_y2[j]);
        new_y2[j]->Delete();
    }

    // clean up
    delete [] new_y1;
    delete [] new_y2;
    delete [] y1;
    delete [] y2;
    if (deleteR1)
        r1->Delete();
    if (deleteR2)
        r2->Delete();

    return rg;
}
