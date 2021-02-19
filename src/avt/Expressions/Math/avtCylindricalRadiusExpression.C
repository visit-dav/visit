// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtCylindricalRadiusExpression.C                    //
// ************************************************************************* //

#include <avtCylindricalRadiusExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <avtVector.h>

#include <avtExprNode.h>
#include <ExprToken.h>
#include <DebugStream.h>

#include <ExpressionException.h>
#include <ImproperUseException.h>

#include <math.h>

#include <string>
#include <vector>

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
//  Modifications:
//
//    Alister Maguire, Fri Oct  9 11:46:22 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtCylindricalRadiusExpression::avtCylindricalRadiusExpression()
{
    canApplyToDirectDatabaseQOT = false;
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
avtCylindricalRadiusExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType npts = in_ds->GetNumberOfPoints();
    vtkDataArray *rv = CreateArrayFromMesh(in_ds);
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(npts);
    
    // The cylindrical radius is:
    // norm(pt) * sin(acos(dot(pt,axis)/(norm(pt)*norm(axis))))
    // 
    
    avtVector ax_vec(axisVector);
    ax_vec.normalize();
    
    for (vtkIdType i = 0 ; i < npts ; i++)
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
        double r = proj.norm();
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
avtCylindricalRadiusExpression::ProcessArguments(
    ArgsExpr *args, ExprPipelineState *state)
{
    axisVector[0] = 0;
    axisVector[1] = 0;
    axisVector[2] = 1;
    
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();
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
        std::string arg_type = second_tree->GetTypeName();
        
        std::string error_msg = "avtCylindricalRadiusExpression: "
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
            std::string val = dynamic_cast<StringConstExpr*>(second_tree)->GetValue();

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


