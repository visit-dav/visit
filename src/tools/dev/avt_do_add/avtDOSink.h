// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtDO_REPL_STRINGSink.h                          //
// ************************************************************************* //

#ifndef AVT_DO_REPL_STRING_SINK_H
#define AVT_DO_REPL_STRING_SINK_H

#include <pipeline_exports.h>

#include <avtDataObjectSink.h>
#include <avtDO_REPL_STRING.h>


// ****************************************************************************
//  Class: avtDO_REPL_STRINGSink
//
//  Purpose:
//      The sink to an avt pipeline.  This could be a transient sink (filter)
//      or a terminating sink (a true end of the pipeline).
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

class PIPELINE_API avtDO_REPL_STRINGSink : virtual public avtDataObjectSink
{
  public:
                                    avtDO_REPL_STRINGSink();
    virtual                        ~avtDO_REPL_STRINGSink();

  protected:
    virtual avtDataObject_p         GetInput(void);
    virtual void                    SetTypedInput(avtDataObject_p);
    avtDO_REPL_STRING_p             GetTypedInput(void) { return input; };

  private:
    avtDO_REPL_STRING_p             input;
};


#endif


