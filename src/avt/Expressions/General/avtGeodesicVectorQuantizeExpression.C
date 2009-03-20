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
//                   avtGeodesicVectorQuantizeExpression.C                   //
// ************************************************************************* //

#include <avtGeodesicVectorQuantizeExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedIntArray.h>

#include <ExprToken.h>
#include <avtExprNode.h>

#include <avtCallback.h>
#include <avtMetaData.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>

#include <GeometricHelpers.h>

// ****************************************************************************
//  Method: avtGeodesicVectorQuantizeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 18, 2009
//
//  Modifications:
//    Jeremy Meredith, Fri Mar 20 15:54:48 EDT 2009
//    Make the second argument optional.
//
// ****************************************************************************

avtGeodesicVectorQuantizeExpression::avtGeodesicVectorQuantizeExpression()
{
    nargs = 0;
}


// ****************************************************************************
//  Method: avtGeodesicVectorQuantizeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 18, 2009
//
// ****************************************************************************

avtGeodesicVectorQuantizeExpression::~avtGeodesicVectorQuantizeExpression()
{
}


// ****************************************************************************
//  Method: avtGeodesicVectorQuantizeExpression::DeriveVariable
//
//  Purpose:
//      Creates an array.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Jeremy Meredith
//  Creation:     March 18, 2009
//
//  Modifications:
//    Jeremy Meredith, Fri Mar 20 15:50:10 EDT 2009
//    Allowed spread to be 0, in which case it quantizes the entire
//    vector to the closest point only instead of spreading it out.
//
// ****************************************************************************

vtkDataArray *
avtGeodesicVectorQuantizeExpression::DeriveVariable(vtkDataSet *in_ds)
{
    vtkDataArray *var;
    const char *varname = varnames[0];

    var = in_ds->GetPointData()->GetArray(varname);
    if (var == NULL)
    {
        var = in_ds->GetCellData()->GetArray(varname);
    }

    if (var == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Cannot quantize because: cannot locate input variable");
    }

    if (var->GetNumberOfComponents() != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Cannot quantize because: required 3D vector");
    }

    // Okay, create the output
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(geodesic_sphere_npts);
    int nvals = var->GetNumberOfTuples();
    rv->SetNumberOfTuples(nvals);
    for (int i = 0 ; i < nvals ; i++)
    {
        double tmp[geodesic_sphere_npts];

        // init output quantization to zero
        for (int j=0; j<geodesic_sphere_npts; j++)
            tmp[j] = 0.0;

        // get the vector input
        double *vorig = var->GetTuple3(i);
        double vlen = sqrt(vorig[0]*vorig[0] +
                           vorig[1]*vorig[1] +
                           vorig[2]*vorig[2]);
        // if it's non-zero, convert it to a quantization
        if (vlen > 0)
        {
            double vunit[3] = {vorig[0]/vlen, vorig[1]/vlen, vorig[2]/vlen};

            // accumulate at all points within "spread" units of a dp of 1.0,
            // weighted by dot product squared
            int maxindex = -1;
            double maxvlen = -1e9;
            double maxdp = -1e9;
            for (int j=0; j<geodesic_sphere_npts; j++)
            {
                double *gs = geodesic_sphere_points[j];
                double dp = vunit[0]*gs[0] + vunit[1]*gs[1] + vunit[2]*gs[2];
                if (spread == 0)
                {
                    if (dp > maxdp)
                    {
                        maxdp = dp;
                        maxvlen = vlen;
                        maxindex = j;
                    }
                }
                else
                {
                    dp = ((dp - (1-spread)) / spread);
                    if (dp > 0)
                    {
                        double accumval = dp*dp * vlen;
                        tmp[j] += accumval;
                    }
                }
            }
            if (spread == 0)
            {
                tmp[maxindex] += vlen;
            }
        }

        for (int j=0; j<geodesic_sphere_npts; j++)
            rv->SetComponent(i, j, tmp[j]);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtGeodesicVectorQuantizeExpression::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.  Parses the last
//      argument as a value for the "spread".
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Jeremy Meredith
//  Creation:     March 18, 2009
//
//  Modifications:
//    Jeremy Meredith, Fri Mar 20 15:55:17 EDT 2009
//    Allow a 0 spread (which means pick the single closest point).
//    Made the spread argument optional, with a default of 0.
//
// ****************************************************************************

void
avtGeodesicVectorQuantizeExpression::ProcessArguments(ArgsExpr *args,
                                        ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    nargs = arguments->size();

    if (nargs > 2)
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Expected only one or two arguments.");

    spread = 0; // default if no spread is given

    // get the spread if they gave it
    if (nargs == 2)
    {
        ArgExpr *listarg = (*arguments)[1];
        ExprParseTreeNode *spreadTree = listarg->GetExpr();
        if (spreadTree->GetTypeName() == "IntegerConst")
        {
            spread = dynamic_cast<IntegerConstExpr*>(spreadTree)->GetValue();
        }
        else if (spreadTree->GetTypeName() == "FloatConst")
        {
            spread = dynamic_cast<FloatConstExpr*>(spreadTree)->GetValue();
        }

        if (spread < 0 || spread > 1)
        {
            EXCEPTION2(ExpressionException, outputVariableName,
                       "Expected a spread (in the range [0,1]) as the final argument.");
        }
    }

    // Let the base class do the rest of the processing.  We only had to
    // over-ride this function to determine the number of arguments and
    // extract the spread.
    avtMultipleInputExpressionFilter::ProcessArguments(args, state);
}


// ****************************************************************************
//  Method: avtGeodesicVectorQuantizeExpression::UpdateDataObjectInfo
//
//  Purpose:
//      Tell the output what the component names are.
//
//  Programmer:   Jeremy Meredith
//  Creation:     March 18, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtGeodesicVectorQuantizeExpression::UpdateDataObjectInfo(void)
{
    avtMultipleInputExpressionFilter::UpdateDataObjectInfo();

    // If we don't know the name of the variable, we can't set it up in the
    // output.
    if (outputVariableName == NULL)
        return;

    std::vector<std::string> subnames(geodesic_sphere_npts);
    for (int i = 0 ; i < geodesic_sphere_npts ; i++)
    {
        char str[256];
        snprintf(str,256,"node%03d",i);
        subnames[i] = str;
    }

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetVariableDimension(subnames.size(), outputVariableName);
    outAtts.SetVariableSubnames(subnames, outputVariableName);
}

