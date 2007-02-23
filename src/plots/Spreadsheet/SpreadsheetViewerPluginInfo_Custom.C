/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <SpreadsheetPluginInfo.h>
#include <SpreadsheetViewer.h>
#include <avtCallback.h>
#include <avtSILRestriction.h>
#include <ViewerPlot.h>

//
// These methods are separated from SpreadsheetViewerPluginInfo.C in case
// someone regenerates the plugin information. By explicitly including this
// file in the viewer plugin XML, there will be a compile time error if
// someone failed to put the alternate display method declarations back into
// SpreadsheetPluginInfo.h
//

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::InitializePlotAtts
//
// Purpose: 
//   Initializes the plot attributes.
//
// Arguments:
//   atts    : The plot attributes to check.
//   md      : The plot metadata.
//   varName : The plot variable.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 11:36:13 PDT 2007
//
// Modifications:
//   
// ****************************************************************************
void
SpreadsheetViewerPluginInfo::InitializePlotAtts(AttributeSubject *atts,
    ViewerPlot *plot)
{
    *(SpreadsheetAttributes*)atts = *defaultAtts;

    PrivateSetPlotAtts(atts, plot);
}

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::ResetPlotAtts
//
// Purpose: 
//   Resets the plot attributes.
//
// Arguments:
//   atts    : The plot attributes to reset.
//   md      : The plot metadata.
//   varName : The plot variable.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 11:36:13 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewerPluginInfo::ResetPlotAtts(AttributeSubject *atts,
    ViewerPlot *plot)

{
    *(SpreadsheetAttributes*)atts = *defaultAtts;

    PrivateSetPlotAtts(atts, plot);
}


// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::ReInitializePlotAtts
//
// Purpose: 
//   Makes sure that the subset name in the plot attributes is valid.
//
// Arguments:
//   atts    : The plot attributes to check.
//   md      : The plot metadata.
//   varName : The plot variable.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 11:36:13 PDT 2007
//
// Modifications:
//   
// ****************************************************************************
void
SpreadsheetViewerPluginInfo::ReInitializePlotAtts(AttributeSubject *atts,
    ViewerPlot *plot)
{
    PrivateSetPlotAtts(atts, plot);
}

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::PrivateSetPlotAtts
//
// Purpose: 
//   Makes sure that the subset name in the plot attributes is valid.
//
// Arguments:
//   atts    : The plot attributes to check.
//   md      : The plot metadata.
//   varName : The plot variable.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 11:36:13 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetViewerPluginInfo::PrivateSetPlotAtts(AttributeSubject *atts, 
    ViewerPlot *plot)
{
    const char *mName = "SpreadsheetViewerPluginInfo::PrivateSetPlotAtts: ";

    // Search for a SIL_DOMAIN category under the current silTopSet and add
    // sets that are used.
    SpreadsheetAttributes *sAtts = (SpreadsheetAttributes *)atts;
    bool validName = false;
    bool firstNameSet = false;
    std::string firstName;
    int nSets = 1;
    avtSILRestriction_p silr = plot->GetSILRestriction();
    avtSILSet_p current = silr->GetSILSet(silr->GetTopSet());
    const std::vector<int> &mapsOut = current->GetMapsOut();
    for(int j = 0; j < mapsOut.size() && !validName; ++j)
    {
        int cIndex = mapsOut[j];
        avtSILCollection_p collection = silr->GetSILCollection(cIndex);
        if(*collection != NULL && collection->GetRole() == SIL_DOMAIN) 
        {
            const std::vector<int> &setIds = collection->GetSubsetList();
            nSets = setIds.size();
            for(int si = 0; si < setIds.size() && !validName; ++si)
            {
                if(!firstNameSet)
                {
                    firstName = silr->GetSILSet(setIds[si])->GetName();
                    firstNameSet = true;
                }
                validName = (silr->GetSILSet(setIds[si])->GetName() == 
                             sAtts->GetSubsetName());
            }
            break;
        }
    }

    // If the subset name is not valid then override it. Set it in the
    // default attributes so the GUI window does not get too confused when
    // the default attributes get sent to the client as when a plot of another
    // type is selected.
    if(!validName)
    {
        if(nSets > 1)
        {
            if(firstNameSet)
            {
                sAtts->SetSubsetName(firstName);
                defaultAtts->SetSubsetName(firstName);
            }
        }
        else
        {
            sAtts->SetSubsetName("Whole");
            defaultAtts->SetSubsetName("Whole");
        }
    }
}

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::AlternateDisplayCreate
//
// Purpose: 
//   Creates an alternate display that will display data for the Spreadsheet
//   plot.
//
// Arguments:
//   plot : Pointer to the ViewerPlot that will own the alternate display.
//
// Returns:    A pointer to the alternate display.
//
// Note:       We refer to the alternate display using void* so it can be
//             whatever type of display that is most appropriate Qt, X, ...
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:53:23 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void *
SpreadsheetViewerPluginInfo::AlternateDisplayCreate(ViewerPlot *plot)
{
    void *dpy = 0;

    if(!avtCallback::GetNowinMode())
    {
        // We're not in nowin mode so create the spreadsheet viewer.
        SpreadsheetViewer *win = new SpreadsheetViewer(plot, 0, "SpreadsheetViewer");
        dpy = (void *)win;
    }

    return dpy;
}

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::AlternateDisplayDestroy
//
// Purpose: 
//   This method is called when the alternate display must be destroyed.
//
// Arguments:
//   dpy : The handle to the alternate display.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:55:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void 
SpreadsheetViewerPluginInfo::AlternateDisplayDestroy(void *dpy)
{
    if(dpy != 0)
    {
        SpreadsheetViewer *v = (SpreadsheetViewer *)dpy;
        delete v;
    }
}

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::AlternateDisplayClear
//
// Purpose: 
//   This method is called when the alternate display must be cleared of data.
//
// Arguments:
//   dpy : The handle to the alternate display.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:55:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void 
SpreadsheetViewerPluginInfo::AlternateDisplayClear(void *dpy)
{
    if(dpy != 0)
    {
        SpreadsheetViewer *v = (SpreadsheetViewer *)dpy;
        v->clear();
    }
}

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::AlternateDisplayHide
//
// Purpose: 
//   This method is called when the alternate display must be hidden.
//
// Arguments:
//   dpy : The handle to the alternate display.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:55:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void 
SpreadsheetViewerPluginInfo::AlternateDisplayHide(void *dpy)
{
    if(dpy != 0)
    {
        SpreadsheetViewer *v = (SpreadsheetViewer *)dpy;
        v->hide();
    }
}

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::AlternateDisplayShow
//
// Purpose: 
//   This method is called when the alternate display must be shown.
//
// Arguments:
//   dpy : The handle to the alternate display.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:55:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void 
SpreadsheetViewerPluginInfo::AlternateDisplayShow(void *dpy)
{
    if(dpy != 0)
    {
        SpreadsheetViewer *v = (SpreadsheetViewer *)dpy;
        if(v->isMinimized())
            v->showNormal();
        else
            v->show();
        v->raise();
    }
}

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::AlternateDisplayIconify
//
// Purpose: 
//   This method is called when the alternate display must be iconified.
//
// Arguments:
//   dpy : The handle to the alternate display.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:55:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void 
SpreadsheetViewerPluginInfo::AlternateDisplayIconify(void *dpy)
{
    if(dpy != 0)
    {
        SpreadsheetViewer *v = (SpreadsheetViewer *)dpy;
        v->showMinimized();
    }
}

// ****************************************************************************
// Method: SpreadsheetViewerPluginInfo::AlternateDisplayDeIconify
//
// Purpose: 
//   This method is called when the alternate display must be de-iconified.
//
// Arguments:
//   dpy : The handle to the alternate display.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:55:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void 
SpreadsheetViewerPluginInfo::AlternateDisplayDeIconify(void *dpy)
{
    if(dpy != 0)
    {
        SpreadsheetViewer *v = (SpreadsheetViewer *)dpy;
        v->showNormal();
    }
}
