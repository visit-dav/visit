/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                    avtMapper.h                            //
// ************************************************************************* //

#ifndef AVT_MAPPER_H
#define AVT_MAPPER_H

#include <plotter_exports.h>

#include <avtTerminatingDatasetSink.h>
#include <avtDrawable.h>


class   vtkActor;
class   vtkDataObjectCollection;
class   vtkDataSetMapper;

class   avtTransparencyActor;
class   ColorAttribute;


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
// ****************************************************************************

class PLOTTER_API avtMapper : public avtTerminatingDatasetSink
{
  public:
                               avtMapper();
    virtual                   ~avtMapper();

    void                       ReleaseData();
    avtDrawable_p              GetDrawable();
    virtual bool               GetRange(double &, double &);
    virtual bool               GetCurrentRange(double &, double &);

    virtual bool               GetLighting(void);

    virtual void               GlobalLightingOn(void);
    virtual void               GlobalLightingOff(void);
    virtual void               GlobalSetAmbientCoefficient(const double);
    double                     GetGlobalAmbientCoefficient() 
                                   { return globalAmbient; };

    void                       SetImmediateModeRendering(bool val);
    bool                       GetImmediateModeRendering();

    int                        SetTransparencyActor(avtTransparencyActor *);

    void                       SetSpecularIsInappropriate(bool val)
                                   { specularIsInappropriate = val; };
    bool                       GetSpecularIsInappropriate()
                                   { return specularIsInappropriate; };

    virtual void               SetSurfaceRepresentation(int rep);
    virtual void               SetSpecularProperties(bool,double,double,
                                                     const ColorAttribute&);

    virtual void               SetColorTexturingFlag(bool) {; }

    virtual bool               SetFullFrameScaling(bool, const double *)
                                   { return false; };

    virtual void               SetAlternateDisplay(void *) {; }

    virtual void               ReducedDetailModeOn() {; }
    virtual bool               ReducedDetailModeOff() { return false; }

  protected:
    bool                       immediateMode;
    bool                       specularIsInappropriate;
    avtDrawable_p              drawable;
    avtTransparencyActor      *transparencyActor;
    int                        transparencyIndex;

    vtkDataSetMapper         **mappers;
    int                        nMappers;
    vtkActor                 **actors;

    double                     globalAmbient;

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


