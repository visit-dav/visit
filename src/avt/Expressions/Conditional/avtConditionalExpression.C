// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtConditionalExpression.C                          //
// ************************************************************************* //

#include <avtConditionalExpression.h>

#include <snprintf.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtConditionalExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtConditionalExpression::avtConditionalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtConditionalExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtConditionalExpression::~avtConditionalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtConditionalExpression::DeriveVariable
//
//  Purpose:
//      Selects between two arrays based on a condition.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     August 20, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Mar 13 10:02:16 PDT 2007
//    Improve error message.
//
//    Cyrus Harrison, Wed Feb 27 16:47:41 PST 2008
//    Test to make sure input data arrays have the same number of tuples and
//    issue an error message if this happens. 
//
//    Kathleen Biagas, Wed Apr 4 08:39:02 PDT 2012 
//    Support other data types besides uchar and float. 
//
// ****************************************************************************

vtkDataArray *
avtConditionalExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    // Our first operand is in the active variable.  We don't know if it's
    // point data or cell data, so check which one is non-NULL.
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

    if (data1 == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Could not identify centering of first variable");
    }
    if (data1->GetNumberOfComponents() != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Cannot interpret conditional with vector dimensionality");
    }

    // Get the second variable.
    if (centering == AVT_ZONECENT)
        data2 = in_ds->GetCellData()->GetArray(varnames[1]);
    else
        data2 = in_ds->GetPointData()->GetArray(varnames[1]);

    if (data2 == NULL)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, 
                   "the first two arguments to \"if\" (%s and %s) have "
                   " different centerings.", varnames[0], varnames[1]);
        EXCEPTION2(ExpressionException, outputVariableName, msg);
    }

    // Get the third variable.
    if (centering == AVT_ZONECENT)
        data3 = in_ds->GetCellData()->GetArray(varnames[2]);
    else
        data3 = in_ds->GetPointData()->GetArray(varnames[2]);
    
    if (data3 == NULL)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, 
                   "the first and third arguments to \"if\" (%s and %s) have "
                   " different centerings.", varnames[0], varnames[2]);
        EXCEPTION2(ExpressionException, outputVariableName, msg);
    }

    // sanity check: make sure the input arrays have the same # of tuples
    int d1_nvals = data1->GetNumberOfTuples();
    int d2_nvals = data2->GetNumberOfTuples();
    int d3_nvals = data2->GetNumberOfTuples();
    
    if ( d1_nvals != d2_nvals || d2_nvals != d3_nvals )
    {
        std::string msg = " the input datasets for the \"if\" expression "
                          " do not have the same number of tuples!";
        EXCEPTION2(ExpressionException, outputVariableName, msg.c_str());
    }
    
    //
    // Set up a VTK variable reflecting the calculated variable
    //
    int nvals  = data2->GetNumberOfTuples();
    vtkDataArray *dv = data2->NewInstance();
    dv->SetNumberOfComponents(data2->GetNumberOfComponents());
    dv->SetNumberOfTuples(nvals);

    if (data1->GetDataType() == VTK_UNSIGNED_CHAR)
    {
        vtkUnsignedCharArray *uca = (vtkUnsignedCharArray *) data1;
        for (int i = 0 ; i < nvals ; i++)
        {
            unsigned char val = uca->GetValue(i);
            if (val != '\0')
            {
                dv->SetTuple(i, data2->GetTuple(i));
            }
            else
            {
                dv->SetTuple(i, data3->GetTuple(i));
            }
        }
    }
    else  
    {
        for (int i = 0 ; i < nvals ; i++)
        {
            double val = data1->GetTuple1(i);
            if (val != 0.)
            {
                dv->SetTuple(i, data2->GetTuple(i));
            }
            else
            {
                dv->SetTuple(i, data3->GetTuple(i));
            }
        }
    }
      
    return dv;
}


// ****************************************************************************
//  Method:  avtConditionalExpression::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
bool
avtConditionalExpression::FilterUnderstandsTransformedRectMesh()
{
    // simple conditionals operate only on variables and
    // shouldn't be directly affected by coordinates
    return true;
}
