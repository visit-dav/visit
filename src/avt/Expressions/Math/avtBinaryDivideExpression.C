// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtBinaryDivideExpression.C                        //
// ************************************************************************* //

#include <avtBinaryDivideExpression.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>

#include <ExpressionException.h>
#include <DebugStream.h>


// ****************************************************************************
//  Method: avtBinaryDivideExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Eddie Rusu, Wed Sep 11 08:59:52 PDT 2019
//    Populate class fields with default values. Added alternate constructor to
//    be used when smart division is requested.
//
// ****************************************************************************

avtBinaryDivideExpression::avtBinaryDivideExpression()
{
    tolerance = 1e-16;
    value_if_zero = 0.0;
    smart_division = false;
}

avtBinaryDivideExpression::avtBinaryDivideExpression(bool _process_anyways)
{
    tolerance = 1e-16;
    value_if_zero = 0.0;
    smart_division = _process_anyways;
}


// ****************************************************************************
//  Method: avtBinaryDivideExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtBinaryDivideExpression::~avtBinaryDivideExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtBinaryDivideExpression::DeriveVariable
//
//  Purpose:
//      Derives the variable that results from the expression. At least two
//      inputs are required, and there may be two option inputs if smart
//      division is enabled.
//
//  Arguments:
//      in_ds   The dataset from which to extract the desired variables
//              involved in the expression.
//
//  Returns:    The derived variable that results from the expression. The
//              calling class must free this memory.
//
//  Programmer: Eddie Rusu
//  Creation:   Wed Sep 11 08:59:52 PDT 2019
//
// ****************************************************************************

vtkDataArray *
avtBinaryDivideExpression::DeriveVariable(vtkDataSet* in_ds,
        int currentDomainsIndex)
{
    debug3 << "Entering avtBinaryDivideExpression::DeriveVariable(vtkDataSet*,"
            "int)" << std::endl;
    // Get the variables and their centerings
    avtCentering var1_centering;
    avtCentering var2_centering;
    vtkDataArray *data1 = DetermineCentering(&var1_centering, in_ds,
            varnames[0]);
    vtkDataArray *data2 = DetermineCentering(&var2_centering, in_ds,
            varnames[1]);

    // If smart_division is activated, then we check for 3rd and 4th inputs.
    if (smart_division)
    {
        // Check if there is a third variable. If so, that variable is the
        // value_if_zero.
        avtCentering dummy;
        if (varnames.size() >= 3 && nProcessedArgs >= 3)
        {
            vtkDataArray *data3 = DetermineCentering(&dummy, in_ds,
                    varnames[2]);
            value_if_zero = data3->GetTuple1(0);
            debug5 << "avtBinaryDivideExpression::DeriveVariable: User "
                    "specified a divide_by_zero_value of " << value_if_zero
                    << "." << std::endl;

        }
        // TODO: this feature can be easily enhanced to to handle a
        // multi-variable divide by zero case. For example, suppose the user
        // wants the value_if_zero to vary over the tuples/components as
        // represented by some variable. This can be easily done here.

        // Check if there is a 4th variable. If so, that variable is the
        // tolerance.
        if (varnames.size() >= 4 && nProcessedArgs >= 4)
        {
            vtkDataArray *data4 = DetermineCentering(&dummy, in_ds,
                    varnames[3]);
            tolerance = data4->GetTuple1(0);
            if (tolerance < 0.0)
            {
                EXCEPTION2(ExpressionException, outputVariableName, 
                        "tolerance must be nonnegative.");
            }
            debug5 << "avtBinaryDivideExpression::DeriveVariable: User "
                    "specified a tolerance of " << tolerance << "."
                    << std::endl;
        }
        // TODO: this feature can be easily enhanced to to handle a
        // multi-variable divide by zero case. For example, suppose the user
        // wants to specify varying tolernaces over the mesh as represented by
        // some variable. This can be easily done here.
    }
    

    debug5 << "avtBinaryDivideExpression::DeriveVariable: Centering "
            "determined. Now we recenter if needed" << std::endl;
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

    debug5 << "avtBinaryDivideExpression::DeriveVariable: Values have been "
            "recentered as needed. Now we perform the operation" << std::endl;
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

    DoOperation(data1, data2, output, nComps, nVals);

    debug3 << "Exiting  avtBinaryDivideExpression::DeriveVariable("
            "vtkDataSet*, int)" << std::endl;
    return output;
}


// ****************************************************************************
//  Method: avtBinaryDivideExpression::DoOperation
//
//  Purpose:
//      Divides the contents of the first array by the second array and puts
//      the output in a third array.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.). Not used for this filter.
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Sean Ahern          <Header added by Hank Childs>
//  Creation:   November 18, 2002   <Header creation date>
//
//  Modifications:
//
//    Hank Childs, Mon Nov 18 07:35:07 PST 2002
//    Added support for vectors and arbitrary data types.
//
//    Hank Childs, Thu Aug 14 11:18:07 PDT 2003
//    Add support for vector and scalar types mixing.
//
//    Hank Childs, Fri Oct  7 10:43:28 PDT 2005
//    Add support for dividing by zero.
//
//    Hank Childs, Mon Jan 14 17:58:58 PST 2008
//    Add support for singleton constants.
//
//    Eddie Rusu, Wed Sep 11 08:59:52 PDT 2019
//    Defined 0 as a tolerance instead of absolutely 0.0. Added smart division
//    in the form of CheckZero at each division.
//
// ****************************************************************************

