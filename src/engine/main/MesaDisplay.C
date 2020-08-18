// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visit-config.h>
#include <cstring>

#include <MesaDisplay.h>

#include <VisItEnv.h>
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
// Method: XDisplay::GetDisplayType
//
// Purpose: 
//   Return the display type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 10 11:39:18 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

VisItDisplay::DisplayType
MesaDisplay::GetDisplayType() const
{
    return VisItDisplay::D_MESA;
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
//  Modifications:
//
//    Tom Fogal, Wed May  4 15:00:24 MDT 2011
//    Fix display type.
//
// ****************************************************************************

bool
MesaDisplay::Initialize(std::string display,
                        const std::vector<std::string> &user_args)
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
//    Tom Fogal, Tue May 25 16:10:10 MDT 2010
//    Interface change: retval void -> bool
//
//    Eric Brugger, Fri May 10 14:41:57 PDT 2013
//    I removed support for mangled mesa.
//
// ****************************************************************************

bool
MesaDisplay::Connect()
{
    VisItEnv::unset("DISPLAY");

    return true;
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
