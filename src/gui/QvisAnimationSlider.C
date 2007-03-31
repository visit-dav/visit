#include <QvisAnimationSlider.h>

// ****************************************************************************
// Method: QvisAnimationSlider::QvisAnimationSlider
//
// Purpose: 
//   Constructor for the QvisAnimationSlider class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 30 14:21:30 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisAnimationSlider::QvisAnimationSlider(Orientation orientation,
    QWidget *parent, const char *name) :  QSlider(orientation, parent, name)
{
    the_sliderIsDown = false;
    emittedSignal = false;

    // Intercept some of QSlider's signals so we can decide when we
    // really want to emit them.
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(handleValueChanged(int)));
    connect(this, SIGNAL(sliderReleased()),
            this, SLOT(handleSliderReleased()));
}

QvisAnimationSlider::QvisAnimationSlider(QWidget *parent, const char *name) : 
    QSlider(parent, name)
{
    the_sliderIsDown = false;
    emittedSignal = false;

    // Intercept some of QSlider's signals so we can decide when we
    // really want to emit them.
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(handleValueChanged(int)));
    connect(this, SIGNAL(sliderReleased()),
            this, SLOT(handleSliderReleased()));
}

// ****************************************************************************
// Method: QvisAnimationSlider::~QvisAnimationSlider
//
// Purpose: 
//   Destructor for the QvisAnimationSlider class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 30 14:21:53 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisAnimationSlider::~QvisAnimationSlider()
{
}

// ****************************************************************************
// Method: QvisAnimationSlider::mousePressEvent
//
// Purpose: 
//   Handles mouse press events.
//
// Arguments:
//   e : The mouse event.
//
// Note:       Stores that the slider is down.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 30 14:22:10 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnimationSlider::mousePressEvent(QMouseEvent *e)
{
    the_sliderIsDown = true;
    QSlider::mousePressEvent(e);
}

// ****************************************************************************
// Method: QvisAnimationSlider::mouseReleaseEvent
//
// Purpose: 
//   Emits a sliderValueChanged or sliderWasReleased signal when we release
//   the slider. We make sure that we don't allow sliderValueChanged to be
//   emitted while the slider is being paged.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 30 14:22:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnimationSlider::mouseReleaseEvent(QMouseEvent *e)
{
    the_sliderIsDown = false;
    emittedSignal = false;

    // Call the base class's event handler.
    QSlider::mouseReleaseEvent(e);

    //
    // If we did not emit a valueChanged signal as a result of the mouse
    // release, then make sure we emit that signal now.
    //
    if(!emittedSignal)
    {
        emit sliderValueChanged(value());
    }
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisAnimationSlider::handleSliderReleased
//
// Purpose: 
//   Emits a sliderWasReleased signal and records that we did emit a signal
//   so we won't emit another one once the slider is released.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 30 14:24:45 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnimationSlider::handleSliderReleased()
{
    emittedSignal = true;
    emit sliderWasReleased();
}

// ****************************************************************************
// Method: QvisAnimationSlider::handleValueChanged
//
// Purpose: 
//   Emits signals that give the current state of the slider.
//
// Arguments:
//   val : The new slider value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 30 14:25:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnimationSlider::handleValueChanged(int val)
{
    if(the_sliderIsDown)
    {
        emittedSignal = false;

        // Emit a moved signal because the file panel just uses it to update
        // the cycle text and nothing more. Since it does not change the
        // active time state, we say that we didn't really emit a signal.
        emit sliderMoved(val);
    }
    else
    {
        emittedSignal = true;
        emit sliderValueChanged(val);
    }
}
