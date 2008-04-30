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
#include "KFListViewItem.h"
#include "KFListView.h"
#include "KFTimeSlider.h"

#include <KeyframeAttributes.h>
#include <ViewerProxy.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <Plot.h>
#include <PlotList.h>
#include <QualifiedFilename.h>
#include <WindowInformation.h>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcursor.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qpainter.h>
#include <qpointarray.h>
#include <qradiobutton.h>
#include <qlistview.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <snprintf.h>

#include <algorithm>

#define KF_TIME_SLIDER "Keyframe animation"

using std::vector;
using std::map;

static QListViewItem *lastChild(QListViewItem *r);
static QListViewItem *lastChild(QListView *r);

static QListViewItem*
lastChild(QListViewItem *r)
{
    QListViewItem *i = r->firstChild();
    QListViewItem *j = NULL;
    while (i != NULL)
    {
        j = i;
        i = i->nextSibling();
    }
    return j;
}

static QListViewItem*
lastChild(QListView *r)
{
    QListViewItem *i = r->firstChild();
    QListViewItem *j = NULL;
    while (i != NULL)
    {
        j = i;
        i = i->nextSibling();
    }
    return j;
}


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
    plotList = NULL;
    windowInfo = NULL;
    lv = NULL;
    ts = NULL;
    viewItem = NULL;
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::AddSubjectToWindow
//
//  Purpose:
//    Add a generic attribute subject to the keyframe window.
//
//  Arguments:
//    subj       the subject
//    name       the name of the subject
//    parent     a possible parent listview item
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
// ****************************************************************************
KFListViewItem *
QvisKeyframeWindow::AddSubjectToWindow(AttributeSubject *subj, const char *name,
                                       KFListViewItem *parent)
{
    KFListViewItem *root;
    if (parent)
    {
        if (lastChild(parent))
            root = new KFListViewItem(parent, lastChild(parent));
        else
            root = new KFListViewItem(parent);
    }
    else
    {
        if (lastChild(lv))
            root = new KFListViewItem(lv, lastChild(lv));
        else
            root = new KFListViewItem(lv);
    }
    if (parent) parent->setOpen(true);
    root->setText(0, name);

#ifdef THIS_IS_FOR_THE_SUB_MEMBERS_OF_THE_ATTRIBUTES
    KFListViewItem *item = NULL;
    for (int i=0; i<subj->NumAttributes(); i++)
    {
        item = new KFListViewItem(root, /*old*/item);
        item->setText(0, subj->GetFieldName(i).c_str());
    }
#endif
    return root;
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::AddPlotToWindow
//
//  Purpose:
//    Add a plot attribute subject to the keyframe window.
//
//  Arguments:
//    subj       the subject
//    name       the name of the subject
//    parent     a possible parent listview item
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:42:13 PST 2003
//    Renamed "Time" to "State"
//
// ****************************************************************************
KFListViewItem *
QvisKeyframeWindow::AddPlotToWindow(AttributeSubject *subj, const char *name,
                                    KFListViewItem *parent)
{
    KFListViewItem *root;
    if (parent)
    {
        if (lastChild(parent))
            root = new KFListViewItem(parent, lastChild(parent));
        else
            root = new KFListViewItem(parent);
    }
    else
    {
        if (lastChild(lv))
            root = new KFListViewItem(lv, lastChild(lv));
        else
            root = new KFListViewItem(lv);
    }
    if (parent) parent->setOpen(true);
    root->setText(0, name);
    root->AddPoint(0.0);
    root->SetStyle(KFListViewItem::Style_Extents_and_Atts);

    KFListViewItem *timeItem = new KFListViewItem(root);
    timeItem->setText(0, tr("State"));
    timeItem->SetStyle(KFListViewItem::Style_Times);
    if (lv->GetNFrames() <= 1)
        timeItem->AddPoint(0);
    else
    {
        for (int j=0; j<lv->GetNFrames(); j++)
            timeItem->AddPoint(double(j)/double(lv->GetNFrames()-1));
    }

    KFListViewItem *attsItem = new KFListViewItem(root, timeItem);
    attsItem->setText(0, tr("Atts"));
    attsItem->AddPoint(0.0);

#ifdef THIS_IS_FOR_THE_SUB_MEMBERS_OF_THE_ATTRIBUTES
    KFListViewItem *item = NULL;
    for (int i=0; i<subj->NumAttributes(); i++)
    {
        item = new KFListViewItem(attsItem, /*old*/item);
        item->setText(0, subj->GetFieldName(i).c_str());
        item->AddPoint(0.0);
    }
#endif

    //root->setOpen(true);
    return root;
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::ConnectAttributes
//
//  Purpose:
//    Connect and add a generic attribute subject to the keyframe window.
//
//  Arguments:
//    subj       the subject
//    name       the name
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
// ****************************************************************************
void
QvisKeyframeWindow::ConnectAttributes(AttributeSubject *subj, const char *name)
{
    atts.push_back(subj);
    if (!name)
        name = subj->TypeName().c_str();

    nameMap[subj] = name;
    subj->Attach(this);
    if (lv)
        AddSubjectToWindow(subj, name);
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::ConnectPlotAttributes
//
//  Purpose:
//    Connect a plot attribute subject to the keyframe window.
//
//  Arguments:
//    subj       the subject
//    index      the index
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
// ****************************************************************************
void
QvisKeyframeWindow::ConnectPlotAttributes(AttributeSubject *subj, int index)
{
    plotAtts[index] = subj;
    subj->Attach(this);
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
// ****************************************************************************
void
QvisKeyframeWindow::ConnectWindowInformation(WindowInformation *subj)
{
    windowInfo = subj;
    windowInfo->Attach(this);
    UpdateWindowInformation();
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
// ****************************************************************************
void
QvisKeyframeWindow::ConnectPlotList(PlotList *subj)
{
    plotList = subj;
    subj->Attach(this);
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
    else if(subj == plotList)
        plotList = 0;
    else
    {
        // STILL NEED TO DO SOME WORK TO REMOVE FROM WINDOW HERE

        for (std::vector<AttributeSubject*>::iterator i=atts.begin(); i!=atts.end(); i++)
        {
            if (*i == subj)
            {
                atts.erase(i);
                break;
            }
        }
    }
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
//    Brad Whitlock, Sat Jan 24 23:59:47 PST 2004
//    I replaced 
// ****************************************************************************

QvisKeyframeWindow::~QvisKeyframeWindow()
{
    if(kfAtts)
        kfAtts->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);

    if(plotList)
        plotList->Detach(this);
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
// ****************************************************************************

void
QvisKeyframeWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(topLayout, 6,3,  4, "mainLayout");
    int row=0;

    keyframeEnabledCheck = new QCheckBox(tr("Keyframing enabled"),
                                         central, "keyframeEnabledCheck");
    mainLayout->addWidget(keyframeEnabledCheck, row,0);
    row++;

    snapToFrameCheck = new QCheckBox(tr("Snap to frame"),
                                     central, "snapToFrameCheck");
    mainLayout->addWidget(snapToFrameCheck, row, 0);
    snapToFrameCheck->setEnabled(false);
    row++;

    mainLayout->addWidget(new QLabel(tr("Number of frames"), central,
                          "nFramesLabel"),row,0);
    nFrames = new QLineEdit(central, "nFrames");
    mainLayout->addWidget(nFrames, row,1);
    row++;

    lv = new KFListView(central, "listview");
    lv->header()->setResizeEnabled(false);
    lv->header()->setClickEnabled(false);
    lv->header()->setMovingEnabled(false);
    lv->header()->setStretchEnabled(1,true);
    lv->setRootIsDecorated(true);
    lv->setSorting(-1);
    lv->addColumn(tr("Attributes"));
    lv->addColumn(tr("Keyframes"));
    mainLayout->addMultiCellWidget(lv, row,row, 0,2);
    row++;

    for (int i=0; i<atts.size(); i++)
        AddSubjectToWindow(atts[i],nameMap[atts[i]].c_str());

    ts = new KFTimeSlider(central, "timeSlider", lv);
    ts->setNSteps(lv->GetNFrames());
    mainLayout->addMultiCellWidget(ts, row,row,0,2);
    row++;

    dbStateLineEdit = new QLineEdit(central, "dbStateLineEdit");
    mainLayout->addWidget(dbStateLineEdit, row, 0);
    dbStateButton = new QPushButton(tr("Add state keyframe"), central,
                                    "dbStateButton");
    mainLayout->addWidget(dbStateButton, row, 1);
    row++;

    mainLayout->setColStretch(0, 0);
    mainLayout->setColStretch(1, 0);
    mainLayout->setColStretch(2, 100);

    snapToFrameCheck->setChecked(lv->GetSnap());

    connect(nFrames, SIGNAL(returnPressed()),
            this, SLOT(nFramesProcessText()));
    connect(nFrames, SIGNAL(textChanged(const QString &)),
            this, SLOT(userSetNFrames(const QString &)));
    connect(ts, SIGNAL(valueChanged(int)),
            lv, SLOT(timeChanged(int)));
    connect(ts, SIGNAL(valueChanged(int)),
            this, SLOT(timeChanged(int)));
    connect(keyframeEnabledCheck, SIGNAL(toggled(bool)),
            this, SLOT(keyframeEnabledToggled(bool)));
    connect(snapToFrameCheck, SIGNAL(toggled(bool)),
            lv, SLOT(snapToFrameToggled(bool)));
    connect(lv, SIGNAL(selectionChanged()),
            this, SLOT(newSelection()));
    connect(dbStateButton, SIGNAL(clicked()),
            this, SLOT(stateKFClicked()));

    UpdateWindowInformation();
    UpdatePlotList();
    UpdateWindowSensitivity();
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
//  Method:  QvisKeyframeWindow::timeChanged
//
//  Purpose:
//    time slider changed callback
//
//  Arguments:
//    t          the new timestep
//
//  Programmer:  Jeremy Meredith
//  Creation:    May  8, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Jan 27 21:47:40 PST 2004
//    Changed to support multiple time sliders, etc.
//
//    Brad Whitlock, Wed Apr 7 00:08:51 PDT 2004
//    I added code to switch to the keyframing time slider if that's not the
//    current time slider.
//
// ****************************************************************************

void
QvisKeyframeWindow::timeChanged(int t)
{
    //
    // Set the active time slider to be the keyframing time slider if that's
    // not the currently active time slider.
    //
    if((windowInfo->GetActiveTimeSlider() >= 0) &&
       (windowInfo->GetTimeSliders()[windowInfo->GetActiveTimeSlider()] !=
        KF_TIME_SLIDER))
    {
        GetViewerMethods()->SetActiveTimeSlider(KF_TIME_SLIDER);
    }

    // Set the active time slider to the kf time slider???
    GetViewerMethods()->SetTimeSliderState(t);
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
    GetCurrentValues(0);
    Apply();
    UpdateWindowInformation();
    UpdatePlotList();
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
// ****************************************************************************
void
QvisKeyframeWindow::keyframeEnabledToggled(bool k)
{
    kfAtts->SetEnabled(k);
    Apply();
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
        if(windowInfo->GetTimeSliders()[i] == KF_TIME_SLIDER)
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
// ****************************************************************************

void
QvisKeyframeWindow::UpdateWindowInformation()
{
    if (!windowInfo || !lv || !ts)
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

    ts->blockSignals(true);
    lv->SetNFrames(numFrames);
    ts->setNSteps(numFrames);
    lv->timeChanged(curFrame);
    ts->setValue(curFrame);
    ts->blockSignals(false);
    temp.sprintf("%d", numFrames);

    nFrames->blockSignals(true);
    nFrames->setText(temp);
    nFrames->blockSignals(false);

    if (!viewItem)
    {
        viewItem = new KFListViewItem(lv, NULL);
        viewItem->setText(0, tr("View"));
        viewItem->SetIsView(true);
    }
    double nframeminus1 = (numFrames - 1);
    if (nframeminus1 < 1)
        nframeminus1 = 1;

    viewItem->Initialize();
    const intVector &vkf = windowInfo->GetViewKeyframes();
    for (int j=0; j<vkf.size(); j++)
        viewItem->AddPoint(double(vkf[j]) / nframeminus1);
    viewItem->repaint();
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::UpdatePlotList
//
//  Purpose:
//    Update the window based on a new plotlist
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 31 09:42:54 PST 2003
//    Fixed logic to see if all db state keyframes were the same across all
//    plots.  Added code to set the plotid for the list view items.
//
// ****************************************************************************
void
QvisKeyframeWindow::UpdatePlotList()
{
    if (!plotList || !lv)
        return;

    // Determine if all plots for each database are the same
    int nplots = plotList->GetNumPlots();
    vector<bool> allsame(nplots, true);
    vector<bool> counted(nplots, false);
    int i;
    for(i = 0; i < nplots; ++i)
    {
        if (counted[i])
        {
            continue;
        }

        counted[i] = true;
        Plot &ploti = (*plotList)[i];
        QualifiedFilename qfi(ploti.GetDatabaseName());
        const intVector &dbkeysi = ploti.GetDatabaseKeyframes();
        int plotbegi = ploti.GetBeginFrame();
        bool same = true;
        int j;
        for (j=i+1; j<nplots && same; j++)
        {
            Plot &plotj = (*plotList)[j];
            QualifiedFilename qfj(plotj.GetDatabaseName());
            if (qfi == qfj)
            {
                counted[j] = true;
                const intVector &dbkeysj = plotj.GetDatabaseKeyframes();
                int plotbegj = plotj.GetBeginFrame();
                if (dbkeysi.size() != dbkeysj.size())
                    same = false;
                int len = dbkeysi.size();
                for (int k=0; k<len && same; k++)
                {
                    if (dbkeysi[k]+plotbegi != dbkeysj[k]+plotbegj)
                        same = false;
                    // NOTE: need to check actual values when available, not
                    //       just the locations of them
                }
            }
        }

        allsame[i] = same;
        for (j=i+1; j<nplots; j++)
        {
            Plot &plotj = (*plotList)[j];
            QualifiedFilename qfj(plotj.GetDatabaseName());
            if (qfi == qfj)
            {
                allsame[j] = same;
            }
        }
    }

    // Add the databases, plots to the window
    vector<int> usedIds;
    for(i = 0; i < plotList->GetNumPlots(); ++i)
    {
        Plot &current = plotList->operator[](i);
        QualifiedFilename qualifiedFile(current.GetDatabaseName());
        double nframeminus1 = (lv->GetNFrames()-1);
        if (nframeminus1 < 1)
            nframeminus1 = 1;

        KFListViewItem *fileitem = NULL;
        KFListViewItem *timeitem = NULL;
        int j;
        for (j=0; j<fileItems.size(); j++)
        {
            // look for an existing database item
            if (fileItems[j]->text(0) == QString(qualifiedFile.filename.c_str()))
                fileitem = fileItems[j];
        }
        if (!fileitem)
        {
            // Make the database item
            fileitem = new KFListViewItem(lv, lastChild(lv));
            fileitem->setText(0, qualifiedFile.filename.c_str());
            fileItems.push_back(fileitem);

            timeitem = new KFListViewItem(fileitem);
        }
        else
        {
            timeitem = (KFListViewItem*)fileitem->firstChild();
        }

        // get the plot begin and end frames (for keyframe index origin)
        int plotbeg = current.GetBeginFrame();
        int plotend = current.GetEndFrame();
            
        // update the database-wide times
        const intVector &dbkeys = current.GetDatabaseKeyframes();
        timeitem->Initialize();
        timeitem->setText(0, tr("State"));
        timeitem->SetStyle(KFListViewItem::Style_Times);
        if (allsame[i])
        {
            for (j=0; j<dbkeys.size(); j++)
            {
                timeitem->AddPoint(double(dbkeys[j]+plotbeg)/nframeminus1);
            }
        }

        PlotPluginManager *pmgr = PlotPluginManager::Instance();
        int type = current.GetPlotType();
        std::string id = pmgr->GetEnabledID(type);

        char plotname[256];
        SNPRINTF(plotname, 256, "%s(%s)", 
                 pmgr->GetCommonPluginInfo(id)->GetName(),
                 current.GetPlotVar().c_str());
        plotname[255]=0;

        KFListViewItem *plotitem;
        if (plotMap.count(current.GetId()) == 0)
        {
            plotitem = AddPlotToWindow(GetViewerState()->GetPlotAttributes(type),
                                       plotname, fileitem);
            plotMap[current.GetId()] = plotitem;
        }
        else
        {
            plotitem = plotMap[current.GetId()];
            plotitem->setText(0, plotname);
        }

        // Set the keyframe points
        timeitem=(KFListViewItem*)plotitem->firstChild();
        KFListViewItem *attsitem=(KFListViewItem*)timeitem->nextSibling();
        plotitem->Initialize();
        timeitem->Initialize();
        attsitem->Initialize();

        plotitem->SetPlotId(i);
        timeitem->SetPlotId(i);
        attsitem->SetPlotId(i);

        plotitem->SetBegin(plotbeg/nframeminus1);
        plotitem->SetEnd(plotend/nframeminus1);
        //const intVector &dbkeys = current.GetDatabaseKeyframes();
        for (j=0; j<dbkeys.size(); j++)
        {
            timeitem->AddPoint(double(dbkeys[j]+plotbeg)/nframeminus1);
        }

        vector<int> keyframes = current.GetKeyframes();
        for (j=0; j<keyframes.size(); j++)
        {
            plotitem->AddPoint(double(keyframes[j]+plotbeg)/nframeminus1);
            attsitem->AddPoint(double(keyframes[j]+plotbeg)/nframeminus1);
        }
            
        usedIds.push_back(current.GetId());
    }

    // Clear any unused plots
    for (std::map<int,KFListViewItem*>::iterator it=plotMap.begin(); it!=plotMap.end(); it++)
    {
        int id = it->first;
        bool used = false;
        for (int j=0; j<usedIds.size(); j++)
            if (usedIds[j] == id)
                used = true;
        if (!used) 
        {
            delete it->second;
            plotMap.erase(it->first);
        }
    }

    // Clear any empty file containers
    bool done = false;
    while (!done)
    {
        done = true;
        std::vector<KFListViewItem*>::iterator i;
        for (i=fileItems.begin(); i!=fileItems.end(); i++)
        {
            if ((*i)->childCount() == 1)
            {
                // Contains only the time stamps
                done = false;
                delete (*i)->firstChild();
                delete (*i);
                fileItems.erase(i);
                break;
            }
        }
    }
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
                ts->setEnabled(kfAtts->GetEnabled());
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
           windowInfo->IsSelected(WindowInformation::ID_viewKeyframes))
        {
            UpdateWindowInformation();
        }
    }
    else if (SelectedSubject() == plotList)
    {
        UpdatePlotList();
    }
    else
    {
        bool isPlot = false;
        std::map<int, AttributeSubject*>::iterator i;
        for (i = plotAtts.begin(); i != plotAtts.end(); i++)
        {
            if ((i->second) == SelectedSubject())
            {
                isPlot = true;
                break;
            }
        }
        if (isPlot)
        {
            //cerr << "Plot atts changed: "<<(i->second)->TypeName()<<"\n";
            for (int i=0; i<plotList->GetNumPlots(); i++)
            {
                const Plot &p = (*plotList)[i];
                if (p.GetActiveFlag())
                {
                    AttributeSubject *a = plotAtts[p.GetPlotType()];
                    KFListViewItem *item = plotMap[p.GetId()];
                    KFListViewItem *field = (KFListViewItem*)item->firstChild();
                    for (int j=0; j<a->NumAttributes(); j++)
                    {
                        if (a->IsSelected(j))
                        {
                            if (lv->GetNFrames() > 1)
                                field->AddPoint(double(GetCurrentFrame())/double(lv->GetNFrames()-1));
                        }
                        field = (KFListViewItem*)field->nextSibling();
                    }
                }
            }
        }
        else
        {
            // ONE OF OUR OTHER KEYFRAMED SUBJECTS CHANGED
            //cerr << "Unknown subject changed\n";
        }
    }

    ts->updateSize();
    qApp->processEvents();
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
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisKeyframeWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do nFrames
    if(which_widget == 0 || doAll)
    {
        temp = nFrames->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            GetViewerMethods()->AnimationSetNFrames(val);
        }

        if(!okay)
        {
            msg = tr("The value for the number of frames was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(kfAtts->GetNFrames());
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
    UpdateWindowSensitivity();
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
// ****************************************************************************
void
QvisKeyframeWindow::stateKFClicked()
{
    KFListViewItem *item = (KFListViewItem*)lv->selectedItem();
    if (item->GetStyle() != KFListViewItem::Style_Times)
        return;

    int state = dbStateLineEdit->text().toInt();
    if (state < 0)
    {
        Error(tr("Cannot set state index to less than zero\n"));
        return;
    }

    int pos = lv->GetCurrentIndex();

    if (item->GetPlotId() >= 0)
    {
        GetViewerMethods()->SetPlotDatabaseState(item->GetPlotId(), pos, state);
    }
    else
    {
        KFListViewItem *p = (KFListViewItem*)item->nextSibling();
        while (p)
        {
            GetViewerMethods()->SetPlotDatabaseState(p->GetPlotId(), pos, state);
            p = (KFListViewItem*)p->nextSibling();
        }
    }
    dbStateLineEdit->clear();
}

// ****************************************************************************
//  Method:  QvisKeyframeWindow::UpdateWindowSensitivity
//
//  Purpose:
//    Updates sensitivity for any items that are variably enabled.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2003
//
// ****************************************************************************
void
QvisKeyframeWindow::UpdateWindowSensitivity()
{
    KFListViewItem *item = (KFListViewItem*)lv->selectedItem();
    if (!item || item->GetStyle() != KFListViewItem::Style_Times)
    {
        dbStateButton->setEnabled(false);
        dbStateLineEdit->setEnabled(false);
    }
    else
    {
        dbStateButton->setEnabled(true);
        dbStateLineEdit->setEnabled(true);
    }
}

