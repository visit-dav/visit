// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Brad Whitlock, Mon Sep 18 11:26:26 PDT 2006
//    Added SetColorTexturingFlag.
//
//    Brad Whitlock, Tue Apr 24 15:59:01 PST 2007
//    Added SetColorTexturingFlagAllowed to disable color texturing from
//    ever taking place if this mapper is being used for a plot should not
//    ever use color texturing.
//
//    Kathleen Biagas, Tue Aug 23 11:42:20 PDT 2016
//    Added virtual inheritance from avtMapper, Removed SetPointSize as
//    this mapper no loner handles points.
//
//    Eric Brugger, Wed Mar  8 16:50:08 PST 2023
//    Moved GetVarRange to avtMapper.
//
// ****************************************************************************

class PLOTTER_API  avtVariableMapper : virtual public avtMapper
{
  public:
                               avtVariableMapper();
    virtual                   ~avtVariableMapper();

    void                       SetLimitsMode(const int);
    void                       SetMin(double);
    void                       SetMinOff(void);
    void                       SetMax(double);
    void                       SetMaxOff(void);
    virtual bool               GetRange(double &, double &);
    virtual bool               GetCurrentRange(double &, double &);

    void                       TurnLightingOn(void);
    void                       TurnLightingOff(void);
    virtual bool               GetLighting(void);

    void                       SetOpacity(double);
    void                       SetLookupTable(vtkLookupTable *);
    void                       SetLineWidth(_LineWidth);

    virtual void               SetColorTexturingFlag(bool);
    void                       SetColorTexturingFlagAllowed(bool);

  protected:
    double                     min, max;
    bool                       setMin, setMax;
    _LineWidth                 lineWidth;
    bool                       lighting;
    double                     opacity;
    int                        limitsMode;
    bool                       colorTexturingFlag;
    bool                       colorTexturingFlagAllowed;
    vtkLookupTable            *lut;

    virtual void               CustomizeMappers(void);

    void                       SetMappersMinMax(void);
};


#endif


