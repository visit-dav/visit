// ************************************************************************* //
//                           avtImageCommunicator.h                          //
// ************************************************************************* //

#ifndef AVT_IMAGE_COMMUNICATOR_H
#define AVT_IMAGE_COMMUNICATOR_H

#include <pipeline_exports.h>

#include <avtImageToImageFilter.h>

class     avtImagePartition;


// ****************************************************************************
//  Class: avtImageCommunicator
//
//  Purpose:
//      Communicates complete portions of images together to make one final
//      image.  This should be viewed as a relatively simple image compositer
//      in which the various images that are contributing to the output
//      do NOT overlap. In fact, they form a partition.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2001
//
// ****************************************************************************

class PIPELINE_API avtImageCommunicator : public avtImageToImageFilter
{
  public:
                       avtImageCommunicator();
    virtual           ~avtImageCommunicator() {;};

    const char        *GetType(void) { return "avtImageCommunicator"; };
    const char        *GetDescription(void) { return "Collecting image"; };

    void               SetImagePartition(avtImagePartition *);

  protected:
    int                numProcs;
    int                myRank;

    avtImagePartition *imagePartition;

    void               Execute(void);

    char              *SerializeImage(void);
    void               UnserializeImage(void);
};


#endif

    
