// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtDatasetSink.h                            //
// ************************************************************************* //

#ifndef AVT_DATASET_SINK_H
#define AVT_DATASET_SINK_H

#include <pipeline_exports.h>

#include <avtDataObjectSink.h>
#include <avtDataset.h>


// ****************************************************************************
//  Class: avtDatasetSink
//
//  Purpose:
//      This is a data object sink whose input is a dataset.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//    Hank Childs, Sun May 26 18:55:01 PDT 2002
//    Make GetInput a public method like it is in the base type.
//
//    Brad Whitlock, Thu Oct 9 15:05:27 PST 2003
//    Fixed the friend declaration so it builds with newer g++ versions.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Tom Fogal, Tue Jun 23 19:57:34 MDT 2009
//    Added const edition of GetInput.
//
// ****************************************************************************

class PIPELINE_API avtDatasetSink : virtual public avtDataObjectSink
{
    friend class                  avtMultipleInputSink;

  public:
                                  avtDatasetSink();
    virtual                      ~avtDatasetSink();

    virtual avtDataObject_p       GetInput(void);
    virtual const avtDataObject_p GetInput(void) const;

  protected:
    avtDataTree_p                 GetInputDataTree();
    avtDataset_p                  GetTypedInput(void) { return input; };

    virtual void                  SetTypedInput(avtDataObject_p);

  private:
    avtDataset_p              input;
};


#endif


