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
//                     avtOpenGL3DTextureVolumeRenderer.h                    //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_3D_TEXTURE_VOLUME_RENDERER_H
#define AVT_OPEN_GL_3D_TEXTURE_VOLUME_RENDERER_H

#include <avtVolumeRendererImplementation.h>

#include <VolumeAttributes.h>
#include <LightList.h>

// ****************************************************************************
//  Class: avtOpenGL3DTextureVolumeRenderer
//
//  Purpose:
//      An implementation of a volume renderer utilizing 3D texturing hardware
//      through OpenGL calls.
//
//  Programmer: Jeremy Meredith
//  Creation:   October  1, 2003
//
//  Modifications:
//    Brad Whitlock, Thu Jan 10 14:47:34 PST 2008
//    Added reducedDetail argument.
//
//    Brad Whitlock, Wed Apr 22 12:05:30 PDT 2009
//    I changed the interface.
//
// ****************************************************************************

class avtOpenGL3DTextureVolumeRenderer : public avtVolumeRendererImplementation
{
  public:
                            avtOpenGL3DTextureVolumeRenderer();
    virtual                ~avtOpenGL3DTextureVolumeRenderer();

  protected:
    virtual void            Render(const RenderProperties &props, const VolumeData &volume);

    unsigned char          *volumetex;
    unsigned int            volumetexId;
    VolumeAttributes        oldAtts;
    LightList               oldLights;
};


#endif


