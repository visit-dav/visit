#include <iostream.h>
#include <SplashScreen.h>
#include <SplashScreenAttributes.h>
#include <AppearanceAttributes.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qprogressbar.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qmotifstyle.h>
#include <qcdestyle.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#include <visit-config.h>   // For version number
#if QT_VERSION >= 230
#include <qsgistyle.h>
#endif

#define NUM_PIX 4
#include "VisIt1.xpm"
#include "VisIt2.xpm"
#include "VisIt3.xpm"
#include "VisIt4.xpm"
#define TIMER_DURATION 2*1000     // 3 seconds

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
// ****************************************************************************

SplashScreen::SplashScreen(const char *name, bool allowShow_) :
    QFrame(NULL, name,
           WStyle_Customize | WStyle_NoBorderEx
          ), SimpleObserver()
{
    splashAtts = 0;
    appearanceAtts = 0;
    splashMode = true;
    allowShow = allowShow_;

    // If the window manager is dumb enough to put decorations on this
    // window, at least put a reasonable title on it.
    setCaption(QString("VisIt ") + VERSION);

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

    // Load the pictures
    curPicture = 0;
    pictures.push_back(QPixmap(VisIt1_xpm));
    pictures.push_back(QPixmap(VisIt2_xpm));
    pictures.push_back(QPixmap(VisIt3_xpm));
    pictures.push_back(QPixmap(VisIt4_xpm));

    // The writing for VisIt is temporarily disabled; until we find
    // a better way to make fonts consistently sized and pretty, we
    // will draw the picture directly on the image.
    //
    // This has not been retrofitted for multiple splashscreen images.
#if 0
    // Put in "VisIt" and the version number
    QPainter painter(&pictures[0]);
    QFont   font("helvetica", 24, QFont::Bold, true);
    font.setPixelSize(30);
    painter.setFont(font);
    DropShadowText(&painter, 5, 30, QString("VisIt ") + VERSION);

    // If the version number starts with a "0", put in "Beta Version"
    if (VERSION[0] == '0')
    {
        painter.rotate(-30);
        painter.scale(2, 2);
        painter.setPen(black);
        font.setPixelSize(200);
        font.setItalic(false);
        painter.setPen(black);
        DropShadowText(&painter, -50, 150, QString("Beta Version"));
    }
#endif

    // Set the picture on the window
    pictureLabel = new QLabel(this);
    pictureLabel->setPixmap(pictures[0]);
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
    progress->setMinimumWidth(pictures[0].width());
    topLayout->addWidget(progress, 0, AlignLeft);

    // Add a dismiss button
    dismissButton = new QPushButton("Dismiss", this, "dismissButton");
    connect(dismissButton, SIGNAL(clicked()),
            this, SLOT(hideSelf()));
    dismissButton->hide();
    topLayout->addWidget(dismissButton, 0, AlignCenter);
    topLayout->addSpacing(5);

    // Figure out where to put the window
    int     W = qApp->desktop()->width();
    int     H = qApp->desktop()->height();
    move((W - pictures[0].width()) / 2, (H - pictures[0].height()) / 2);

    if (allowShow == true)
    {
        show();
        timer->start(TIMER_DURATION);
    }
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
//   Brad Whitlock, Thu Sep 6 16:51:37 PST 2001
//   Made the two subjects detach if they are still around.
//
// ****************************************************************************
SplashScreen::~SplashScreen()
{
    if (splashAtts)
        splashAtts->Detach(this);

    if (appearanceAtts)
        appearanceAtts->Detach(this);
}

// ****************************************************************************
// Method: SplashScreen::DropShadowText
//
// Purpose: 
//   Convenience function to draw text with a drop shadow
//
// Arguments:
//   p : The QPainter to use
//   x : The x location of the text
//   y : The y location of the text
//   s : The text
//
// Programmer: Sean Ahern
// Creation:   Wed Sep 12 12:23:05 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreen::DropShadowText(QPainter *p, int x, int y, QString s)
{
    p->setPen(black);
    p->drawText(x, y, s);              // Drop shadow
    p->setPen(white);
    p->drawText(x - 2, y - 2, s);      // Normal text
}

// ****************************************************************************
// Method: SplashScreen::Update
//
// Purpose: 
//   This method is called when the subjects that this object observes are
//   modified.
//
// Arguments:
//   subj : The subject that is being modified.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 6 16:41:01 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Apr 26 16:30:38 PST 2002
//   Made it raise the splashscreen after showing it.
//
//   Sean Ahern, Thu Jun 13 20:38:50 PDT 2002
//   Stopped the splashscreen from raising every time it got updated.
//
// ****************************************************************************
void
SplashScreen::Update(Subject *subj)
{
    if (splashAtts == NULL || appearanceAtts == NULL)
        return;

    if (splashAtts == subj)
    {
        SetDisplayMode(splashAtts->GetDisplayAsSplashScreen());

        // Show or hide the widget.
        if (splashAtts->GetVisible() == true)
        {
            // Only show it if we allowing shows
            if ((splashMode == false) || (allowShow != false))
            {
                if (isHidden() == true)
                {
                    timer->start(TIMER_DURATION);
                    show();
                    raise();
                }
            }
        }
        else
        {
            timer->stop();

            if (isHidden() == false)
                hide();
        }

        text->setText(splashAtts->GetText().c_str());
        progress->setProgress(splashAtts->GetProgress());

        if (splashAtts->GetQuit() == true)
            qApp->quit();
    }
    else if (appearanceAtts == subj)
    {
        // Customize the widget appearance.
        CustomizeAppearance();
    }
}

// ****************************************************************************
// Method: SplashScreen::SubjectRemoved
//
// Purpose: 
//   Detaches a subject from this observer.
//
// Arguments:
//   subj : The subject that is being detached.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 16:49:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreen::SubjectRemoved(Subject *subj)
{
    if (subj == splashAtts)
        splashAtts = NULL;
    else if (subj == appearanceAtts)
        appearanceAtts = NULL;
}

// ****************************************************************************
// Method: SplashScreen::ConnectSplashScreenAtts
//
// Purpose: 
//   Connects the SplashScreenAttributes to this observer.
//
// Arguments:
//   atts : The subject to connect.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 17:09:44 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreen::ConnectSplashScreenAtts(SplashScreenAttributes *atts)
{
    splashAtts = atts;
    splashAtts->Attach(this);
}

// ****************************************************************************
// Method: SplashScreen::ConnectAppearanceAtts
//
// Purpose: 
//   Connects the AppearanceAttributes to this observer.
//
// Arguments:
//   atts : The subject to connect.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 17:09:44 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreen::ConnectAppearanceAtts(AppearanceAttributes *atts)
{
    appearanceAtts = atts;
    appearanceAtts->Attach(this);
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
// ****************************************************************************
void
SplashScreen::SetDisplayMode(bool asSplash)
{
    // Check if we need to do anything.
    if (splashMode == asSplash)
        return;

    if (asSplash == false)
    {
        // Go to about mode.
        splashAtts->SetText("");
        progress->hide();
        dismissButton->show();
        allowShow = true;
    } else
    {
        // Go to splashscreen mode.
        progress->show();
        dismissButton->hide();
    }

    // Save the mode
    splashMode = asSplash;
}

// ****************************************************************************
// Method: SplashScreen::CustomizeAppearance
//
// Purpose: 
//   Sets the application's appearance based on the appearance attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 16:56:39 PST 2001
//
// Modifications:
//     Sean Ahern, Mon Apr 15 15:40:24 PDT 2002
//     Commented out an unused variable.
//   
// ****************************************************************************
void
SplashScreen::CustomizeAppearance()
{
    bool    backgroundSelected = appearanceAtts->IsSelected(0);
    bool    foregroundSelected = appearanceAtts->IsSelected(1);
    //Not used //bool    fontSelected = appearanceAtts->IsSelected(2);
    bool    styleSelected = appearanceAtts->IsSelected(3);

    // Set the style
    if (styleSelected)
    {
        if (appearanceAtts->GetStyle() == "cde")
            QApplication::setStyle(new QCDEStyle);
        else if (appearanceAtts->GetStyle() == "windows")
            QApplication::setStyle(new QWindowsStyle);
        else if (appearanceAtts->GetStyle() == "platinum")
            QApplication::setStyle(new QPlatinumStyle);
#if QT_VERSION >= 230
        else if (appearanceAtts->GetStyle() == "sgi")
            QApplication::setStyle(new QSGIStyle);
#endif
        else
            QApplication::setStyle(new QMotifStyle);
    }
    // Set the application colors
    if (backgroundSelected || foregroundSelected || styleSelected)
    {
        QColor  bg(appearanceAtts->GetBackground().c_str());
        QColor  fg(appearanceAtts->GetForeground().c_str());
        QColor  btn(bg);

        int     h, s, v;
        fg.hsv(&h, &s, &v);
        QColor  base = Qt::white;
        bool    bright_mode = false;
        if (v >= 255 - 50)
        {
            base = btn.dark(150);
            bright_mode = TRUE;
        }

        QColorGroup cg(fg, btn, btn.light(),
                       btn.dark(), btn.dark(150), fg, Qt::white, base, bg);
        if (bright_mode)
        {
            cg.setColor(QColorGroup::HighlightedText, base);
            cg.setColor(QColorGroup::Highlight, Qt::white);
        } else
        {
            cg.setColor(QColorGroup::HighlightedText, Qt::white);
            cg.setColor(QColorGroup::Highlight, Qt::darkBlue);
        }
        QColor  disabled((fg.red() + btn.red()) / 2,
                         (fg.green() + btn.green()) / 2,
                         (fg.blue() + btn.blue()) / 2);
        QColorGroup dcg(disabled, btn, btn.light(125), btn.dark(),
                        btn.dark(150), disabled, Qt::white, Qt::white, bg);
        if (bright_mode)
        {
            dcg.setColor(QColorGroup::HighlightedText, base);
            dcg.setColor(QColorGroup::Highlight, Qt::white);
        } else
        {
            dcg.setColor(QColorGroup::HighlightedText, Qt::white);
            dcg.setColor(QColorGroup::Highlight, Qt::darkBlue);
        }
        QPalette pal(cg, dcg, cg);
        QApplication::setPalette(pal, true);
    }
}

// ****************************************************************************
// Method: SplashScreen::hideSelf
//
// Purpose: 
//   Turns off the timer and hides.
//
// Programmer: Sean Ahern
// Creation:   Mon May 20 15:37:00 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
void
SplashScreen::hideSelf()
{
    timer->stop();
    hide();
}

// ****************************************************************************
// Method: SplashScreen::nextPicture
//
// Purpose: 
//   Cycles the images.
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
    if (NUM_PIX <= 1)
        return;

    curPicture++;
    if (curPicture >= NUM_PIX)
        curPicture = 0;

    pictureLabel->setPixmap(pictures[curPicture]);
}
