// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     avtDataTreeIteratorPreprocessor.h                     //
// ************************************************************************* //

#ifndef AVT_DATA_TREE_ITERATOR_PREPROCESSOR_H
#define AVT_DATA_TREE_ITERATOR_PREPROCESSOR_H

#include <prep_exports.h>


#include <avtPreprocessorModule.h>

class     vtkDataSet;


// ****************************************************************************
//  Class: avtDataTreeIteratorPreprocessor
//
//  Purpose:
//      A base class for any preprocessor module that would like to have 
//      domains fed through one at a time.
//
//  Programmer: Hank Childs
//  Creation:   September 7, 2001
//
// ****************************************************************************

class PREP_API avtDataTreeIteratorPreprocessor : public avtPreprocessorModule
{
  public:
                              avtDataTreeIteratorPreprocessor();
    virtual                  ~avtDataTreeIteratorPreprocessor();

  protected:
    virtual void              Preprocess(void);

    virtual void              Initialize(int nDomains);
    virtual void              ProcessDomain(vtkDataSet *, int) = 0;
    virtual void              Finalize(void);

    void                      PreprocessTree(avtDataTree_p);
};


#endif


