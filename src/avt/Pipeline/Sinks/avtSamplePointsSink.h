// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtSamplePointsSink.h                            //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINTS_SINK_H
#define AVT_SAMPLE_POINTS_SINK_H
#include <pipeline_exports.h>


#include <avtDataObjectSink.h>
#include <avtSamplePoints.h>


// ****************************************************************************
//  Class: avtSamplePointsSink
//
//  Purpose:
//      This is a data object sink whose input is sample points.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun  4 08:55:55 PDT 2001
//    Inherited from avtDataObjectSink.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Tom Fogal, Tue Jun 23 20:00:22 MDT 2009
//    Added const version of GetInput.
//
// ****************************************************************************

class PIPELINE_API avtSamplePointsSink : virtual public avtDataObjectSink
{
  public:
                                    avtSamplePointsSink();
    virtual                        ~avtSamplePointsSink();

  protected:
    virtual avtDataObject_p         GetInput(void);
    virtual const avtDataObject_p   GetInput(void) const;
    virtual void                    SetTypedInput(avtDataObject_p);
    avtSamplePoints_p               GetTypedInput(void) { return input; };

  private:
    avtSamplePoints_p               input;
};


#endif


