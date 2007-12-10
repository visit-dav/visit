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
//                          avtCurvatureExpression.C                         //
// ************************************************************************* //

#include <avtCurvatureExpression.h>

#include <vtkCellData.h>
#include <vtkCurvatures.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtCurvatureExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtCurvatureExpression::avtCurvatureExpression()
{
    doGauss = false;
}


// ****************************************************************************
//  Method: avtCurvatureExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtCurvatureExpression::~avtCurvatureExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurvatureExpression::DeriveVariable
//
//  Purpose:
//      Calculates the localized compactness at every point.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     May 11, 2006
//
// ****************************************************************************

vtkDataArray *
avtCurvatureExpression::DeriveVariable(vtkDataSet *in_ds)
{
    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "The curvature expression "
                   "can only be calculated on surfaces.  VisIt tries to "
                   "evaluate expressions as soon as they are read from a "
                   "database.  For curvature, the expression is typically "
                   "desired after some operators have been applied.  You "
                   "can defer the evaluation of the curvature expression "
                   "using the DeferExpression operator."
                   " The defer expression operator is available through "
                   "the plugin manager located under the Options menu");
    }

    vtkPolyData *pd = (vtkPolyData *) in_ds;

    vtkCurvatures *curvatures = vtkCurvatures::New();
    curvatures->SetCurvatureType((doGauss ? VTK_CURVATURE_GAUSS 
                                          : VTK_CURVATURE_MEAN));
    curvatures->SetInput(pd);
    curvatures->Update();

    vtkPolyData *out = curvatures->GetOutput();
    vtkDataArray *curvature = out->GetPointData()->GetArray(
                               doGauss ? "Gauss_Curvature" : "Mean_Curvature");
    bool shouldDelete = false;

    vtkFloatArray *flt_curvature = vtkFloatArray::New();
    int npts = pd->GetNumberOfPoints();
    flt_curvature->SetNumberOfTuples(npts);
    if (curvature == NULL)
    {
        for (int i = 0 ; i < npts ; i++)
            flt_curvature->SetTuple1(i, 0.);
    }
    else
    {
        for (int i = 0 ; i < npts ; i++)
            flt_curvature->SetTuple1(i, curvature->GetTuple1(i));
    }

    curvatures->Delete();

    // Calling function will clean up memory.
    return flt_curvature;
}


