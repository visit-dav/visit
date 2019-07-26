// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtIntegrationRF.h                           //
// ************************************************************************* //

#ifndef AVT_INTEGRATION_RF_H
#define AVT_INTEGRATION_RF_H

#include <pipeline_exports.h>

#include <avtRayFunction.h>


// ****************************************************************************
//  Class: avtIntegrationRF
//
//  Purpose:
//      The derived type of ray function that will perform the integration
//      of $int_0^{maxX} \rho(X) dx$ for each ray.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2006
//
//  Modifications:
//
//    Hank Childs, Tue Mar 13 16:13:05 PDT 2007
//    Add support for setting distance along the plane.
//
//    Hank Childs, Sun Aug 31 08:42:03 PDT 2008
//    Modify the signature of GetRayValue.
//
// ****************************************************************************

class PIPELINE_API avtIntegrationRF : public avtRayFunction
{
  public:
                        avtIntegrationRF(avtLightingModel *);
    virtual            ~avtIntegrationRF();

    virtual void        GetRayValue(const avtRay *,
                                    unsigned char rgb[3], double);
    virtual bool        NeedPixelIndices(void) { return true; };

    static void         SetWindowSize(int, int);
    static void         OutputRawValues(const char *);

    void                SetRange(double m1, double m2)
                                  { min = m1; max = m2; };
    void                SetDistance(double d) { distance = d; };

  protected:
    static int          windowSize[2];
    static double      *vals;
    static bool         issuedWarning;
    double              min, max;
    double              distance;
};


#endif


