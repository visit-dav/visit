// ************************************************************************* //
//                           avtHexahedronExtractor.h                        //
// ************************************************************************* //

#ifndef AVT_HEXAHEDRON_EXTRACTOR_H
#define AVT_HEXAHEDRON_EXTRACTOR_H
#include <pipeline_exports.h>


#include <avtExtractor.h>


// ****************************************************************************
//  Class: avtHexahedronExtractor
//
//  Purpose:
//      Extracts sample points from hexahedron.  It assumes that the
//      hexahedrons it has been given are in camera space and does not try to
//      populate points that are not in the cube [-1, 1], [-1, 1], [-1, 1].
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
// ****************************************************************************

class PIPELINE_API avtHexahedronExtractor : public avtExtractor
{
  public:
                     avtHexahedronExtractor(int, int, int, avtVolume *,
                                            avtCellList *);
    virtual         ~avtHexahedronExtractor() {;};

    void             Extract(const avtHexahedron &);

  protected:
    static int       triangulationTables[256][16];
    static int       verticesFromEdges[12][2];
};


#endif


