// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtHexahedron20Extractor.h                       //
// ************************************************************************* //

#ifndef AVT_HEXAHEDRON20_EXTRACTOR_H
#define AVT_HEXAHEDRON20_EXTRACTOR_H

#include <pipeline_exports.h>

#include <avtHexahedronExtractor.h>


// ****************************************************************************
//  Class: avtHexahedron20Extractor
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

class PIPELINE_API avtHexahedron20Extractor : public avtHexahedronExtractor
{
  public:
    //
    // Typedef for the different method of treating the domain. The
    // domain can be:
    //
    // avtHex20Constant:  Approximate the domain using a single linear hex 
    //                    with vertices 0-7 as corners
    // avtHex20Linear:    Approximate the domain using eight linear hexes 
    //                    where the face vertices are the average of the 
    //                    appropriate edges and the center the average of the 
    //                    size face centers
    // avtHex20Quadratic: Compute the correct quadratic domain for sampling
    //
    enum DomainAppStyle 
    {
        avtHex20Constant = 0,
        avtHex20Linear,
        avtHex20Quadratic
    };

    // Default domain approximation
    static const DomainAppStyle DEFAULT_DOMAIN_APPROXIMATION;

                     avtHexahedron20Extractor(int, int, int, avtVolume *,
                                              avtCellList *);
    virtual         ~avtHexahedron20Extractor();

    void             Extract(const avtHexahedron20 &);
   
    void             ConstantHexExtract(const avtHexahedron20 &);
    void             LinearHexExtract(const avtHexahedron20 &);
    void             QuadraticHexExtract(const avtHexahedron20 &);


    virtual void     StoreRay(int, int, int, int,
                              const double (*)[AVT_VARIABLE_LIMIT]);
    double            TriLinearWeight(int, double, double, double);
    double            QuadraticWeight(int, double, double, double);
    double            ClosestParametricNeighbor(double, double, double);

  protected:
    static const int            sControlPointParam[27][3];
    static const unsigned char  sSubHexCorners[8][8];
    const avtHexahedron20      *currentHex;
    DomainAppStyle              domainApproximation;
};


#endif


