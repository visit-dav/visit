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

#include <stdio.h>
#include <QPushButton>
#include <QLayout>
#include <QvisVCRControl.h>

//#define DEBUG_VCR

//
// Icons for the VCR buttons.
//
#if defined(Q_WS_MACX)
static const char * frevpix_icon[] = {
"34 24 2 1",
". c None",
"X c #000",
"..................................",
"..................................",
"..................................",
"..................................",
"................XX......XXXXXX....",
"..............XXXX......XXXXXX....",
"............XXXXXX......XXXXXX....",
"..........XXXXXXXX......XXXXXX....",
"........XXXXXXXXXX......XXXXXX....",
"......XXXXXXXXXXXX......XXXXXX....",
"....XXXXXXXXXXXXXX......XXXXXX....",
"..XXXXXXXXXXXXXXXX......XXXXXX....",
"..XXXXXXXXXXXXXXXX......XXXXXX....",
"....XXXXXXXXXXXXXX......XXXXXX....",
"......XXXXXXXXXXXX......XXXXXX....",
"........XXXXXXXXXX......XXXXXX....",
"..........XXXXXXXX......XXXXXX....",
"............XXXXXX......XXXXXX....",
"..............XXXX......XXXXXX....",
"................XX......XXXXXX....",
"..................................",
"..................................",
"..................................",
".................................."
};

static const char * backpix_icon[] = {
"34 24 2 1",
". c None",
"X c #000",
"..................................",
"..................................",
"..................................",
"..................................",
".......................XX.........",
".....................XXXX.........",
"...................XXXXXX.........",
".................XXXXXXXX.........",
"...............XXXXXXXXXX.........",
".............XXXXXXXXXXXX.........",
"...........XXXXXXXXXXXXXX.........",
".........XXXXXXXXXXXXXXXX.........",
".........XXXXXXXXXXXXXXXX.........",
"...........XXXXXXXXXXXXXX.........",
".............XXXXXXXXXXXX.........",
"...............XXXXXXXXXX.........",
".................XXXXXXXX.........",
"...................XXXXXX.........",
".....................XXXX.........",
".......................XX.........",
"..................................",
"..................................",
"..................................",
".................................."
};

static const char * stoppix_icon[] = {
"34 24 2 1",
". c None",
"X c #000",
"..................................",
"..................................",
"..................................",
"..................................",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..........XXXXXXXXXXXXXX..........",
"..................................",
"..................................",
"..................................",
".................................."
};

static const char * playpix_icon[] = {
"34 24 2 1",
". c None",
"X c #000",
"..................................",
"..................................",
"..................................",
"..................................",
".........XX.......................",
".........XXXX.....................",
".........XXXXXX...................",
".........XXXXXXXX.................",
".........XXXXXXXXXX...............",
".........XXXXXXXXXXXX.............",
".........XXXXXXXXXXXXXX...........",
".........XXXXXXXXXXXXXXXX.........",
".........XXXXXXXXXXXXXXXX.........",
".........XXXXXXXXXXXXXX...........",
".........XXXXXXXXXXXX.............",
".........XXXXXXXXXX...............",
".........XXXXXXXX.................",
".........XXXXXX...................",
".........XXXX.....................",
".........XX.......................",
"..................................",
"..................................",
"..................................",
".................................."
};

