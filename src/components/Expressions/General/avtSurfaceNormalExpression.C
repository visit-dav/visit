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
//                       avtSurfaceNormalExpression.C                        //
// ************************************************************************* //

#include <avtSurfaceNormalExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkVisItPolyDataNormals.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtSurfaceNormalExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

avtSurfaceNormalExpression::avtSurfaceNormalExpression()
{
    isPoint = true;
}


// ****************************************************************************
//  Method: avtSurfaceNormalExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

avtSurfaceNormalExpression::~avtSurfaceNormalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSurfaceNormalExpression::DeriveVariable
//
//  Purpose:
//      Assigns the zone ID to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     September 22, 2005
//
// ****************************************************************************

vtkDataArray *
avtSurfaceNormalExpression::DeriveVariable(vtkDataSet *in_ds)
{
    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "The Surface normal expression "
                   "can only be calculated on surfaces.  Use the External"
                   "Surface operator to generate the external surface of "
                   "this object.  You must also use the DeferExpression "
                   "operator to defer the evaluation of this expression until "
                   "after the external surface operator.  The external surface"
                   " and defer expression operators are available through "
                   "the plugin manager located under the Options menu");
    }

    vtkPolyData *pd = (vtkPolyData *) in_ds;

    vtkVisItPolyDataNormals *n = vtkVisItPolyDataNormals::New();
    n->SetSplitting(false);
    if (isPoint)
        n->SetNormalTypeToPoint();
    else
        n->SetNormalTypeToCell();
    n->SetInput(pd);
    vtkPolyData *out = n->GetOutput();
    out->Update();

    vtkDataArray *arr = NULL;
    if (isPoint)
        arr = out->GetPointData()->GetNormals();
    else
        arr = out->GetCellData()->GetNormals();
    
    if (arr == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "An internal error occurred where "
                   "the surface normals could not be calculated.  Please "
                   "contact a VisIt developer.");
    }
    arr->Register(NULL);
    n->Delete();

    return arr;
}


