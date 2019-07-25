// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtLightingModel.h                            //
// ************************************************************************* //

#ifndef AVT_LIGHTING_MODEL
#define AVT_LIGHTING_MODEL

#include <pipeline_exports.h>

#include <LightList.h>

class     avtRay;


// ****************************************************************************
//  Class: avtLightingModel
//
//  Purpose:
//      An abstract type that defines a lighting model.  It is only an
//      interface.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Apr 24 10:18:48 PDT 2002
//    Added constructor and destructor so the vtable gets into the Windows DLL.
//
// ****************************************************************************

class PIPELINE_API avtLightingModel
{
  public:
                      avtLightingModel();
    virtual          ~avtLightingModel();

    virtual void      AddLighting(int, const avtRay *, unsigned char *) 
                             const = 0;
    virtual void      AddLightingHeadlight(int, const avtRay *, unsigned char *, double alpha, double matProperties[4]) 
                             const = 0;

    void              SetGradientVariableIndex(int gvi)
                             { gradientVariableIndex = gvi; };
    void              SetViewDirection(double *vd)
                             { view_direction[0] = vd[0];
                               view_direction[1] = vd[1];
                               view_direction[2] = vd[2]; 
                               ComputeViewRight();
                             };
    void              SetViewUp(double *vu)
                             { view_up[0] = vu[0];
                               view_up[1] = vu[1];
                               view_up[2] = vu[2];
                               ComputeViewRight(); 
                             };
    void              SetLightInfo(const LightList &ll)
                             { lights = ll; };
    void              SetSpecularInfo(bool ds, double sc, double sp)
                             { doSpecular = ds; specularCoeff = sc;
                               specularPower = sp; };

    void              ComputeViewRight()
                             {
                                view_right[0] = view_direction[1]*view_up[2] - view_direction[2]*view_up[1];
                                view_right[1] = view_direction[2]*view_up[0] - view_direction[0]*view_up[2];
                                view_right[2] = view_direction[0]*view_up[1] - view_direction[1]*view_up[0];
                             }

  protected:
    int               gradientVariableIndex;
    double            view_direction[3];
    double            view_up[3];
    double            view_right[3];
    LightList         lights;
    bool              doSpecular;
    double            specularCoeff;
    double            specularPower;
};


#endif


