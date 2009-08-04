/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                             avtVolumeRenderer.h                           //
// ************************************************************************* //

#ifndef AVT_VOLUME_RENDERER_H
#define AVT_VOLUME_RENDERER_H

#include <avtCustomRenderer.h>
#include <VolumeAttributes.h>

class vtkDataArray;
class avtVolumeRendererImplementation;

#define USE_HISTOGRAM 1

// ****************************************************************************
//  Class: avtVolumeRenderer
//
//  Purpose:
//      An implementation of an avtCustomRenderer for a volume plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 19 08:07:40 PST 2001
//    Added a convenience routine to retrieve the variables from a dataset.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Wed Apr 24 09:16:51 PDT 2002 
//    Added derived types for graphics packages.
//
//    Jeremy Meredith, Tue Sep 30 11:49:42 PDT 2003
//    Added method "ReleaseGraphicsResources".  Moved alphatex to subclass.
//
//    Jeremy Meredith, Thu Oct  2 13:13:23 PDT 2003
//    Made this class be a concrete implementation of a custom renderer.
//    It will chose between actual rendering methods by instantiating an
//    avtVolumeRendererImplementation at render time.
//
//    Brad Whitlock, Mon Dec 15 14:36:29 PST 2008
//    I removed some methods.
//
// ****************************************************************************

class avtVolumeRenderer : public avtCustomRenderer
{
  public:
                            avtVolumeRenderer();
    virtual                ~avtVolumeRenderer();
    static avtVolumeRenderer *New(void);

    void                    SetAtts(const AttributeGroup*);

    virtual bool            OperatesOnScalars(void) { return true; };
    virtual void            ReleaseGraphicsResources();
    virtual void            Render(vtkDataSet *);

    virtual void            ReducedDetailModeOn();
    virtual bool            ReducedDetailModeOff();
  protected:
    avtVolumeRendererImplementation  *rendererImplementation;
    bool                              currentRendererIsValid;

    VolumeAttributes        atts;

    void                    Initialize(vtkDataSet*);
    bool                    initialized;

    float                   vmin,vmax,vsize;
    float                   omin,omax,osize;
    float                  *gx, *gy, *gz, *gm, *gmn;

    bool                    reducedDetail;

    bool                    GetScalars(vtkDataSet *ds, vtkDataArray *&d, vtkDataArray *&o);
};


typedef ref_ptr<avtVolumeRenderer> avtVolumeRenderer_p;


#endif


