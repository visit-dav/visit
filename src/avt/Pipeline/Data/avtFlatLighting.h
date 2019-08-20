// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtFlatLighting.h                             //
// ************************************************************************* //

#ifndef AVT_FLAT_LIGHTING_H
#define AVT_FLAT_LIGHTING_H

#include <pipeline_exports.h>

#include <avtLightingModel.h>


// ****************************************************************************
//  Class: avtFlatLighting
//
//  Purpose:
//      Defines a lighting model for flat lighting.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Brad Whitlock, Wed Apr 24 10:18:48 PDT 2002
//    Added constructor and destructor so the vtable gets into the Windows DLL.
//
//    Hank Childs, Sun Aug 31 10:18:24 PDT 2008
//    Change the method that does the actual lighting.
//
// ****************************************************************************

class PIPELINE_API avtFlatLighting : public avtLightingModel
{
  public:
                            avtFlatLighting();
    virtual                ~avtFlatLighting();

    virtual void            AddLighting(int, const avtRay *, unsigned char *)
                                 const;

    virtual void            AddLightingHeadlight(int, const avtRay *, unsigned char *, double alpha, double matProperties[4]) 
                  const;
};


#endif


