// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtPointExtractor.h                          //
// ************************************************************************* //

#ifndef AVT_POINT_EXTRACTOR_H
#define AVT_POINT_EXTRACTOR_H

#include <pipeline_exports.h>

#include <avtExtractor.h>


// ****************************************************************************
//  Class: avtPointExtractor
//
//  Purpose:
//      Extracts sample points using a kernel based technique.
//
//  Programmer: Hank Childs
//  Creation:   January 24, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Feb 24 16:07:45 PST 2006
//    Add weight cutoff.
//
// ****************************************************************************

class PIPELINE_API avtPointExtractor : public avtExtractor
{
  public:
                     avtPointExtractor(int, int, int, avtVolume *,
                                             avtCellList *);
    virtual         ~avtPointExtractor();

    void             Extract(const avtPoint &);
    static double    GetMinimumWeightCutoff(void) { return 0.01; };

  protected:
    double            epsilon;
    double            correction;
    double            smallestX;
    double            smallestY;
    double            smallestZ;
};


#endif


