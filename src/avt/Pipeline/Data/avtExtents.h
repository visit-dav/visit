// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtExtents.h                              //
// ************************************************************************* //

#ifndef AVT_EXTENTS_H
#define AVT_EXTENTS_H

#include <pipeline_exports.h>

#include <visitstream.h>

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
//
//    Jeremy Meredith, Mon Sep 24 14:18:03 PDT 2001
//    Added Transform function.
//
//    Hank Childs, Wed Feb 25 09:00:17 PST 2004
//    Added Print method.
//
//    Hank Childs, Tue Dec 18 11:38:40 PST 2007
//    Remove const return type of assignment operator to ensure that compiler
//    doesn't define a second assignment operator with a non-const return
//    type that does a bitwise copy.
//
//    Tom Fogal, Tue Jun 23 20:14:16 MDT 2009
//    I made some methods const.
//
//    Hank Childs, Sun Nov 28 17:00:06 PST 2010
//    Add a destruct method for void_ref_ptrs.
//
//    Eric Brugger, Thu Oct 27 09:58:47 PDT 2011
//    Add const to the CopyTo method.
//
// ****************************************************************************

class PIPELINE_API avtExtents
{
  public:
                          avtExtents(int);
                          avtExtents(const avtExtents &);
    virtual              ~avtExtents();

    void                  Print(ostream &) const;
    static void           Destruct(void *);

    bool                  HasExtents(void) const;
    int                   GetDimension(void)  { return dimension; };

    void                  CopyTo(double *) const;
    void                  Set(const double *);
    void                  Clear(void);

    avtExtents           &operator=(const avtExtents &);

    void                  Merge(const avtExtents &);
    void                  Merge(const double *);

    void                  Write(avtDataObjectString &,
                                const avtDataObjectWriter *) const;
    int                   Read(char *);

    void                  Transform(vtkMatrix4x4 *);

  protected:
    int                   dimension;
    double               *extents;
};

#endif