static const char * fadvpix_icon[] = {
"34 24 2 1",
". c None",
"X c #000",
"..................................",
"..................................",
"..................................",
"..................................",
"....XXXXXX......XX................",
"....XXXXXX......XXXX..............",
"....XXXXXX......XXXXXX............",
"....XXXXXX......XXXXXXXX..........",
"....XXXXXX......XXXXXXXXXX........",
"....XXXXXX......XXXXXXXXXXXX......",
"....XXXXXX......XXXXXXXXXXXXXX....",
"....XXXXXX......XXXXXXXXXXXXXXXX..",
"....XXXXXX......XXXXXXXXXXXXXXXX..",
"....XXXXXX......XXXXXXXXXXXXXX....",
"....XXXXXX......XXXXXXXXXXXX......",
"....XXXXXX......XXXXXXXXXX........",
"....XXXXXX......XXXXXXXX..........",
"....XXXXXX......XXXXXX............",
"....XXXXXX......XXXX..............",
"....XXXXXX......XX................",
"..................................",
"..................................",
"..................................",
".................................."
};
#else
static const char * frevpix_icon[] = {
"58 24 2 1",
". c None",
"X c #000",
"..........................................................",
"..........................................................",
"..........................................................",
"..........................................................",
"............................XX......XXXXXX................",
"..........................XXXX......XXXXXX................",
"........................XXXXXX......XXXXXX................",
"......................XXXXXXXX......XXXXXX................",
"....................XXXXXXXXXX......XXXXXX................",
"..................XXXXXXXXXXXX......XXXXXX................",
"................XXXXXXXXXXXXXX......XXXXXX................",
"..............XXXXXXXXXXXXXXXX......XXXXXX................",
"..............XXXXXXXXXXXXXXXX......XXXXXX................",
"................XXXXXXXXXXXXXX......XXXXXX................",
"..................XXXXXXXXXXXX......XXXXXX................",
"....................XXXXXXXXXX......XXXXXX................",
"......................XXXXXXXX......XXXXXX................",
"........................XXXXXX......XXXXXX................",
"..........................XXXX......XXXXXX................",
"............................XX......XXXXXX................",
"..........................................................",
"..........................................................",
"..........................................................",
".........................................................."
};

static const char * backpix_icon[] = {
"58 24 2 1",
". c None",
"X c #000",
"..........................................................",
"..........................................................",
"..........................................................",
"..........................................................",
"...................................XX.....................",
".................................XXXX.....................",
"...............................XXXXXX.....................",
".............................XXXXXXXX.....................",
"...........................XXXXXXXXXX.....................",
".........................XXXXXXXXXXXX.....................",
".......................XXXXXXXXXXXXXX.....................",
".....................XXXXXXXXXXXXXXXX.....................",
".....................XXXXXXXXXXXXXXXX.....................",
".......................XXXXXXXXXXXXXX.....................",
".........................XXXXXXXXXXXX.....................",
"...........................XXXXXXXXXX.....................",
".............................XXXXXXXX.....................",
"...............................XXXXXX.....................",
".................................XXXX.....................",
"...................................XX.....................",
"..........................................................",
"..........................................................",
"..........................................................",
".........................................................."
};

static const char * stoppix_icon[] = {
"58 24 2 1",
". c None",
"X c #000",
"..........................................................",
"..........................................................",
"..........................................................",
"..........................................................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"......................XXXXXXXXXXXXXX......................",
"..........................................................",
"..........................................................",
"..........................................................",
".........................................................."
};

static const char * playpix_icon[] = {
"58 24 2 1",
". c None",
"X c #000",
"..........................................................",
"..........................................................",
"..........................................................",
"..........................................................",
".....................XX...................................",
".....................XXXX.................................",
".....................XXXXXX...............................",
".....................XXXXXXXX.............................",
".....................XXXXXXXXXX...........................",
".....................XXXXXXXXXXXX.........................",
".....................XXXXXXXXXXXXXX.......................",
".....................XXXXXXXXXXXXXXXX.....................",
".....................XXXXXXXXXXXXXXXX.....................",
".....................XXXXXXXXXXXXXX.......................",
".....................XXXXXXXXXXXX.........................",
".....................XXXXXXXXXX...........................",
".....................XXXXXXXX.............................",
".....................XXXXXX...............................",
".....................XXXX.................................",
".....................XX...................................",
"..........................................................",
"..........................................................",
"..........................................................",
".........................................................."
};

