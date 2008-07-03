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

#include "QvisProjectWindow.h"

#include <ProjectAttributes.h>
#include <ViewerProxy.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisProjectWindow::QvisProjectWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisProjectWindow::QvisProjectWindow(const int type,
                         ProjectAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisProjectWindow::~QvisProjectWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisProjectWindow::~QvisProjectWindow()
{
}


// ****************************************************************************
// Method: QvisProjectWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Apr 25 08:47:35 PDT 2008
//   Added tr()'s
//
// ****************************************************************************

void
QvisProjectWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 1,2,  10, "mainLayout");


    projectionTypeLabel = new QLabel(tr("Projection type"), central, "projectionTypeLabel");
    mainLayout->addWidget(projectionTypeLabel,0,0);
    projectionType = new QButtonGroup(central, "projectionType");
    projectionType->setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *projectionTypeLayout = new QHBoxLayout(projectionType);
    projectionTypeLayout->setSpacing(10);
    QRadioButton *projectionTypeProjectionTypeXYCartesian = new QRadioButton(tr("XYCartesian"), projectionType);
    projectionTypeLayout->addWidget(projectionTypeProjectionTypeXYCartesian);
    QRadioButton *projectionTypeProjectionTypeZRCylindrical = new QRadioButton(tr("ZRCylindrical"), projectionType);
    projectionTypeLayout->addWidget(projectionTypeProjectionTypeZRCylindrical);
    connect(projectionType, SIGNAL(clicked(int)),
            this, SLOT(projectionTypeChanged(int)));
    mainLayout->addWidget(projectionType, 0,1);

}


// ****************************************************************************
// Method: QvisProjectWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 1 11:55:55 PDT 2008
//   Removed unreferenced variables.
//
// ****************************************************************************

void
QvisProjectWindow::UpdateWindow(bool doAll)
{
    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
          case 0: //projectionType
            projectionType->setButton(atts->GetProjectionType());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisProjectWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 1 11:55:55 PDT 2008
//   Removed unreferenced variables.
//   
// ****************************************************************************

void
QvisProjectWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    // Do projectionType
    if(which_widget == 0 || doAll)
    {
        // Nothing for projectionType
    }

}


//
// Qt Slot functions
//


void
QvisProjectWindow::projectionTypeChanged(int val)
{
    if(val != atts->GetProjectionType())
    {
        atts->SetProjectionType(ProjectAttributes::ProjectionType(val));
        Apply();
    }
}


