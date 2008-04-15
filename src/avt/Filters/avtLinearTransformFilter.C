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
//  File: avtLinearTransformFilter.C
// ************************************************************************* //

#include <avtLinearTransformFilter.h>

#include <avtExtents.h>

#include <BadVectorException.h>


// ****************************************************************************
//  Method: avtLinearTransformFilter constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

avtLinearTransformFilter::avtLinearTransformFilter()
{
    M = NULL;
    invM = NULL;
}


// ****************************************************************************
//  Method: avtLinearTransformFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

avtLinearTransformFilter::~avtLinearTransformFilter()
{
    if (M)
        M->Delete();
    M = NULL;
    if (invM)
        invM->Delete();
    invM = NULL;
}


// ****************************************************************************
//  Method:      avtLinearTransformFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtLinearTransformFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const LinearTransformAttributes*)a;
    if (M)
        M->Delete();
    M = NULL;
    if (invM)
        invM->Delete();
    invM = NULL;
}


// ****************************************************************************
//  Method:  avtLinearTransformFilter::SetupMatrix
//
//  Purpose:
//    Setup the vtk matrix from the transform attributs
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtLinearTransformFilter::SetupMatrix()
{
    if (M)
        return;

    M = vtkMatrix4x4::New();
    M->Identity();
    invM = vtkMatrix4x4::New();
    invM->Identity();

    M->Identity();
    (*M)[0][0] = atts.GetM00();
    (*M)[0][1] = atts.GetM01();
    (*M)[0][2] = atts.GetM02();

    (*M)[1][0] = atts.GetM10();
    (*M)[1][1] = atts.GetM11();
    (*M)[1][2] = atts.GetM12();

    (*M)[2][0] = atts.GetM20();
    (*M)[2][1] = atts.GetM21();
    (*M)[2][2] = atts.GetM22();

    if (atts.GetInvertLinearTransform())
    {
        invM->DeepCopy(M);
        vtkMatrix4x4::Invert(invM, M);
    }
    else
    {
        vtkMatrix4x4::Invert(M, invM);
    }
}


// ****************************************************************************
//  Method: avtLinearTransformFilter::PerformRestriciton
//
//  Purpose:
//    Turn on Zone numbers flag if needed, so that original cell array
//    will be propagated throught the pipeline.
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

avtContract_p
avtLinearTransformFilter::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = new avtContract(spec);
    if (rv->GetDataRequest()->MayRequireZones())
    {
        rv->GetDataRequest()->TurnZoneNumbersOn();
    }
    if (rv->GetDataRequest()->MayRequireNodes())
    {
        rv->GetDataRequest()->TurnNodeNumbersOn();
    }
    return rv;
}


// ****************************************************************************
//  Method: avtLinearTransformFilter::PostExecute
//
//  Purpose:
//      This is called to set the inverse transformation matrix in the output.  
//
//  Programmer: Jeremy Meredith
//  Creation:   April 15, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtLinearTransformFilter::PostExecute()
{
    GetOutput()->GetInfo().GetAttributes().SetInvTransform((*invM)[0]);
    GetOutput()->GetInfo().GetAttributes().SetTransform((*M)[0]);
}
