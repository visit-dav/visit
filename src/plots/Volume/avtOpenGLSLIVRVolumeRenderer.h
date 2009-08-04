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
//                       avtOpenGLSLIVRVolumeRenderer.h                      //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_SLIVR_VOLUME_RENDERER_H
#define AVT_OPEN_GL_SLIVR_VOLUME_RENDERER_H
#include <visit-config.h>
#ifdef HAVE_LIBSLIVR
#include <avtVolumeRendererImplementation.h>
#include <vector>

#include <VolumeAttributes.h>

#include <slivr/Texture.h>
#include <slivr/ColorMap.h>
#include <slivr/ColorMap2.h>
#include <slivr/VolumeRenderer.h>

// ****************************************************************************
//  Class avtOpenGLSLIVRVolumeRenderer
//
//  Purpose
//      An implementation of a volume renderer that uses the SLIVR library.
//
//  Programmer Brad Whitlock
//  Creation   Fri Aug 17 17:55:44 PST 2007
//
//  Modifications:
//    Brad Whitlock, Wed Apr 22 12:09:09 PDT 2009
//    I changed the interface.
//
// ****************************************************************************

class avtOpenGLSLIVRVolumeRenderer : public avtVolumeRendererImplementation
{
public:
                            avtOpenGLSLIVRVolumeRenderer();
    virtual                ~avtOpenGLSLIVRVolumeRenderer();

protected:
    virtual void            Render(const RenderProperties &props, const VolumeData &volume);
    struct SlivrContext
    {
        SlivrContext();
        ~SlivrContext();

        Nrrd                            *data;
        Nrrd                            *gm_data;
        SLIVR::Texture                  *tex;
        SLIVR::ColorMap                 *cm;
        SLIVR::VolumeRenderer           *renderer;
        std::vector<SLIVR::ColorMap2 *>  cmap2;
        Nrrd                            *cmap2_image;
        std::vector<SLIVR::Plane *>      planes;
    };

    void             FreeContext();
    void             CreateColormap(const VolumeAttributes &atts, bool &);
    void             CheckContext(SlivrContext* context,
                                  const VolumeAttributes &atts);
    void             CreateContext(vtkRectilinearGrid *grid, 
                                   vtkDataArray *data, 
                                   vtkDataArray *opac, 
                                   const VolumeAttributes &atts, 
                                   float vmin, float vmax, 
                                   float omin, float omax, float *gmn);
    bool             OnlyLightingFlagIsDifferent(const VolumeAttributes &lhs,
                                                 const VolumeAttributes &rhs) const;

    static bool      slivrInit;
    SlivrContext    *context;
    VolumeAttributes oldAtts;
};

#endif
#endif
