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
#include <visit-config.h>
#include <cstring>

#include <MesaDisplay.h>

#include <Environment.h>
#include <InitVTKRendering.h>

// ****************************************************************************
//  Method: MesaDisplay constructor
//
//  Purpose:
//    Sets the default state for a Mesa display; a no-op, we have no state.
//
//  Programmer:  Tom Fogal
//  Creation:    September 1, 2008
//
// ****************************************************************************

MesaDisplay::MesaDisplay() { }

// ****************************************************************************
//  Method: MesaDisplay destructor
//
//  Purpose:
//    Ensures we've disassociated ourselves from any Mesa context.
//
//  Programmer:  Tom Fogal
//  Creation:    September 1, 2008
//
// ****************************************************************************

MesaDisplay::~MesaDisplay()
{
    // A MesaDisplay has no state, and thus we can safely always consider it
    // initialized.  This might need to be revisited should Teardown's
    // implementation change from a no-op.
    this->Teardown();
}

// ****************************************************************************
//  Method: MesaDisplay::Initialize
//
//  Purpose:
//    No-op; Mesa only does something at the `connect' stage.
//
//  Returns: success, always.
//
//  Programmer:  Tom Fogal
//  Creation:    September 1, 2008
//
// ****************************************************************************

bool
MesaDisplay::Initialize(size_t display, const std::vector<std::string> &user_args)
{
    return true;
}

// ****************************************************************************
//  Method: MesaDisplay::Connect
//
//  Purpose:
//    Associates this process with an X server.
//
//  Programmer:  Tom Fogal
//  Creation:    September 1, 2008
//
//  Modifications:
//
//    Brad Whitlock, Fri Aug 29 09:55:09 PDT 2008
//    Added Mac-specific code for unsetenv since it returns void on Mac.
//
//    Kathleen Bonnell, Tue Spe  2 15:54:17 PDT 2008 
//    If-def'd out unsetenv for windows.
//
//    Eric Brugger, Tue Oct 21 16:54:04 PDT 2008
//    I made use of unsetenv dependent on HAVE_SETENV (only gcc 3.2 on
//    Solaris).  I Replaced strerror_r with strerror.
//
//    Tom Fogal, Wed Apr 22 18:37:35 MDT 2009
//    Use `Environment' namespace function instead of `unsetenv' directly.
//
// ****************************************************************************

void
MesaDisplay::Connect()
{
    InitVTKRendering::ForceMesa();

    Environment::unset("DISPLAY");
}

// ****************************************************************************
//  Method: MesaDisplay::Teardown
//
//  Purpose:
//    No-op; there's really no way to `disassociate' from a Mesa context in
//    VTK/VisIt
//
//  Programmer:  Tom Fogal
//  Creation:    September 1, 2008
//
// ****************************************************************************

void
MesaDisplay::Teardown()
{
}
