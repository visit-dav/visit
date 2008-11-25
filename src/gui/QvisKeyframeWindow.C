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

#include "QvisKeyframeWindow.h"

#include <KeyframeAttributes.h>
#include <ViewerProxy.h>
#include <PlotList.h>

#include <WindowInformation.h>

#include <KeyframeDataModel.h>
#include <QvisKeyframeDelegate.h>

#include <QCheckBox>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPolygon>
#include <QPushButton>
#include <QTreeView>

// ****************************************************************************
//  Constructor:  QvisKeyframeWindow::QvisKeyframeWindow
//
//  Arguments:
//    subj       the keyframe attributes
//    caption    the window caption
//    shortname  the short version of the window name
//    notepad    the nodepad pointer
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 19 16:22:19 PDT 2002
//    Initialized globalAtts.
//
//    Jeremy Meredith, Tue Feb  4 17:49:09 PST 2003
//    Initialize viewItem.
//
//    Brad Whitlock, Fri Nov 7 17:29:56 PST 2003
//    Added ApplyButton flag.
//
//    Brad Whitlock, Wed Apr  9 11:09:51 PDT 2008
//    QString for caption, shortName.
//
//    Brad Whitlock, Mon Nov 10 11:59:01 PST 2008
//    Create keyframe data model.
//
// ****************************************************************************

QvisKeyframeWindow::QvisKeyframeWindow(KeyframeAttributes *subj,
                                       const QString &caption,
                                       const QString &shortName,
                                       QvisNotepadArea *notepad)
    : QvisPostableWindowSimpleObserver(caption, shortName, notepad,
                                       ApplyButton, false)
{
    kfAtts = subj;
    kfAtts->Attach(this);
    windowInfo = NULL;
    nFrames = 0;
    kv = 0;
    model = new KeyframeDataModel(this);
    model->ConnectKeyframeAttributes(kfAtts);
}

// ****************************************************************************
//  Destructor:  QvisKeyframeWindow::~QvisKeyframeWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Brad Whitlock, Fri May 10 10:43:33 PDT 2002
//    Added code to detach the keyframe atts.
//
// ****************************************************************************