static const char * fadvpix_icon[] = {
"58 24 2 1",
". c None",
"X c #000",
"..........................................................",
"..........................................................",
"..........................................................",
"..........................................................",
"................XXXXXX......XX............................",
"................XXXXXX......XXXX..........................",
"................XXXXXX......XXXXXX........................",
"................XXXXXX......XXXXXXXX......................",
"................XXXXXX......XXXXXXXXXX....................",
"................XXXXXX......XXXXXXXXXXXX..................",
"................XXXXXX......XXXXXXXXXXXXXX................",
"................XXXXXX......XXXXXXXXXXXXXXXX..............",
"................XXXXXX......XXXXXXXXXXXXXXXX..............",
"................XXXXXX......XXXXXXXXXXXXXX................",
"................XXXXXX......XXXXXXXXXXXX..................",
"................XXXXXX......XXXXXXXXXX....................",
"................XXXXXX......XXXXXXXX......................",
"................XXXXXX......XXXXXX........................",
"................XXXXXX......XXXX..........................",
"................XXXXXX......XX............................",
"..........................................................",
"..........................................................",
"..........................................................",
".........................................................."
};
#endif

char *QvisVCRControl::augmentedData[27];
char QvisVCRControl::augmentedForeground[15];

// ****************************************************************************
// Method: QvisVCRControl::QvisVCRControl
//
// Purpose: This is the constructor for the QvisVCRControl class. It
//          creates some QPushButtons and adds them to the button
//          group.
//   
//
// Notes:      The QvisVCRControl class is a widget that contains 5 buttons
//             that correspond to reverse, rewind, stop, play, advance.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 11:26:42 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Sep 1 10:19:42 PDT 2000
//   Changed the widget so it derives from QWidget.
//
//   Brad Whitlock, Fri Dec 1 17:01:01 PST 2000
//   Added code to ensure that the widget's foreground color is used instead
//   of always using black.
//
//   Jeremy Meredith, Fri Aug 22 14:15:21 PDT 2003
//   Made it respond to mousedown (press()) events instead of click() events.
//
//   Brad Whitlock, Tue Jun  3 13:44:42 PDT 2008
//   Qt 4.
//
//   Jeremy Meredith, Fri Feb 19 21:00:38 EST 2010
//   Set the icon size to something reasonable.  (It's way too small in
//   virtually every Qt style.)
//
// ****************************************************************************

QvisVCRControl::QvisVCRControl(QWidget *parent) : QWidget(parent)
{
    // Make the stop button active.
    activeButton = 2;

    // Create pixmaps for the buttons.
    AugmentPixmap(frevpix_icon);
    QPixmap p1((const char **)augmentedData);
    AugmentPixmap(backpix_icon);
    QPixmap p2((const char **)augmentedData);
    AugmentPixmap(stoppix_icon);
    QPixmap p3((const char **)augmentedData);
    AugmentPixmap(playpix_icon);
    QPixmap p4((const char **)augmentedData);
    AugmentPixmap(fadvpix_icon);
    QPixmap p5((const char **)augmentedData);

    // Create the top layout.
    QHBoxLayout *topLayout = new QHBoxLayout(this);
    topLayout->setMargin(0);
#if defined(Q_WS_MACX)
    topLayout->setSpacing(1);
#else
    topLayout->setSpacing(10);
#endif

    QSize iconsize(34,24);

    // Create the buttons and add the pixmaps to them.
    buttons[0] = new QPushButton(this);
    buttons[0]->setIcon(QIcon(p1));
    buttons[0]->setIconSize(iconsize);

    buttons[1] = new QPushButton(this);
    buttons[1]->setIcon(QIcon(p2));
    buttons[1]->setCheckable(true);
    buttons[1]->setDown(false);
    buttons[1]->setIconSize(iconsize);

    buttons[2] = new QPushButton(this);
    buttons[2]->setIcon(QIcon(p3));
    buttons[2]->setCheckable(true);
    buttons[2]->setDown(true);
    buttons[2]->setIconSize(iconsize);

    buttons[3] = new QPushButton(this);
    buttons[3]->setIcon(QIcon(p4));
    buttons[3]->setCheckable(true);
    buttons[3]->setDown(false);
    buttons[3]->setIconSize(iconsize);

    buttons[4] = new QPushButton(this);
    buttons[4]->setIcon(QIcon(p5));
    buttons[4]->setIconSize(iconsize);

    // Connect the buttons' "pressed" signals to the appropriate VCR
    // clicked slot.
    connect(buttons[0], SIGNAL(pressed()), this, SLOT(b0_clicked()));
    connect(buttons[1], SIGNAL(pressed()), this, SLOT(b1_clicked()));
    connect(buttons[2], SIGNAL(pressed()), this, SLOT(b2_clicked()));
    connect(buttons[3], SIGNAL(pressed()), this, SLOT(b3_clicked()));
    connect(buttons[4], SIGNAL(pressed()), this, SLOT(b4_clicked()));

    // Add all the buttons to the top layout.
    for(int i = 0; i < 5; ++i)
        topLayout->addWidget(buttons[i]);
}

