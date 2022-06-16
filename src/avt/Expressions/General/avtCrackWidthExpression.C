// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//   avtCrackWidthExpression.C
// ****************************************************************************

#include <avtCrackWidthExpression.h>

#include <avtExprNode.h>
#include <Expression.h>
#include <ExpressionException.h>
#include <ExpressionList.h>
#include <ImproperUseException.h>
#include <ParsingExprList.h>

#include <vtkVisItUtility.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkMassProperties.h>
#include <vtkSlicer.h>

//
// This duplicates code in operators/CracksClipper/vtkCrackWidthFilter
//
namespace avtCrackWidthExpression_Internal
{
    vtkNew<vtkSlicer> Slicer;
    vtkNew<vtkMassProperties> MassProp;

    void
    SetInput(vtkDataSet* ds)
    {
        Slicer->SetInputData(ds);
    }

    double
    LengthForCell(vtkIdType cellId, 
                  const double *center, const double *dir, const double zVol,
                  const double L1L2)
    {
        double L = 0;
        if (L1L2 == 0)
        {
            Slicer->SetCellList(&cellId, 1);
            Slicer->SetNormal(const_cast<double*>(dir));
            Slicer->SetOrigin(const_cast<double*>(center));
            MassProp->SetInputConnection(Slicer->GetOutputPort());
            MassProp->Update();
            double sa = MassProp->GetSurfaceArea();
            if (sa != 0.)
                L =  zVol / sa;
        }
        else
        {
            L = zVol / L1L2;
        }
        return L;
    }


    void OrderThem(double delta[3], int co[3])
    {
        int min, mid, max;
        if (delta[0] <= delta[1] && delta[0] <= delta[2])
            min = 0; 
        else if (delta[1] <= delta[0] && delta[1] <= delta[2])
            min = 1; 
        else 
            min = 2; 

        if (delta[0] >= delta[1] && delta[0] >= delta[2])
            max = 0; 
        else if (delta[1] >= delta[0] && delta[1] >= delta[2])
            max = 1; 
        else 
            max = 2; 

        if (min == 0)
            mid = (max == 1 ? 2 : 1);    
        else if (min == 1)
            mid = (max == 2 ? 0 : 2);    
        else 
            mid = (max == 0 ? 1 : 0);    

        co[0] = max;
        co[1] = mid;
        co[2] = min;
    }
}

// ****************************************************************************
//  Method: avtCrackWidthExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 13, 2022
//
// ****************************************************************************
avtCrackWidthExpression::avtCrackWidthExpression()
{
    crackNum = 0;
}


// ****************************************************************************
//  Method: avtCrackWidthExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 13, 2022
//
// ****************************************************************************
avtCrackWidthExpression::~avtCrackWidthExpression()
{
}


// ****************************************************************************
//  Method: avtCrackWidthExpression::ProcessArguments
//
//  Purpose:
//      Parses arguments.
//
//  Arguments:
//      args      Expression arguments
//      state     Expression pipeline state
//
//  Programmer: Kathleen Biagas
//  Creation:   June 13, 2022
//
// ****************************************************************************

void
avtCrackWidthExpression::ProcessArguments(ArgsExpr *args,
                                          ExprPipelineState *state)
{
    // Get the argument list and number of arguments.
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();
    if (nargs != 6)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "crack_width(): Incorrect syntax.\n"
                   " usage: crack_width(crack_num, crack1_dir, crack2_dir, crack3_dir, strain_tensor, vol_expr)\n");
    }

    // crack_num_argument variable.
    ExprParseTreeNode *tree0 = (*arguments)[0]->GetExpr();
    if (tree0->GetTypeName()  == "IntegerConst")
    {
        int val = dynamic_cast<IntegerConstExpr*>(tree0)->GetValue();
        if (val < 1 || val > 3)
            EXCEPTION2(ExpressionException, outputVariableName,
                       "avtCrackWidthExpression: Invalid crack_num argument.\n"
                       "Must be 1, 2, or 3");
        crackNum = val;
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "avtCrackWidthExpression: Invalid crack_num argument.\n"
                   "Must be 1, 2, or 3");
    }

    // crack1 variable.
    avtExprNode *tree1 = dynamic_cast<avtExprNode*>((*arguments)[1]->GetExpr());
    tree1->CreateFilters(state);

    // crack2 variable.
    avtExprNode *tree2 = dynamic_cast<avtExprNode*>((*arguments)[2]->GetExpr());
    tree2->CreateFilters(state);

    // crack3 variable.
    avtExprNode *tree3 = dynamic_cast<avtExprNode*>((*arguments)[3]->GetExpr());
    tree3->CreateFilters(state);

    // strain variable.
    avtExprNode *tree4 = dynamic_cast<avtExprNode*>((*arguments)[4]->GetExpr());
    tree4->CreateFilters(state);

    // volume expression variable.
    avtExprNode *tree5 = dynamic_cast<avtExprNode*>((*arguments)[5]->GetExpr());
    tree5->CreateFilters(state);

}

// ****************************************************************************
//  Method: avtCrackWidthExpression::DeriveVariable
//
//  Purpose:
//      Called to calc the expression results.
//      This duplicates code in operators/CracksClipper/vtkCrackWidthFilter.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 13, 2022
//
// ****************************************************************************