QvisKeyframeWindow::~QvisKeyframeWindow()
{
    if(kfAtts)
        kfAtts->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::ConnectWindowInformation
//
//  Purpose:
//    Connect the window information state object to the keyframe window.
//
//  Arguments:
//    subj       the subject
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Oct 28 12:11:09 PDT 2008
//    Added the subject to the keyframe data model.
//
// ****************************************************************************

void
QvisKeyframeWindow::ConnectWindowInformation(WindowInformation *subj)
{
    windowInfo = subj;
    windowInfo->Attach(this);

    model->ConnectWindowInformation(subj);
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::ConnectPlotList
//
//  Purpose:
//    Connect the plotlist to the keyframe window.
//
//  Arguments:
//    subj       the subject
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Oct 28 12:11:09 PDT 2008
//    Added the subject to the keyframe data model.
//
// ****************************************************************************

void
QvisKeyframeWindow::ConnectPlotList(PlotList *subj)
{
    model->ConnectPlotList(subj);
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::SubjectRemoved
//
//  Purpose:
//    Called when a subject was removed as an observee.
//
//  Arguments:
//    subj       the subject
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Brad Whitlock, Fri May 10 10:41:29 PDT 2002
//    Added code to mark certain attributes as deleted.
//
//    Brad Whitlock, Sat Jan 24 23:59:13 PST 2004
//    I made it observe window information because of next generation file
//    handling.
//
// ****************************************************************************

void
QvisKeyframeWindow::SubjectRemoved(Subject *subj)
{
    if(subj == kfAtts)
        kfAtts = 0;
    else if(subj == windowInfo)
        windowInfo = 0;
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::CreateWindowContents
//
//  Purpose:
//    Create the innards of the window.
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 16:05:18 PST 2003
//    Added database keyframe widgets.
//
//    Brad Whitlock, Tue Apr 6 23:53:31 PST 2004
//    I hooked up a new signal to nFrames.
//
//    Jeremy Meredith, Thu Nov  1 16:35:43 EDT 2007
//    Tell the header to fill the empty space with the second column's width.
//    This works better than trying to do it manually.
//
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Thu Oct 23 10:59:22 PDT 2008
//    Qt 4.
//
// ****************************************************************************

void
QvisKeyframeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout;
    topLayout->addLayout(mainLayout);
    int row=0;

    mainLayout->addWidget(new QLabel(tr("Number of frames"), central),row,0);
    nFrames = new QLineEdit(central);
    connect(nFrames, SIGNAL(returnPressed()),
            this, SLOT(nFramesProcessText()));
    connect(nFrames, SIGNAL(textChanged(const QString &)),
            this, SLOT(userSetNFrames(const QString &)));
    mainLayout->addWidget(nFrames, row,1);
    row++;

    keyframeEnabledCheck = new QCheckBox(tr("Keyframing enabled"), central);
    connect(keyframeEnabledCheck, SIGNAL(toggled(bool)),
            this, SLOT(keyframeEnabledToggled(bool)));
    mainLayout->addWidget(keyframeEnabledCheck, row,0);
    row++;

    kv = new QTreeView(central);
    kv->setModel(model);
    connect(model, SIGNAL(modelReset()),
            kv, SLOT(expandAll()));
    connect(kv->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(newSelection()));
    kv->header()->setResizeMode(QHeaderView::ResizeToContents);
    kv->setEditTriggers(QAbstractItemView::AllEditTriggers);
    kv->setItemDelegateForColumn(1, new QvisKeyframeDelegate(this));
    mainLayout->addWidget(kv, row,0,1,3);
    row++;

    viewButton = new QPushButton(tr("Add view keyframe"), central);
    connect(viewButton, SIGNAL(clicked()),
            this, SLOT(addViewKeyframe()));
    mainLayout->addWidget(viewButton, row, 0);
    useViewKeyframes = new QCheckBox(tr("Use view keyframes"), central);
    connect(useViewKeyframes, SIGNAL(toggled(bool)),
            this, SLOT(useViewKFClicked(bool)));
    mainLayout->addWidget(useViewKeyframes, row,1);
    row++;

    dbStateLineEdit = new QLineEdit(central);
    mainLayout->addWidget(dbStateLineEdit, row, 1);
    dbStateButton = new QPushButton(tr("Add state keyframe"), central);
    connect(dbStateButton, SIGNAL(clicked()),
            this, SLOT(stateKFClicked()));
    mainLayout->addWidget(dbStateButton, row, 0);
    row++;

    mainLayout->setColumnStretch(0, 0);
    mainLayout->setColumnStretch(1, 0);
    mainLayout->setColumnStretch(2, 100);

    UpdateWindowInformation();
}

// ****************************************************************************
// Method: QvisKeyframeWindow::GetCurrentFrame
//
// Purpose: 
//   Returns the current frame for animation.
//
// Returns:    The current animation frame.
//
// Programmer: Brad Whitlock
// Creation:   Sun Jan 25 00:19:01 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Apr 6 23:46:31 PST 2004
//   I changed the name of the keyframe time slider.
//
// ****************************************************************************

int
QvisKeyframeWindow::GetCurrentFrame() const
{
    int curFrame = 0;

    for(int i = 0; i < windowInfo->GetTimeSliders().size(); ++i)
    {
        if(windowInfo->GetTimeSliders()[i] == KeyframeDataModel::KF_TIME_SLIDER)
        {
            curFrame = windowInfo->GetTimeSliderCurrentStates()[i];
            break;
        }
    }

    return curFrame;
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::UpdateWindowInformation
//
//  Purpose:
//    Updates the window based on new window information.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Feb  4 17:48:30 PST 2003
//    Added code for the view keyframing item.
//
//    Brad Whitlock, Sun Jan 25 00:03:44 PDT 2004
//    I made it use the window information and I made the number of frames
//    come from the keyframe atts.
//
//    Brad Whitlock, Tue Apr 6 23:54:37 PST 2004
//    I added code to block signals from nFrames.
//
//    Brad Whitlock, Mon Nov 10 11:36:20 PST 2008
//    Qt 4.
//
// ****************************************************************************

void
QvisKeyframeWindow::UpdateWindowInformation()
{
    if (!windowInfo || nFrames==0)
        return;

    QString temp;

    //
    // If we're in keyframing mode then there will be a time slider called
    // "Keyframe animation". Look for it in the time slider list and get
    // the number of frames from the keyframe attributes.
    //
    int curFrame = GetCurrentFrame();

    //
    // Get the number of frames from the keyframe atts.
    //
    int numFrames = kfAtts->GetNFrames();
    if (numFrames == 0)
    {
        numFrames = 1;
        curFrame = 0;
    }
    temp.sprintf("%d", numFrames);
    nFrames->blockSignals(true);
    nFrames->setText(temp);
    nFrames->blockSignals(false);

    useViewKeyframes->blockSignals(true);
    useViewKeyframes->setChecked(windowInfo->GetCameraViewMode());
    useViewKeyframes->blockSignals(false);
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::UpdateWindow
//
//  Purpose:
//    Called when one of the many observed subjects changes.
//
//  Arguments:
//    doAll      false if only the changed (i.e. selected) attributes
//               should be redrawn.
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 23 15:54:01 PST 2003
//    Moved most of the code into new methods.
//
//    Jeremy Meredith, Tue Feb  4 17:49:27 PST 2003
//    Added watch over camera view mode.
//
//    Brad Whitlock, Thu Mar 20 11:08:33 PDT 2003
//    I updated the attribute numbering for GlobalAttributes.
//
//    Brad Whitlock, Wed Jul 23 15:21:13 PST 2003
//    I made the Keyframing enabled checkbox get set when the keyframing
//    attributes are updated.
//
//    Brad Whitlock, Sun Jan 25 00:16:58 PDT 2004
//    I made it windowInfo instead of globalAtts.
//
//    Brad Whitlock, Wed Apr 7 00:13:09 PDT 2004
//    I added code to disable the time slider if we're not in keyframing mode.
//
//    Jeremy Meredith, Thu Nov  1 16:34:14 EDT 2007
//    Removed manual setting of the second column's width; it is now being
//    done automatically for us by a flag in the header, which seems to
//    fix a couple bugs.
//
//    Brad Whitlock, Fri Dec 14 17:29:46 PST 2007
//    Made it use ids.
//
//    Brad Whitlock, Mon Nov 10 11:35:37 PST 2008
//    Qt 4.
//
// ****************************************************************************

void
QvisKeyframeWindow::UpdateWindow(bool doAll)
{
    if (!SelectedSubject())
        return;

    if (SelectedSubject() == kfAtts)
    {
        for(int i = 0; i < kfAtts->NumAttributes(); ++i)
        {
            if(!doAll)
            {
                if(!kfAtts->IsSelected(i))
                {
                    continue;
                }
            }

            switch(i)
            {
            case KeyframeAttributes::ID_enabled:
                keyframeEnabledCheck->blockSignals(true);
                keyframeEnabledCheck->setChecked(kfAtts->GetEnabled());
                keyframeEnabledCheck->blockSignals(false);
                break;
            case KeyframeAttributes::ID_nFrames:
                UpdateWindowInformation();
                break;
            }
        }
    }
    if (SelectedSubject() == windowInfo)
    {
        // Be selective over what to update everything for.
        if(windowInfo->IsSelected(WindowInformation::ID_timeSliders) ||
           windowInfo->IsSelected(WindowInformation::ID_timeSliderCurrentStates) ||
           windowInfo->IsSelected(WindowInformation::ID_viewKeyframes) ||
           windowInfo->IsSelected(WindowInformation::ID_cameraViewMode))
        {
            UpdateWindowInformation();
        }
    }
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::GetCurrentValues
//
//  Purpose:
//    Get the current values of all text fields
//
//  Arguments:
//    which_widget   which widget to get
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Brad Whitlock, Sun Jan 25 00:22:14 PDT 2004
//    I made it use the keyframing attributes for the number of frames.
//
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Thu Oct 23 11:12:35 PDT 2008
//    QT 4.
//
// ****************************************************************************

void
QvisKeyframeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do nFrames
    if(which_widget == KeyframeAttributes::ID_nFrames || doAll)
    {
        int val;
        if(LineEditGetInt(nFrames, val))
            GetViewerMethods()->AnimationSetNFrames(val);
        else
        {
            ResettingError("number of frames", IntToQString(kfAtts->GetNFrames()));
            Message(msg);
            GetViewerMethods()->AnimationSetNFrames(kfAtts->GetNFrames());
        }
    }
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::Apply
//
//  Purpose:
//    Apply the current attributes.
//
//  Arguments:
//    ignore     true if we should get all values again
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Apr 7 00:02:24 PDT 2004
//    I changed the code so the number of frames is not set unless the user
//    first set it. I did this because the viewer has support for
//    automatically determining a good number of frames when you first enter
//    keyframing mode and by always setting the number before entering
//    keyframe mode, we can't automatically determine a good number of frames.
//
// ****************************************************************************

void
QvisKeyframeWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        //
        // If the user changed the number of frames then send it. Otherwise,
        // don't set the number of frames so the number can be determined
        // automatically.
        //
        if(kfAtts->GetNFramesWasUserSet())
            GetCurrentValues(-1);

        kfAtts->Notify();

        GetViewerMethods()->SetKeyframeAttributes();
    }
    else
        kfAtts->Notify();
}

//
// Qt slots.
//

// ****************************************************************************
//  Method:  QvisKeyframeWindow::newSelection
//
//  Purpose:
//    Make changes necessary when the selected KF item changes.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2003
//
// ****************************************************************************
void
QvisKeyframeWindow::newSelection()
{
    dbStateLineEdit->clear();
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::stateKFClicked
//
//  Purpose:
//    Callback for then the "Set state keyframe" button is clicked.
//    Adds a new database state keyframe.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2003
//
//  Modifications:
//    Brad Whitlock, Mon Nov 10 14:42:31 PST 2008
//    Rewrote for Qt 4.
//
// ****************************************************************************

void
QvisKeyframeWindow::stateKFClicked()
{
    QModelIndex index(kv->currentIndex());
    if(!index.isValid())
        return;

    int state = dbStateLineEdit->text().toInt();
    if (state < 0)
    {
        Error(tr("Cannot set state index to less than zero\n"));
        return;
    }

    int dt = index.data(KeyframeDataModel::DelegateTypeRole).toInt();
    int pos = index.data(KeyframeDataModel::CurrentIndexRole).toInt();
    int plotId = -1;
    if(dt == KeyframeDataModel::PlotDelegate ||
       dt == KeyframeDataModel::PlotAttsDelegate ||
       dt == KeyframeDataModel::PlotStateDelegate)
    {
        plotId = index.data(KeyframeDataModel::GetIdRole).toInt();
    }

    if(plotId != -1)
        GetViewerMethods()->SetPlotDatabaseState(plotId, pos, state);
    else
    {
        int nPlots = GetViewerState()->GetPlotList()->GetNumPlots();
        for(int i = 0; i < nPlots; ++i)
            GetViewerMethods()->SetPlotDatabaseState(i, pos, state);
    }
    dbStateLineEdit->clear();
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::apply
//
//  Purpose:
//    apply button callback
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
// ****************************************************************************
void
QvisKeyframeWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisKeyframeWindow::userSetNFrames
//
// Purpose: 
//   This is a Qt slot function that is called when the number of frames
//   changes. The purpose is only to record that the user entered a number 
//   of frames so it is not okay to automatically calculate a number of
//   frames.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 6 23:50:53 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWindow::userSetNFrames(const QString &)
{
    kfAtts->SetNFramesWasUserSet(true);
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::nFramesProcessText
//
//  Purpose:
//    number of frames text callback
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:42:36 PST 2003
//    Added code to update the global atts.
//
// ****************************************************************************
void
QvisKeyframeWindow::nFramesProcessText()
{
    GetCurrentValues(KeyframeAttributes::ID_nFrames);
    Apply();
    UpdateWindowInformation();
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::keyframeEnabledToggled
//
//  Purpose:
//    toggle the keyframe enabled mode
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Brad Whitlock, Tue Oct 28 09:27:41 PDT 2008
//    Don't update.
//
// ****************************************************************************
void
QvisKeyframeWindow::keyframeEnabledToggled(bool k)
{
    kfAtts->SetEnabled(k);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisKeyframeWindow::addViewKeyframe
//
// Purpose: 
//   Add a view keyframe.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov  4 16:48:16 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWindow::addViewKeyframe()
{
    GetViewerMethods()->SetViewKeyframe();
}

// ****************************************************************************
// Method: QvisKeyframeWindow::useViewKFClicked
//
// Purpose: 
//   Tells VisIt to use the view keyframes for setting the view.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 14:02:26 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisKeyframeWindow::useViewKFClicked(bool)
{
    GetViewerMethods()->ToggleCameraViewMode();
}
