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
//                           avtDisplacementFilter.C                         //
// ************************************************************************* //

#include <avtDisplacementFilter.h>
#include <vtkVisItUtility.h>
#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointSet.h>
#include <vtkCell.h>
#include <vtkCellType.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>

#include <vtkMath.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtDisplacementFilter constructor
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 14:28:33 PST 2006
//
// ****************************************************************************

avtDisplacementFilter::avtDisplacementFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDisplacementFilter destructor
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 14:28:33 PST 2006
//
// ****************************************************************************

avtDisplacementFilter::~avtDisplacementFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDisplacementFilter::DeriveVariable
//
//  Purpose:
//      Determines the displacement at nodes given the current 
//      geometry and initial configuration.
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 14:28:33 PST 2006
//
// ****************************************************************************

vtkDataArray *
avtDisplacementFilter::DeriveVariable (vtkDataSet *in_ds)
{
    char msg[1024];
    double vals[3];
    double out2[3];
    double x0, y0, z0;               // Initial element geom.
    
    // Let's get the points from the input dataset.
    if (in_ds->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "The displacement expression "
                   "only operates on unstructured grids.");
    }
    vtkUnstructuredGrid *in_usg = vtkUnstructuredGrid::SafeDownCast(in_ds);
    int nPoints = in_usg->GetNumberOfPoints();
    vtkDataArray *coord_data = in_ds->GetPointData()->GetArray(varnames[1]);
    if (coord_data == NULL)
    {
        sprintf(msg, "The displacement expression "
            "could not extract the data array for: %s", varnames[1]);
        EXCEPTION2(ExpressionException, outputVariableName, msg);
    }
    vtkFloatArray *out = vtkFloatArray::New();
    out->SetNumberOfComponents(3);
    out->SetNumberOfTuples(nPoints);
    for (int i = 0; i < nPoints; i++)
    {   // Get initial element geometry points
        coord_data->GetTuple(i, vals);
        x0 = vals[0];
        y0 = vals[1];
        z0 = vals[2];
        // Get current element geometry points
        in_usg->GetPoint(i, vals);
        out2[0] = vals[0] - x0;
        out2[1] = vals[1] - y0;
        out2[2] = vals[2] - z0;
        out->SetTuple(i, out2);
    }
    return out;
}
// ****************************************************************************
//  Method: avtDisplacementFilter::GetVariableDimension
//
//  Purpose:
//      Determines what the variable dimension of the output is.
//      
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 14:28:33 PST 2006
//  
// ****************************************************************************
    
int
avtDisplacementFilter::GetVariableDimension(void)
{
    if (*(GetInput()) == NULL)
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    if (varnames.size() != 2)
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (! atts.ValidVariable(varnames[0]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int ncomp1 = atts.GetVariableDimension(varnames[0]);
    
    if (! atts.ValidVariable(varnames[1]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int ncomp2 = atts.GetVariableDimension(varnames[1]);
    
    return GetNumberOfComponentsInOutput(ncomp1, ncomp2);
}
