// ************************************************************************* //
//                        avtTerminatingImageSource.h                        //
// ************************************************************************* //

#ifndef AVT_TERMINATING_IMAGE_SOURCE_H
#define AVT_TERMINATING_IMAGE_SOURCE_H

#include <pipeline_exports.h>

#include <avtImageSource.h>
#include <avtTerminatingSource.h>


// ****************************************************************************
//  Class: avtTerminatingImageSource
//
//  Purpose:
//      A source that terminates a pipeline.  It does an update differently
//      than what a non-terminating source (filter) would.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 18, 2000 
//
//  Modifications:
//
//    Hank Childs, Mon Jun  4 08:02:35 PDT 2001
//    Changed inheritance hierarchy.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtTerminatingImageSource
    : public virtual avtImageSource, public virtual avtTerminatingSource
{
  public:
                                 avtTerminatingImageSource();
    virtual                     ~avtTerminatingImageSource();

  protected:
    virtual bool                 FetchData(avtDataSpecification_p);
    virtual bool                 FetchImage(avtDataSpecification_p,
                                                 avtImageRepresentation &) = 0;
};


#endif


