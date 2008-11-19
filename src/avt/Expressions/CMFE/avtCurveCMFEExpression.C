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
    vtkDataSet *curve1 = tree1->GetSingleLeaf();
    if (curve1 == NULL)
        EXCEPTION0(ImproperUseException);
    vtkDataSet *curve2 = tree2->GetSingleLeaf();
    if (curve2 == NULL)
        EXCEPTION0(ImproperUseException);

    //
    // Construct the first curve.
    //
    vtkDataArray *xc = ((vtkRectilinearGrid*)curve1)->GetXCoordinates();
    vtkDataArray *sc = curve1->GetPointData()->GetScalars();
    int n1 = xc->GetNumberOfTuples();
    string var1 = sc->GetName();
    float *n1x = new float[n1];
    float *n1y = new float[n1];
    for (int i = 0 ; i < n1 ; i++)
    {
         n1x[i] = xc->GetTuple1(i);
         n1y[i] = sc->GetTuple1(i);
    }

    //
    // Construct the second curve.
    //
    xc = ((vtkRectilinearGrid*)curve2)->GetXCoordinates();
    sc = curve2->GetPointData()->GetScalars();
    int n2 = xc->GetNumberOfTuples();
    string var2 = sc->GetName();
    float *n2x = new float[n2];
    float *n2y = new float[n2];
    for (int i = 0 ; i < n2 ; i++)
    {
         n2x[i] = xc->GetTuple1(i);
         n2y[i] = sc->GetTuple1(i);
    }


    floatVector newC1Vals;
    floatVector newC2Vals;
    floatVector X;
    PutOnSameXIntervals(n1, n1x, n1y, n2, n2x, n2y, X, newC1Vals, newC2Vals);
    int nx = X.size(); 

    vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(nx, VTK_FLOAT);
    xc = rg->GetXCoordinates();
    vtkFloatArray *c1 = vtkFloatArray::New();
    c1->SetNumberOfComponents(1);
    c1->SetNumberOfTuples(nx);
    c1->SetName(var1.c_str());
    vtkFloatArray *c2 = vtkFloatArray::New();
    c2->SetNumberOfComponents(1);
    c2->SetNumberOfTuples(nx);
    c2->SetName(outvar.c_str());
    //c2->SetName(var2.c_str());
    for (int i = 0; i < nx; ++i)
    {
        xc->SetTuple1(i, X[i]);
        c1->SetValue(i, newC1Vals[i]);
        c2->SetValue(i, newC2Vals[i]);
    }
     

    rg->GetPointData()->AddArray(c1); 
    rg->GetPointData()->AddArray(c2); 
    avtDataTree_p output = new avtDataTree(rg, 1);

    rg->Delete();
    c1->Delete();
    c2->Delete();
    delete [] n1x;
    delete [] n1y;
    delete [] n2x;
    delete [] n2y;
    return output;
}


