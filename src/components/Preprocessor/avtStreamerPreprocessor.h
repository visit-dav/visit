// ************************************************************************* //
//                        avtStreamerPreprocessor.h                          //
// ************************************************************************* //

#ifndef AVT_STREAMER_PREPROCESSOR_H
#define AVT_STREAMER_PREPROCESSOR_H
#include <prep_exports.h>


#include <avtPreprocessorModule.h>

class     vtkDataSet;


// ****************************************************************************
//  Class: avtStreamerPreprocessor
//
//  Purpose:
//      A base class for any preprocessor module that would like to have 
//      domains fed through one at a time.
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
// ****************************************************************************

class PREP_API avtStreamerPreprocessor : public avtPreprocessorModule
{
  public:
                              avtStreamerPreprocessor();
    virtual                  ~avtStreamerPreprocessor();

  protected:
    virtual void              Preprocess(void);

    virtual void              Initialize(int nDomains);
    virtual void              ProcessDomain(vtkDataSet *, int) = 0;
    virtual void              Finalize(void);

    void                      PreprocessTree(avtDataTree_p);
};


#endif


