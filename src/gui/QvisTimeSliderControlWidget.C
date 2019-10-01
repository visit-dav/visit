/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <visitstream.h>

#include <QComboBox>
#include <QDebug>
#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>

#include <QvisTimeSliderControlWidget.h>
#include <QvisAnimationSlider.h>
#include <QvisVCRControl.h>

#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DebugStream.h>
#include <FileServerList.h>
#include <GlobalAttributes.h>
#include <NameSimplifier.h>
#include <Plot.h>
#include <PlotList.h>
#include <WindowInformation.h>
#include <Utility.h>
#include <ViewerProxy.h>
#include <avtDatabaseMetaData.h>
#include <KeyframeAttributes.h>


// ****************************************************************************
// Method: QvisTimeSliderControl::QvisTimeSliderControl
//
// Purpose:
//   Constructor for the QvisTimeSliderControl class.
//
// Arguments:
//   parent     : A pointer to the widget's parent widget.
//   name       : The widget's name.
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:35:07 PDT 2000
//
// Modifications:
//  Kathleen Biagas, Mon Sep 30 09:25:45 PDT 2019
//  Set inital state of VCR play buttons to disabled. Add plotList.
//
// ****************************************************************************

QvisTimeSliderControlWidget::QvisTimeSliderControlWidget(QWidget *parent) :
   QGroupBox(tr("Time"),parent), SimpleObserver(), GUIBase(),
   timeStateFormat()
{
    // Create the top layout that will contain the widgets.
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(5);
    topLayout->setMargin(5);

    // Create the active time slider.
    QHBoxLayout *tsLayout = new QHBoxLayout(0);
    tsLayout->setMargin(0);
    topLayout->addLayout(tsLayout);
    activeTimeSlider = new QComboBox(this);
    connect(activeTimeSlider, SIGNAL(activated(int)),
            this, SLOT(changeActiveTimeSlider(int)));
    activeTimeSliderLabel = new QLabel(tr("Active time slider"), this);
    tsLayout->addWidget(activeTimeSliderLabel);
    activeTimeSliderLabel->hide();
    tsLayout->addWidget(activeTimeSlider, 10);
    activeTimeSlider->hide();

    // Create the animation position slider bar
    QHBoxLayout *animationLayout = new QHBoxLayout(0);
    animationLayout->setMargin(0);
    topLayout->addLayout(animationLayout);
    topLayout->setStretchFactor(animationLayout, 10);
    animationPosition = new QvisAnimationSlider(Qt::Horizontal, this);
    animationPosition->setEnabled(false);
    connect(animationPosition, SIGNAL(sliderPressed()),
            this, SLOT(sliderStart()));
    connect(animationPosition, SIGNAL(sliderMoved(int)),
            this, SLOT(sliderMove(int)));
    connect(animationPosition, SIGNAL(sliderWasReleased()),
            this, SLOT(sliderEnd()));
    connect(animationPosition, SIGNAL(sliderValueChanged(int)),
            this, SLOT(sliderChange(int)));
    animationLayout->addWidget(animationPosition, 25);

    // Create the animation time field.
    timeField = new QLineEdit(this);
    timeField->setEnabled(false);
    connect(timeField, SIGNAL(returnPressed()), this, SLOT(processTimeText()));
    animationLayout->addWidget(timeField, 5);

    // Create the VCR controls.
    vcrControls = new QvisVCRControl(this);
    vcrControls->setEnabled(false);
    vcrControls->SetPlayEnabledState(false);
    connect(vcrControls, SIGNAL(prevFrame()), this, SLOT(backwardStep()));
    connect(vcrControls, SIGNAL(reversePlay()), this, SLOT(reversePlay()));
    connect(vcrControls, SIGNAL(stop()), this, SLOT(stop()));
    connect(vcrControls, SIGNAL(play()), this, SLOT(play()));
    connect(vcrControls, SIGNAL(nextFrame()), this, SLOT(forwardStep()));
    topLayout->addWidget(vcrControls);

    // Initialize the attached subjects
    windowInfo = NULL;
    plotList = NULL;

}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::~QvisTimeSliderControlWidget
//
// Purpose:
//   Destructor for the QvisTimeSliderControl class.
//
// Programmer: Cyrus Harrison
// Creation:   Fri Mar 12 10:59:55 PST 2010
//
// Modifications:
//  Kathleen Biagas, Mon Sep 30 09:29:12 PDT 2019
//  Added plotList.
//
// ****************************************************************************

