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

#include <QvisPostableWindowSimpleObserver.h>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>

#include <GlobalAttributes.h>
#include <ViewerProxy.h>

// Static members.
const int QvisPostableWindowSimpleObserver::NoExtraButtons    = 0;
const int QvisPostableWindowSimpleObserver::ApplyButton       = 1;
const int QvisPostableWindowSimpleObserver::MakeDefaultButton = 2;
const int QvisPostableWindowSimpleObserver::ResetButton       = 4;
const int QvisPostableWindowSimpleObserver::AllExtraButtons   = 7;

const int QvisPostableWindowSimpleObserver::LoadButton                 = 8;
const int QvisPostableWindowSimpleObserver::SaveButton                 = 16;
const int QvisPostableWindowSimpleObserver::AllExtraButtonsAndLoadSave = 31;

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::QvisPostableWindowSimpleObserver
//
// Purpose: 
//   Constructor for the QvisPostableWindowSimpleObserver class.
//
// Arguments:
//   caption   : The window caption.
//   shortName : The name used when the window is posted.
//   notepad   : The notepad area into which the window posts.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:39:44 PDT 2001
//
// Modifications:
//   Jeremy Meredith, Wed May  8 10:51:09 PDT 2002
//   Added stretch flag.
//
//   Brad Whitlock, Mon Sep 9 11:00:03 PDT 2002
//   Added applyButton flag.
//
//   Brad Whitlock, Wed Apr  9 10:49:15 PDT 2008
//   QString for caption and shortName.
//
// ****************************************************************************

