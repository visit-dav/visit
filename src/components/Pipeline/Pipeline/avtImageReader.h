// ************************************************************************* //
//                              avtImageReader.h                             //
// ************************************************************************* //

#ifndef AVT_IMAGE_READER_H
#define AVT_IMAGE_READER_H
#include <pipeline_exports.h>


#include <avtImageRepresentation.h>
#include <avtTerminatingImageSource.h>

#include <ref_ptr.h>


// ****************************************************************************
//  Class: avtImageReader
//
//  Purpose:
//      A class which takes as input an avtImage and can serialize it.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 18, 2000 
//
//  Modifications:
//
//    Hank Childs, Thu Dec 28 17:27:01 PST 2000
//    Pushed some functionality into avtDataObjectReader.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtImageReader : public avtTerminatingImageSource
{
  public:
                                avtImageReader();
    virtual                    ~avtImageReader();

    int                         Read(char *input);

  protected:
    bool                        haveReadImage;
    avtImageRepresentation      imgRep;

    virtual bool                FetchImage(avtDataSpecification_p,
                                           avtImageRepresentation &);
};

typedef ref_ptr<avtImageReader> avtImageReader_p;

#endif

