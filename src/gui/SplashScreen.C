#include <SplashScreen.h>
#include <qapplication.h>
#include <qfont.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qprogressbar.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <visit-config.h>   // For version number

#define MULTIPLE_IMAGES
#define TIMER_DURATION 2*1000     // 2 seconds

//
// Include splashscreen images
//
#include <icons/VisIt1.xpm>
#ifdef MULTIPLE_IMAGES
#include <icons/VisIt2.xpm>
#include <icons/VisIt3.xpm>
#include <icons/VisIt4.xpm>
#define NUM_PIX 4
#else
#define NUM_PIX 1
#endif

// ****************************************************************************
//  Method: SplashScreen::SplashScreen
//
//  Purpose: 
//    This is the constructor for the SplashScreen class.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Sep 6 16:39:25 PST 2001
//
//  Modifications:
//    Brad Whitlock, Thu Sep 6 19:35:12 PST 2001
//    Made it inherit from QFrame and I also added a button.
//
//    Jeremy Meredith, Mon Mar 11 17:05:23 PST 2002
//    I painted the version number directly on the new image and disabled the
//    runtime version drawing functionality.
//
//    Sean Ahern, Tue Apr 16 15:27:35 PDT 2002
//    Removed all raising behavior.  It's not needed any more.
//
//    Jeremy Meredith, Fri Jul 12 17:48:47 PDT 2002
//    Added a couple splash screens.
//
//    Brad Whitlock, Wed Sep 25 10:16:15 PDT 2002
//    I made the window a popup window so it does not show up in the taskbar.
//
//    Brad Whitlock, Tue Mar 18 14:43:41 PST 2003
//    I prevented the window from erasing with the background color.
//
//    Brad Whitlock, Mon Mar 31 13:29:23 PST 2003
//    I changed the splashscreen so it is not a popup window anymore. It
//    was obscuring the password window - that's more serious than making
//    sure that the window has no taskbar entry.
//
//    Brad Whitlock, Wed Jun 18 17:52:52 PST 2003
//    I removed one of the base classes and I changed the code so that
//    images can cycle or they don't have to cycle.
//
//    Brad Whitlock, Tue Apr 27 14:21:15 PST 2004
//    I made it modal on MacOS X to fix a menu bug that I ran into.
//
//    Brad Whitlock, Tue Mar 8 16:07:36 PST 2005
//    Added some code to draw the "Beta" marking back in.
//
// ****************************************************************************

SplashScreen::SplashScreen(bool cyclePictures, const char *name) :
    QFrame(0, name,
           WStyle_Customize | WStyle_NoBorderEx
#ifdef Q_WS_MACX
           | WShowModal
#endif
          )
{
    splashMode = true;

    // If the window manager is dumb enough to put decorations on this
    // window, at least put a reasonable title on it.
    QString ver(VERSION);
    setCaption(QString("VisIt ") + ver);

    // Set up a box to put the picture in
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(4);
    setMargin(10);

    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(5);
    topLayout->setMargin(5);

    // Create a timer to switch pictures
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),
            this, SLOT(nextPicture()));

    //
    // Load the pictures. We may only load 1, or if we are cycling the
    // pictures, we load them all.
    //
    QTime t(QTime::currentTime());
    int seconds = (t.hour() * 60) + (t.minute() * 60) + t.second();
    int firstPicture = seconds % NUM_PIX;
    curPicture = cyclePictures ? firstPicture : 0;
    if(firstPicture == 0 || cyclePictures)
         pictures.push_back(QPixmap(VisIt1_xpm));
#ifdef MULTIPLE_IMAGES
    if(firstPicture == 1 || cyclePictures)
         pictures.push_back(QPixmap(VisIt2_xpm));
    if(firstPicture == 2 || cyclePictures)
         pictures.push_back(QPixmap(VisIt3_xpm));
    if(firstPicture == 3 || cyclePictures)
         pictures.push_back(QPixmap(VisIt4_xpm));
