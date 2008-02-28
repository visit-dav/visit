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
//                           avtConditionalExpression.C                          //
// ************************************************************************* //

#include <avtConditionalExpression.h>

#include <snprintf.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtConditionalExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtConditionalExpression::avtConditionalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtConditionalExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtConditionalExpression::~avtConditionalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtConditionalExpression::DeriveVariable
//
//  Purpose:
//      Selects between two arrays based on a condition.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     August 20, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Mar 13 10:02:16 PDT 2007
//    Improve error message.
//
//    Cyrus Harrison, Wed Feb 27 16:47:41 PST 2008
//    Test to make sure input data arrays have the same number of tuples and
//    issue an error message if this happens. 
//
// ****************************************************************************

vtkDataArray *
avtConditionalExpression::DeriveVariable(vtkDataSet *in_ds)
{
    // Our first operand is in the active variable.  We don't know if it's
    // point data or cell data, so check which one is non-NULL.
    vtkDataArray *cell_data1 = in_ds->GetCellData()->GetArray(varnames[0]);
    vtkDataArray *point_data1 = in_ds->GetPointData()->GetArray(varnames[0]);
    vtkDataArray *data1 = NULL, *data2 = NULL, *data3 = NULL;

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

    if (data1 == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Could not identify centering of first variable");
    }
    if (data1->GetNumberOfComponents() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Cannot interpret conditional with vector dimensionality");
    }

    // Get the second variable.
    if (centering == AVT_ZONECENT)
        data2 = in_ds->GetCellData()->GetArray(varnames[1]);
    else
        data2 = in_ds->GetPointData()->GetArray(varnames[1]);

    if (data2 == NULL)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, 
                   "the first two arguments to \"if\" (%s and %s) have "
                   " different centerings.", varnames[0], varnames[1]);
        EXCEPTION2(ExpressionException, outputVariableName, msg);
    }

    // Get the third variable.
    if (centering == AVT_ZONECENT)
        data3 = in_ds->GetCellData()->GetArray(varnames[2]);
    else
        data3 = in_ds->GetPointData()->GetArray(varnames[2]);
    
    if (data3 == NULL)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, 
                   "the first and third arguments to \"if\" (%s and %s) have "
                   " different centerings.", varnames[0], varnames[2]);
        EXCEPTION2(ExpressionException, outputVariableName, msg);
    }

    // sanity check: make sure the input arrays have the same # of tuples
    int d1_nvals = data1->GetNumberOfTuples();
    int d2_nvals = data2->GetNumberOfTuples();
    int d3_nvals = data2->GetNumberOfTuples();
    
    if ( d1_nvals != d2_nvals || d2_nvals != d3_nvals )
    {
        std::string msg = " the input datasets for the \"if\" expression "
                          " do not have the same number of tuples!";
        EXCEPTION2(ExpressionException, outputVariableName, msg.c_str());
    }
    
    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int nvals  = data2->GetNumberOfTuples();
    vtkDataArray *dv = data2->NewInstance();
    dv->SetNumberOfComponents(data2->GetNumberOfComponents());
    dv->SetNumberOfTuples(nvals);

    if (data1->GetDataType() == VTK_FLOAT)
    {
        for (int i = 0 ; i < nvals ; i++)
        {
            float val = data1->GetTuple1(i);
            if (val != 0.)
            {
                dv->SetTuple(i, data2->GetTuple(i));
            }
            else
            {
                dv->SetTuple(i, data3->GetTuple(i));
            }
        }
    }
    else if (data1->GetDataType() == VTK_UNSIGNED_CHAR)
    {
        vtkUnsignedCharArray *uca = (vtkUnsignedCharArray *) data1;
        for (int i = 0 ; i < nvals ; i++)
        {
            unsigned char val = uca->GetValue(i);
            if (val != '\0')
            {
                dv->SetTuple(i, data2->GetTuple(i));
            }
            else
            {
                dv->SetTuple(i, data3->GetTuple(i));
            }
        }
    }
      
    return dv;
}


// ****************************************************************************
//  Method:  avtConditionalExpression::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
bool
avtConditionalExpression::FilterUnderstandsTransformedRectMesh()
{
    // simple conditionals operate only on variables and
    // shouldn't be directly affected by coordinates
    return true;
}
