// ************************************************************************* //
//                       avtMultipleInputExpressionFilter.h                  //
// ************************************************************************* //

#include <avtMultipleInputExpressionFilter.h>
#include <string.h>

void
avtMultipleInputExpressionFilter::AddInputVariableName(const char *var)
{
    if (varnames.size() == 0)
        SetActiveVariable(var);
    else
        AddSecondaryVariable(var);

    varnames.push_back(strdup(var));
}
