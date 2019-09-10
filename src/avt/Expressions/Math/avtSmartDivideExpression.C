/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                          avtSmartDivideExpression.h                       //
// ************************************************************************* //

#include <avtSmartDivideExpression.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ExpressionException.h>

avtSmartDivideExpression::avtSmartDivideExpression()
{
    ;
}

avtSmartDivideExpression::~avtSmartDivideExpression()
{
    ;
}

vtkDataArray *
avtSmartDivideExpression::DeriveVariable(vtkDataSet* in_ds, int currentDomainsIndex)
{
    // Get the variables and their centerings
    vtkDataArray *data1 = NULL;
    vtkDataArray *data2 = NULL;
    vtkDataArray *data3 = NULL;
    avtCentering var1_centering = DetermineCentering(data1, in_ds, varnames[0]);
    avtCentering var2_centering = DetermineCentering(data2, in_ds, varnames[1]);
    avtCentering var3_centering = DetermineCentering(data3, in_ds, varnames[2]);

    // Determine the centering that should be used.
    // If they have different centering (i.e. one has zone centering), then
    // zone centering will be used. If they have the same centering, then
    // whatever that centering is will be used.
    if (var1_centering != var2_centering)
    {
        centering = AVT_ZONECENT;
        if (var1_centering == AVT_NODECENT)
        {
            data1 = Recenter(in_ds, data1, var1_centering, outputVariableName);
        }
        else
        {
            data2 = Recenter(in_ds, data2, var2_centering, outputVariableName);
        }
    }
    else
    {
        centering = var1_centering;
    }

    // Enforce the 3rd variable to take on the centering of the other two
    // because the third variable is just a value that the user wants, and
    // its centering does not bear any meaning.
    if (var3_centering != centering)
    {
        data3 = Recenter(in_ds, data3, var3_centering, outputVariableName);
    }
    
    // Setup the output variable
    int nComps1 = data1->GetNumberOfComponents();
    int nComps2 = data2->GetNumberOfComponents();
    int nComps = nComps1 >= nComps2 ? nComps1 : nComps2;
    int nVals = data1->GetNumberOfTuples();
    if (nVals == 1) // data1 is a singleton
        nVals = data2->GetNumberOfTuples();
    
    vtkDataArray *output = vtkDoubleArray::New();
    output->SetNumberOfComponents(nComps);
    output->SetNumberOfTuples(nVals);

    cur_mesh = in_ds;
    DoOperation(output, data1, data2, data3, nComps, nVals);
    cur_mesh = NULL;

    return output;
}

avtCentering
avtSmartDivideExpression::DetermineCentering(vtkDataArray *out, vtkDataSet *in_ds, const char *varname)
{
    out = in_ds->GetCellData()->GetArray(varname);
    if (out == NULL)
    {
        out = in_ds->GetPointData()->GetArray(varname);
        if (out == NULL)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                    "An internal error occurred when calculating an expression."
                    "  Please contact a VisIt developer.");
        }
        else
        {
            return AVT_NODECENT;
        }
    }
    else
    {
        return AVT_ZONECENT;
    }
}

void
avtSmartDivideExpression::DoOperation(vtkDataArray* output, vtkDataArray *top, vtkDataArray *bottom, vtkDataArray* value_if_zero, int nComps, int nVal)
{
    return;
}

int
avtSmartDivideExpression::GetVariableDimension()
{
    if (*(GetInput()) == NULL)
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    if (varnames.size() != 2)
        return avtMultipleInputExpressionFilter::GetVariableDimension();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (! atts.ValidVariable(varnames[0]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int nComp1 = atts.GetVariableDimension(varnames[0]);

    if (! atts.ValidVariable(varnames[1]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int nComp2 = atts.GetVariableDimension(varnames[1]);

    return nComp1 >= nComp2 ? nComp1 : nComp2;
}

bool
avtSmartDivideExpression::FilterUnderstandsTransformedRectMesh()
{
    return false;
}





