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
    float            epsilon;
    float            correction;
    float            smallestX;
    float            smallestY;
    float            smallestZ;
};


#endif


