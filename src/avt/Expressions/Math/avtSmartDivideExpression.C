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
    avtCentering var1_centering = DetermineCentering(data1, in_ds, varnames[0]);
    avtCentering var2_centering = DetermineCentering(data2, in_ds, varnames[1]);

    // The third variable should just be a constant double (e.g. divide(one, two, 1.0))
    // So we ensure here that it is a singleton and set its value as the
    // divide_by_zero value.

    vtkDataArray *data3 = NULL;
    DetermineCentering(data3, in_ds, varnames[2]);
    if (data3->GetNumberOfComponents() != 1 || data3->GetNumberOfTuples() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                    "The 'value_if_zero' argument in the 'divide' function must be a constant double.");
    }
    else
    {
        value_if_zero = data3->GetTuple1(0);
    }
    

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
    DoOperation(output, data1, data2, nComps, nVals);
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
avtSmartDivideExpression::DoOperation(vtkDataArray* output, vtkDataArray *in1,
        vtkDataArray *in2, int nComps, int nVals)
{
    // Get the number of components
    bool var1IsSingleton = (in1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (in2->GetNumberOfTuples() == 1);
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();

    if ((in1ncomps == 1) && (in2ncomps == 1))
    {
        for (int i = 0 ; i < nVals ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = in1->GetTuple1(tup1);
            double val2 = in2->GetTuple1(tup2);
            output->SetTuple1(i, CheckZero(val1,val2));
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        for (int i = 0 ; i < nVals ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val2 = in2->GetTuple1(tup2);
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                double val1 = in1->GetComponent(tup1, j);
                output->SetComponent(i, j, CheckZero(val1, val2));
            }
        }
    }
    else if (in1ncomps == 1 && in2ncomps > 1)
    {
        for (int i = 0 ; i < nVals ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = in1->GetTuple1(tup1);
            for (int j = 0 ; j < in2ncomps ; j++)
            {
                double val2 = in2->GetComponent(tup2, j);
                output->SetComponent(i, j, CheckZero(val1, val2));
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Division of vectors in undefined.");
    }
    
    return;
}

double
avtSmartDivideExpression::CheckZero(double top, double bottom)
{
    if (fabs(bottom) < this->tolerance)
    {
        return this->value_if_zero;
    }
    else {
        return top / bottom;
    }
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