void
avtBinaryDivideExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                   vtkDataArray *out, int ncomponents,
                                   int ntuples)
{
    debug4 << "Entering avtBinaryDivideExpression::DoOperation(vtkDataArray*, "
            "vtkDataArray*, vtkDataArray*, int, int)" << std::endl;
    bool var1IsSingleton = (in1->GetNumberOfTuples() == 1);
    bool var2IsSingleton = (in2->GetNumberOfTuples() == 1);
    int in1ncomps = in1->GetNumberOfComponents();
    int in2ncomps = in2->GetNumberOfComponents();
    if ((in1ncomps == 1) && (in2ncomps == 1))
    {
        debug5 << "avtBinaryDivideExpression::DoOperation: Scalar top and "
                "bottom." << std::endl;
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = in1->GetTuple1(tup1);
            double val2 = in2->GetTuple1(tup2);
            out->SetTuple1(i, CheckZero(val1,val2));
        }
    }
    else if (in1ncomps > 1 && in2ncomps == 1)
    {
        debug5 << "avtBinaryDivideExpression::DoOperation: Vector top, scalar "
                "bottom." << std::endl;
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val2 = in2->GetTuple1(tup2);
            for (int j = 0 ; j < in1ncomps ; j++)
            {
                double val1 = in1->GetComponent(tup1, j);
                out->SetComponent(i, j, CheckZero(val1, val2));
            }
        }
    }
    else if (in1ncomps == 1 && in2ncomps > 1)
    {
        debug5 << "avtBinaryDivideExpression::DoOperation: Scalar top, vector "
                "bottom." << std::endl;
        for (int i = 0 ; i < ntuples ; i++)
        {
            vtkIdType tup1 = (var1IsSingleton ? 0 : i);
            vtkIdType tup2 = (var2IsSingleton ? 0 : i);
            double val1 = in1->GetTuple1(tup1);
            for (int j = 0 ; j < in2ncomps ; j++)
            {
                double val2 = in2->GetComponent(tup2, j);
                out->SetComponent(i, j, CheckZero(val1, val2));
            }
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "Division of vectors in undefined.");
    }
    debug4 << "Exiting  avtBinaryDivideExpression::DoOperation(vtkDataArray*, "
            "vtkDataArray*, vtkDataArray*, int, int)" << std::endl;
}


// ****************************************************************************
//  Method: avtBinaryDivideExpression::DetermineCentering
//
//  Purpose:
//      Determines the centering of an input variable and outputs the
//      data array.
//
//  Arguments:
//      centering_out   A pointer to the centering variable.
//      in_ds           Dataset from which to extract the array.
//      varname         The name of the variable we want to extract.
//
//  Returns:    The cell- or node-centered data array.
//
//  Programmer: Eddie Rusu
//  Creation:   Wed Sep 11 08:59:52 PDT 2019
//
// ****************************************************************************

vtkDataArray*
avtBinaryDivideExpression::DetermineCentering(avtCentering *centering_out,
        vtkDataSet *in_ds, const char *varname)
{
    debug5 << "Entering avtBinaryDivideExpression::DetermineCentering("
            "avtCentering*, vtkDataSet*, const char*)" << std::endl;
    debug5 << "\t For " << varname << std::endl;
    vtkDataArray* out = in_ds->GetCellData()->GetArray(varname);
    if (out == NULL)
    {
        out = in_ds->GetPointData()->GetArray(varname);
        if (out == NULL)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                    "An internal error occurred when calculating an "
                    "expression. Please contact a VisIt developer.");
        }
        else
        {
            *(centering_out) = AVT_NODECENT;
            debug5 << "Exiting  avtBinaryDivideExpression::DetermineCentering("
                    "avtCentering*, vtkDataSet*, const char*)" << std::endl;
            return out;
        }
    }
    else
    {
        *(centering_out) = AVT_ZONECENT;
        debug5 << "Exiting  avtBinaryDivideExpression::DetermineCentering("
                "avtCentering*, vtkDataSet*, const char*)" << std::endl;
        return out;
    }
}


// ****************************************************************************
//  Method: avtBinaryDivideExpression::CheckZero
//
//  Purpose:
//      Checks the values involved in the division. If the denominator is
//      within tolerance of zero, then we throw an exception. However, if smart
//      division is activated, then we return the specified divide by zero
//      value.
//
//  Arguments:
//      top           The numerator.
//      bottom        The denominator.
//
//  Programmer: Eddie Rusu
//  Creation:   Thu Aug 29 15:05:08 PDT 2019
//
// ****************************************************************************

double
avtBinaryDivideExpression::CheckZero(double top, double bottom)
{
    if (fabs(bottom) < this->tolerance)
    {
        if (this->smart_division)
        {
            return this->value_if_zero;
        }
        else
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                        "You can't divide by zero");
        }
    }
    else
    {
        return top / bottom;
    }
}


