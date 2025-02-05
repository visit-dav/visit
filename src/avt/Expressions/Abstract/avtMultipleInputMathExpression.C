// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtMultipleInputMathExpression.C                      //
// ************************************************************************* //

#include <avtMultipleInputMathExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>


#include <ExpressionException.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtMultipleInputMathExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
// ****************************************************************************

avtMultipleInputMathExpression::avtMultipleInputMathExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMultipleInputMathExpression destructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
// ****************************************************************************

avtMultipleInputMathExpression::~avtMultipleInputMathExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMultipleInputMathExpression::DeriveVariable
//
//  Purpose:
//      Creates data arrays of all the input arguments and determines their
//      centering. Performs any recentering as necessary. Performs the math
//      operation and returns the output of that operation.
//
//  Arguments:
//      in_ds   The vtkDataSet from which to extract the data arrays.
//      dummy   A dummy variable needed for the class hierarchy.
//
//  Returns:    The derived variable. The caller must free this memory.
//
//  Programmer: Eddie Rusu
//  Creation:   Tue Sep 24 09:07:44 PDT 2019
//
//  Modifications:
//
//      Alister Maguire, Mon Mar 29 11:17:07 PDT 2021
//      Clear our containers after we've performed our work.
//
// ****************************************************************************

vtkDataArray*
avtMultipleInputMathExpression::DeriveVariable(vtkDataSet* in_ds, int dummy)
{
    debug3 << "Entering avtMultipleInputMathExpression::DeriveVariable("
            "vtkDataSet*, int)" << std::endl;

    // Get the variables and their centering
    for (int i = 0; i < nProcessedArgs; ++i)
    {
        avtCentering currentCenter;
        dataArrays.push_back(ExtractCenteredData(&currentCenter, in_ds, varnames[i]));
        centerings.push_back(currentCenter);
    }

    RecenterData(in_ds);
    vtkDataArray* output = DoOperation();

    //
    // Before we leave, we need to clear out our data containers. There are
    // times when an instantiation of this class will be used more than once
    // (like with domain decomposed data), and we can't have old information
    // hanging around.
    //
    dataArrays.clear();
    centerings.clear();

    debug3 << "Exiting  avtMultipleInputMathExpression::DeriveVariable("
            "vtkDataSet*, int)" << std::endl;
    return output;
}

// ****************************************************************************
//  Method: avtMultipleInputMathExpression::ExtractCenteredData
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
avtMultipleInputMathExpression::ExtractCenteredData(avtCentering *centering_out,
        vtkDataSet *in_ds, const char *varname)
{
    debug5 << "Entering avtMultipleInputMathExpression::ExtractCenteredData("
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
            debug5 << "Exiting  "
                    "avtMultipleInputMathExpression::ExtractCenteredData("
                    "avtCentering*, vtkDataSet*, const char*)" << std::endl;
            return out;
        }
    }
    else
    {
        *(centering_out) = AVT_ZONECENT;
        debug5 << "Exiting  avtMultipleInputMathExpression::ExtractCenteredData("
                "avtCentering*, vtkDataSet*, const char*)" << std::endl;
        return out;
    }
}


// ****************************************************************************
//  Method: avtMultipleInputMathExpression::CreateOutputVariable
//
//  Purpose:
//      Generate the output variable with the correct number of components
//      and tuples.
//
//  Arguments:
//      arraysToConsider    Indicates how many arrays (in order of input) to
//                          consider in determining the component and tuple
//                          count. Default behavior is to use all of the
//                          processed arrays.
//
//  Returns:    Data array of appropriate size.
//
//  Programmer: Eddie Rusu
//  Creation:   Mon Sep 30 14:49:38 PDT 2019
//
// ****************************************************************************

vtkDataArray*
avtMultipleInputMathExpression::CreateOutputVariable()
{
    return CreateOutputVariable(dataArrays.size());
}

vtkDataArray*
avtMultipleInputMathExpression::CreateOutputVariable(int arraysToConsider)
{
    debug5 << "Entering avtMultipleInputMathExpression::"
              "CreateOutputVariable(int)" << std::endl;
    // Loop over all inputs and determine the number of components and
    // tuples
    int nComps = 1;
    int nVals = 1;
    for (int i = 0; i < arraysToConsider; ++i)
    {
        int nCompsi = dataArrays[i]->GetNumberOfComponents();
        if (nCompsi != 1 && nCompsi != nComps)
        {
            // We can support one-multi components, but we can only support
            // multi-multi if they are the same values.
            if (nComps == 1)
            {
                nComps = nCompsi;
            }
            else
            {
                EXCEPTION2(ExpressionException, outputVariableName, "Cannot "
                        "process variables with different number of "
                        "dimensions.");
            }
        }

        int nValsi = dataArrays[i]->GetNumberOfTuples();
        if (nValsi != 1 && nValsi != nVals)
        {
            // We can support singleton values but we cannot support mismatched
            // number of tuples
            if (nVals == 1)
            {
                nVals = nValsi;
            }
            else
            {
                EXCEPTION2(ExpressionException, outputVariableName, "Cannot "
                        "process variables with different number of "
                        "elements.");
            }
        }
    }

    // Setup the output variable
    vtkDataArray* output = vtkDoubleArray::New();
    output->SetNumberOfComponents(nComps);
    output->SetNumberOfTuples(nVals);
    debug5 << "Number of tuples: " << nVals << std::endl;
    debug5 << "Number of components: " << nComps << std::endl;

    debug5 << "Exiting  avtMultipleInputMathExpression::"
              "CreateOutputVariable(int)" << std::endl;
    return output;
}


// ****************************************************************************
//  Method: avtMultipleInputMathExpression::RecenterData
//
//  Purpose:
//      Determines the centering of the input variables. If there is mixed
//      centering, default to zone-centered.
//
//  Arguments:
//      in_ds   The vtkDataSet that holds all the arrays. Arrays and
//              centerings are already stored in dataArrays and centerings,
//              which are class vectors, so in_ds is only needed because
//              the call to avtExpressionFilter::Recenter requires it.
//
//  Programmer: Eddie Rusu
//  Creation:   Mon Sep 30 10:38:44 PDT 2019
//
// ****************************************************************************

void
avtMultipleInputMathExpression::RecenterData(vtkDataSet* in_ds)
{
    debug5 << "Entering avtMultipleInputMathExpression::RecenterData(vtkDataSet*)"
            << std::endl;

    // Determine the centering
    centering = centerings[0];
    for (int i = 1; i < nProcessedArgs; ++i)
    {
        if (centerings[i] != centering)
        {
            centering = AVT_ZONECENT;
            break;
        }
    }

    // Recenter variables as needed
    for (int i = 0; i < nProcessedArgs; ++i)
    {
        if (centerings[i] != centering)
        {
            dataArrays[i] = Recenter(in_ds, dataArrays[i], centerings[i],
                outputVariableName);
        }
    }
    debug5 << "Exiting  avtMultipleInputMathExpression::RecenterData(vtkDataSet*)"
            << std::endl;
}
