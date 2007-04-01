// ************************************************************************* //
//                             avtPyramidExtractor.h                         //
// ************************************************************************* //

#ifndef AVT_PYRAMID_EXTRACTOR_H
#define AVT_PYRAMID_EXTRACTOR_H

#include <pipeline_exports.h>

#include <avtExtractor.h>


// ****************************************************************************
//  Class: avtPyramidExtractor
//
//  Purpose:
//      Extracts sample points from a pyramid.  It assumes that the
//      pyramids have been given in camera space and does not try to
//      populate points that are not in the cube [-1, 1], [-1, 1], [-1, 1].
//
//  Programmer: Hank Childs
//  Creation:   December 11, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtPyramidExtractor : public avtExtractor
{
  public:
                     avtPyramidExtractor(int, int, int, avtVolume *,
                                         avtCellList *);
    virtual         ~avtPyramidExtractor();

    void             Extract(const avtPyramid &);

  protected:
    static int       triangulationTables[32][13];
    static int       verticesFromEdges[8][2];
};


#endif


