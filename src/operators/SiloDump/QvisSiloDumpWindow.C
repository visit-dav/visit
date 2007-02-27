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

#include "QvisSiloDumpWindow.h"

#include <SiloDumpAttributes.h>
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
// Method: QvisSiloDumpWindow::QvisSiloDumpWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisSiloDumpWindow::QvisSiloDumpWindow(const int type,
                         SiloDumpAttributes *subj,
                         const char *caption,
                         const char *shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisSiloDumpWindow::~QvisSiloDumpWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisSiloDumpWindow::~QvisSiloDumpWindow()
{
}


// ****************************************************************************
// Method: QvisSiloDumpWindow::CreateWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSiloDumpWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 2,2,  10, "mainLayout");


    mainLayout->addWidget(new QLabel("Filename", central, "filenameLabel"),0,0);
    filename = new QLineEdit(central, "filename");
    connect(filename, SIGNAL(returnPressed()),
            this, SLOT(filenameProcessText()));
    mainLayout->addWidget(filename, 0,1);

    display = new QCheckBox("Display output", central, "display");
    connect(display, SIGNAL(toggled(bool)),
            this, SLOT(displayChanged(bool)));
    mainLayout->addWidget(display, 1,0);

}


// ****************************************************************************
// Method: QvisSiloDumpWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSiloDumpWindow::UpdateWindow(bool doAll)
{
    QString temp;

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
          case 0: //filename
            temp = atts->GetFilename().c_str();
            filename->setText(temp);
            break;
          case 1: //display
            display->setChecked(atts->GetDisplay());
            break;
        }
    }
}


// ****************************************************************************
// Method: QvisSiloDumpWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:40:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSiloDumpWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do filename
    if(which_widget == 0 || doAll)
    {
        temp = filename->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetFilename(temp.latin1());
        }

        if(!okay)
        {
            msg.sprintf("The value of filename was invalid. "
                "Resetting to the last good value of %s.",
                atts->GetFilename().c_str());
            Message(msg);
            atts->SetFilename(atts->GetFilename());
        }
    }

    // Do display
    if(which_widget == 1 || doAll)
    {
        // Nothing for display
    }

}


//
// Qt Slot functions
//


void
QvisSiloDumpWindow::filenameProcessText()
{
    GetCurrentValues(0);
    Apply();
}


void
QvisSiloDumpWindow::displayChanged(bool val)
{
    atts->SetDisplay(val);
    Apply();
}


