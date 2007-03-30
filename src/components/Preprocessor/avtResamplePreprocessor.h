// ************************************************************************* //
//                            avtResamplePreprocessor.h                      //
// ************************************************************************* //

#ifndef AVT_RESAMPLE_PREPROCESSOR_H
#define AVT_RESAMPLE_PREPROCESSOR_H
#include <prep_exports.h>


#include <avtPreprocessorModule.h>


// ****************************************************************************
//  Class: avtResamplePreprocessor
//
//  Purpose:
//      A preprocessor module that will resample the input.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

class PREP_API avtResamplePreprocessor : public avtPreprocessorModule
{
  public:
                          avtResamplePreprocessor();
    virtual              ~avtResamplePreprocessor();

    virtual const char   *GetType(void) { return "avtResamplePreprocessor"; };

  protected:
    virtual void          Preprocess(void);
};


#endif


