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
//                               avtDrawable.h                               //
// ************************************************************************* //

#ifndef AVT_DRAWABLE_H
#define AVT_DRAWABLE_H

#include <plotter_exports.h>

#include <ref_ptr.h>

#include <avtDataObject.h>


class     vtkRenderer;
class     ColorAttribute;
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
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Removed method to SetExternallyRenderedImagesActor
//
//    Kathleen Bonnell, Mon Sep 27 10:21:15 PDT 2004 
//    Added MakePickable and MakeUnPickable. 
//
//    Kathleen Bonnell, Mon Jun 27 15:19:14 PDT 2005 
//    Added GetZPosition.
//
//    Brad Whitlock, Mon Sep 18 11:21:54 PDT 2006
//    Added color texturing method.
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

    virtual void               ShiftByVector(const double [3]) = 0;
    virtual void               ScaleByVector(const double [3]) = 0;
    virtual void               UpdateScaleFactor(void) = 0;

    virtual void               TurnLightingOn(void) = 0;
    virtual void               TurnLightingOff(void) = 0;
    virtual void               SetAmbientCoefficient(const double) = 0;

    virtual void               SetSurfaceRepresentation(int rep) {;};
    virtual void               SetImmediateModeRendering(bool val) {;};

    virtual void               SetSpecularProperties(bool,double,double,
                                                     const ColorAttribute&) {;}

    virtual void               SetColorTexturingFlag(bool) {;}

    virtual avtDataObject_p    GetDataObject(void) = 0;

    virtual void               MakePickable(void){;};
    virtual void               MakeUnPickable(void){;};
    virtual double             GetZPosition(void){ return 0.;};
};


typedef ref_ptr<avtDrawable> avtDrawable_p;


#endif


