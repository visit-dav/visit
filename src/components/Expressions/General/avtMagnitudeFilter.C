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
//                             avtMagnitudeFilter.C                          //
// ************************************************************************* //

#include <avtMagnitudeFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtMagnitudeFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMagnitudeFilter::avtMagnitudeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtMagnitudeFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtMagnitudeFilter::~avtMagnitudeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtMagnitudeFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Matthew Haddox
//  Creation:     July 30, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Nov 18 15:11:11 PST 2002
//    Beefed up error code a little.
//
//    Kathleen Bonnell, Thu Jan  2 15:16:50 PST 2003 
//    Replace MakeObject() with NewInstance() to match new vtk api. 
//
//    Kathleen Bonnell, Wed Aug 18 16:55:49 PDT 2004 
//    Retrieve vectorValues array based on activeVariableName. (There may be
//    a Vectors array in both the CellData and the PointData, and the old way
//    would always retrieve the PointData vectors in this instance).  We are
//    assuming that the PoinData Vectors and CellData Vectors will have 
//    different names.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Clean up some wrapped lines.
//
// ****************************************************************************

vtkDataArray *
avtMagnitudeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    //
    // The base class will set the variable of interest to be the 
    // 'activeVariable'.  This is a by-product of how the base class sets its
    // input.  If that method should change (SetActiveVariable), this
    // technique for inferring the variable name may stop working.
    //
    const char *varname = activeVariable;

    vtkDataArray *vectorValues = in_ds->GetPointData()->GetArray(varname);
    
    if (vectorValues == NULL)
    {
        vectorValues = in_ds->GetCellData()->GetArray(varname);
    }
    if (vectorValues == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                  "Unable to locate variable for magnitude expression");
    }

    if (vectorValues->GetNumberOfComponents() != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Can only take magnitude of vectors.");
    }
    int ntuples = vectorValues->GetNumberOfTuples();

    vtkDataArray *results = vectorValues->NewInstance();
    results->SetNumberOfComponents(1);
    results->SetNumberOfTuples(ntuples);

#define COMPUTE_MAG(dtype) \
{ \
    dtype *x   = (dtype*)vectorValues->GetVoidPointer(0); \
    dtype *r   = (dtype*)results->GetVoidPointer(0); \
    for (int i = 0, idx = 0 ; i < ntuples ; i++, idx += 3) \
    { \
        r[i] = sqrt(x[idx+0]*x[idx+0]+x[idx+1]*x[idx+1]+x[idx+2]*x[idx+2]); \
    } \
} 

    if (vectorValues->GetDataType() == VTK_FLOAT)
    {    
        COMPUTE_MAG(float);
    }
    else  // assuming double
    {    
        COMPUTE_MAG(double);
    }

    return results;
}


