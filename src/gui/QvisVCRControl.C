#include <stdio.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QvisVCRControl.h>

//#define DEBUG_VCR

//
// Icons for the VCR buttons.
//
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
// ****************************************************************************

QvisVCRControl::QvisVCRControl(QWidget *parent, const char *name) :
    QWidget(parent, name)
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
    topLayout->setSpacing(10);

    // Create the buttons and add the pixmaps to them.
    buttons[0] = new QPushButton(this, "wreverse");
    buttons[0]->setPixmap(p1);

    buttons[1] = new QPushButton(this, "wrewind");
    buttons[1]->setPixmap(p2);
//    buttons[1]->setToggleButton(true);
    buttons[1]->setDown(false);

    buttons[2] = new QPushButton(this, "wstop");
    buttons[2]->setPixmap(p3);
//    buttons[2]->setToggleButton(true);
    buttons[2]->setDown(true);

    buttons[3] = new QPushButton(this, "wplay");
    buttons[3]->setPixmap(p4);
//    buttons[3]->setToggleButton(true);
    buttons[3]->setDown(false);

    buttons[4] = new QPushButton(this, "wadvance");
    buttons[4]->setPixmap(p5);

    // Connect the buttons' clicked signals to the appropriate VCR
    // clicked slot.
    connect(buttons[0], SIGNAL(clicked()), this, SLOT(b0_clicked()));
    connect(buttons[1], SIGNAL(clicked()), this, SLOT(b1_clicked()));
    connect(buttons[2], SIGNAL(clicked()), this, SLOT(b2_clicked()));
    connect(buttons[3], SIGNAL(clicked()), this, SLOT(b3_clicked()));
    connect(buttons[4], SIGNAL(clicked()), this, SLOT(b4_clicked()));

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
//   
// ****************************************************************************

void
QvisVCRControl::AugmentPixmap(const char *xpm[])
{
    for(int i = 0; i < 27; ++i)
        augmentedData[i] = (char *)xpm[i];

    // Turn the third element into the foreground color.
    sprintf(augmentedForeground, "X c #%02x%02x%02x", 
            foregroundColor().red(), foregroundColor().green(),
            foregroundColor().blue());
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

#ifdef DEBUG_VCR
    qDebug("nextFrame()");
#endif
    emit nextFrame();
}