QvisPostableWindowSimpleObserver::QvisPostableWindowSimpleObserver(
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad,
    int buttonCombo, bool stretch) :
    QvisPostableWindow(caption, shortName, notepad), SimpleObserver()
{
    selectedSubject = 0;
    stretchWindow = stretch;
    buttonCombination = buttonCombo;
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::~QvisPostableWindowSimpleObserver
//
// Purpose: 
//   Destructor for the QvisPostableWindowSimpleObserver class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:40:40 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisPostableWindowSimpleObserver::~QvisPostableWindowSimpleObserver()
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::Update
//
// Purpose: 
//   This method is called when the window's subjects are modified.
//
// Arguments:
//   subj : A pointer to the subject that was modified.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:40:58 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::Update(Subject *subj)
{
    // This is what derived types will query to determine which subject
    // was modified.
    selectedSubject = subj;

    if(isCreated)
        UpdateWindow(false);
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::SubjectRemoved
//
// Purpose: 
//   This method is called when a subject is deleted.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:41:39 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::SubjectRemoved(Subject *TheRemovedSubject)
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::SelectedSubject
//
// Purpose: 
//   Returns which subject is currently being updated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:42:10 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

Subject *
QvisPostableWindowSimpleObserver::SelectedSubject()
{
    return selectedSubject;
}

// ****************************************************************************
// Method: QvisPostableWindow::CreateEntireWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:36:50 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 11:27:15 PDT 2002
//   Added code to return if the window is already created.
//
//   Jeremy Meredith, Wed May  8 10:50:54 PDT 2002
//   Only add stretch if the stretchWindow flag is set.
//
//   Brad Whitlock, Mon Sep 9 11:01:00 PDT 2002
//   I made the apply button optional.
//
//   Brad Whitlock, Fri Nov 7 16:48:36 PST 2003
//   Added support for more buttons.
//
//   Brad Whitlock, Mon Nov 14 10:36:48 PDT 2005
//   Disable post button if posting is not enabled.
//
//   Brad Whitlock, Tue Jan 22 16:45:13 PST 2008
//   If posting is disabled, put the contents of the window into a scrollview.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Jun  6 10:50:39 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Fri Jan  2 17:05:57 EST 2009
//   Added Load/Save button support.  Put them in between the
//   Make Default and Reset buttons for now.
//
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::CreateEntireWindow()
{
    // If the window is already created, return.
    if(isCreated)
        return;

    // Create the central widget and the top layout.
    QWidget *topCentral = 0;
    QVBoxLayout *vLayout = 0;
    if(notepad)
    {
        central = new QWidget( this );
        setCentralWidget( central );
        topCentral = central;
        topLayout = new QVBoxLayout(central);
        topLayout->setMargin(10);
        vLayout = topLayout;
    }
    else
    {
        topCentral = new QWidget(this);
        vLayout = new QVBoxLayout(topCentral);
        vLayout->setMargin(10);
        vLayout->setSpacing(5);
        setCentralWidget( topCentral );
        
        QScrollArea *sv = new QScrollArea(topCentral);
        sv->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        sv->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        sv->setWidgetResizable(true);
        central = new QWidget(0);
        sv->setWidget(central);
        vLayout->addWidget(sv);
        topLayout = new QVBoxLayout(central);
        topLayout->setMargin(10);
    }

    // Call the Sub-class's CreateWindowContents function to create the
    // internal parts of the window.
    CreateWindowContents();

    // Create a button layout and the buttons.
    vLayout->addSpacing(10);
    QGridLayout *buttonLayout = new QGridLayout(0);
    vLayout->addLayout(buttonLayout);
    buttonLayout->setColumnStretch(1, 50);

    // Create the extra buttons if necessary.
    if(buttonCombination & MakeDefaultButton)
    {
        QPushButton *makeDefaultButton = new QPushButton(tr("Make default"),
            topCentral);
        connect(makeDefaultButton, SIGNAL(clicked()),
                this, SLOT(makeDefaultHelper()));
        buttonLayout->addWidget(makeDefaultButton, 0, 0);
    }
    if(buttonCombination & ResetButton)
    {
        QPushButton *resetButton = new QPushButton(tr("Reset"), topCentral);
        connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
        buttonLayout->addWidget(resetButton, 0, 4);
    }

    if(buttonCombination & LoadButton)
    {
        QPushButton *loadButton = new QPushButton(tr("Load"),
            topCentral);
        connect(loadButton, SIGNAL(clicked()), this, SLOT(loadSubject()));
        buttonLayout->addWidget(loadButton, 0, 2);
    }
    if(buttonCombination & SaveButton)
    {
        QPushButton *saveButton = new QPushButton(tr("Save"), topCentral);
        connect(saveButton, SIGNAL(clicked()), this, SLOT(saveSubject()));
        buttonLayout->addWidget(saveButton, 0, 3);
    }

    if(buttonCombination & ApplyButton)
    {
        QPushButton *applyButton = new QPushButton(tr("Apply"), topCentral);
        connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
        buttonLayout->addWidget(applyButton, 1, 0);
    }
    else
    {
        // Add a little space to try and make up for the absence of the
        // grid layout.
        buttonLayout->setColumnStretch(1, 50);
    }

    postButton = new QPushButton(tr("Post"), topCentral);
    // Make the window post itself when the post button is clicked.
    if(notepad)
    {
        connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
        postButton->setEnabled(postEnabled);
    }
    else
        postButton->setEnabled(false);
    buttonLayout->addWidget(postButton, 1, 3);
    QPushButton *dismissButton = new QPushButton(tr("Dismiss"), topCentral);
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
    buttonLayout->addWidget(dismissButton, 1, 4);
    if(notepad != 0 && stretchWindow)
        vLayout->addStretch(0);

    // Set the isCreated flag.
    isCreated = true;
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the Apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 16:56:08 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::apply()
{
    // override in derived class.
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::makeDefaultHelper
//
// Purpose: 
//   This is a Qt slot function that calls the makeDefault() slot after
//   optionally displaying a confirmation dialog box.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 23 8:31:23 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Aug 27 11:17:21 PDT 2001
//   I changed the behavior of the third button in the confirmation window.
//
//   Brad Whitlock, Mon Nov 4 14:11:22 PST 2002
//   I made it so the "make default" behavior can be turned off permanently.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::makeDefaultHelper()
{
    if(makeDefaultConfirm)
    {
        QString msg(tr("Do you really want to make these the default attributes?"));

        // Ask the user if he really wants to set the defaults
        int button = QMessageBox::warning(0, "VisIt", msg, tr("Ok"), tr("Cancel"),
                                          tr("Yes, Do not prompt again"), 0, 1);

        if(button == 0)
        {
            // The user actually chose to set the defaults.
            makeDefault();
        }
        else if(button == 2)
        {
            // Make it so no confirmation is needed.
            makeDefaultConfirm = false;
            GlobalAttributes *globalAtts = GetViewerState()->GetGlobalAttributes();
            globalAtts->SetMakeDefaultConfirm(false);
            globalAtts->Notify();

            makeDefault();
        }
    }
    else
        makeDefault();
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::makeDefault
//
// Purpose: 
//   Causes the current attributes to be copied into the default
//   attributes. This should be overridden in derived classes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 13:40:19 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::makeDefault()
{
    // override in derived class.
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::reset
//
// Purpose: 
//   Causes the last applied attributes to be restored. This will
//   result in the window's UpdateWindow method being called to
//   update the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 13:40:19 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::reset()
{
    // override in derived class.
}

// ****************************************************************************
//  Method:  QvisPostableWindowSimpleObserver::loadSubject
//
//  Purpose:
//    Load the window's subject from a file.  This is empty
//    because this derived class doesn't have a sole subject.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  2, 2009
//
// ****************************************************************************
void
QvisPostableWindowSimpleObserver::loadSubject()
{
    // override in derived class.
}

// ****************************************************************************
//  Method:  QvisPostableWindowSimpleObserver::saveSubject
//
//  Purpose:
//    Save the window's subject to a file.  This is empty
//    because this derived class doesn't have a sole subject.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  2, 2009
//
// ****************************************************************************
void
QvisPostableWindowSimpleObserver::saveSubject()
{
    // override in derived class.
}

