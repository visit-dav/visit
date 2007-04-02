/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// ****************************************************************************

class PLOTTER_API  avtVariableMapper : public avtMapper
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
    bool                       GetVarRange(double &, double &);

    void                       TurnLightingOn(void);
    void                       TurnLightingOff(void);
    virtual bool               GetLighting(void);

    void                       SetOpacity(double);
    void                       SetLookupTable(vtkLookupTable *);
    void                       SetLineWidth(_LineWidth);
    void                       SetLineStyle(_LineStyle);
    void                       SetPointSize(double);

    virtual void               SetColorTexturingFlag(bool);

  protected:
    double                     min, max;
    bool                       setMin, setMax;
    _LineWidth                 lineWidth;
    _LineStyle                 lineStyle;
    bool                       lighting;
    double                     opacity;
    int                        limitsMode;
    bool                       colorTexturingFlag;
    vtkLookupTable            *lut;

    virtual void               CustomizeMappers(void);

    void                       SetMappersMinMax(void);
};


#endif


