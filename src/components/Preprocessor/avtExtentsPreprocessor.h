// ************************************************************************* //
//                           avtExtentsPreprocessor.h                        //
// ************************************************************************* //

#ifndef AVT_EXTENTS_PREPROCESSOR_H
#define AVT_EXTENTS_PREPROCESSOR_H
#include <prep_exports.h>


#include <avtStreamerPreprocessor.h>


// ****************************************************************************
//  Class: avtExtentsPreprocessor
//
//  Purpose:
//      Determines the extents of each domain as well as all the extents for
//      the whole dataset.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

class PREP_API avtExtentsPreprocessor : public avtStreamerPreprocessor
{
  public:
                              avtExtentsPreprocessor();
    virtual                  ~avtExtentsPreprocessor();

    virtual const char       *GetType(void)
                                  { return "avtExtentsPreprocessor"; };

  protected:
    double                   *varExtents;
    double                   *spatialExtents;
    int                       numDomains;

    virtual void              Initialize(int);
    virtual void              ProcessDomain(vtkDataSet *, int);
    virtual void              Finalize(void);
};


#endif


