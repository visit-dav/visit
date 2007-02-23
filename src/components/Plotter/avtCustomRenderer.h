/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                             avtCustomRenderer.h                           //
// ************************************************************************* //

#ifndef AVT_CUSTOM_RENDERER_H
#define AVT_CUSTOM_RENDERER_H

#include <plotter_exports.h>

#include <ref_ptr.h>

#include <avtDataObject.h>
#include <avtViewInfo.h>

class     ColorAttribute;
class     vtkDataSet;
class     vtkRenderer;

typedef void (*OverrideRenderCallback)(void *, avtDataObject_p &);


// ****************************************************************************
//  Class: avtCustomRenderer
//
//  Purpose:
//      An interface that any custom renderer should derive from.
//
//  Programmer: Hank Childs
//  Creation:   March 26, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 19 15:31:00 PST 2001
//    Added hooks for setting extents.
//
//    Hank Childs, Tue Apr 23 18:25:55 PDT 2002
//    Renamed from OpenGL renderer, since it now has derived types that are
//    not OpenGL.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002  
//    Added methods in support of lighting.
//
//    Kathleen Bonnell, Mon Aug  4 11:14:22 PDT 2003 
//    Added member and methods in support of immediate mode rendering.
//
//    Kathleen Bonnell,  Thu Sep  2 11:44:09 PDT 2004
//    Added SetSurfaceRepresentation and SetSpecularProperties. 
//
// ****************************************************************************

class PLOTTER_API avtCustomRenderer
{
  public:
                            avtCustomRenderer();
    virtual                ~avtCustomRenderer();

    void                    Execute(vtkDataSet *);
    void                    SetView(avtViewInfo &);

    void                    SetRange(double, double);

    virtual bool            OperatesOnScalars(void) { return false; };

    virtual void            GlobalLightingOn(void);
    virtual void            GlobalLightingOff(void);
    virtual void            GlobalSetAmbientCoefficient(const double); 

    virtual void            ImmediateModeRenderingOn(void); 
    virtual void            ImmediateModeRenderingOff(void);
    virtual void            SetImmediateModeRendering(bool);
    virtual bool            GetImmediateModeRendering(void);

    virtual void            SetSurfaceRepresentation(int rep);
    virtual void            SetSpecularProperties(bool,double,double,
                                                  const ColorAttribute&);
 

    void                    RegisterOverrideRenderCallback(
                                               OverrideRenderCallback, void *);
    void                    SetVTKRenderer(vtkRenderer *r);
    virtual void            SetAlternateDisplay(void *dpy);

  protected:
    avtViewInfo             view;
    double                  varmin, varmax;
    vtkRenderer            *VTKRen;
    bool                    immediateModeRendering;

    virtual void            Render(vtkDataSet *) = 0;

    OverrideRenderCallback  overrideRenderCallback;
    void                   *overrideRenderCallbackArgs;
};


typedef ref_ptr<avtCustomRenderer> avtCustomRenderer_p;


#endif


