// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtCoordinateExtremaExpression.C                    //
// ************************************************************************* //

#include <avtCoordinateExtremaExpression.h>

#include <math.h>

#include <avtExprNode.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>

#include <ExpressionException.h>

#include <string>
#include <vector>

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
//    Alister Maguire, Fri Oct  9 11:04:05 PDT 2020
//    Setting canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtCoordinateExtremaExpression::avtCoordinateExtremaExpression()
{
    getMinimum = true;
    coordinateType = CT_X;
    canApplyToDirectDatabaseQOT = false;
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
//  Modifications:
//    Kathleen Biagas, Thu Apr  5 13:13:21 PDT 2012
//    Create output array with same data type as input coordinates, use 
//    vtkIdType for values returned by vtk classes.
//
// ****************************************************************************

vtkDataArray *
avtCoordinateExtremaExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType ncells = in_ds->GetNumberOfCells();

    vtkDataArray *rv = CreateArrayFromMesh(in_ds);
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
        for (vtkIdType i = 0 ; i < ncells ; i++)
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
        for (vtkIdType i = 0 ; i < ncells ; i++)
        {
            double mostExtreme = 0;
            if (coordinateType == CT_Radius)
                mostExtreme = (getMinimum ? 1e+40 : 0.);
            else if (coordinateType == CT_Theta)
                mostExtreme = (getMinimum ? 10 : 0.);
            else if (coordinateType == CT_Phi)
                mostExtreme = (getMinimum ? 10 : 0.);

            in_ds->GetCellPoints(i, ptIds);
            vtkIdType nIds = ptIds->GetNumberOfIds();
            for (vtkIdType j = 0 ; j < nIds ; j++)
            {
                double pt[3];
                in_ds->GetPoint(ptIds->GetId(j), pt);
                double thisPointsValue = 0;
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
    size_t nargs = arguments->size();

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
    std::string second_type = second_tree->GetTypeName();

    if ((second_type == "StringConst"))
    {
        std::string sval =
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


