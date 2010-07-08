/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//                       avtCoordinateExtremaExpression.C                    //
// ************************************************************************* //

#include <avtCoordinateExtremaExpression.h>

#include <math.h>

#include <avtExprNode.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtCoordinateExtremaExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 28, 2010
//
//  Modifications:
//
//    Hank Childs, Thu Jul  8 06:48:38 PDT 2010
//    Add support for polar coordinates.
//
// ****************************************************************************

avtCoordinateExtremaExpression::avtCoordinateExtremaExpression()
{
    getMinimum = true;
    coordinateType = CT_X;
}


// ****************************************************************************
//  Method: avtCoordinateExtremaExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   June 28, 2010
//
// ****************************************************************************

avtCoordinateExtremaExpression::~avtCoordinateExtremaExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCoordinateExtremaExpression::DeriveVariable
//
//  Purpose:
//      Calculates the extrema of a coordinate.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     June 28, 2010
//
// ****************************************************************************

vtkDataArray *
avtCoordinateExtremaExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int ncells = in_ds->GetNumberOfCells();

    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(1);
    rv->SetNumberOfTuples(ncells);
    if (coordinateType == CT_X || 
        coordinateType == CT_Y || 
        coordinateType == CT_Z)
    {
        int dim = 0;
        if (coordinateType == CT_X)
            dim = 0;
        else if (coordinateType == CT_Y)
            dim = 1;
        else if (coordinateType == CT_Z)
            dim = 2;
        for (int i = 0 ; i < ncells ; i++)
        {
            double bbox[6];
            in_ds->GetCellBounds(i, bbox);
            int idx = 2*dim + (getMinimum ? 0 : 1);
            rv->SetTuple1(i, bbox[idx]);
        }
    }
    if (coordinateType == CT_Radius ||
        coordinateType == CT_Theta ||
        coordinateType == CT_Phi)
    {
        vtkIdList *ptIds = vtkIdList::New();
        for (int i = 0 ; i < ncells ; i++)
        {
            double mostExtreme;
            if (coordinateType == CT_Radius)
                mostExtreme = (getMinimum ? 1e+40 : 0.);
            else if (coordinateType == CT_Theta)
                mostExtreme = (getMinimum ? 10 : 0.);
            else if (coordinateType == CT_Phi)
                mostExtreme = (getMinimum ? 10 : 0.);

            in_ds->GetCellPoints(i, ptIds);
            int nIds = ptIds->GetNumberOfIds();
            for (int j = 0 ; j < nIds ; j++)
            {
                double pt[3];
                in_ds->GetPoint(ptIds->GetId(j), pt);
                double thisPointsValue;
                if (coordinateType == CT_Radius)
                    thisPointsValue = sqrt(pt[0]*pt[0]+pt[1]*pt[1]+pt[2]*pt[2]);
                else if (coordinateType == CT_Theta)
                    thisPointsValue = atan2(pt[1], pt[0]);
                else if (coordinateType == CT_Phi)
                {
                    double r = sqrt(pt[0]*pt[0]+pt[1]*pt[1]+pt[2]*pt[2]);
                    thisPointsValue = acos(pt[2]/r);
                }

                if (getMinimum)
                    mostExtreme = (thisPointsValue < mostExtreme ? thisPointsValue : mostExtreme);
                else
                    mostExtreme = (thisPointsValue > mostExtreme ? thisPointsValue : mostExtreme);
            }
            rv->SetTuple1(i, mostExtreme);
        }
        ptIds->Delete();
    }

    return rv;
}


// ****************************************************************************
//  Method: avtGradientExpression::ProcessArguments
//
//  Purpose:
//      Parses optional algorithm argument.
//
//  Arguments:
//      args      Expression arguments
//      state     Expression pipeline state
//
//  Programmer:   Hank Childs
//  Creation:     July 8, 2010
//
// ****************************************************************************

void
avtCoordinateExtremaExpression::ProcessArguments(ArgsExpr *args,
                                        ExprPipelineState *state)
{
    // get the argument list and # of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();

    // check for call with no args
    if (nargs != 2)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "min_coords() Incorrect syntax.\n"
                   " usage: min_coords(meshname, axis)\n"
                   " The axis parameter "
                   "specifies which axis to find the minimum over.\n"
                   "Valid Options:\n"
                   " \"X\", "
                   " \"Y\", "
                   " \"Z\", "
                   " \"Radius\", "
                   " \"Theta\", "
                   " \"Phi\"\n");
    }

    // first argument is the var name, let it do its own magic
    ArgExpr *first_arg = (*arguments)[0];
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>(first_arg->GetExpr());
    first_tree->CreateFilters(state);

    ArgExpr *second_arg= (*arguments)[1];
    ExprParseTreeNode *second_tree= second_arg->GetExpr();
    string second_type = second_tree->GetTypeName();

    if ((second_type == "StringConst"))
    {
        string sval =
                    dynamic_cast<StringConstExpr*>(second_tree)->GetValue();

        if (sval == "X")
            coordinateType = CT_X;
        else if (sval == "Y")
            coordinateType = CT_Y;
        else if (sval == "Z")
            coordinateType = CT_Z;
        else if (sval == "Radius")
            coordinateType = CT_Radius;
        else if (sval == "Theta")
            coordinateType = CT_Theta;
        else if (sval == "Phi")
            coordinateType = CT_Phi;
        else
        {
            EXCEPTION2(ExpressionException, outputVariableName,
            "avtCoordinateExtremaExpression: Invalid second argument.\n"
               " Valid options are: \"X\", \"Y\", \"Z\", \"Radius\", \"Theta\", "
               " \"Phi\".");
        }
    }
    else // invalid arg type
    {

        EXCEPTION2(ExpressionException, outputVariableName,
        "avtCoordinateExtremaExpression: Expects a string second "
        "argument.\n"
        " Valid options are: \"X\", \"Y\", \"Z\", \"Radius\", \"Theta\", "
           "\"Phi\".");
    }
}


