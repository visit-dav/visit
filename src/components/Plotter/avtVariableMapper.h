// ************************************************************************* //
//                             avtVariableMapper.h                           //
// ************************************************************************* //

#ifndef AVT_VARIABLE_MAPPER_H
#define AVT_VARIABLE_MAPPER_H
#include <plotter_exports.h>


#include <avtMapper.h>
#include <LineAttributes.h>

class vtkLookupTable;

// ****************************************************************************
//  Class: avtVariableMapper
//
//  Purpose:
//      A mapper for variables.  This extends the functionality of a mapper by
//      allowing data ranges to be set, etc.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar 15 14:52:32 PST 2001
//    Added methods SetLUT, SetVisibility, SetLineWidth.
//    Removed Hide/Unhide methods.
//
//    Kathleen Bonnell, Thu Mar 22 15:24:43 PST 2001 
//    Added methods SetLineStyle, SetPointSize.
//
//    Kathleen Bonnell, Mon Apr  2 18:30:33 PDT 2001 
//    Added method GetVarRange, added vtkLookupTable member. 
//
//    Hank Childs, Fri Apr 20 14:00:26 PDT 2001
//    Pushed GetDataRange into base class.
//
//    Brad Whitlock, Thu Jun 14 19:59:47 PST 2001
//    I added the SetLUTColors method.
//
//    Kathleen Bonnell, Thu Jun 21 17:33:08 PDT 2001 
//    Add member stipplePattern. 
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk.
//
//    Kathleen Bonnell, Mon Aug 20 17:53:30 PDT 2001 
//    Removed parameter from method SetLUT, and method
//    to SetVisibility.  It referred to a rendering mode 
//    no longer used.  
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Renamed SetLUT to SetLookupTable for consistency with other
//    mappers.  Removed SetLUTColors. 
//    
//    Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001
//    Added member limitsMode, method SetLimitsMode, GetCurrentRange. 
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002   
//    Added GetLighting virtual method. 
//
// ****************************************************************************

class PLOTTER_API  avtVariableMapper : public avtMapper
{
  public:
                               avtVariableMapper();
    virtual                   ~avtVariableMapper();

    void                       SetLimitsMode(const int);
    void                       SetMin(float);
    void                       SetMinOff(void);
    void                       SetMax(float);
    void                       SetMaxOff(void);
    virtual bool               GetRange(float &, float &);
    virtual bool               GetCurrentRange(float &, float &);
    bool                       GetVarRange(float &, float &);

    void                       TurnLightingOn(void);
    void                       TurnLightingOff(void);
    virtual bool               GetLighting(void);

    void                       SetOpacity(float);
    void                       SetLookupTable(vtkLookupTable *);
    void                       SetLineWidth(_LineWidth);
    void                       SetLineStyle(_LineStyle);
    void                       SetPointSize(float);

  protected:
    float                      min, max;
    bool                       setMin, setMax;
    _LineWidth                 lineWidth;
    _LineStyle                 lineStyle;
    bool                       lighting;
    float                      opacity;
    int                        limitsMode;
    vtkLookupTable            *lut;

    virtual void               CustomizeMappers(void);

    void                       SetMappersMinMax(void);
};


#endif


