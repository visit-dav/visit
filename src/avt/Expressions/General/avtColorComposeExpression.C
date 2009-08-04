/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                         avtColorComposeExpression.C                           //
// ************************************************************************* //

#include <avtColorComposeExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtColorComposeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Apr 23 17:09:37 PST 2007
//
// ****************************************************************************

avtColorComposeExpression::avtColorComposeExpression(int nc)
{
    ncomp = nc;
}


// ****************************************************************************
//  Method: avtColorComposeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Apr 23 17:09:37 PST 2007
//
// ****************************************************************************

avtColorComposeExpression::~avtColorComposeExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtColorComposeExpression::DeriveVariable
//
//  Purpose:
//      Creates a vector variable from components.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Notes: Mostly taken from avtVectorComposeExpression.C
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Apr 23 17:09:37 PST 2007
//
//  Modifications:
//
// ****************************************************************************

inline unsigned char 
ClampColor(const double c)
{
    int ic = (int)c;
    if(ic < 0) ic = 0;
    if(ic > 255) ic = 255;
    return (unsigned char)ic;
}

vtkDataArray *
avtColorComposeExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int numinputs = varnames.size();

    //
    // Our first operand is in the active variable. We don't know if it's
    // point data or cell data, so check which one is non-NULL.
    //
    vtkDataArray *cell_data1 = in_ds->GetCellData()->GetArray(varnames[0]);
    vtkDataArray *point_data1 = in_ds->GetPointData()->GetArray(varnames[0]);
    vtkDataArray *data1 = NULL, *data2 = NULL, *data3 = NULL, *data4 = NULL;

    avtCentering centering;
    if (cell_data1 != NULL)
    {
        data1 = cell_data1;
        centering = AVT_ZONECENT;
    }
    else
    {
        data1 = point_data1;
        centering = AVT_NODECENT;
    }
    if (data1 != NULL && data1->GetNumberOfComponents() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The first variable is not a scalar.");
    }

    // Get the second variable.
    if (centering == AVT_ZONECENT)
        data2 = in_ds->GetCellData()->GetArray(varnames[1]);
    else
        data2 = in_ds->GetPointData()->GetArray(varnames[1]);

    if (data2 == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The first two variables have different centering.");
    }
    if (data2->GetNumberOfComponents() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The second variable is not a scalar.");
    }

    if (numinputs >= 3)
    {
        // Get the third variable.
        if (centering == AVT_ZONECENT)
            data3 = in_ds->GetCellData()->GetArray(varnames[2]);
        else
            data3 = in_ds->GetPointData()->GetArray(varnames[2]);
    
        if (data3 == NULL)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                   "The first and third variables have different centering.");
        }
        if (data3->GetNumberOfComponents() != 1)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                       "The third variable is not a scalar.");
        }
    }

    if (numinputs == 4)
    {
        // Get the fourth variable.
        if (centering == AVT_ZONECENT)
            data4 = in_ds->GetCellData()->GetArray(varnames[3]);
        else
            data4 = in_ds->GetPointData()->GetArray(varnames[3]);
    
        if (data4 == NULL)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                   "The first and fourth variables have different centering.");
        }
        if (data4->GetNumberOfComponents() != 1)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                       "The fourth variable is not a scalar.");
        }
    }

    vtkIdType nvals = data1->GetNumberOfTuples();
    vtkDataArray *dv = data1->NewInstance();
    dv->SetNumberOfComponents(4);
    dv->SetNumberOfTuples(data1->GetNumberOfTuples());
    if(numinputs == 1)
    {
        for(vtkIdType id = 0; id < nvals; ++id)
        {
            double val1 = ClampColor(data1->GetTuple1(id));
            dv->SetTuple4(id, val1, 0., 0., 255.);
        }
    }
    else if(numinputs == 2)
    {
        for(vtkIdType id = 0; id < nvals; ++id)
        {
            double val1 = ClampColor(data1->GetTuple1(id));
            double val2 = ClampColor(data2->GetTuple1(id));
            dv->SetTuple4(id, val1, val2, 0., 255.);
        }
    }
    else if(numinputs == 3)
    {
        for(vtkIdType id = 0; id < nvals; ++id)
        {
            double val1 = ClampColor(data1->GetTuple1(id));
            double val2 = ClampColor(data2->GetTuple1(id));
            double val3 = ClampColor(data3->GetTuple1(id));
            dv->SetTuple4(id, val1, val2, val3, 255.);
        }
    }
    else if(numinputs == 4)
    {
        for(vtkIdType id = 0; id < nvals; ++id)
        {
            double val1 = ClampColor(data1->GetTuple1(id));
            double val2 = ClampColor(data2->GetTuple1(id));
            double val3 = ClampColor(data3->GetTuple1(id));
            double val4 = ClampColor(data4->GetTuple1(id));
            dv->SetTuple4(id, val1, val2, val3, val4);
        }
    }

    return dv;
}


