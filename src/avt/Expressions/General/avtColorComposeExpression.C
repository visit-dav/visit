// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtColorComposeExpression.C                           //
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
avtColorComposeExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    size_t numinputs = varnames.size();

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