QvisTimeSliderControlWidget::~QvisTimeSliderControlWidget()
{
    if(fileServer)
        fileServer->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);

    if(plotList)
        plotList->Detach(this);

}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::SetTimeStateFormat
//
// Purpose:
//   Sets the display mode for the file panel. We can make it display files
//   using cycle information or we can make it use time information.
//
// Arguments:
//   m : The new timestate display mode.
//
// Notes:      This method resets the text on the expanded databases, which
//             are open databases with more than one time state, so that
//             they show time using the new timestate display mode.
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 16:15:46 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 29 10:13:15 PDT 2003
//   I added code to help determine how labels are displayed if a database is
//   virtual.
//
//   Brad Whitlock, Sun Jan 25 01:28:55 PDT 2004
//   I added support for multiple time sliders.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
//
//   Mark C. Miller, Fri Aug 10 23:11:55 PDT 2007
//   Propogated knowledge that item was updated with metadata that was forced
//   to get accurate cycles/times.
//
//   Cyrus Harrison, Tue Jul  1 16:04:25 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::SetTimeStateFormat(const TimeFormat &m)
{
    if(m != timeStateFormat)
    {
        timeStateFormat = m;

        // Update the time text field using the current time slider.
        //
        int activeTS = windowInfo->GetActiveTimeSlider();
        int currentState = 0;
        if(activeTS >= 0)
            currentState = windowInfo->GetTimeSliderCurrentStates()[activeTS];
        UpdateTimeFieldText(currentState);
    }
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::GetTimeStateFormat
//
// Purpose:
//   Returns the time state format.
//
// Returns:    The time state format.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 17:19:20 PST 2003
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//
// ****************************************************************************

const TimeFormat &
QvisTimeSliderControlWidget::GetTimeStateFormat() const
{
    return timeStateFormat;
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::Update
//
// Purpose:
//   This method tells the widget to update itself when the subject
//   changes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 10:37:19 PDT 2000
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Brad Whitlock, Sun Jan 25 01:28:23 PDT 2004
//   I made it use windowInfo instead of globalAtts.
//
//   Kathleen Biagas, Mon Sep 30 09:29:50 PDT 2019
//   Added plotList. Set 'shouldEnablePlay; based on active, completed
//   time-varying plot that follows time.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::Update(Subject *TheChangedSubject)
{
    if(fileServer == 0 || windowInfo == 0 || plotList == 0)
        return;
    else if(TheChangedSubject == windowInfo)
        UpdateAnimationControls(false);
    else if(TheChangedSubject == plotList)
    {
        bool shouldEnablePlay = false;
        for (int i = 0; i < plotList->GetNumPlots() && !shouldEnablePlay; ++i)
        {
            const Plot &current = plotList->operator[](i);
            shouldEnablePlay = (current.GetActiveFlag() &&
                                current.GetFollowsTime() &&
                                current.GetStateType() == Plot::Completed);
        }
        vcrControls->SetPlayEnabledState(shouldEnablePlay);
    }
}


// ****************************************************************************
// Method: QvisTimeSliderControlWidget::UpdateAnimationControls
//
// Purpose:
//   This method is called when the GlobalAttributes subject that this
//   widget watches is updated.
//
// Arguments:
//   doAll : A flag indicating whether to ignore any partial selection.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 16:04:38 PST 2000
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:53:13 PST 2001
//   Moved the cycle/time code into a sub-function.
//
//   Eric Brugger, Fri Nov  2 17:07:24 PST 2001
//   I added code to handle the current file changing.
//
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//
//   Brad Whitlock, Thu Mar 20 11:11:14 PDT 2003
//   I changed the ordering of fields in GlobalAttributes.
//
//   Brad Whitlock, Thu May 15 13:19:13 PST 2003
//   I changed the call to FileServer::OpenFile.
//
//   Brad Whitlock, Wed Jul 2 15:58:47 PST 2003
//   I added exception handling code for when the metadata cannot be read.
//
//   Brad Whitlock, Tue Sep 9 15:40:40 PST 2003
//   I made it return early if the slider is down.
//
//   Brad Whitlock, Mon Dec 8 15:39:56 PST 2003
//   I changed the code so the file server's open file is never changed unless
//   it is changed to the same file that is open. The purpose of that is to
//   expand databases that we previously displayed as a single file (like
//   .visit files). This also allows the selected files list to show the active
//   time state for the database.
//
//   Brad Whitlock, Tue Dec 30 14:31:03 PST 2003
//   I made it use the animation slider instead of sliderDown.
//
//   Brad Whitlock, Sat Jan 24 22:53:05 PST 2004
//   I made it use the new time and file scheme. I also added code to set
//   the values in the new activeTimeSlider combobox.
//
//   Brad Whitlock, Fri Mar 18 13:39:02 PST 2005
//   I improved how the time slider names are shortened so all of the sources
//   are taken into account, which can make it easier to distinguish between
//   time sliders when there are multiple time sliders whose names only
//   differ by the path to their database.
//
//   Brad Whitlock, Mon Dec 17 11:02:28 PST 2007
//   Made it use ids.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::UpdateAnimationControls(bool doAll)
{
    if(fileServer == 0 || windowInfo == 0 ||
       (animationPosition != 0 && animationPosition->sliderIsDown()))
        return;

    //
    // Try and find a correlation for the active time slider.
    //
    int activeTS = windowInfo->GetActiveTimeSlider();

    // timeSliders changed. That's the list of time slider names.
    if(windowInfo->IsSelected(WindowInformation::ID_timeSliders) || doAll)
    {
        //
        // Set the values in the active time slider selector and set its
        // visibility. If we don't have an active time slider then we must
        // have no time sliders and thus should not show the time slider
        // selector.
        //
        if(activeTS >= 0)
        {
            activeTimeSlider->blockSignals(true);
            activeTimeSlider->clear();
            const stringVector &tsNames = windowInfo->GetTimeSliders();

            //
            // Use a name simplifier to shorten the source names.
            //
            NameSimplifier simplifier;
            const stringVector &sources = GetViewerState()->
                GetGlobalAttributes()->GetSources();
            for(size_t i = 0; i < sources.size(); ++i)
                simplifier.AddName(sources[i]);
            stringVector shortNames;
            simplifier.GetSimplifiedNames(shortNames);

            //
            // Fill in the combo box using the short name for sources and
            // time slider names for items that are not sources.
            //
            for(size_t i = 0; i < tsNames.size(); ++i)
            {
                int index = -1;
                for(size_t j = 0; j < sources.size(); ++j)
                {
                    if(sources[j] == tsNames[i])
                    {
                        index = j;
                        break;
                    }
                }
                if(index == -1)
                {
                    // The time slider is not a source so use the original
                    // time slider name.
                    activeTimeSlider->addItem(tsNames[i].c_str());
                }
                else
                {
                    // The time slider was a source, use the short name.
                    activeTimeSlider->addItem(shortNames[index].c_str());
                }
            }
            activeTimeSlider->setCurrentIndex(activeTS);
            activeTimeSlider->blockSignals(false);

            bool enableSlider = windowInfo->GetTimeSliders().size() > 1;
            activeTimeSlider->setEnabled(enableSlider);
            activeTimeSliderLabel->setEnabled(enableSlider);

            if(enableSlider && !activeTimeSlider->isVisible())
            {
                activeTimeSlider->show();
                activeTimeSliderLabel->show();
                updateGeometry();
            }
            else if(!enableSlider && activeTimeSlider->isVisible())
            {
                activeTimeSlider->hide();
                activeTimeSliderLabel->hide();
                updateGeometry();
            }
        }
        else if(activeTimeSlider->isVisible())
        {
            activeTimeSlider->setEnabled(false);
            activeTimeSliderLabel->setEnabled(false);
            activeTimeSlider->hide();
            activeTimeSliderLabel->hide();
            updateGeometry();
        }
    }

    // Update the animation controls.
    if(windowInfo->IsSelected(WindowInformation::ID_activeTimeSlider) ||
       windowInfo->IsSelected(WindowInformation::ID_timeSliderCurrentStates) || doAll)
    {
        DatabaseCorrelationList *cL = GetViewerState()->
            GetDatabaseCorrelationList();
        DatabaseCorrelation *activeTSCorrelation = 0;
        if(activeTS >= 0)
        {
            // Try and find a correlation for the active time slider so we
            // can get the number of states in the correlation.
            const std::string &activeTSName = windowInfo->GetTimeSliders()[activeTS];
            activeTSCorrelation = cL->FindCorrelation(activeTSName);
        }

        int currentState = 0;
        int nTotalStates = 1;
        if(activeTSCorrelation)
        {
            currentState = windowInfo->GetTimeSliderCurrentStates()[activeTS];
            nTotalStates = activeTSCorrelation->GetNumStates();
        }
        else if(GetViewerState()->GetKeyframeAttributes()->GetEnabled())
        {
            //
            // Keyframing is enabled so we must be using the keyframing time
            // slider if we didn't find a correlation for the active time
            // slider. Get the number of keyframes and use that as the
            // length of the time slider.
            //
            if(activeTS >= 0)
            {
                currentState = windowInfo->GetTimeSliderCurrentStates()[activeTS];
                nTotalStates = GetViewerState()->GetKeyframeAttributes()->GetNFrames();
            }
        }

        animationPosition->blockSignals(true);
        animationPosition->setRange(0, nTotalStates - 1);
        animationPosition->setPageStep(1);
        animationPosition->setValue(currentState);
        animationPosition->blockSignals(false);

        // Set the time field to the cycle number.
        UpdateTimeFieldText(currentState);

        // Set the enabled state of the animation widgets.
        UpdateAnimationControlsEnabledState();
    }

    // If the VCR controls are selected (animationMode), set the mode.
    if(windowInfo->IsSelected(WindowInformation::ID_animationMode) || doAll)
    {
        vcrControls->blockSignals(true);
        vcrControls->SetActiveButton(windowInfo->GetAnimationMode());
        vcrControls->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::UpdateAnimationControlsEnabledState
//
// Purpose:
//   Set the enabled state for the animation controls.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 12:01:29 PDT 2004
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   If viewing a ddtsim-based simulation, set each of the VCR controls
//   to a suitable enabled status
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::UpdateAnimationControlsEnabledState()
{
    //
    // Set the enabled state for the animation controls. They should be
    // available whenever we have an active time slider.
    //
    bool enabled = (windowInfo->GetActiveTimeSlider() >= 0);
    bool ddtsimEnabled = windowInfo->GetDDTSim();
    vcrControls->SetDDTSimEnabled(ddtsimEnabled);
    vcrControls->setEnabled(enabled || ddtsimEnabled);
    animationPosition->setEnabled(enabled);
    timeField->setEnabled(enabled);
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::UpdateTimeFieldText
//
// Purpose:
//   Updates the text in the time/cycle text field.
//
// Arguments:
//   timeState : The timestate to update the cycle to.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 21 16:52:35 PST 2001
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Brad Whitlock, Mon Oct 13 16:01:58 PST 2003
//   Added support for showing time in the text field.
//
//   Brad Whitlock, Sat Jan 24 20:43:30 PST 2004
//   Updated for the new time and file scheme.
//
//   Cyrus Harrison, Fri Aug  1 09:06:03 PDT 2008
//   Changed set of time field text to use SetTimeFieldText to make sure
//   long time values remain visible.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::UpdateTimeFieldText(int timeState)
{
    if(fileServer == 0 || windowInfo == 0)
        return;

    QString timeString;

    // Set the time field to the cycle number.
    int activeTS = windowInfo->GetActiveTimeSlider();
    if(activeTS >= 0)
    {
        const std::string &tsName = windowInfo->GetTimeSliders()[activeTS];

        //
        // Look for a database correlation with that name.
        //
        DatabaseCorrelationList *dbCorrelations = GetViewerState()->
            GetDatabaseCorrelationList();
        DatabaseCorrelation *correlation = dbCorrelations->FindCorrelation(tsName);
        if(correlation)
        {
            //
            // Figure out how to display
            //
            if(correlation->GetNumDatabases() == 1)
            {
//
// Multi DB correlations that are correlated in time could also show time but I'm not
// quite sure how to get it out of the correlation. Only time though - not cycle.
//
                if(timeStateFormat.GetDisplayMode() == TimeFormat::Cycles ||
                   timeStateFormat.GetDisplayMode() == TimeFormat::CyclesAndTimes)
                {
                    const intVector &cycles = correlation->GetDatabaseCycles();
                    if((size_t)timeState < cycles.size())
                        timeString = FormattedCycleString(cycles[timeState]);
                }
                else if(timeStateFormat.GetDisplayMode() == TimeFormat::Times)
                {
                    const doubleVector &times = correlation->GetDatabaseTimes();
#if 0
//
// There's nothing in the correlation that says whether we can believe the times.
//
                    const unsignedCharVector &timesAccurate =
                        correlation->GetDatabaseTimesAccurate();
                    if(timeState < times.size())
                    {
                        timeString = FormattedTimeString(times[timeState],
                            timesAccurate[timeState] == 1);
                    }
                    else
                        timeString = "?";
#else
                    if((size_t)timeState < times.size())
                    {
                        timeString = FormattedTimeString(times[timeState], true);
                    }
                    else
                        timeString = "?";
#endif
                }
                SetTimeFieldText(timeString);
            }
            else
            {
                // The correlation has multiple databases so how we display time will vary
                // depending on the correlation method.
                bool timeNeedsToBeSet = true;
                if(timeState < correlation->GetNumStates())
                {
                    if(correlation->GetMethod() == DatabaseCorrelation::TimeCorrelation)
                    {
                        timeNeedsToBeSet = false;
                        timeString = FormattedTimeString(
                            correlation->GetCondensedTimeForState(timeState), true);
                        SetTimeFieldText(timeString);
                    }
                    else if(correlation->GetMethod() == DatabaseCorrelation::CycleCorrelation)
                    {
                        timeNeedsToBeSet = false;
                        timeString = FormattedCycleString(
                            correlation->GetCondensedCycleForState(timeState));
                        SetTimeFieldText(timeString);
                    }
                }

                // If we did not set the time yet, do it with the index.
                if(timeNeedsToBeSet)
                {
                    timeString.sprintf("%d", timeState);
                    SetTimeFieldText(timeString);
                }
            }
        }
        else
        {
            // There was no correlation but we know the time state that we want to
            // display so let's show that in the time line edit.
            timeString.sprintf("%d", timeState);
            SetTimeFieldText(timeString);
        }
    }
    else
        SetTimeFieldText("");
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::SetTimeFieldText
//
// Purpose:
//   Sets the text in the time/cycle text field, makes sure the text is
//   visible to the user.
//
// Arguments:
//   text: Text value to set.
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Programmer: Cyrus Harrison
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::SetTimeFieldText(const QString &text)
{
    int w  = timeField->width();
    int nw = timeField->fontMetrics().width("  " + text);
    if(w < nw)
        timeField->setMinimumWidth(nw);
    timeField->setText(text);
}


// ****************************************************************************
// Method: QvisTimeSliderControlWidget::FormattedCycleString
//
// Purpose:
//   Returns a formatted cycle string.
//
// Arguments:
//   cycle : The cycle that we want to convert to a string.
//
// Returns:    A formatted cycle string.
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 11:31:42 PDT 2003
//
// Modifications:
//
// ****************************************************************************

QString
QvisTimeSliderControlWidget::FormattedCycleString(const int cycle) const
{
    QString retval;
    retval.sprintf("%04d", cycle);
    return retval;
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::FormattedTimeString
//
// Purpose:
//   Returns a formatted time string.
//
// Arguments:
//   t        : The time value to format.
//   accurate : Whether the time can be believed. If it can't then we return
//              a question mark string.
//
// Returns:    A formatted time string.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 16:03:37 PST 2003
//
// Modifications:
//
// ****************************************************************************

QString
QvisTimeSliderControlWidget::FormattedTimeString(const double t, bool accurate) const
{
    QString retval("?");
    if(accurate)
    {
        QString formatString;
        formatString.sprintf("%%.%dg", timeStateFormat.GetPrecision());
        retval.sprintf(formatString.toStdString().c_str(), t);
    }
    return retval;
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::SubjectRemoved
//
// Purpose:
//   Removes the windowInfo or fileserver subjects that this widget
//   observes.
//
// Arguments:
//   TheRemovedSubject : The subject that is being removed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 15:29:39 PST 2000
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Brad Whitlock, Thu May 9 16:40:58 PST 2002
//   Removed file server.
//
//   Brad Whitlock, Sat Jan 24 23:44:56 PST 2004
//   I made it observe windowInfo instead of globalAtts.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == windowInfo)
        windowInfo = 0;
}

//
// Methods to attach to the windowInfo and fileserver objects.
//

void
QvisTimeSliderControlWidget::ConnectFileServer(FileServerList *fs)
{
    fileServer->Attach(this);

    if(fileServer != 0 && windowInfo != 0 && fileServer->FileChanged())
    {
        // Set the enabled state for the animation controls.
        UpdateAnimationControlsEnabledState();
    }
}

void
QvisTimeSliderControlWidget::ConnectWindowInformation(WindowInformation *wi)
{
    windowInfo = wi;
    windowInfo->Attach(this);

    // Update the animation area.
    UpdateAnimationControls(true);
}

void
QvisTimeSliderControlWidget::ConnectPlotList(PlotList *pl)
{
    plotList = pl;
    plotList->Attach(this);

    bool shouldEnablePlay = false;
    for (int i = 0; i < plotList->GetNumPlots() && !shouldEnablePlay; ++i)
    {
        const Plot &current = plotList->operator[](i);
        shouldEnablePlay = (current.GetActiveFlag() &&
                            current.GetFollowsTime() &&
                            current.GetStateType() == Plot::Completed);
    }
    vcrControls->SetPlayEnabledState(shouldEnablePlay);
}


// ****************************************************************************
// Method: QvisTimeSliderControlWidget::SetTimeSliderState
//
// Purpose:
//   Sets the animation frame.
//
// Arguments:
//   index            : The index of the new time state.
//   indexIsTimeState : Whether the index is a time state or an animation frame
//                      index.
//
// Note: Taken/Refactored from QvisFilePanel.
//
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 24 14:36:50 PST 2003
//
// Modifications:
//   Brad Whitlock, Sun Jan 25 01:39:50 PDT 2004
//   I made it compare the state against the active time slider's current state.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::SetTimeSliderState(int state)
{
    // Figure out the time state for the current time slider.
    int activeTS = windowInfo->GetActiveTimeSlider();
    if(activeTS >= 0)
    {
         int currentState = windowInfo->GetTimeSliderCurrentStates()[activeTS];

         //
         // Set the state for the viewer's active time slider.
         //
         if(state != currentState)
         {
             GetViewerMethods()->SetTimeSliderState(state);
         }
         else
         {
             QString msg;
             msg.sprintf(" %d.", state);
             Message(tr("The active time slider is already at state") + msg);
         }
    }
}


//
// Qt slot functions.
//


// ****************************************************************************
// Method: QvisTimeSliderControlWidget::backwardStep
//
// Purpose:
//   This is a Qt slot function that tells the viewer to switch to
//   the previous time state for the active time slider.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
//
// Modifications:
//   Brad Whitlock, Tue Jan 27 18:13:18 PST 2004
//   I renamed the method.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::backwardStep()
{
    // Tell the viewer to go to the previous frame.
    GetViewerMethods()->TimeSliderPreviousState();
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::reversePlay
//
// Purpose:
//   This is a Qt slot function that tells the viewer to play the
//   current animation in reverse.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::reversePlay()
{
    // Tell the viewer to play the animation in reverse.
    GetViewerMethods()->AnimationReversePlay();
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::stop
//
// Purpose:
//   This is a Qt slot function that tells the viewer to stop playing
//   the current animation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::stop()
{
    // Tell the viewer to stop the animation. Use the viewer proxy because
    // it has an additional mechanism for making the animation stop quicker.
    GetViewerProxy()->AnimationStop();
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::play
//
// Purpose:
//   This is a Qt slot function that tells the viewer to play the
//   current animation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::play()
{
    // Tell the viewer to play the animation.
    GetViewerMethods()->AnimationPlay();
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::forwardStep
//
// Purpose:
//   This is a Qt slot function that tells the viewer to switch to
//   the next frame in an animation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 1 10:28:22 PDT 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Brad Whitlock, Wed Jul 30 16:52:23 PST 2003
//   I made it emit the new reopenOnNextFrame signal if we're on the last
//   frame of the animation.
//
//   Brad Whitlock, Tue Jan 27 18:09:28 PST 2004
//    I renamed it to forwardStep and changed the coding to support multiple
//    time sliders.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::forwardStep()
{
#ifdef BEFORE_NEW_FILE_SELECTION
    //
    // If we're not playing an animation and we're at the last frame in the
    // animation, then try and reopen the current file to see if there are
    // more timestates.
    //
    if(globalAtts->GetAnimationMode() == 2 &&
       globalAtts->GetCurrentFrame() == globalAtts->GetNFrames() - 1)
    {
        emit reopenOnNextFrame();
    }
    else
    {
        // Tell the viewer to go to the next frame.
        GetViewerMethods()->AnimationNextFrame();
    }
#else
    GetViewerMethods()->TimeSliderNextState();
#endif
}



// ****************************************************************************
// Method: QvisTimeSliderControlWidget::sliderStart
//
// Purpose:
//   This is a Qt slot function that is called when the animation
//   slider is pressed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 17:14:35 PST 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//
//   Brad Whitlock, Tue Dec 30 14:32:02 PST 2003
//   I removed sliderDown.
//
//   Brad Whitlock, Tue Jan 27 20:31:25 PST 2004
//   I added support for multiple time sliders.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::sliderStart()
{
    int activeTS = windowInfo->GetActiveTimeSlider();
    if(activeTS >= 0)
        sliderVal = windowInfo->GetTimeSliderCurrentStates()[activeTS];
    else
        sliderVal = 0;
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::sliderMove
//
// Purpose:
//   This is a Qt slot function that is called when the animation
//   slider is moved.
//
// Arguments:
//   val : The new value to use for the current state.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 17:14:35 PST 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Brad Whitlock, Tue Aug 21 16:55:44 PST 2001
//   Added code to update the cycle time text field as the slider moves.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::sliderMove(int val)
{
    sliderVal = val;

    // Update the cycle/time text field.
    UpdateTimeFieldText(sliderVal);
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::sliderEnd
//
// Purpose:
//   This is a Qt slot function that is called when the slider is
//   released. It will change the current frame of the animation to
//   the last slider value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 18:22:15 PST 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//
//   Brad Whitlock, Fri Oct 24 14:31:33 PST 2003
//   I made it use the new SetTimeSliderState method and I removed old code to
//   set the current frame in the global atts. The viewer now sends back the
//   current frame but it didn't used to a long time ago.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::sliderEnd()
{
    // Set the new frame.
    SetTimeSliderState(sliderVal);
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::sliderChange
//
// Purpose:
//   This is a Qt slot function that is called when the slider's
//   valueChanged signal is emitted.
//
// Arguments:
//   val : The new slider value.
//
// Note:       This method is called when clicking on areas of the slider to
//             make it page up/down.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 27 11:59:05 PDT 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Eric Brugger, Mon Dec 16 13:05:01 PST 2002
//   I seperated the concepts of state and frame, since they are no longer
//   equivalent with keyframe support.
//
//   Brad Whitlock, Fri Oct 24 14:31:33 PST 2003
//   I made it use the new SetTimeSliderState method and I removed old code to
//   set the current frame in the global atts. The viewer now sends back the
//   current frame but it didn't used to a long time ago.
//
//   Brad Whitlock, Tue Dec 30 14:31:03 PST 2003
//   I made it use the animation slider instead of sliderDown.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::sliderChange(int val)
{
    if(animationPosition->sliderIsDown())
        return;

    // Set the new frame.
    SetTimeSliderState(val);
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::processTimeText
//
// Purpose:
//   This is a Qt slot function that processes the text entered by
//   the user and looks for the closest cycle number and sets the
//   animation to that cycle number.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 17:16:01 PST 2000
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Modifications:
//   Brad Whitlock, Mon Oct 13 16:05:11 PST 2003
//   I changed the code so it's possible to enter time into the text field.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interface to FileServerList::GetMetaData
//
//   Cyrus Harrison, Tue Jul  1 16:04:25 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Fri Aug  1 09:06:03 PDT 2008
//   Changed set of time field text to use SetTimeFieldText to make sure
//   long time values remain visible.
//
//   Brad Whitlock, Mon Dec 13 10:39:28 PST 2010
//   Return early if we get NULL metadata.
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::processTimeText()
{
    // Convert the text to an integer value.
    QString temp(timeField->text().trimmed());
    if(temp.isEmpty())
        return;

    const avtDatabaseMetaData *md = fileServer->GetMetaData(
                                        fileServer->GetOpenFile(),
                                        GetStateForSource(fileServer->GetOpenFile()),
                                        FileServerList::ANY_STATE,
                                       !FileServerList::GET_NEW_MD);
    if(md == 0)
    {
        SetTimeFieldText("");
        return;
    }

    int  index = 0;
    bool okay = false;

    //
    // If the string has a decimal in it then assume it is a time.
    //
    if(temp.indexOf('.') != -1)
    {
        double t = temp.toDouble(&okay);
        if(!okay)
        {
            SetTimeFieldText("");
            return;
        }

        // Loop through the times for the current file while the
        // time that was entered is greater than or equal to the
        // time in the list.
        if(md->GetTimes().size() == (size_t)md->GetNumStates())
        {
            for(int i = 0; i < md->GetNumStates(); ++i)
            {
                if(t <= md->GetTimes()[i])
                    break;
                else
                    ++index;
            }
        }
    }
    else
    {
        int cycle = temp.toInt(&okay);
        if(!okay)
        {
            SetTimeFieldText("");
            return;
        }

        // Loop through the cycles for the current file while the
        // cycle that was entered is greater than or equal to the
        // cycle in the list.
        for(int i = 0; i < md->GetNumStates(); ++i)
        {
            if(cycle <= md->GetCycles()[i])
                break;
            else
                ++index;
        }
    }

    // Make sure that index is no larger than numstates.
    if(index >= md->GetNumStates())
        index = md->GetNumStates() - 1;

    // Call the slider's slot function.
    sliderVal = index;
    sliderEnd();
}

// ****************************************************************************
// Method: QvisTimeSliderControlWidget::changeActiveTimeSlider
//
// Purpose:
//   Tell the viewer to switch to the new active time slider.
//
// Arguments:
//   tsName : The name of the new active time slider.
//
//
// Note: Taken/Refactored from QvisFilePanel.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 27 21:30:12 PST 2004
//
// Modifications:
//
// ****************************************************************************

void
QvisTimeSliderControlWidget::changeActiveTimeSlider(int tsIndex)
{
    if(windowInfo->GetTimeSliders().size() > 1)
    {
        const stringVector &tsNames = windowInfo->GetTimeSliders();
        if(tsIndex >= 0 && (size_t)tsIndex < tsNames.size())
            GetViewerMethods()->SetActiveTimeSlider(tsNames[tsIndex]);
    }
}

