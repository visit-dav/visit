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

#include "QvisTruecolorPlotWindow.h"

#include <TruecolorAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QvisOpacitySlider.h>
#include <stdio.h>


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::QvisTruecolorPlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisTruecolorPlotWindow::QvisTruecolorPlotWindow(const int type,
                         TruecolorAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::~QvisTruecolorPlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisTruecolorPlotWindow::~QvisTruecolorPlotWindow()
{
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Mon Jul 25 15:27:06 PDT 2005
//   Added lighting checkbox.
//
//   Brad Whitlock, Wed Apr 23 12:09:16 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Tue Aug 5 20:03:23 PST 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisTruecolorPlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(0);
    topLayout->addLayout(mainLayout);

    opacityLabel = new QLabel(tr("Opacity"), central);
    mainLayout->addWidget(opacityLabel,0,0);
    opacity = new QvisOpacitySlider(0,255,25,255, central, NULL);
    opacity->setTickInterval(64);
    opacity->setGradientColor(QColor(0, 0, 0));
    connect(opacity, SIGNAL(valueChanged(int, const void*)),
            this, SLOT(opacityChanged(int, const void*)));
    mainLayout->addWidget(opacity, 0,1);

    lighting = new QCheckBox(tr("Lighting"), central);
    connect(lighting, SIGNAL(toggled(bool)), this, SLOT(lightingToggled(bool)));
    mainLayout->addWidget(lighting, 1, 0);
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Chris Wojtan
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Mon Jul 25 15:27:06 PDT 2005
//   Added lighting.
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::UpdateWindow(bool doAll)
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
          case 0: //opacity
            opacity->blockSignals(true);
            opacity->setValue(int((float)atts->GetOpacity()*255.f));
            opacity->blockSignals(false);
            break;
          case 1: //lighting
            lighting->blockSignals(true);
            lighting->setChecked(atts->GetLightingFlag());
            lighting->blockSignals(false);
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   Kathleen Bonnell, Wed Jun 4 07:57:23 PDT 2008
//   Removed unused variable.
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do opacity
    if(which_widget == 0 || doAll)
    {
        // Nothing for opacity
    }

}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisTruecolorPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Tue Jun 15 11:10:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisTruecolorPlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisTruecolorPlotWindow::opacityChanged(int opacity, const void*)
{
    atts->SetOpacity((float)opacity/255.);
    Apply();
}

void
QvisTruecolorPlotWindow::lightingToggled(bool val)
{
    atts->SetLightingFlag(val);
    Apply();
}


