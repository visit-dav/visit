// ************************************************************************* //
//                               avtDrawable.h                               //
// ************************************************************************* //

#ifndef AVT_DRAWABLE_H
#define AVT_DRAWABLE_H

#include <plotter_exports.h>

#include <ref_ptr.h>

#include <avtDataObject.h>


class     vtkRenderer;
class     ColorAttribute;
class     avtExternallyRenderedImagesActor;
class     avtTransparencyActor;


// ****************************************************************************
//  Class: avtDrawable
//
//  Purpose:
//      A base class that provides an interface for adding and removing plots
//      independent of whether the drawable is a geometry drawable or an
//      image drawable.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Fri May 24 15:25:51 PDT 2002
//    Added GetDataset.
//
//    Hank Childs, Sun May 26 18:37:23 PDT 2002
//    Replaced GetDataset with more general GetDataObject.
//
//    Hank Childs, Sun Jul  7 14:16:38 PDT 2002
//    Add support for transparency.
//
//    Kathleen Bonnell, Fri Jul 12 16:21:37 PDT 2002  
//    Added method ScaleByVector. 
//
//    Kathleen Bonnell, Fri Jul 19 08:39:04 PDT 2002 
//    Added method UpdateScaleFactor. 
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002   
//    Added methods in support of lighting.
//
//    Brad Whitlock, Mon Sep 23 15:55:16 PST 2002
//    Added a method to support changing surface representations. I added
//    another method to set the drawable's immediate rendering mode.
//
//    Jeremy Meredith, Fri Nov 14 11:10:53 PST 2003
//    Added a method to set the specular properties.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PLOTTER_API avtDrawable
{
  public:
                               avtDrawable();
    virtual                   ~avtDrawable();

    virtual bool               Interactive(void) = 0;

    virtual void               Add(vtkRenderer *) = 0;
    virtual void               Remove(vtkRenderer *) = 0;

    virtual void               VisibilityOn(void) = 0;
    virtual void               VisibilityOff(void) = 0;
    virtual int                SetTransparencyActor(avtTransparencyActor*) = 0;
    virtual void               SetExternallyRenderedImagesActor(
                                   avtExternallyRenderedImagesActor*) = 0;

    virtual void               ShiftByVector(const float [3]) = 0;
    virtual void               ScaleByVector(const float [3]) = 0;
    virtual void               UpdateScaleFactor(void) = 0;

    virtual void               TurnLightingOn(void) = 0;
    virtual void               TurnLightingOff(void) = 0;
    virtual void               SetAmbientCoefficient(const float) = 0;

    virtual void               SetSurfaceRepresentation(int rep) {;};
    virtual void               SetImmediateModeRendering(bool val) {;};

    virtual void               SetSpecularProperties(bool,float,float,
                                                     const ColorAttribute&) {;}

    virtual avtDataObject_p    GetDataObject(void) = 0;
};


typedef ref_ptr<avtDrawable> avtDrawable_p;


#endif


