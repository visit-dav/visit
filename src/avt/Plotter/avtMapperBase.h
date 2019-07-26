// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtMapperBase.h                                   //
// ************************************************************************* //

#ifndef AVT_MAPPERBASE_H
#define AVT_MAPPERBASE_H

#include <plotter_exports.h>

#include <avtTerminatingDatasetSink.h>
#include <avtDrawable.h>

#include <vector>
#include <string>

class   vtkAlgorithmOutput;

class   avtTransparencyActor;
class   ColorAttribute;


// ****************************************************************************
//  Class:  avtMapperBase
//
//  Purpose:
//      This takes geometry and makes a drawable by mapping the variable to
//      colors.
//
//  Notes:
//      Pulled from avtMapper. KSB April 13, 2017
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Mar 15 19:15:10 PST 2001
//    Added members nRenderingModes, modeVisibility,
//    modeRepresentation, and supporting Set methods.
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001
//    Removed methods setting Mode Visibility, Representation,
//    nRenderingModes.  These are no longer needed.
//
//    Kathleen Bonnell, Mon Sep 24 08:27:42 PDT 2001
//    Added virtual method SetLabels.
//
//    Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001
//    Added GetCurrentRange.
//
//    Hank Childs, Sun Jul  7 12:31:10 PDT 2002
//    Added support for transparency.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    Added support for lighting.
//
//    Brad Whitlock, Mon Sep 23 16:54:10 PST 2002
//    Added ability to toggle immediate mode rendering.
//
//    Kathleen Bonnell, Sat Oct 19 15:08:41 PDT 2002
//    Added storage for the global Ambient coefficient, and a method
//    to retrieve it.
//
//    Mark C. Miller Tue May 11 20:21:24 PDT 2004
//    Removed extRenderdImagesActor data member and method to set it
//
//    Kathleen Bonnell, Thu Sep  2 11:44:09 PDT 2004
//    Added specularIsInappropriate to control whether or not specular
//    properties get applied.  Moved SetSpecularProperties and
//    SetSurfaceRepresentation from avtGeometryDrawable so that derived
//    mappers may override.
//
//    Brad Whitlock, Mon Jul 24 13:53:46 PST 2006
//    Added SetFullFrameScaling.
//
//    Brad Whitlock, Mon Sep 18 11:26:26 PDT 2006
//    Added SetColorTexturingFlag.
//
//    Brad Whitlock, Wed Aug 22 11:32:25 PDT 2007
//    Added reduced detail mode.
//
//    Kathleen Biagas, Wed Feb 6 19:38:27 PDT 2013
//    Changed signature of InsertFilters.
//
//    Kathleen Biagas, Wed Apr 13 18:02:22 MST 2016
//    Added InvalidateTransparencyCache.
//
//    Kathleen Biagas, Tue Jul 12 13:27:25 MST 2016
//    Simplify this class by removing surface/wireframe/points settings.
//    Add SetUpTransparencyActor.
//
//    Alister Maguire, Tue Jun  5 10:08:59 PDT 2018
//    Added SetViewScale. 
//
// ****************************************************************************

class PLOTTER_API avtMapperBase : public avtTerminatingDatasetSink
{
  public:
                               avtMapperBase();
    virtual                   ~avtMapperBase();

    void                       ReleaseData();
    avtDrawable_p              GetDrawable();
    virtual bool               GetRange(double &, double &);
    virtual bool               GetCurrentRange(double &, double &);

    virtual bool               GetLighting(void);

    virtual void               GlobalLightingOn(void) = 0;
    virtual void               GlobalLightingOff(void) = 0;
    virtual void               GlobalSetAmbientCoefficient(const double) = 0;
    double                     GetGlobalAmbientCoefficient()
                                   { return globalAmbient; }


    virtual int                SetTransparencyActor(avtTransparencyActor *) = 0;

    void                       SetSpecularIsInappropriate(bool val)
                                   { specularIsInappropriate = val; }
    bool                       GetSpecularIsInappropriate()
                                   { return specularIsInappropriate; }

    virtual void               SetSurfaceRepresentation(int) {;}
    virtual void               SetSpecularProperties(bool,double,double,
                                                     const ColorAttribute&) = 0;

    virtual void               SetColorTexturingFlag(bool) {; }

    virtual bool               SetFullFrameScaling(bool, const double *)
                                   { return false; };

    virtual bool               SetViewScale(const double) { return false; };

    virtual void               SetAlternateDisplay(void *) {; }

    virtual void               ReducedDetailModeOn() {; }
    virtual bool               ReducedDetailModeOff() { return false; }

    void                       InvalidateTransparencyCache(void);
    virtual bool               ActorIsShiftable(int) { return true; }

  protected:
    bool                       specularIsInappropriate;
    avtDrawable_p              drawable;
    avtTransparencyActor      *transparencyActor;
    int                        transparencyIndex;

    int                        nMappers;

    double                     globalAmbient;

    virtual void               ClearSelf(void);
    void                       SetUpMappers(void);
    virtual void               CreateActorMapperPairs(vtkDataSet **) = 0;
    void                       PrepareExtents(void);
    virtual void               SetUpTransparencyActor(void);
    virtual void               SetUpDrawable(void) = 0;

    virtual void               ChangedInput(void);
    virtual void               CustomizeMappers(void) = 0;
    virtual void               MapperChangedInput(void);
    virtual void               InputIsReady(void);

    virtual void               SetLabels(std::vector<std::string> &, bool);
};


#endif

