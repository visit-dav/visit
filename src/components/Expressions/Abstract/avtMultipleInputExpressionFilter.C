// ************************************************************************* //
//                       avtMultipleInputExpressionFilter.h                  //
// ************************************************************************* //

#include <avtMultipleInputExpressionFilter.h>

#include <string.h>

#include <avtDatasetExaminer.h>
#include <avtTypes.h>


// ****************************************************************************
//  Method: avtMultipleInputExpressionFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2003
//
// ****************************************************************************

avtMultipleInputExpressionFilter::~avtMultipleInputExpressionFilter()
{
    for (int i = 0 ; i < varnames.size() ; i++)
    {
        //
        // We should use the C-style de-allocation, since this string was
        // created by strdup (which uses malloc).  If we don't, purify
        // complains.
        //
        free(varnames[i]);
    }
}


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
//  Modifications:
//
//    Hank Childs, Wed Feb 25 14:44:19 PST 2004
//    Updated to account for multiple variables in data attributes.
//
//    Kathleen Bonnell, Mon Mar 29 11:28:25 PST 2004 
//    Make sure that varnames is not empty before attempting to index. 
//
//    Hank Childs, Thu Jan  6 10:42:13 PST 2005
//    Do a better job when there are mixed centerings.
//
// ****************************************************************************

bool
avtMultipleInputExpressionFilter::IsPointVariable(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    bool hasNodal = false;
    bool hasZonal = false;
    for (int i = 0 ; i < varnames.size() ; i++)
    {
        if (!atts.ValidVariable(varnames[i]))
            return avtExpressionFilter::IsPointVariable();

        if (atts.GetCentering(varnames[i]) == AVT_ZONECENT)
            hasZonal = true;
        else
            hasNodal = true;
    }

    if (hasZonal && hasNodal)
        return !(MixedCenteringYieldsZonal());

    return hasNodal;
}


// ****************************************************************************
//  Method: avtMultipleInputExpressionFilter::ClearInputVariableNames
//
//  Purpose:
//      Clears the input variable names.  This allows a filter to be used
//      repeatedly.
//
//  Programmer: Hank Childs
//  Creation:   February 4, 2004
//
// ****************************************************************************

void
avtMultipleInputExpressionFilter::ClearInputVariableNames(void)
{
    varnames.clear();
}