// ****************************************************************************
// Method: QvisVCRControl
//
// Purpose: This is the destructor for the QvisVCRControl class.
//   
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 11:28:07 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisVCRControl::~QvisVCRControl()
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisVCRControl::SetActiveButton
//
// Purpose: 
//   This method sets the active button.
//
// Arguments:
//   btn : The index of the button to make active.
//
// Note:       
//   This method will emit signals due to the calls of the b#_clicked
//   slot functions.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 15:55:06 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisVCRControl::SetActiveButton(int btn)
{
    switch(btn)
    {
    case 0:
        b0_clicked();
        break;
    case 1:
        b1_clicked();
        break;
    case 2:
        b2_clicked();
        break;
    case 3:
        b3_clicked();
        break;
    case 4:
        b4_clicked();
    }    
}

// ****************************************************************************
// Method: QvisVCRControl::AugmentPixmap
//
// Purpose: 
//   This method augments pixmap data so that the application's foreground
//   color is used instead of the default of black.
//
// Arguments:
//   xpm : A pointer to xpm pixmap strings.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 1 16:29:56 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 13:47:35 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVCRControl::AugmentPixmap(const char *xpm[])
{
    for(int i = 0; i < 27; ++i)
        augmentedData[i] = (char *)xpm[i];

    QColor foreground(palette().color(QPalette::Text));

    // Turn the third element into the foreground color.
    sprintf(augmentedForeground, "X c #%02x%02x%02x", 
            foreground.red(), foreground.green(),
            foreground.blue());
    augmentedData[2] = augmentedForeground;
}

// ****************************************************************************
// Method: QvisVCRControl::b0_clicked()
//
// Purpose: slot function that emits the prevFrame signal.
//   
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 13:33:47 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Sep 1 10:19:05 PDT 2000
//   Made sure the new button is not set to toggled.
//
//   Brad Whitlock, Thu Feb 27 12:25:53 PDT 2003
//   Changed the code to try and reduce number of draws required.
//
//   Brad Whitlock, Tue Jun  3 14:02:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVCRControl::b0_clicked()
{
    // If the stop button is not active, make it active and turn off the
    // button that was last active.
    if(activeButton != 2)
    {
        if(activeButton > 0 && activeButton < 4)
            buttons[activeButton]->setDown(false);
        activeButton = 2;
        buttons[activeButton]->setDown(true);
    }

    buttons[1]->blockSignals(true);
    buttons[1]->setChecked(false);
    buttons[1]->blockSignals(false);
    buttons[2]->blockSignals(true);
    buttons[2]->setChecked(true);
    buttons[2]->blockSignals(false);
    buttons[3]->blockSignals(true);
    buttons[3]->setChecked(false);
    buttons[3]->blockSignals(false);
#ifdef DEBUG_VCR
    qDebug("prevFrame()");
#endif
    emit prevFrame();
}

// ****************************************************************************
// Method: QvisVCRControl::b1_clicked()
//
// Purpose: slot function that emits the reversePlay signal.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 13:33:47 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 14:02:23 PDT 2008
//   Qt 4.
//   
// ****************************************************************************

