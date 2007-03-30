// ************************************************************************* //
//                             avtLightingModel.h                            //
// ************************************************************************* //

#ifndef AVT_LIGHTING_MODEL
#define AVT_LIGHTING_MODEL
#include <pipeline_exports.h>

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
    virtual ~avtLightingModel();
    virtual double             GetShading(double, const double[3]) const = 0;
    virtual bool               NeedsGradients() = 0;
};


#endif


