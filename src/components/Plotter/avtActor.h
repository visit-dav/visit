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
//                                 avtActor.h                                //
// ************************************************************************* //

#ifndef AVT_ACTOR_H
#define AVT_ACTOR_H

#include <plotter_exports.h>

#include <ref_ptr.h>

#include <avtBehavior.h>
#include <avtDataObject.h>
#include <avtDrawable.h>
#include <VisWindowTypes.h>

class     vtkRenderer;

class     avtTransparencyActor;


// ****************************************************************************
//  Class: avtActor
//
//  Purpose:
//      This is a class that is output by an avtPlot that can be inputted to
//      a VisWindow.  An actor is a drawable with a behavior.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  8 10:31:46 PST 2001
//    Allows for different renderers for the decoration and the actual
//    rendering.
//
//    Kathleen Bonnell, Tue Apr  3 15:24:00 PDT 2001 
//    Add method to retrieve renderOrder.
//
//    Kathleen Bonnell, Tue May  7 09:36:15 PDT 2002 
//    Add method GetDataExtents. 
//
//    Hank Childs, Sun Jul  7 12:55:05 PDT 2002
//    Added support for transparency.
//
//    Kathleen Bonnell, Fri Jul 12 16:20:08 PDT 2002 
//    Added support for decorations.
//
//    Kathleen Bonnell, Fri Jul 19 08:39:04 PDT 2002 
//    Added UpdateScaleFactor. 
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002 
//    Added methods in support of lighting.
//
//    Brad Whitlock, Mon Sep 23 15:50:38 PST 2002
//    I added a method to set the actor's surface representation and another
//    method to set its immediate mode rendering flag.
//
//    Mark C. Miller, Thu Dec 19 16:19:23 PST 2002
//    Added support for externally rendered images
//
//    Eric Brugger, Wed Aug 20 09:49:33 PDT 2003
//    Added GetWindowMode.
//
//    Kathleen Bonnell, Mon Sep 29 13:44:01 PDT 2003 
//    Added bool arg to GetRenderOrder. 
//
//    Jeremy Meredith, Fri Nov 14 11:10:53 PST 2003
//    Added a method to set the specular properties.
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Elminated externally rendered images actor dependence
//
//    Chris Wojtan, Fri Jul 23 13:57:50 PDT 2004
//    Added isVisible variable and function to check whether this actor
//    is supposed to be drawn to the screen
//
//    Chris Wojtan, Mon Jul 26 17:19:58 PDT 2004
//    Added 4 methods to turn the opaque or tranlucent actors' visibility
//    on or off.
//
//    Jeremy Meredith, Thu Oct 21 12:27:38 PDT 2004
//    Removed the transparency visibility settings, because it is less
//    error-prone (and easier and more efficient) to turn off the entire
//    avtTransparencyActor at once.
//
//    Kathleen Bonnell, Mon Sep 27 10:19:11 PDT 2004 
//    Added MakePickable and MakeUnPickable. 
//
//    Kathleen Bonnell, Mon Jun 27 14:48:12 PDT 2005 
//    Added GetZPosition.
//
//    Mark Blair, Wed Aug 30 14:09:00 PDT 2006
//    Added SetTypeName and GetTypeName.
//
//    Brad Whitlock, Mon Sep 18 11:19:11 PDT 2006
//    Added SetColorTexturingFlag.
//
// ****************************************************************************

class PLOTTER_API avtActor
{
  public:
                                  avtActor();
    virtual                      ~avtActor();

    void                          SetBehavior(avtBehavior_p);
    avtBehavior_p                 GetBehavior(void) { return behavior; };
    void                          SetDrawable(avtDrawable_p);
    void                          SetDecorations(avtDrawable_p);

    void                          SetTypeName(const char *typeName_);
    const char                   *GetTypeName();

    void                          Add(vtkRenderer *, vtkRenderer *);
    void                          Remove(vtkRenderer *, vtkRenderer *);

    void                          GetActualBounds(double [6]);
    void                          GetOriginalBounds(double [6]);
    void                          GetDataExtents(double &dmin, double &dmax);
    int                           GetDimension(void);
    WINDOW_MODE                   GetWindowMode(void);
    int                           GetRenderOrder(bool aa);
    avtLegend_p                   GetLegend(void);

    void                          ShiftByVector(const double [3]);
    void                          ScaleByVector(const double [3]);
    void                          UpdateScaleFactor();

    void                          VisibilityOn(void);
    void                          VisibilityOff(void);
    void                          SetTransparencyActor(avtTransparencyActor *);

    void                          TurnLightingOn(void);
    void                          TurnLightingOff(void);
    void                          SetAmbientCoefficient(const double);

    void                          SetSurfaceRepresentation(int rep);
    void                          SetImmediateModeRendering(bool val);

    void                          SetSpecularProperties(bool,double,double,
                                                        const ColorAttribute&);

    void                          SetColorTexturingFlag(bool);

    avtDataObject_p               GetDataObject(void);

    bool                          IsVisible();

    void                          OpaqueVisibilityOn();
    void                          OpaqueVisibilityOff();

    void                          MakePickable(void);
    void                          MakeUnPickable(void);
    double                        GetZPosition(void);

  protected:
    avtBehavior_p                 behavior;
    avtDrawable_p                 drawable;
    avtDrawable_p                 decorations;
    avtTransparencyActor         *transparencyActor;
    int                           transparencyIndex;
    bool                          isVisible;
    
    const char                   *typeName;

    vtkRenderer                  *renderer;
};


typedef ref_ptr<avtActor> avtActor_p;


#endif


