// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtDO_REPL_STRING.C                           //
// ************************************************************************* //

#include <avtDO_REPL_STRING.h>

#include <avtDO_REPL_STRINGSource.h>


// ****************************************************************************
//  Method: avtDO_REPL_STRING constructor
//
//  Arguments:
//      src     An data object source that is the upstream object.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRING::avtDO_REPL_STRING(avtDataObjectSource *src)
    : avtDataObject(src)
{
    ;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRING::~avtDO_REPL_STRING()
{
    ;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING::InstantiateWriter
//
//  Purpose:
//      Instantiates a writer that is appropriate for an avtDO_REPL_STRING.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

/*
avtDataObjectWriter *
avtDO_REPL_STRING::InstantiateWriter(void)
{
    return new avtDO_REPL_STRINGWriter;
}
 */


// ****************************************************************************
//  Method: avtDO_REPL_STRING::ReleaseData
//
//  Purpose:
//      Free the data associated with this DO_REPL_STRING.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

void
avtDO_REPL_STRING::ReleaseData(void)
{
    YOU MUST DEFINE WHAT IT MEANS TO RELEASE DATA FOR YOUR DATA OBJECT.
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING::Instance
//
//  Purpose:
//      Creates an instance of an avtDO_REPL_STRING.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDataObject *
avtDO_REPL_STRING::Instance(void)
{
    avtDataObjectSource *src = NULL;
    avtDO_REPL_STRING *new_do = new avtDO_REPL_STRING(src);
    return new_do;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING::DerivedCopy
//
//  Purpose:
//      Copy over the image.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

void
avtDO_REPL_STRING::DerivedCopy(avtDataObject *dob)
{
    avtDO_REPL_STRING *typed_dob = (avtDO_REPL_STRING *) dob;
    YOU MUST IMPLEMENT HOW TO COPY THE DATA MEMBERS FROM typed_dob INTO
    "this" OBJECT.
    AS IN:
    data_member1 = typed_dob->data_member1;
    ...
    data_memberN = typed_dob->data_memberN;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING::GetNumberOfCells
//
//  Purpose:
//      Estimates the number of cells needed to render this object.  This is
//      used for determining if VisIt should go into its scalable rendering
//      mode.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

long long
avtDO_REPL_STRING::GetNumberOfCells(bool onlyWantPolygonCount) const
{
   YOU MUST IMPLEMENT THIS.  IF YOU WILL NEVER BE RENDERING YOUR DATA OBJECT
   (BECAUSE YOU WILL ALWAYS BE CONVERTING TO AN avtDataset), THEN JUST RETURN 0.
}
