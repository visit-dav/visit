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

#include <stdio.h>
#include <QvisAnimationWindow.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <AnimationAttributes.h>
#include <ViewerProxy.h>

#define SLOWEST_TIMEOUT 1001

// ****************************************************************************
// Method: QvisAnimationWindow::QvisAnimationWindow
//
// Purpose: 
//   This is the constructor for the QvisAnimationWindow class.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Oct 7 09:37:42 PDT 2003
//   Added playbackModeButtonGroup.
//
// ****************************************************************************

QvisAnimationWindow::QvisAnimationWindow(AnimationAttributes *subj,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    animationAtts = subj;
    playbackModeButtonGroup = 0;
}

// ****************************************************************************
// Method: QvisAnimationWindow::~QvisAnimationWindow
//
// Purpose: 
//   This is the destructor for the QvisAnimationWindow class.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Oct 7 09:38:31 PDT 2003
//   Deleted playbackModeButtonGroup since it has no parent.
//
// ****************************************************************************

QvisAnimationWindow::~QvisAnimationWindow()
{
    delete playbackModeButtonGroup;
}

// ****************************************************************************
// Method: QvisAnimationWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the window.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 14 11:39:42 PDT 2002
//   Added a slider for the animation playback speed.
//
//   Brad Whitlock, Mon Oct 6 16:21:12 PST 2003
//   Added radio buttons that allow the user to set the animation style.
//
// ****************************************************************************

void
QvisAnimationWindow::CreateWindowContents()
{
    // Create a grid layout.
    QGridLayout *animLayout = new QGridLayout(topLayout, 6, 4);
    animLayout->setSpacing(5);

    // Create the check box for pipeline caching.
    pipelineCachingToggle = new QCheckBox("Cache animation for faster playback",
        central, "pipelineCachingToggle");
    connect(pipelineCachingToggle, SIGNAL(toggled(bool)),
            this, SLOT(pipelineCachingToggled(bool)));
    animLayout->addMultiCellWidget(pipelineCachingToggle, 0, 0, 0, 3);

    // Add the animation style controls.
    QLabel *styleLabel = new QLabel("Animation playback", central, "styleLabel");
    animLayout->addMultiCellWidget(styleLabel, 1, 1, 0, 3);
    playbackModeButtonGroup = new QButtonGroup(0, "playbackModeButtonGroup");
    QRadioButton *rb = new QRadioButton("Looping", central);
    playbackModeButtonGroup->insert(rb);
    animLayout->addWidget(rb, 2, 1);
    rb = new QRadioButton("Play once", central);
    playbackModeButtonGroup->insert(rb);
    animLayout->addWidget(rb, 2, 2);
    rb = new QRadioButton("Swing", central);
    playbackModeButtonGroup->insert(rb);
    animLayout->addWidget(rb, 2, 3);
    connect(playbackModeButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(playbackModeChanged(int)));

    // Create the slider and some labels.
    timeoutSlider = new QSlider(Qt::Horizontal, central, "timeoutSlider");
    timeoutSlider->setTickmarks(QSlider::Below);
    timeoutSlider->setTracking(false);
    timeoutSlider->setRange(1, SLOWEST_TIMEOUT - 1);
    timeoutSlider->setPageStep(100);
    connect(timeoutSlider, SIGNAL(valueChanged(int)),
            this, SLOT(timeoutChanged(int)));
    animLayout->addMultiCellWidget(timeoutSlider, 4, 4, 0, 3);
    QLabel *speedLabel = new QLabel(timeoutSlider, "Animation speed",
        central, "speedLabel");
    animLayout->addMultiCellWidget(speedLabel, 3, 3, 0, 3);

    // Create the slower and faster labels.
    QLabel *slowerLabel = new QLabel("slower", central, "speedLabel");
    animLayout->addMultiCellWidget(slowerLabel, 5, 5, 0, 1);
    QLabel *fasterLabel = new QLabel("faster", central, "speedLabel");
    animLayout->addWidget(fasterLabel, 5, 3, Qt::AlignRight);
}

// ****************************************************************************
// Method: QvisAnimationWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the animation attributes are updated.
//
// Arguments:
//   doAll : Whether or not to update all widgets.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 14 11:49:44 PDT 2002
//   Added animation timeout slider.
//
//   Brad Whitlock, Mon Oct 6 16:24:09 PST 2003
//   Added the animation style button group.
//
// ****************************************************************************

void
QvisAnimationWindow::UpdateWindow(bool doAll)
{
    AnimationAttributes *atts = (AnimationAttributes *)subject;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!atts->IsSelected(i) && !doAll)
            continue;

        switch(i)
        {
        case AnimationAttributes::ID_pipelineCachingMode:
            pipelineCachingToggle->blockSignals(true);
            pipelineCachingToggle->setChecked(atts->GetPipelineCachingMode());
            pipelineCachingToggle->blockSignals(false);
            break;
        case AnimationAttributes::ID_timeout:
            timeoutSlider->blockSignals(true);
            timeoutSlider->setValue(SLOWEST_TIMEOUT - atts->GetTimeout());
            timeoutSlider->blockSignals(false);
            break;
        case AnimationAttributes::ID_playbackMode:
            playbackModeButtonGroup->blockSignals(true);
            playbackModeButtonGroup->setButton(int(atts->GetPlaybackMode()));
            playbackModeButtonGroup->blockSignals(false);
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisAnimationWindow::Apply
//
// Purpose: 
//   This method is called when the animation attributes are updated.
//
// Arguments:
//   ignore : Whether or not to ignore the apply.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// ****************************************************************************

void
QvisAnimationWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Tell the viewer to set the animation attributes.
        if(animationAtts->NumAttributesSelected() > 0 || ignore)
        {
            animationAtts->Notify();
            GetViewerMethods()->SetAnimationAttributes();
        }
    }
    else
    {
        // Send the new state to the viewer.
        animationAtts->Notify();
    }
}
 
//
// Qt Slot functions...
//

void
QvisAnimationWindow::apply()
{
    Apply(true);
}

void
QvisAnimationWindow::pipelineCachingToggled(bool val)
{
    animationAtts->SetPipelineCachingMode(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnimationWindow::timeoutChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the animation timeout
//   slider is changed.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 14 11:52:30 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnimationWindow::timeoutChanged(int val)
{
    animationAtts->SetTimeout(SLOWEST_TIMEOUT - val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAnimationWindow::playbackModeChanged
//
// Purpose: 
//   This a Qt slot function that is called when the animation style changes.
//
// Arguments:
//   val : The new animation style.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 6 16:28:00 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnimationWindow::playbackModeChanged(int val)
{
    animationAtts->SetPlaybackMode((AnimationAttributes::PlaybackMode)val);
    SetUpdate(false);
    Apply();
}
