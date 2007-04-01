// ************************************************************************* //
//                                    avtMapper.h                            //
// ************************************************************************* //

#ifndef AVT_MAPPER_H
#define AVT_MAPPER_H

#include <plotter_exports.h>

#include <avtOriginatingDatasetSink.h>
#include <avtDrawable.h>


class   vtkActor;
class   vtkDataObjectCollection;
class   vtkDataSetMapper;

class   avtTransparencyActor;


// ****************************************************************************
//  Class:  avtMapper
//
//  Purpose:
//      This takes geometry and makes a drawable by mapping the variable to 
//      colors.
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
// ****************************************************************************

class PLOTTER_API avtMapper : public avtOriginatingDatasetSink
{
  public:
                               avtMapper();
    virtual                   ~avtMapper();

    void                       ReleaseData();
    avtDrawable_p              GetDrawable();
    virtual bool               GetRange(float &, float &);
    virtual bool               GetCurrentRange(float &, float &);

    virtual bool               GetLighting(void);

    virtual void               GlobalLightingOn(void);
    virtual void               GlobalLightingOff(void);
    virtual void               GlobalSetAmbientCoefficient(const float);
    float                      GetGlobalAmbientCoefficient() 
                                   { return globalAmbient; };

    void                       SetImmediateModeRendering(bool val);
    bool                       GetImmediateModeRendering();

    int                        SetTransparencyActor(avtTransparencyActor *);

  protected:
    bool                       immediateMode;
    avtDrawable_p              drawable;
    avtTransparencyActor      *transparencyActor;
    int                        transparencyIndex;

    vtkDataSetMapper         **mappers;
    int                        nMappers;
    vtkActor                 **actors;

    float                      globalAmbient;

    void                       ClearSelf(void);
    void                       SetUpMappers(void);
    void                       SetDefaultRange(void);
    void                       PrepareExtents(void);
    void                       SetUpTransparency(void);

    virtual void               ChangedInput(void);
    virtual void               CustomizeMappers(void) = 0;
    virtual void               MapperChangedInput(void);
    virtual void               InputIsReady(void);

    virtual void               SetUpFilters(int nDoms);
    virtual vtkDataSet        *InsertFilters(vtkDataSet *, int dom);

    virtual vtkDataSetMapper  *CreateMapper(void);
    virtual void               SetLabels(vector<string> &, bool);
};


#endif


