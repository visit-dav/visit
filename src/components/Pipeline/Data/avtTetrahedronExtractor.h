// ************************************************************************* //
//                           avtTetrahedronExtractor.h                       //
// ************************************************************************* //

#ifndef AVT_TETRAHEDRON_EXTRACTOR_H
#define AVT_TETRAHEDRON_EXTRACTOR_H
#include <pipeline_exports.h>


#include <avtExtractor.h>


// ****************************************************************************
//  Class: avtTetrahedronExtractor
//
//  Purpose:
//      Extracts sample points from a tetrahedron.  It assumes that the
//      tetrahedrons it has been given are in camera space and does not try to
//      populate points that are not in the cube [-1, 1], [-1, 1], [-1, 1].
//
//  Programmer: Hank Childs
//  Creation:   December 11, 2000
//
// ****************************************************************************

class PIPELINE_API avtTetrahedronExtractor : public avtExtractor
{
  public:
                     avtTetrahedronExtractor(int, int, int, avtVolume *,
                                             avtCellList *);
    virtual         ~avtTetrahedronExtractor() {;};

    void             Extract(const avtTetrahedron &);

  protected:
    static int       triangulationTables[16][7];
    static int       verticesFromEdges[6][2];
};


#endif