void
QvisVCRControl::b1_clicked()
{
    if(activeButton != 1)
    {
        buttons[activeButton]->setDown(false);
        activeButton = 1;
        buttons[activeButton]->setDown(true);

#ifdef DEBUG_VCR
        qDebug("reversePlay()");
#endif
        emit reversePlay();
    }
    else if(!buttons[activeButton]->isDown())
        buttons[activeButton]->setDown(true);

    buttons[2]->blockSignals(true);
    buttons[2]->setChecked(false);
    buttons[2]->blockSignals(false);
    buttons[3]->blockSignals(true);
    buttons[3]->setChecked(false);
    buttons[3]->blockSignals(false);
}

// ****************************************************************************
// Method: QvisVCRControl::b2_clicked()
//
// Purpose: slot function that emits the stop signal.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 13:33:47 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Sep 1 10:19:05 PDT 2000
//   Made sure the new button is not set to toggled.
//
//   Brad Whitlock, Thu Feb 27 12:20:44 PDT 2003
//   I made the button be toggled.
//
//   Brad Whitlock, Tue Jun  3 14:02:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVCRControl::b2_clicked()
{
    if(activeButton != 2)
    {
        buttons[activeButton]->setDown(false);
        activeButton = 2;
        buttons[activeButton]->setDown(true);

#ifdef DEBUG_VCR
        qDebug("stop()");
#endif
        emit stop();
    }
    else if(!buttons[activeButton]->isDown())
        buttons[activeButton]->setDown(true);

    buttons[1]->blockSignals(true);
    buttons[1]->setChecked(false);
    buttons[1]->blockSignals(false);
    buttons[3]->blockSignals(true);
    buttons[3]->setChecked(false);
    buttons[3]->blockSignals(false);
}

// ****************************************************************************
// Method: QvisVCRControl::b3_clicked()
//
// Purpose: slot function that emits the play signal.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 13:33:47 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Feb 27 12:30:10 PDT 2003
//   I changed how the buttons work.
//
//   Brad Whitlock, Tue Jun  3 14:02:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVCRControl::b3_clicked()
{
    if(activeButton != 3)
    {
        buttons[activeButton]->setDown(false);
        activeButton = 3;
        buttons[activeButton]->setDown(true);

#ifdef DEBUG_VCR
        qDebug("play()");
#endif
        emit play();
    }
    else if(!buttons[activeButton]->isDown())
        buttons[activeButton]->setDown(true);

    buttons[1]->blockSignals(true);
    buttons[1]->setChecked(false);
    buttons[1]->blockSignals(false);
    buttons[2]->blockSignals(true);
    buttons[2]->setChecked(false);
    buttons[2]->blockSignals(false);
}

// ****************************************************************************
// Method: QvisVCRControl::b4_clicked()
//
// Purpose: slot function that emits the nextFrame signal.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 13:33:47 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Sep 1 10:19:05 PDT 2000
//   Made sure the new button is not set to toggled.
//
//   Brad Whitlock, Thu Feb 27 12:30:10 PDT 2003
//   I changed how the buttons work.
//
//   Brad Whitlock, Tue Jun  3 14:02:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVCRControl::b4_clicked()
{
    // If the stop button is not active, make it active and turn off the
    // button that was last active.
    if(activeButton != 2)
    {
        buttons[activeButton]->setDown(false);
        activeButton = 2;
        buttons[activeButton]->setDown(true);
    }

    buttons[1]->blockSignals(true);
    buttons[1]->setChecked(false);
    buttons[1]->blockSignals(false);
    buttons[2]->blockSignals(true);
    buttons[2]->setChecked(true);
    buttons[2]->blockSignals(false);
    buttons[3]->blockSignals(true);
    buttons[3]->setChecked(false);
    buttons[3]->blockSignals(false);
#ifdef DEBUG_VCR
    qDebug("nextFrame()");
#endif
    emit nextFrame();
}
