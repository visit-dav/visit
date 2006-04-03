// ************************************************************************* //
//                              VisWinLighting.h                              //
// ************************************************************************* //

#ifndef VIS_WIN_LIGHTING_H
#define VIS_WIN_LIGHTING_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>
#include <avtLightList.h>

#define MAX_LIGHTS 8

class     vtkLight;


// ****************************************************************************
//  Class: VisWinLighting
//
//  Purpose:
//      Manages the lighting for the window. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 27 13:25:42 PDT 2002
//    Add methods Start3DMode, Stop3DMode, UpdateVTKLights, InitDefaultLight.  
//    Removed argument from UpdateLightPositions, as the avtLightList is now 
//    stored as a member.
//
// ****************************************************************************

class VISWINDOW_API VisWinLighting : public VisWinColleague
{
  public:
                                  VisWinLighting(VisWindowColleagueProxy &);
    virtual                      ~VisWinLighting();

    void                          SetLightList(avtLightList & );

    double                         GetAmbientCoefficient();
    bool                          GetAmbientOn();
    int                           GetNumLightsEnabled();
    void                          UpdateLightPositions();

    virtual void                  Start3DMode();
    virtual void                  Stop3DMode();

  private:
    void                          UpdateVTKLights();
    void                          InitDefaultLight();

  protected:
    avtLightList                  avtlights;
    vtkLight                     *lights[MAX_LIGHTS];
    double                         ambientCoefficient;
    bool                          ambientOn;
    int                           numLightsEnabled;
};


#endif


