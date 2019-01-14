/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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


#include "vtkOffScreenRenderingFactory.h"
#include <visit-config.h>
#if defined(HAVE_OSMESA)
  #include <vtkOSOpenGLRenderWindow.h>
  VTK_CREATE_CREATE_FUNCTION(vtkOSOpenGLRenderWindow);
#elif defined(HAVE_EGL)
  #include <vtkEGLRenderWindow.h>
  VTK_CREATE_CREATE_FUNCTION(vtkEGLRenderWindow);
#endif



// ****************************************************************************
//  Method:  vtkOffScreenRenderingFactory Constructor
//
//  Purpose:
//
//  Arguments:
//
//  Modifications:
//
// ****************************************************************************

vtkOffScreenRenderingFactory::vtkOffScreenRenderingFactory()
{
    // If VTK was compiled with VTK_OPENGL_HAS_OSMESA then
    // vtkOSOpenGLRenderWindow was compiled into vtkRenderingOpenGL.
    //
    // If VTK was compiled with VTK_OPENGL_HAS_EGL then
    // vtkEGLRenderWindow was compiled into vtkRenderingOpenGL.
    //
    // If VTK was comiled with VTK_USE_X, then, despite either of the above
    // vtkXOpenGLRenderWindow will be the override when instantiating a
    // vtkRenderWindow.
    //
    // This will cause problems for us on headless nodes, so here we create
    // overrides for vtkXOpenGLRenderWindow, depending on whether we want
    // OSMesa or EGL.
    //

#if defined(HAVE_OSMESA)
    this->RegisterOverride("vtkXOpenGLRenderWindow",
                           "vtkOSOpenGLRenderWindow",
                           "Render Window Hijack Overrride",
                           1,
                           vtkObjectFactoryCreatevtkOSOpenGLRenderWindow);
#elif defined(HAVE_EGL)
    this->RegisterOverride("vtkXOpenGLRenderWindow",
                           "vtkEGLRenderWindow",
                           "Render Window Hijack Overrride",
                           1,
                           vtkObjectFactoryCreatevtkEGLRenderWindow);
#endif
}



// ****************************************************************************
//  Method:  vtkOffScreenRenderingFactory::ForceOffScreen
//
//  Purpose:
//
//  Arguments:
//
// ****************************************************************************

void
vtkOffScreenRenderingFactory::ForceOffScreen()
{
#if defined(HAVE_OSMESA) || defined(HAVE_EGL)
    vtkOffScreenRenderingFactory *os_factory = vtkOffScreenRenderingFactory::New();
    vtkObjectFactory::RegisterFactory(os_factory);
    os_factory->Delete();
#endif
}

