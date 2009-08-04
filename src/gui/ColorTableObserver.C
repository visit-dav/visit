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

#include <ColorTableObserver.h>
#include <ColorTableAttributes.h>
#include <QvisColorTableButton.h>

// ****************************************************************************
// Method: ColorTableObserver::ColorTableObserver
//
// Purpose: 
//   Constructor for the ColorTableObserver class.
//
// Arguments:
//   subj : A pointer to the ColorTableAttributes subject that this object
//          will observe.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 20 13:36:35 PST 2001
//
// Modifications:
//   
// ****************************************************************************

ColorTableObserver::ColorTableObserver(Subject *subj) : Observer(subj)
{
    // nothing here
}

// ****************************************************************************
// Method: ColorTableObserver::~ColorTableObserver
//
// Purpose: 
//   This is the destructor for the ColorTableObserver class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 20 13:37:24 PST 2001
//
// Modifications:
//   
// ****************************************************************************

ColorTableObserver::~ColorTableObserver()
{
    // nothing here
}

// ****************************************************************************
// Method: ColorTableObserver::Update
//
// Purpose: 
//   This method is called when the ColorTableAttributes subject that this
//   object observes is updated.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 20 13:37:56 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Dec 17 11:06:50 PST 2007
//   Made it use ids.
//
// ****************************************************************************

void
ColorTableObserver::Update(Subject *)
{
    ColorTableAttributes *colorAtts = (ColorTableAttributes *)subject;

    // For when we have pixmaps later, here are the rules...
    // 1. if just #0 is ever selected, we're adding/deleting colortables.
    // 2. If just #1 is selected , we're modifying a color table.
    // 3. If just #2 is selected, we're changing the default colormap. This is
    //    only of concern if I decide to show what the default is in a widget.

    // If the names or the color table attributes are changing, then we
    // have to update the widget.
    if(colorAtts->IsSelected(ColorTableAttributes::ID_names) ||
       colorAtts->IsSelected(ColorTableAttributes::ID_colorTables))
    {
        // Clear all of the color tables.
        QvisColorTableButton::clearAllColorTables();

        int nNames = colorAtts->GetNames().size();
        const stringVector &names = colorAtts->GetNames();
        for(int i = 0; i < nNames; ++i)
        {
            QvisColorTableButton::addColorTable(names[i].c_str());
        }

        // Update all of the QvisColorTableButton widgets.
        QvisColorTableButton::updateColorTableButtons();
    }
}
