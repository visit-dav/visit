// ************************************************************************* //
//                         avtVariableSummationQuery.C                       //
// ************************************************************************* //

#include <avtVariableSummationQuery.h>

#include <avtTerminatingSource.h>
#include <BadIndexException.h>

using     std::string;


// ****************************************************************************
//  Method: avtVariableSummationQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   February 3, 2004 
//
// ****************************************************************************

avtVariableSummationQuery::avtVariableSummationQuery() : avtSummationQuery()
{
}


// ****************************************************************************
//  Method: avtVariableSummationQuery destructor
//
//  Programmer: Hank Childs 
//  Creation:   February 3, 2004 
//
// ****************************************************************************

avtVariableSummationQuery::~avtVariableSummationQuery()
{
}


// ****************************************************************************
//  Method: avtVariableSummationQuery::VerifyInput
//
//  Purpose:
//      Now that we have an input, we can determine what the variable name is
//      and tell the base class about it.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 28 08:26:05 PDT 2004
//    Retrieve variable's units, if available.
//
//    Kathleen Bonnell, Thu Jan  6 10:34:57 PST 2005 
//    Remove TRY-CATCH block in favor of testing for ValidVariable.
//
// ****************************************************************************

void
avtVariableSummationQuery::VerifyInput(void)
{
    //
    // We want to do this in addition to what the base class does, so call the
    // base class' version of this method as well.
    //
    avtSummationQuery::VerifyInput();

    avtDataSpecification_p dspec = GetInput()->GetTerminatingSource()
                                     ->GetFullDataSpecification();

    avtDataAttributes &dataAtts = GetInput()->GetInfo().GetAttributes();

    string varname = dspec->GetVariable();
    SetVariableName(varname);
    SumGhostValues(false);
    SetSumType(varname);
    if (dataAtts.ValidVariable(varname.c_str()))
    {
        //
        // Set the base class units to be used in output.
        //
        SetUnits(dataAtts.GetVariableUnits(varname.c_str()));
    }
}


