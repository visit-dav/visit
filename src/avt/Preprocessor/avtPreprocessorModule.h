// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtPreprocessorModule.h                           //
// ************************************************************************* //

#ifndef AVT_PREPROCESSOR_MODULE_H
#define AVT_PREPROCESSOR_MODULE_H
#include <prep_exports.h>


#include <avtTerminatingDatasetSink.h>


// ****************************************************************************
//  Class: avtPreprocessorModule
//
//  Purpose:
//      A base class for any module that can be used by a preprocessor.
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
// ****************************************************************************

class PREP_API avtPreprocessorModule : public avtTerminatingDatasetSink
{
  public:
                              avtPreprocessorModule();
    virtual                  ~avtPreprocessorModule();

    virtual void              ProcessCommandLine(char **&argv, int &argc);
    virtual void              SetStem(const char *);

    virtual const char       *GetType(void) = 0;

  protected:
    char                     *stem;

    virtual void              InputIsReady(void);
    virtual void              Preprocess(void) = 0;
};


#endif


