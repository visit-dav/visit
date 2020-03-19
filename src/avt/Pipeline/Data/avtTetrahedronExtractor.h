// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtTetrahedronExtractor : public avtExtractor
{
  public:
                     avtTetrahedronExtractor(int, int, int, avtVolume *,
                                             avtCellList *);
    virtual         ~avtTetrahedronExtractor();

    void             Extract(const avtTetrahedron &);

  protected:
    static int       triangulationTables[16][7];
    static int       verticesFromEdges[6][2];
};


#endif


