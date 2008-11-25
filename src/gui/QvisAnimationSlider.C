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
//   Brad Whitlock, Fri May 30 09:49:16 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisAnimationSlider::QvisAnimationSlider(Qt::Orientation orientation,
    QWidget *parent) :  QSlider(orientation, parent)
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

QvisAnimationSlider::QvisAnimationSlider(QWidget *parent) : 
    QSlider(parent)
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
