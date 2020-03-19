// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtDO_REPL_STRINGSource.h                         //
// ************************************************************************* //

#ifndef AVT_DO_REPL_STRING_SOURCE_H
#define AVT_DO_REPL_STRING_SOURCE_H

#include <pipeline_exports.h>

#include <avtDataObjectSource.h>
#include <avtDO_REPL_STRING.h>


// ****************************************************************************
//  Class: avtDO_REPL_STRINGSource
//
//  Purpose:
//      A data object source whose data object is an image.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

class PIPELINE_API avtDO_REPL_STRINGSource : virtual public avtDataObjectSource
{
  public:
                                avtDO_REPL_STRINGSource();
    virtual                    ~avtDO_REPL_STRINGSource();

    virtual avtDataObject_p     GetOutput(void);
    avtDO_REPL_STRING_p         GetTypedOutput(void) { return output; };

  protected:
    avtDO_REPL_STRING_p         output;

    void                        SetOutput(avtDO_REPL_STRING_p);
};


#endif


