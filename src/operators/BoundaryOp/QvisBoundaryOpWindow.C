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

#include "QvisBoundaryOpWindow.h"

#include <BoundaryOpAttributes.h>
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
#include <QvisVariableButton.h>

#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisBoundaryOpWindow::QvisBoundaryOpWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisBoundaryOpWindow::QvisBoundaryOpWindow(const int type,
                         BoundaryOpAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisBoundaryOpWindow::~QvisBoundaryOpWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisBoundaryOpWindow::~QvisBoundaryOpWindow()
{
}


// ****************************************************************************
// Method: QvisBoundaryOpWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryOpWindow::CreateWindowContents()
{
    // Create the smoothing level buttons
    smoothingLevelButtons = new QButtonGroup(0, "smoothingButtons");
    connect(smoothingLevelButtons, SIGNAL(clicked(int)),
	    this, SLOT(smoothingLevelChanged(int)));
    QGridLayout *smoothingLayout = new QGridLayout(topLayout, 1, 5);
    smoothingLayout->setSpacing(10);
    smoothingLayout->setColStretch(4, 1000);
    smoothingLayout->addWidget(new QLabel("Geometry smoothing", central), 0,0);
    QRadioButton *rb = new QRadioButton("None", central, "NoSmoothing");
    smoothingLevelButtons->insert(rb);
    smoothingLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton("Fast", central, "LowSmoothing");
    smoothingLevelButtons->insert(rb);
    smoothingLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton("High", central, "HighSmoothing");
    smoothingLevelButtons->insert(rb);
    smoothingLayout->addWidget(rb, 0, 3);
}


// ****************************************************************************
// Method: QvisBoundaryOpWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryOpWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        const double         *dptr;
        const float          *fptr;
        const int            *iptr;
        const char           *cptr;
        const unsigned char  *uptr;
        const string         *sptr;
        QColor                tempcolor;
        switch(i)
	{
	    case BoundaryOpAttributes::ID_smoothingLevel:
		smoothingLevelButtons->blockSignals(true);
		smoothingLevelButtons->setButton(atts->GetSmoothingLevel());
		smoothingLevelButtons->blockSignals(false);
		break;
	}
    }
}


// ****************************************************************************
// Method: QvisBoundaryOpWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Tue Aug 14 10:29:40 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisBoundaryOpWindow::GetCurrentValues(int which_widget)
{
}


//
// Qt Slot functions
//

// ****************************************************************************
//  Method:  QvisBoundaryPlotWindow::smoothingLevelChanged
//
//  Purpose:
//    Qt slot function that is called when one of the smoothing buttons
//    is clicked.
//
//  Arguments:
//    level  :   The new level.
//
//  Programmer:  Gunther H. Weber
//  Creation:    August 14, 2007
//
//  Note:  Taken verbatim from the Boundary plot
//
//  Modifications:
//
// ****************************************************************************

void
QvisBoundaryOpWindow::smoothingLevelChanged(int level)
{
    atts->SetSmoothingLevel(level);
    SetUpdate(false);
    Apply();
}
