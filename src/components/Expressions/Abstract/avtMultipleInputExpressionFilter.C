// ************************************************************************* //
//                       avtMultipleInputExpressionFilter.h                  //
// ************************************************************************* //

#include <avtMultipleInputExpressionFilter.h>

#include <string.h>

#include <avtDatasetExaminer.h>
#include <avtTypes.h>


// ****************************************************************************
//  Method: avtMultipleInputExpressionFilter::AddInputVariableName
//
//  Purpose:
//      Sets one of the variables that will be needed for this filter.
//
//  Programmer: Sean Ahern
//  Creation:   June 14, 2002
//
// ****************************************************************************

void
avtMultipleInputExpressionFilter::AddInputVariableName(const char *var)
{
    if (varnames.size() == 0)
        SetActiveVariable(var);
    else
        AddSecondaryVariable(var);

    varnames.push_back(strdup(var));
}


// ****************************************************************************
//  Method: avtMultipleInputExpressionFilter::IsPointVariable
//
//  Purpose:
//      Determines whether or not this is a point variable.
//
//  Note:   This routine can only determine whether or not this is a point
//          variable once the data has been read.  However, this function
//          is sometimes called before that.  So the best we can do is defer
//          to the base class.  We need a better long term solution, but this
//          is better than what we are doing now.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2003
//
// ****************************************************************************

bool
avtMultipleInputExpressionFilter::IsPointVariable(void)
{
    if (varnames.size() == 0)
        return avtExpressionFilter::IsPointVariable();

    avtDataset_p input = GetTypedInput();
    avtCentering cent = avtDatasetExaminer::
                             GetVariableCentering(input, varnames[0]);

    if (cent != AVT_UNKNOWN_CENT)
    {
        return (cent == AVT_NODECENT);
        return cent;
    }

    return avtExpressionFilter::IsPointVariable();
}


