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
//                              avtDataIdExpression.C                            //
// ************************************************************************* //

#include <avtDataIdExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <avtCallback.h>


// ****************************************************************************
//  Method: avtDataIdExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtDataIdExpression::avtDataIdExpression()
{
    doZoneIds = true;
    doGlobalNumbering = false;
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtDataIdExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtDataIdExpression::~avtDataIdExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDataIdExpression::PreExecute
//
//  Purpose:
//      Reset haveIssuedWarning, so we can issue it only once per execution.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

void
avtDataIdExpression::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtDataIdExpression::DeriveVariable
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
//  Creation:     September 23, 2002
//
// ****************************************************************************

vtkDataArray *
avtDataIdExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int nvals = 0;
    if (doZoneIds)
        nvals = in_ds->GetNumberOfCells();
    else
        nvals = in_ds->GetNumberOfPoints();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(nvals);

    vtkDataArray *arr = NULL;
    if (doZoneIds && doGlobalNumbering)
        arr = in_ds->GetCellData()->GetArray("avtGlobalZoneNumbers");
    else if (doZoneIds && !doGlobalNumbering)
        arr = in_ds->GetCellData()->GetArray("avtOriginalCellNumbers");
    else if (!doZoneIds && doGlobalNumbering)
        arr = in_ds->GetPointData()->GetArray("avtGlobalNodeNumbers");
    else if (!doZoneIds && !doGlobalNumbering)
        arr = in_ds->GetPointData()->GetArray("avtOriginalNodeNumbers");

    if (arr == NULL)
    {
        for (int i = 0 ; i < nvals ; i++)
            rv->SetTuple1(i, i);
        char standard_msg[1024] = "VisIt was not able to create the requested"
                " ids.  Please see a VisIt developer.";
        char globalmsg[1024] = "VisIt was not able to create global ids, most "
                "likely because ids of this type were not stored in the file.";
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning(
                               (doGlobalNumbering ? globalmsg : standard_msg));
            haveIssuedWarning = true;
        }

        return rv;
    }

    for (int i = 0 ; i < nvals ; i++)
    {
        if (arr->GetNumberOfComponents() == 2)
            // When there are two components, they are typically packed as 
            // <dom, id>.  We will want the second one.
            rv->SetTuple1(i, arr->GetComponent(i, 1));
        else
            rv->SetTuple1(i, arr->GetComponent(i, 0));
    }

    return rv;
}


// ****************************************************************************
//  Method: avtDataIdExpression::PerformRestriction
//
//  Purpose:
//      Tells the input create zone ids.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtPipelineSpecification_p
avtDataIdExpression::PerformRestriction(avtPipelineSpecification_p spec)
{
    if (doZoneIds && doGlobalNumbering)
        spec->GetDataSpecification()->TurnGlobalZoneNumbersOn();
    else if (doZoneIds && !doGlobalNumbering)
        spec->GetDataSpecification()->TurnZoneNumbersOn();
    else if (!doZoneIds && doGlobalNumbering)
        spec->GetDataSpecification()->TurnGlobalNodeNumbersOn();
    else if (!doZoneIds && !doGlobalNumbering)
        spec->GetDataSpecification()->TurnNodeNumbersOn();

    return spec;
}