#endif

    // If we have a beta in the version number, draw "Beta" on the pictures.
    if(ver.right(1) == "b")
    {
        for(int i = 0; i < pictures.size(); ++i)
        {
            QPainter painter(&pictures[i]);
            double scale = 4.;
            painter.scale(scale, scale);
            QFont font("helvetica", 24, QFont::Bold, true);
            font.setItalic(false);
            int x = 10;
            int y = pictures[i].height() - 10;
            int offset = 4;
            painter.setPen(black);
            painter.drawText(int(x / scale), int(y / scale), "Beta");
            painter.setPen(white);
            painter.drawText(int((x - offset) / scale),
                             int((y - offset) / scale), "Beta");
        }
    }

    // Set the picture on the window
    pictureLabel = new QLabel(this);
    pictureLabel->setPixmap(pictures[curPicture]);
    pictureLabel->setBackgroundMode(NoBackground);
    topLayout->addWidget(pictureLabel, 0, AlignCenter);

    // Put in a label for text
    text = new QLabel(this);
    text->setText("Starting VisIt...");
    topLayout->addWidget(text, 0, AlignLeft);
    topLayout->addSpacing(10);

    // Add a progress bar
    progress = new QProgressBar(this);
    progress->setProgress(0);
    progress->setMinimumWidth(pictures[curPicture].width());
    topLayout->addWidget(progress, 0, AlignLeft);

    // Add a dismiss button
    dismissButton = new QPushButton("Dismiss", this, "dismissButton");
    connect(dismissButton, SIGNAL(clicked()),
            this, SLOT(hide()));
    dismissButton->hide();
    topLayout->addWidget(dismissButton, 0, AlignCenter);
    topLayout->addSpacing(5);
}

// ****************************************************************************
// Method: SplashScreen::~SplashScreen
//
// Purpose: 
//   This is the destructor for the SplashScreen class.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:40:33 PST 2001
//
// Modifications:
//
// ****************************************************************************

SplashScreen::~SplashScreen()
{
}

// ****************************************************************************
// Method: SplashScreen::show
//
// Purpose: 
//   Shows the widget.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 18 17:52:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
SplashScreen::show()
{
    // Figure out where to put the window
    int     W = qApp->desktop()->width();
    int     H = qApp->desktop()->height();
    move((W - pictures[0].width()) / 2, (H - pictures[0].height()) / 2);

    // Show the window
    QFrame::show();
    QFrame::raise();

    // Start the picture cycling timer.
    if(pictures.size() > 1)
        timer->start(TIMER_DURATION);
}

// ****************************************************************************
// Method: SplashScreen::hide
//
// Purpose: 
//   Hides the widget.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 18 17:51:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
SplashScreen::hide()
{
    timer->stop();
    QFrame::hide();
}

// ****************************************************************************
// Method: SplashScreen::Progress
//
// Purpose: 
//   Shows progress in the splashscreen.
//
// Arguments:
//   msg : The message to show.
//   percent : The percent to show on the status bar.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 18 17:50:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
SplashScreen::Progress(const char *msg, int percent)
{
    text->setText(msg);
    progress->setProgress(percent);
}

// ****************************************************************************
// Method: SplashScreen::About
//
// Purpose: 
//   Shows the splashscreen as an about box.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 18 17:57:09 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
SplashScreen::About()
{
    SetDisplayAsSplashScreen(false);
    show();
}

// ****************************************************************************
// Method: SplashScreen::SetDisplayMode
//
// Purpose: 
//   Switches between splashscreen mode and about mode.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 17:04:56 PST 2001
//
// Modifications:
//    Eric Brugger, Tue Sep 18 09:13:20 PDT 2001
//    Correct an assignment statement where "==" was used instead of "=".
//
//    Brad Whitlock, Wed Jun 18 17:50:31 PST 2003
//    Made it work with the splashscreen as a widget.
//
// ****************************************************************************

void
SplashScreen::SetDisplayAsSplashScreen(bool asSplash)
{
    // Check if we need to do anything.
    if (splashMode == asSplash)
        return;

    if (!asSplash)
    {
        // Go to about mode.
        text->setText("");
        progress->hide();
        dismissButton->show();
    }
    else
    {
        // Go to splashscreen mode.
        progress->show();
        dismissButton->hide();
    }

    // Save the mode
    splashMode = asSplash;
}

// ****************************************************************************
// Method: SplashScreen::nextPicture
//
// Purpose: 
//   This is a Qt slot function that cycles the images.
//
// Programmer: Sean Ahern
// Creation:   Mon May 20 15:37:00 PDT 2002
//
// Modifications:
//    Jeremy Meredith, Fri Jul 12 14:38:53 PDT 2002
//    Added code to exit if we have only one image.
//
// ****************************************************************************

void
SplashScreen::nextPicture()
{
    if (pictures.size() > 1)
    {
        ++curPicture;
        if (curPicture >= pictures.size())
            curPicture = 0;

        pictureLabel->setPixmap(pictures[curPicture]);
    }
}
