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
//    Brad Whitlock, Wed Apr 24 10:18:48 PDT 2002
//    Added constructor and destructor so the vtable gets into the Windows DLL.
//
// ****************************************************************************

class PIPELINE_API avtFlatLighting : public avtLightingModel
{
  public:
    avtFlatLighting();
    virtual ~avtFlatLighting();
    virtual double      GetShading(double, const double[3]) const
                            { return 1.; };

    virtual bool        NeedsGradients(void) { return false; };
};


#endif


