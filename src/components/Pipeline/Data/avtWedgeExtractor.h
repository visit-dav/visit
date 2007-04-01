// ************************************************************************* //
//                             avtWedgeExtractor.h                           //
// ************************************************************************* //

#ifndef AVT_WEDGE_EXTRACTOR_H
#define AVT_WEDGE_EXTRACTOR_H

#include <pipeline_exports.h>

#include <avtExtractor.h>


// ****************************************************************************
//  Class: avtWedgeExtractor
//
//  Purpose:
//      Extracts sample points from a wedge.  It assumes that the wedges it has
//      been given are in camera space and does not try to populate points that
//      are not in the cube [-1, 1], [-1, 1], [-1, 1].
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

class PIPELINE_API avtWedgeExtractor : public avtExtractor
{
  public:
                     avtWedgeExtractor(int, int, int, avtVolume *,
                                       avtCellList *);
    virtual         ~avtWedgeExtractor();

    void             Extract(const avtWedge &);

  protected:
    static int       triangulationTables[64][13];
    static int       verticesFromEdges[9][2];
};


#endif


