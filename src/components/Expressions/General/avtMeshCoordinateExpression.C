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
//                          avtMeshCoordinateExpression.C                       //
// ************************************************************************* //

#include <avtMeshCoordinateExpression.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>


// ****************************************************************************
//  Method: avtMeshCoordinateExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMeshCoordinateExpression::avtMeshCoordinateExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMeshCoordinateExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMeshCoordinateExpression::~avtMeshCoordinateExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMeshCoordinateExpression::DeriveVariable
//
//  Purpose:
//      Pulls out the coordinates a mesh.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     November 19, 2002
//
//  Modifications:
//      Sean Ahern, Thu Mar  6 01:49:31 America/Los_Angeles 2003
//      Merged this with the other coordinate filters.
//
//      Sean Ahern, Fri Mar  7 21:20:29 America/Los_Angeles 2003
//      Made this return a vector of coordinates, rather than just one of them.
//
//      Hank Childs, Fri Mar 31 08:49:06 PST 2006
//      Add special handling for rectilinear grids.
//
// ****************************************************************************

vtkDataArray *
avtMeshCoordinateExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int npts = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(3);
    rv->SetNumberOfTuples(npts);

    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rg = (vtkRectilinearGrid *) in_ds;
        int dims[3];
        rg->GetDimensions(dims);
        float *X = (float *) rg->GetXCoordinates()->GetVoidPointer(0);
        float *Y = (float *) rg->GetYCoordinates()->GetVoidPointer(0);
        float *Z = (float *) rg->GetZCoordinates()->GetVoidPointer(0);
        float *ptr = rv->GetPointer(0);
        for (int k = 0 ; k < dims[2] ; k++)
            for (int j = 0 ; j < dims[1] ; j++)
                for (int i = 0 ; i < dims[0] ; i++)
                {
                    *ptr++ = X[i];
                    *ptr++ = Y[j];
                    *ptr++ = Z[k];
                }
    }
    else
    {
        for (int i = 0 ; i < npts ; i++)
        {
            double pt[3];
            in_ds->GetPoint(i, pt);
            rv->SetTuple3(i, pt[0], pt[1], pt[2]);
        }
    }

    return rv;
}


