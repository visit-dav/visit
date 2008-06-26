/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <QvisOperatorWindow.h>
#include <QvisNotepadArea.h>
#include <qmessagebox.h>

#include <GlobalAttributes.h>
#include <OperatorPluginInfo.h>
#include <Plot.h>
#include <PlotList.h>
#include <ViewerProxy.h>

// ****************************************************************************
// Method: QvisOperatorWindow::QvisOperatorWindow
//
// Purpose: 
//   This is the constructor for the QvisOperatorWindow class.
//
// Arguments:
//   type      : The integer id that is associated with the operator plugin.
//   subj      : The AsliceAttributes object that the window observes.
//   caption   : The string that appears in the window decoration.
//   shortName : The name to use when the window is posted.
//   notepad   : The notepad widget to which the window posts.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:34:59 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Mar 6 11:29:17 PDT 2003
//   I added stretch.
//
//   Brad Whitlock, Wed Apr  9 12:48:10 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisOperatorWindow::QvisOperatorWindow(const int type, Subject *subj,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad,
    bool stretch) : QvisPostableWindowObserver(subj, caption, shortName,
    notepad, QvisPostableWindowObserver::AllExtraButtons, stretch)
{
    operatorType = type;
}

// ****************************************************************************
// Method: QvisOperatorWindow::~QvisOperatorWindow
//
// Purpose: 
//   This is the destructor for the QvisOperatorWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:35:55 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

QvisOperatorWindow::~QvisOperatorWindow()
{
}

// ****************************************************************************
// Method: QvisOperatorWindow::Apply
//
// Purpose: 
//   This method applies the operator attributes and optionally tells
//   the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            aslice attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:38:36 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        subject->Notify();

        // Tell the viewer to set the operator attributes.
        SetOperatorOptions();
    }
    else
        subject->Notify();
}

// ****************************************************************************
// Method: QvisOperatorWindow::GetCurrentValues
//
// Purpose: 
//   This method gets the current values for widgets in the window. This is
//   a default implementation that does nothing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:41:57 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::GetCurrentValues(int)
{
    // override in derived classes.
}

// ****************************************************************************
// Method: QvisOperatorWindow::SetOperatorOptions
//
// Purpose: 
//   This looks to see if the operator is applied to any plot in the plot list
//   and ask the user if the operator should be added if it is not found.
//
// Arguments:
//   operatorType : The operator type.
//
// Programmer: Brad Whitlock
// Creation:   
//
// Modifications:
//   Brad Whitlock, Wed Jun 26 15:28:08 PST 2002
//   Made the operator prompt window only pop up if there are plots.
//
//   Jeremy Meredith, Mon Jun 23 16:18:29 PDT 2003
//   Changed GetAllID to GetEnabledID.
//
//   Kathleen Bonnell, Thu Sep 11 10:35:39 PDT 2003 
//   When the operator needs to be added, set client atts (instead of default),
//   but let the operator know to initalize from client via a flag passed
//   to AddOperator.  This allows the 'Reset' operation to work as expected.
//
//   Brad Whitlock, Thu Aug 5 16:59:38 PST 2004
//   I added a 3rd option to the dialog so you can answer Yes once but have
//   it always add the operator if it does not exist in the future.
//
//   Brad Whitlock, Tue May 8 16:58:12 PST 2007
//   Changed from setting client atts to using AddInitializedOperator, which
//   does the same thing but can get logged in the CLI better.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisOperatorWindow::SetOperatorOptions()
{
    PlotList *plots = GetViewerState()->GetPlotList();

    // If there are plots, see if the operator should be applied.
    if(plots->GetNumPlots() > 0)
    {
        bool applyToAll = GetViewerState()->GetGlobalAttributes()->GetApplyOperator();
        int i, j;

        // Look to see if the operator exists in the plots
        bool found = false;
        for(i = 0; i < plots->GetNumPlots() && !found; ++i)
        {
            const Plot &plot = plots->operator[](i);
            if(plot.GetActiveFlag() || applyToAll)
            {
                for(j = 0; j < plot.GetNumOperators(); ++j)
                {
                     if(operatorType == plot.GetOperator(j))
                     {
                         found = true;
                         break;
                     }
                }
            }
        }

        // If the operator was not found in the plot list, ask the user whether
        // the operator should be added to the plots.
        if(!found)
        {
            int button = 0;

            // Only ask the user if we are not automatically adding the
            // operator.
            if(!GetViewerState()->GetGlobalAttributes()->GetAutomaticallyAddOperator())
            {
                OperatorPluginManager *opMgr = GetViewerProxy()->GetOperatorPluginManager();

                // Create a prompt for the user.
                GUIOperatorPluginInfo *info = opMgr->GetGUIPluginInfo(
                                              opMgr->GetEnabledID(operatorType));
                QString menuName, *s = 0;
                if(info)
                {
                    s = info->GetMenuName();
                    menuName = *s;
                }

                QString msg = tr("No %1 operator was found for the selected plots.\n"
                                 "Do you want to apply the %2 operator?\n\n").
                              arg(menuName).arg(menuName);

                if(s != 0)
                    delete s;

                // Ask the user if he really wants to close the engine.
                button = QMessageBox::warning(this, "VisIt",
                    msg.latin1(), tr("Yes"), tr("No"), tr("Yes, Do not prompt again"),
                    0, 1 );
            }

            if(button == 0)
            {
                // Set the client attributes, and set the 'fromDefault' flag
                // to false in the call to AddOperator, so that the operator 
                // knows to initialize the atts from client rather than the 
                // ususal default atts. 
                GetViewerMethods()->AddInitializedOperator(operatorType);
            }
            else if (button == 2)
            {
                // Make it so no confirmation is needed.
                GlobalAttributes *globalAtts = GetViewerState()->GetGlobalAttributes();
                globalAtts->SetAutomaticallyAddOperator(true);
                globalAtts->Notify();

                // Set the client attributes, and set the 'fromDefault' flag
                // to false in the call to AddOperator, so that the operator 
                // knows to initialize the atts from client rather than the 
                // ususal default atts. 
                GetViewerMethods()->AddInitializedOperator(operatorType);
            }
        }
        else
            GetViewerMethods()->SetOperatorOptions(operatorType);
    }
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisOperatorWindow::apply
//
// Purpose: 
//   This is a Qt slot function to apply the operator attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 11:40:04 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisOperatorWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function to make the current operator attributes
//   the defaults.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:38:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::makeDefault()
{
    // Tell the viewer to set the default pc attributes.
    GetCurrentValues(-1);
    subject->Notify();
    GetViewerMethods()->SetDefaultOperatorOptions(operatorType);
}

// ****************************************************************************
// Method: QvisOperatorWindow::reset
//
// Purpose: 
//   This is a Qt slot function to reset the AsliceAttributes to the
//   last applied values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 12 12:39:03 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOperatorWindow::reset()
{
    // Tell the viewer to reset the aslice attributes to the last
    // applied values.
    GetViewerMethods()->ResetOperatorOptions(operatorType);
}





