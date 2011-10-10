/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

#ifndef VISIT_X_DISPLAY_H
#define VISIT_X_DISPLAY_H

#include <VisItDisplay.h>
#include <engine_main_exports.h>

// ****************************************************************************
//  Class:  XDisplay
//
//  Purpose:
//    Manages X servers, utilized for HW rendering on UNIX clusters.
//
//  Programmer:  Tom Fogal
//  Creation:    August 29, 2008
//
//  Modifications:
//
//    Tom Fogal, Tue May 25 15:49:00 MDT 2010
//    Add hostname storage, for printing error messages.
//    Change ::Connect retval to bool.
//
//    Tom Fogal, Wed May 26 09:10:08 MDT 2010
//    Add a method to indicate we should not launch the server.
//
//    Tom Fogal, Wed May  4 14:30:00 MDT 2011
//    Change display to a string for more flexibility.
//
//    Brad Whitlock, Mon Oct 10 11:40:10 PDT 2011
//    Added GetDisplayType.
//
// ****************************************************************************

class ENGINE_MAIN_API XDisplay : public VisItDisplay
{
  public:
                   XDisplay();
    virtual       ~XDisplay();

    virtual bool   Initialize(std::string display,
                              const std::vector<std::string> &args);
    virtual bool   Connect();
    virtual void   Teardown();

    virtual DisplayType GetDisplayType() const;

    // Tell the implementation whether it should launch the X server or just
    // use it.  Must be set before Initialize!
    void           Launch(bool);

  private:
    pid_t       xserver;
    std::string display;
    char        hostname[256];
    bool        launch;
};
#endif /* VISIT_X_DISPLAY_H */
