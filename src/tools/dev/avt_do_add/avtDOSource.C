// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtDO_REPL_STRINGSource.C                        //
// ************************************************************************* //

#include <avtDO_REPL_STRINGSource.h>


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSource constructor
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRINGSource::avtDO_REPL_STRINGSource()
{
    output = new avtDO_REPL_STRING(this);
}


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSource destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRINGSource::~avtDO_REPL_STRINGSource()
{
    ;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSource::GetOutput
//
//  Purpose:
//      Gets the output avtDO_REPL_STRING as an avtDataObject.
//
//  Returns:    The source's DO_REPL_STRING, typed as an avtDataObject
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDataObject_p
avtDO_REPL_STRINGSource::GetOutput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, output);

    return rv;
}



// ****************************************************************************
//  Method: avtDO_REPL_STRINGSource::SetOutput
//
//  Purpose:
//      Sets the output to be the same as the argument.
//
//  Arguments:
//      dob     The new DO_REPL_STRING.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

void
avtDO_REPL_STRINGSource::SetOutput(avtDO_REPL_STRING_p dob)
{
    output = dob;
}


