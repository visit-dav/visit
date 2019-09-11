// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtDataObjectSink.h                             //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_SINK_H
#define AVT_DATA_OBJECT_SINK_H

#include <pipeline_exports.h>

#include <avtDataObject.h>

#include <avtContract.h>


// ****************************************************************************
//  Class: avtDataObjectSink
//
//  Purpose:
//      The sink for a data object.  This can be either an originating sink
//      (one that originates a pipeline) or part of a filter.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Sep 12 10:08:46 PDT 2001
//    Added UpdateInput.
//
//    Hank Childs, Mon Oct  1 14:13:09 PDT 2001
//    Made GetInput public.  Needed for comparisons.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Tom Fogal, Tue Jun 23 19:54:03 MDT 2009
//    Added a const version of GetInput.
//
//    Kathleen Biagas, Mon Jun  5 16:27:45 PDT 2017
//    Added ResetAllExtents.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectSink
{
    friend class                  avtMultipleInputSink;

  public:
                                  avtDataObjectSink();
    virtual                      ~avtDataObjectSink();

    void                          SetInput(avtDataObject_p);
    virtual avtDataObject_p       GetInput(void) = 0;
    virtual const avtDataObject_p GetInput(void) const = 0;

  protected:
    virtual void                  SetTypedInput(avtDataObject_p) = 0;
    virtual void                  ChangedInput(void);
    virtual bool                  UpdateInput(avtContract_p);
    virtual void                  ResetAllExtents(void);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                            avtDataObjectSink(const avtDataObjectSink &) {;};
    avtDataObjectSink      &operator=(const avtDataObjectSink &) 
                                                           { return *this; };
};


#endif


