// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtDO_REPL_STRINGSink.C                         //
// ************************************************************************* //

#include <avtDO_REPL_STRINGSink.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSink constructor
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRINGSink::avtDO_REPL_STRINGSink()
{
    input = NULL;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSink destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRINGSink::~avtDO_REPL_STRINGSink()
{
    ;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSink::SetTypedInput
//
//  Purpose:
//      Sets the input of the sink and performs some type checking.
//
//  Arguments:
//      in      The image as a data object.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

void
avtDO_REPL_STRINGSink::SetTypedInput(avtDataObject_p in)
{
    if (*in != NULL && strcmp(in->GetType(), "avtDO_REPL_STRING") != 0)
    {
        //
        // Should create a new exception here, but I'm under time constraints.
        //
        debug1 << "Looking for avtDO_REPL_STRING, but found type \""
               << in->GetType() << "\"." << endl;
        EXCEPTION0(ImproperUseException);
    }

    CopyTo(input, in);
}


// ****************************************************************************
//  Method: avtDO_REPL_STRINGSink::GetInput
//
//  Purpose:
//      Gets the input of the sink (properly typed as an avtDataObject).
//
//  Returns:    The input of the sink.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDataObject_p
avtDO_REPL_STRINGSink::GetInput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, input);
    return rv;
}


