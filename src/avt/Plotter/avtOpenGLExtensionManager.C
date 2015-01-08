/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                   avtOpenGLExtensionManager.C                             //
// ************************************************************************* //

#include "avtOpenGLExtensionManager.h"

#include <iostream>
using namespace std;

namespace avt { namespace glew {

// Won't call init again if true.
static bool initialized = false;

// ****************************************************************************
//  Function: avt::glew::initialize
//
//  Purpose: Initializes GLEW using GL library from RuntimeSetting.
//           Maintains state to avoid duplicate initialization.
//
//  Arguments:
//    force : force initialization even if we've previously initialized.
//            Needed if, for example, contexts change behind our back.
//
//  Programmer: Tom Fogal
//
//  Modifications:
//
//    Tom Fogal, Tue Nov 24 10:48:00 MST 2009
//    Update for GLEW API change.
//
//    Tom Fogal, Tue Dec  8 14:07:38 MST 2009
//    Allow colon-separated list of libraries, trying them in sequence until
//    one works.
//
//    Tom Fogal, Mon Feb  8 12:52:06 MST 2010
//    Change separator to ";" to be more windows-friendly.
//
//    Kathleen Bonnell, Tue Feb 16 13:37:52 MST 2010
//    Don't use mesa on windows.
//
//    Brad Whitlock, Fri Oct 21 13:08:02 PDT 2011
//    Make it work with static linking.
//
//    Eric Brugger, Wed Feb 22 10:19:22 PST 2012
//    I added support for building without mesa.
//
// ****************************************************************************

bool initialize(bool force)
{
#ifdef HAVE_LIBGLEW
    if(initialized && !force) // Bail early if we've already been here.
    {
        return true;
    }
    GLenum err = glewInit();
    if (GLEW_OK == err)
        initialized = true;
    else
        cerr << "GLEW initialization failed: " << glewGetErrorString(err) << endl;

#endif
    return initialized;
}

bool
supported(const char *key)
{
#ifdef HAVE_LIBGLEW
    return glewIsSupported(key);
#else
    return true;
#endif
}

}; /* glew */ }; /* avt */
