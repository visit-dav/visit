// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtHexahedron27Extractor.h                       //
// ************************************************************************* //

#ifndef AVT_HEXAHEDRON27_EXTRACTOR_H
#define AVT_HEXAHEDRON27_EXTRACTOR_H

#include <pipeline_exports.h>

#include <avtHexahedronExtractor.h>

// ****************************************************************************
//  Class: avtHexahedron27Extractor
//
//  Purpose:
//      Extracts sample points from a higher order hexahedron.  It assumes 
//      that the hexahedrons it has been given are in camera space and 
//      does not try to populate points that are not in the cube 
//      [-1, 1], [-1, 1], [-1, 1].
//
//  Programmer: Timo Bremer
//  Creation:   September 13, 2007
//
// ****************************************************************************

class PIPELINE_API avtHexahedron27Extractor : public avtHexahedronExtractor
{
  public:
    //
    // Typedef for the different method of treating the domain. The
    // domain can be 
    //
    // avtHex27Constant:  Approximate the domain using a single linear hex 
    //                    with vertices 0-7 as corners
    // avtHex27Lienar:    Approximate the domain using eight linear hexes 
    //                    where the face vertices are the average of the 
    //                    appropriate edges and the center the average of the 
    //                    size face centers
    // avtHex27Quadratic: Compute the correct quadratic domain for sampling
    //
    enum DomainAppStyle 
    {
        avtHex27Constant = 0,
        avtHex27Linear,
        avtHex27Quadratic
    };

  
    static const DomainAppStyle DEFAULT_DOMAIN_APPROXIMATION;
  


                     avtHexahedron27Extractor(int, int, int, avtVolume *,
                                             avtCellList *);
    virtual         ~avtHexahedron27Extractor();

    void             Extract(const avtHexahedron27 &);
    void             ConstantHexExtract(const avtHexahedron27 &);
    void             LinearHexExtract(const avtHexahedron27 &);
    void             QuadraticHexExtract(const avtHexahedron27 &);


    virtual void      StoreRay(int, int, int, int,
                               const double (*)[AVT_VARIABLE_LIMIT]);
    double             TriLinearWeight(int, double, double, double);
    double             QuadraticWeight(int, double, double, double);
    double             ClosestParametricNeighbor(double, double, double);

  protected:
    static const int              sControlPointParam[27][3];
    static const unsigned char    sSubHexCorners[8][8];
    const avtHexahedron27        *currentHex;
    DomainAppStyle                domainApproximation;
};


#endif