vtkDataArray *
avtCrackWidthExpression::DeriveVariable(vtkDataSet *inDS, int )
{
    vtkCellData *inCD = inDS->GetCellData();

    vtkDataArray *crackVars[3];
    crackVars[0]  = inCD->GetArray(varnames[0]);
    crackVars[1]  = inCD->GetArray(varnames[1]);
    crackVars[2]  = inCD->GetArray(varnames[2]);
    vtkDataArray *strainVar = inCD->GetArray(varnames[3]);
    vtkDataArray *vol   = inCD->GetArray(varnames[4]);

    if(crackVars[0] == nullptr ||
       crackVars[1] == nullptr ||
       crackVars[2] == nullptr ||
       strainVar == nullptr    ||
       vol == nullptr)
    {
        EXCEPTION0(ImproperUseException);
    }

    avtCrackWidthExpression_Internal::SetInput(inDS);

    //
    // formula for calculating crack width:
    //    crackwidth = L * (1 - (exp(-delta))  
    //    where: 
    //      L = ZoneVol / (Area perpendicular to crackdir)
    //        find Area by slicing the cell by plane with origin == cell center
    //        and Normal == crackdir.  Take area of that slice.
    //
    //      delta = T11 for crack dir1 = component 0 of strain_tensor 
    //              T22 for crack dir2 = component 4 of strain_tensor
    //              T33 for crack dir3 = component 8 of strain_tensor
    //

    // 
    // step through cells, calculating cell centers and crack widths for 
    // each crack direction.  Terminate early when possible.
    // 

    vtkIdType numCells = inDS->GetNumberOfCells();
    vtkDoubleArray *crackWidths[3];

    crackWidths[0] = vtkDoubleArray::New();
    crackWidths[0]->SetNumberOfComponents(1);
    crackWidths[0]->SetNumberOfTuples(numCells);

    crackWidths[1] = vtkDoubleArray::New();
    crackWidths[1]->SetNumberOfComponents(1);
    crackWidths[1]->SetNumberOfTuples(numCells);

    crackWidths[2] = vtkDoubleArray::New();
    crackWidths[2]->SetNumberOfComponents(1);
    crackWidths[2]->SetNumberOfTuples(numCells);

    vtkDoubleArray *crackWidth = nullptr;

    double deltaC = 0, L, cw, zVol, *dir = NULL, *maxCW = NULL;
    int crackOrder[3];
    double delta[3];

    for (vtkIdType cellId = 0; cellId < numCells; cellId++)
    {
        double center[3] = {VTK_FLOAT_MAX, VTK_FLOAT_MAX, VTK_FLOAT_MAX};
        delta[0] = strainVar->GetComponent(cellId, 0);
        delta[1] = strainVar->GetComponent(cellId, 4);
        delta[2] = strainVar->GetComponent(cellId, 8);

        if (delta[0] == 0 && delta[1] == 0 && delta[2] == 0)
        {
          crackWidths[0]->SetValue(cellId, 0);
          crackWidths[1]->SetValue(cellId, 0);
          crackWidths[2]->SetValue(cellId, 0);
          continue;
        }

        avtCrackWidthExpression_Internal::OrderThem(delta, crackOrder);

        vtkCell *cell = inDS->GetCell(cellId);
        vtkVisItUtility::GetCellCenter(cell, center);
  
        zVol = vol->GetComponent(cellId, 0);

        double L1L2 = 1.; 
        for (int crack = 0; crack < 3; ++crack)
        {
            crackWidth = crackWidths[crackOrder[crack]];
            dir = crackVars[crackOrder[crack]]->GetTuple(cellId);
            deltaC = delta[crackOrder[crack]];
            if (deltaC == 0 || (dir[0] == 0 && dir[1] == 0 && dir[2] == 0))
            {
                crackWidth->SetValue(cellId, 0);
                continue;
            }

            if (crack < 2)
            {
                L = avtCrackWidthExpression_Internal::LengthForCell(
                        cellId, center, dir, zVol, 0);
                L1L2 *= L;
            }
            else 
            {
                L = avtCrackWidthExpression_Internal::LengthForCell(
                        cellId, center, dir, zVol, L1L2);
            }
            cw = L*(1.0-exp(-deltaC));
            crackWidth->SetValue(cellId, cw);
        }
    }


    // if a return-all option is set (eg for use by cracks clipper)
    // create a vector instead of scalar, each component corresponding
    // to a crack num.  cracks clipper would have to separate them.
    if (crackNum == 1)
    {
        crackWidths[1]->Delete();
        crackWidths[2]->Delete();
        crackWidths[0]->SetName(outputVariableName);
        return crackWidths[0];
    }
    else if (crackNum == 2)
    {
        crackWidths[0]->Delete();
        crackWidths[2]->Delete();
        crackWidths[1]->SetName(outputVariableName);
        return crackWidths[1];
    }
    else
    {
        crackWidths[0]->Delete();
        crackWidths[1]->Delete();
        crackWidths[2]->SetName(outputVariableName);
        return crackWidths[2];
    }
}

