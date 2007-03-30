#include <stdio.h>
#include <QvisAnimationWindow.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
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
// ****************************************************************************

QvisAnimationWindow::QvisAnimationWindow(AnimationAttributes *subj,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    animationAtts = subj;
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
// ****************************************************************************

QvisAnimationWindow::~QvisAnimationWindow()
{
    // Nothing here
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
// ****************************************************************************

void
QvisAnimationWindow::CreateWindowContents()
{
    // Create a grid layout.
    QGridLayout *animLayout = new QGridLayout(topLayout, 4, 2);
    animLayout->setSpacing(5);

    // Create the check box
    pipelineCachingToggle = new QCheckBox("Pipeline caching", central,
        "pipelineCachingToggle");
    connect(pipelineCachingToggle, SIGNAL(toggled(bool)),
            this, SLOT(pipelineCachingToggled(bool)));
    animLayout->addMultiCellWidget(pipelineCachingToggle, 0, 0, 0, 1);

    // Create the slider and some labels.
    timeoutSlider = new QSlider(Qt::Horizontal, central, "timeoutSlider");
    timeoutSlider->setTickmarks(QSlider::Below);
    timeoutSlider->setTracking(false);
    timeoutSlider->setRange(1, SLOWEST_TIMEOUT - 1);
    timeoutSlider->setPageStep(100);
    connect(timeoutSlider, SIGNAL(valueChanged(int)),
            this, SLOT(timeoutChanged(int)));
    animLayout->addMultiCellWidget(timeoutSlider, 2, 2, 0, 1);
    QLabel *speedLabel = new QLabel(timeoutSlider, "Animation speed",
        central, "speedLabel");
    animLayout->addWidget(speedLabel, 1, 0);

    // Create the slower and faster labels.
    QLabel *slowerLabel = new QLabel("slower", central, "speedLabel");
    animLayout->addWidget(slowerLabel, 3, 0);
    QLabel *fasterLabel = new QLabel("faster", central, "speedLabel");
    animLayout->addWidget(fasterLabel, 3, 1, Qt::AlignRight);
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
        case 0: // pipelineCaching.
            pipelineCachingToggle->blockSignals(true);
            pipelineCachingToggle->setChecked(atts->GetPipelineCachingMode());
            pipelineCachingToggle->blockSignals(false);
            break;
        case 1: // timeout
            timeoutSlider->blockSignals(true);
            timeoutSlider->setValue(SLOWEST_TIMEOUT - atts->GetTimeout());
            timeoutSlider->blockSignals(false);
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
            viewer->SetAnimationAttributes();
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
