// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtHSVColorComposeExpression.C                           //
// ************************************************************************* //

#include <avtHSVColorComposeExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtHSVColorComposeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2007
//
// ****************************************************************************

avtHSVColorComposeExpression::avtHSVColorComposeExpression()
{
}


// ****************************************************************************
//  Method: avtHSVColorComposeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2007
//
// ****************************************************************************

avtHSVColorComposeExpression::~avtHSVColorComposeExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtHSVColorComposeExpression::DeriveVariable
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
//  Notes: Mostly taken from avtColorComposeExpression.C
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2007
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
avtHSVColorComposeExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    size_t numinputs = varnames.size();
    if (numinputs != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Required three arguments.");
    }

    //
    // Our first operand is in the active variable. We don't know if it's
    // point data or cell data, so check which one is non-NULL.
    //
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

    vtkIdType nvals = data1->GetNumberOfTuples();
    vtkDataArray *dv = data1->NewInstance();
    dv->SetNumberOfComponents(4);
    dv->SetNumberOfTuples(data1->GetNumberOfTuples());
    for(vtkIdType id = 0; id < nvals; ++id)
    {
        double r, g, b;
        double h = data1->GetTuple1(id);
        double s = data2->GetTuple1(id);
        double v = data3->GetTuple1(id);
        if (h<0) h=0;
        if (h>1) h=1;
        if (s<0) s=0;
        if (s>1) s=1;
        if (v<0) v=0;
        if (v>1) v=1;
        h *= 6;
        int sector = int(floor(h));
        double f = h - double(sector);
        if (sector == 6)
        {
            sector = 0;
            f = 0.;
        }
        double x = v * (1 - s);
        double y = v * (1 - s*f);
        double z = v * (1 - s*(1-f));

        switch (sector)
        {
          case 0:  r = v; g = z; b = x; break;
          case 1:  r = y; g = v; b = x; break;
          case 2:  r = x; g = v; b = z; break;
          case 3:  r = x; g = y; b = v; break;
          case 4:  r = z; g = x; b = v; break;
          default: r = v; g = x; b = y; break;
        }

        double val1 = ClampColor(255.*r);
        double val2 = ClampColor(255.*g);
        double val3 = ClampColor(255.*b);
        dv->SetTuple4(id, val1, val2, val3, 255.);
    }

    return dv;
}


