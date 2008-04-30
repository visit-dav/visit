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

#include <ViewerConnectionProgressDialog.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtimer.h>
#include <qwmatrix.h>

// Icons
#include <leftdesk.xpm>
#include <super.xpm>

// Window flags to make the dialog modal.
#define DIALOG_FLAGS (WType_Modal | WStyle_Customize | WStyle_NormalBorder | WStyle_Title)

// ****************************************************************************
// Method: ViewerConnectionProgressDialog::ViewerConnectionProgressDialog
//
// Purpose: 
//   Constructor for the ViewerConnectionProgressDialog class.
//
// Arguments:
//   component : The name of the component being launched.
//   host      : The name of the computer where the component is launching.
//   par       : Whether or not the component is parallel.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 14:38:50 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed May 7 10:16:53 PDT 2003
//   I initialized some new flags.
//
// ****************************************************************************

ViewerConnectionProgressDialog::ViewerConnectionProgressDialog(const QString &component,
    const QString &host, bool par, int t) : QWidget(0, "dialog", DIALOG_FLAGS),
    componentName(component), hostName(host)
{
    parallel = par;
    iconFrame = 0;
    cancelled = false;
    timeout = t;
    cancelledShow = false;
    ignoreHide = false;

    // Create the timer that animates the icons.
    timer = new QTimer(this, "timer");
    connect(timer, SIGNAL(timeout()),
            this, SLOT(updateAnimation()));

    // Add the widgets...
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(10);
    topLayout->setSpacing(10);

    QHBoxLayout *iconLayout = new QHBoxLayout(topLayout);
    iconLayout->setSpacing(10);
    QPixmap left(leftdesk_xpm);
    QLabel *leftComputer = new QLabel("", this, "leftComputer");
    leftComputer->setPixmap(left);
    iconLayout->addWidget(leftComputer);

    // Create some radio buttons to display the progress.
    QHBoxLayout *dotLayout = new QHBoxLayout(iconLayout);
    dotLayout->setMargin(0);
    dotLayout->setSpacing(5);
    for(int i = 0; i < 6; ++i)
    {
        rb[i] = new QRadioButton("", this);
        rb[i]->blockSignals(true);
        dotLayout->addWidget(rb[i]);
    }

    // Create the pixmap for the host that we're connecting to.
    QPixmap right;
    if(parallel)
        right = QPixmap(super_xpm);
    else
    {
        QWMatrix m;
        m.scale(-1., 1.);
        right = left.xForm(m);
    }
    QLabel *rightComputer = new QLabel("", this, "rightComputer");
    rightComputer->setPixmap(right);
    iconLayout->addWidget(rightComputer);
    iconLayout->addSpacing(20);
    iconLayout->addStretch(5);

    // Add the cancel button.
    cancelButton = new QPushButton(tr("Cancel"), this, "cancelButton");
    connect(cancelButton, SIGNAL(clicked()),
            this, SLOT(cancel()));
    iconLayout->addWidget(cancelButton);

    // Add a message string.
    msgLabel = new QLabel("", this, "msgLabel");
    topLayout->addWidget(msgLabel);

    // Set the component name.
    setComponentName(component);
}

// ****************************************************************************
// Method: ViewerConnectionProgressDialog::~ViewerConnectionProgressDialog
//
// Purpose: 
//   Destructor for the ViewerConnectionProgressDialog class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 14:40:24 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ViewerConnectionProgressDialog::~ViewerConnectionProgressDialog()
{
}

// ****************************************************************************
// Method: ViewerConnectionProgressDialog::setComponentName
//
// Purpose: 
//   Sets the component name so that the window displays the proper messages.
//
// Arguments:
//   cm : The new component name.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 7 10:20:41 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 13:18:38 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerConnectionProgressDialog::setComponentName(const QString &cn)
{
    componentName = cn;

    // Set the window caption.
    QString str; str = componentName + " launch progress";
    setCaption(str);

    // Set the message string.
    QString msg;
    if(parallel)
        msg = tr("VisIt is waiting for a parallel %1 to launch on %2.").
            arg(componentName).arg(hostName);
    else
        msg = tr("VisIt is waiting for a %1 to launch on %2.").
            arg(componentName).arg(hostName);
    msgLabel->setText(msg);
}

// ****************************************************************************
// Method: ViewerConnectionProgressDialog::show
//
// Purpose: 
//   This Qt slot function tells the window to show itself after a delay.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 14:40:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerConnectionProgressDialog::show()
{
    iconFrame = 0;
    cancelled = false;
    cancelledShow = false;

    // Start the timer so the event loop has something to do and is not
    // obligated to process the single shot timer show event.
    timer->start(300);

    // Show the dialog a few seconds after this method is called. This way
    // the window will not pop-up for fast launching processes. If the timeout
    // is zero, don't bother with a timer.
    if(timeout == 0)
        timedShow();
    else
        QTimer::singleShot(timeout, this, SLOT(timedShow()));
}

// ****************************************************************************
// Method: ViewerConnectionProgressDialog::hide
//
// Purpose: 
//   This Qt slot function hides the dialog.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 14:41:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerConnectionProgressDialog::hide()
{
    cancelledShow = true;
    timer->stop();
    QWidget::hide();
}

// ****************************************************************************
// Method: ViewerConnectionProgressDialog::timedShow
//
// Purpose: 
//   This Qt slot function shows the dialog after a delay.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 14:42:10 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 09:03:17 PDT 2003
//   I made the window raise and center iself.
//
// ****************************************************************************

void
ViewerConnectionProgressDialog::timedShow()
{
    if(!cancelledShow)
    {
        QWidget::show();
        raise();
        
        // Move the window a little above center.
        int w = qApp->desktop()->width();
        int h = qApp->desktop()->height();
        int x = (w - width()) / 2;
        int y = (h - height()) / 2 - (height() * 6 / 5);
        move(x, y);

        // Make it the active viewer window.
        setActiveWindow();
    }
}

// ****************************************************************************
// Method: ViewerConnectionProgressDialog::updateAnimation
//
// Purpose: 
//   This Qt slot function updates the dialog's progress indicators.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 14:42:50 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerConnectionProgressDialog::updateAnimation()
{
    if(isVisible())
    {
        ++iconFrame;
        if(iconFrame > 5)
            iconFrame = 0;

        for(int i = 0; i < 6; ++i)
            rb[i]->setChecked(i == iconFrame);
    }   
}

// ****************************************************************************
// Method: ViewerConnectionProgressDialog::cancel
//
// Purpose: 
//   This Qt slot function sets the cancel flag which is queried later to
//   determine whether or not the launch should be cancelled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 14:43:25 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerConnectionProgressDialog::cancel()
{
    cancelled = true;
    hide();
}

