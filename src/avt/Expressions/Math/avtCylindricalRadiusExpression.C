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
//                       avtCylindricalRadiusExpression.C                    //
// ************************************************************************* //

#include <avtCylindricalRadiusExpression.h>

#include <math.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <avtVector.h>

#include <avtExprNode.h>
#include <ExprToken.h>
#include <DebugStream.h>

#include <ExpressionException.h>
#include <ImproperUseException.h>

// ****************************************************************************
//  Method: avtCylindricalRadiusExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   April 2, 2008
//
// ****************************************************************************

avtCylindricalRadiusExpression::avtCylindricalRadiusExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCylindricalRadiusExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   April 2, 2008
//
// ****************************************************************************

avtCylindricalRadiusExpression::~avtCylindricalRadiusExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCylindricalRadiusExpression::DeriveVariable
//
//  Purpose:
//      Calculates the radius of each input point in a cylindrial cooridnates 
//      system with a user defined cylinder axis. The default cylinder axis 
//      is the z-vector unit vector [0,0,1]
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Cyrus Harrison
//  Creation:     April 2, 2007
//
// ****************************************************************************

vtkDataArray *
avtCylindricalRadiusExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int npts = in_ds->GetNumberOfPoints();
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(npts);
    
    // The cylindrical radius is:
    // norm(pt) * sin(acos(dot(pt,axis)/(norm(pt)*norm(axis))))
    // 
    
    avtVector ax_vec(axisVector);
    ax_vec.normalize();
    
    for (int i = 0 ; i < npts ; i++)
    {
        double pt[3];
        in_ds->GetPoint(i, pt);
        avtVector pt_vec(pt);
        
        double pt_vec_mag = pt_vec.norm();
     
        // dot prod of normalized vecs to get angle
        double dp = pt_vec * ax_vec;
        dp = dp / pt_vec_mag ;
        double ang = acos(dp);
        
        // find the orthogonal component
        avtVector proj = pt_vec * sin(ang);
        float r = proj.norm();
        rv->SetComponent(i, 0, r);
    }

    return rv;
}

// ****************************************************************************
//  Method: avtCylindricalRadiusExpression::ProcessArguments
//
//  Purpose:
//      Parses optional arguments. 
//      Allows the user to pass a string constaint specifying the cylinder
//      axis (default = z ) or a vector that defines the cylinder axis.
// 
//
//  Programmer:   Cyrus Harrison 
//  Creation:     March 31, 2008
//
//  Modifications:
//
// ****************************************************************************
void
avtCylindricalRadiusExpression::ProcessArguments
(ArgsExpr *args, ExprPipelineState *state)
{
    axisVector[0] = 0;
    axisVector[1] = 0;
    axisVector[2] = 1;
    
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                  "avtCylindricalRadiusExpression: No arguments given.");
    }

    // First arg should be a mesh name, let it gen is filters. 
    ArgExpr *first_arg = (*arguments)[0];
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>(first_arg->GetExpr());
    first_tree->CreateFilters(state);

    // If we only have two arguments, we expect a string const or vector const 
    if (nargs == 2)
    {
        ArgExpr *second_arg = (*arguments)[1];
        ExprParseTreeNode *second_tree = second_arg->GetExpr();
        string arg_type = second_tree->GetTypeName();
        
        string error_msg = "avtCylindricalRadiusExpression: "
                           "Invalid second argument."
                           "Expected \"x\", \"y\", or \"z\"";

        if (arg_type != "StringConst" && arg_type != "Vector" )
        {
            debug5 << error_msg << endl;
            EXCEPTION2(ExpressionException, outputVariableName, 
                       error_msg.c_str());
        }
        
        if (arg_type == "StringConst")
        {
            // string case
            string val = dynamic_cast<StringConstExpr*>(second_tree)->GetValue();

            if ( ! ( val == "x" || val == "y"  || val == "z"))
            {
                error_msg += "\nPassed value \"" + val + "\"";
                debug5 << error_msg << endl;
                EXCEPTION2(ExpressionException, outputVariableName, 
                           error_msg.c_str());
            }
        
            debug5 << "avtCylindricalCoordinatesExpression:" 
                   << "Using " << val << " as Cylinder Axis." << endl;
            if(val == "x")
            {
                axisVector[0] = 1;
                axisVector[1] = 0;
                axisVector[2] = 0;
            }
            else if(val == "y")
            {
                axisVector[0] = 0;
                axisVector[1] = 1;
                axisVector[2] = 0;
            }
            else if(val == "z")
            {
                axisVector[0] = 0;
                axisVector[1] = 0;
                axisVector[2] = 1;
            }
        }
        else if(arg_type == "Vector")
        {
            VectorExpr *vec = dynamic_cast<VectorExpr*>(second_tree);
            
            if(!vec->Z())
            {
                error_msg += "\nVector missing z-component.";
                debug5 << error_msg << endl;
                EXCEPTION2(ExpressionException, outputVariableName, 
                           error_msg.c_str());
            }
            
            // get the vector 
            double val = 0;
            
            if(GetNumericVal(vec->X(),val))
            {
                axisVector[0] = val;
            }
            else
            {                
                error_msg += "\nVector x-component is not a floating point number.";
                debug5 << error_msg << endl;
                EXCEPTION2(ExpressionException, outputVariableName, 
                           error_msg.c_str());
            }
            
            if(GetNumericVal(vec->Y(),val))
            {
                axisVector[1] = val;
            }
            else
            {                
                error_msg += "\nVector z-component is not a floating point number.";
                debug5 << error_msg << endl;
                EXCEPTION2(ExpressionException, outputVariableName, 
                           error_msg.c_str());
            }
            
            if(GetNumericVal(vec->Z(),val))
            {
                axisVector[2] = val;
            }
            else
            {                
                error_msg += "\nVector z-component is not a floating point number.";
                debug5 << error_msg << endl;
                EXCEPTION2(ExpressionException, outputVariableName, 
                           error_msg.c_str());
            }

            if ( axisVector[0] == 0 && axisVector[1] == 0 && axisVector[2] == 0 )
            {
                error_msg += "\nDegenerate vector {0,0,0}.";
                debug5 << error_msg << endl;
                EXCEPTION2(ExpressionException, outputVariableName, 
                           error_msg.c_str());
            }
        }
        
    }
}


