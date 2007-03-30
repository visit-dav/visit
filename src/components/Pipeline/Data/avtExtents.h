// ************************************************************************* //
//                                 avtExtents.h                              //
// ************************************************************************* //

#ifndef AVT_EXTENTS_H
#define AVT_EXTENTS_H
#include <pipeline_exports.h>


class   avtDataObjectString;
class   avtDataObjectWriter;
class   vtkMatrix4x4;

// ****************************************************************************
//  Class: avtExtents
//
//  Purpose:
//      Manages a set of extents.
//
//      The extents are an array of 'dimension' doubles of min/max pairs.
//      Even numbered indices are for min values while odd numbered indices
//      are for max values.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 24 14:18:03 PDT 2001
//    Added Transform function.
//
// ****************************************************************************

class PIPELINE_API avtExtents
{
  public:
                          avtExtents(int);
                          avtExtents(const avtExtents &);
    virtual              ~avtExtents();

    bool                  HasExtents(void);
    int                   GetDimension(void)  { return dimension; };

    void                  CopyTo(double *);
    void                  Set(const double *);
    void                  Clear(void);

    const avtExtents     &operator=(const avtExtents &);

    void                  Merge(const avtExtents &);
    void                  Merge(const double *);

    void                  Write(avtDataObjectString &,
                                const avtDataObjectWriter *);
    int                   Read(char *);

    void                  Transform(vtkMatrix4x4 *);

  protected:
    int                   dimension;
    double               *extents;
};


#endif


