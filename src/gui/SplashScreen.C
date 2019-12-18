// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SplashScreen.h>
#include <QApplication>
#include <QDate>
#include <QFont>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QDateTime>
#include <QTimer>
#include <QProgressBar>
#include <QPainter>
#include <QPixmap>
#include <QDesktopWidget>
#include <InstallationFunctions.h>
#include <ConfigureInfo.h>

#define TIMER_DURATION 2*1000     // 2 seconds
#define NUM_PIX 4

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
//    Brad Whitlock, Tue Nov 21 13:37:26 PST 2006
//    I renamed the splashscreen so it has a different name than the main
//    window so window managers display something a little more sensible
//    in the taskbar.
//
//    Brad Whitlock, Mon Dec 10 17:33:35 PST 2007
//    Changed the version string that gets drawn into the splashscreen.
//
//    Brad Whitlock, Mon Jan  7 16:30:14 PST 2008
//    Changed how the splashscreen looks.
//
//    Eric Brugger, Thu Mar  6 16:33:40 PST 2008
//    Changed the date on the splash screen and the size and location of
//    the patch level so that it wouldn't overlap the main version number
//    and look better.
//
//    Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//    Support for internationalization.
//
//    Eric Brugger, Thu Apr 10 16:26:43 PDT 2008
//    Changed the date on the splash screen.
//
//    Cyrus Harrison, Tue Jul  1 10:33:10 PDT 2008
//    Initial Qt4 Port.
//
//    Eric Brugger, Fri Aug  8 13:01:47 PDT 2008
//    Changed the date on the splash screen.
//
//    Eric Brugger, Tue Oct 14 11:26:43 PDT 2008
//    Changed the date on the splash screen.
//
//    Eric Brugger, Mon Dec 29 15:08:57 PST 2008
//    Changed the date on the splash screen.
//
//    Eric Brugger, Tue Mar 17 15:36:27 PDT 2009
//    Changed the date on the splash screen.
//
//    Tom Fogal, Sun Jan 24 17:09:18 MST 2010
//    Patch from Andreas Kloeckner to tell Qt we've got a splash screen window,
//    instead of just a window without a frame.
//
//    Eric Brugger, Fri Dec 10 16:28:59 PST 2010
//    Changed the date on the splash screen.
//
//    Eric Brugger, Thu Mar 31 09:35:28 PDT 2011
//    Changed the date on the splash screen.
//
//    Eric Brugger, Mon Aug 29 14:44:44 PDT 2011
//    Changed the date on the splash screen to September 2011.
//
//    Eric Brugger, Tue Nov  1 10:25:55 PDT 2011
//    Changed the date on the splash screen to November 2011.
//
//    Eric Brugger, Mon Jan 30 10:55:12 PST 2012
//    Changed the date on the splash screen to February 2012.
//
//    Brad Whitlock, Wed Feb  1 15:28:19 PST 2012
//    Automate the copyright year.
//
//    Eric Brugger, Wed Nov 14 14:06:00 PST 2012
//    Changed the date on the splash screen to November 2012.
//
//    Eric Brugger, Wed Jan 23 09:06:55 PST 2013
//    Changed the date on the splash screen to January 2013.
//
//    Eric Brugger, Wed Jul 10 10:56:47 PDT 2013
//    Changed the date on the splash screen to July 2013.
//
//    Brad Whitlock, Wed Sep 25 13:22:27 PDT 2013
//    Get splashscreen from resources.
//
//    Eric Brugger, Wed Oct  9 14:25:58 PDT 2013
//    Changed the date on the splash screen to October 2013.
//
//    Eric Brugger, Mon Jan 13 14:05:09 PST 2014
//    Changed the date on the splash screen to January 2014.
//
//    Eric Brugger, Thu Mar 13 09:50:50 PDT 2014
//    Changed the date on the splash screen to March 2014.
//
//    Eric Brugger, Wed Jun  4 07:34:26 PDT 2014
//    Changed the date on the splash screen to June 2014.
//
//    Eric Brugger, Wed Aug 13 08:43:26 PDT 2014
//    Changed the date on the splash screen to September 2014.
//
//    Eric Brugger, Wed Sep 24 11:34:42 PDT 2014
//    Changed the date on the splash screen to October 2014.
//
//    Eric Brugger, Fri Nov 21 09:02:31 PST 2014
//    Changed the date on the splash screen to December 2014.
//
//    Eric Brugger, Wed Jan 21 08:28:25 PST 2015
//    Changed the date on the splash screen to February 2015.
//
//    Eric Brugger, Wed Apr 22 11:38:06 PDT 2015
//    Changed the date on the splash screen to May 2015.
//
//    Eric Brugger, Mon Jun  8 08:38:45 PDT 2015
//    Changed the date on the splash screen to June 2015.
//
//    Eric Brugger, Mon Jun  8 08:38:45 PDT 2015
//    Changed the date on the splash screen to October 2015.
//
//    Eric Brugger, Wed Feb 17 16:17:38 PST 2016
//    Changed the date on the splash screen to February 2016.
//
//    Eric Brugger, Wed Feb 24 17:08:45 PST 2016
//    Changed the year on the splash screen to 2016.
//
//    Eric Brugger, Fri Mar 25 09:51:03 PDT 2016
//    Changed the date on the splash screen to March 2016.
//
//    Eric Brugger, Fri Mar 25 09:51:03 PDT 2016
//    Changed the date on the splash screen to June 2016.
//
//    Eric Brugger, Tue Aug  9 11:39:33 PDT 2016
//    Changed the date on the splash screen to August 2016.
//
//    Eric Brugger, Tue Oct 25 12:42:32 PDT 2016
//    Changed the date on the splash screen to November 2016.
//
//    Eric Brugger, Tue Jan  3 07:54:18 PST 2017
//    Changed the date on the splash screen to January 2017.
//
//    Eric Brugger, Wed Mar 22 14:44:13 PDT 2017
//    Changed the date on the splash screen to April 2017.
//
//    Eric Brugger, Thu Jun  8 15:15:03 PDT 2017
//    Changed the date on the splash screen to June 2017.
//
//    Eric Brugger, Thu Aug 31 16:28:56 PDT 2017
//    Changed the date on the splash screen to September 2017.
//
//    Eric Brugger, Fri Sep 22 12:49:25 PDT 2017
//    Changed the date on the splash screen to October 2017.
//
//    Eric Brugger, Mon Mar 19 14:38:44 PDT 2018
//    Changed the date on the splash screen to March 2018.
//
//    Eric Brugger, Tue May 22 08:43:06 PDT 2018
//    Changed the date on the splash screen to May 2018.
//
//    Eric Brugger, Fri Sep  7 13:17:36 PDT 2018
//    Changed the date on the splash screen to September 2018.
//
//    Eric Brugger, Wed Oct 17 14:37:35 PDT 2018
//    Changed the date on the splash screen to October 2018.
//
//    Eric Brugger, Thu Jan  3 15:52:59 PST 2019
//    Changed the date on the splash screen to January 2019.
//
//    Eric Brugger, Wed Feb 20 17:03:34 PST 2019
//    Changed the date on the splash screen to February 2019.
//
//    Eric Brugger, Thu Apr 18 16:24:00 PDT 2019
//    Changed the date on the splash screen to April 2019.
//
//    Eric Brugger, Wed Nov  6 15:02:49 PST 2019
//    Changed the date on the splash screen to November 2019.
//
//    Eric Brugger, Thu Dec  5 12:17:36 PST 2019
//    Changed the date on the splash screen to December 2019.
//
// ****************************************************************************

SplashScreen::SplashScreen(bool cyclePictures) : QFrame(0, Qt::SplashScreen)
{
#if defined(Q_OS_MAC)
    setWindowModality(Qt::WindowModal);
#endif    
    
    splashMode = true;

    // If the window manager is dumb enough to put decorations on this
    // window, at least put a reasonable title on it.
    QString caption(tr("VisIt %1 splash screen").arg(visitcommon::Version().c_str()));
    setWindowTitle(caption);

    // Set up a box to put the picture in
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(4);

    topLayout = new QVBoxLayout(this);
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
         pictures.push_back(QPixmap(GetVisItResourcesFile(VISIT_RESOURCES_IMAGES, "VisIt1.png").c_str()));
    if(firstPicture == 1 || cyclePictures)
         pictures.push_back(QPixmap(GetVisItResourcesFile(VISIT_RESOURCES_IMAGES, "VisIt2.png").c_str()));
    if(firstPicture == 2 || cyclePictures)
         pictures.push_back(QPixmap(GetVisItResourcesFile(VISIT_RESOURCES_IMAGES, "VisIt3.png").c_str()));
    if(firstPicture == 3 || cyclePictures)
         pictures.push_back(QPixmap(GetVisItResourcesFile(VISIT_RESOURCES_IMAGES, "VisIt4.png").c_str()));

    // If we have more stuff than just a version number in the version
    // string then draw that information onto the splashscreen.
    QString ver;
    bool drawVersion = false;
    int major = 0, minor = 0, patch = 0;
    int ret = GetVisItVersionFromString(visitcommon::Version().c_str(), major, minor, patch);
    if(ret < 0)
    {
        ver = QString(visitcommon::Version().c_str());
        if(ver.right(1) == "b")
            ver = tr("Beta");
        drawVersion = true;
    }
    else if(patch > 0)
    {
        ver = tr("Patch %1").arg(patch);
        drawVersion = true;
    }

    if(drawVersion)
    {
        for(size_t i = 0; i < pictures.size(); ++i)
        {
            QPainter painter(&pictures[i]);
            double scale = 1.5;
            painter.scale(scale, scale);
            QFont font("helvetica", 20, QFont::Bold, true);
            font.setItalic(false);
            int x = 290;
            int y = pictures[i].height() - 10;
            painter.setPen(QColor(210,37,74));
            painter.drawText(int(x / scale), int(y / scale), ver);
        }
    }

    // Set the picture on the window
    pictureLabel = new QLabel(this);
    pictureLabel->setPixmap(pictures[curPicture]);
    topLayout->addWidget(pictureLabel, 0, Qt::AlignCenter);

    QHBoxLayout *lrLayout = new QHBoxLayout();
    topLayout->addLayout(lrLayout);
    
    lLayout = new QVBoxLayout();
    rLayout = new QVBoxLayout();
    rLayout->addStretch(1);
    
    lrLayout->addLayout(lLayout);
    lrLayout->addLayout(rLayout );
    

    QString C(QString("(c) 2000-%1 LLNS. ").arg(QDate::currentDate().year()));
    C += tr("All Rights Reserved");
    C += ".";
    lLayout->addWidget(new QLabel(C, this));

    QString versionText;
    versionText.sprintf("VisIt %s, ", visitcommon::Version().c_str());
    versionText += QString(visitcommon::VersionControlVersionString().c_str());

    // Create a lookup of month names so the internationalization
    // files don't have to change.
    QStringList months;
    months << tr("January")
           << tr("February")
           << tr("March")
           << tr("April")
           << tr("May")
           << tr("June")
           << tr("July")
           << tr("August")
           << tr("September")
           << tr("October")
           << tr("November")
           << tr("December");
    int currentMonth = 12;
    lLayout->addWidget(new QLabel(versionText, this));
    lLayout->addWidget(new QLabel(months[currentMonth-1] + " 2019", this));

    copyrightButton = 0;
    contributorButton = 0;
    dismissButton = 0;

    QFrame *splitter1 = new QFrame(this);
    splitter1->setFrameStyle(QFrame::HLine + QFrame::Raised);
    topLayout->addWidget(splitter1);

    // Put in a label for text
    text = new QLabel(this);
    text->setText(tr("Starting VisIt..."));
    topLayout->addWidget(text, 0, Qt::AlignLeft);
    topLayout->addSpacing(5);

    // Add a progress bar
    progress = new QProgressBar(this);
    progress->setValue(0);
    progress->setMinimumWidth(pictures[curPicture].width());
    topLayout->addWidget(progress, 0, Qt::AlignLeft);
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
// Method: SplashScreen::CreateAboutButtons
//
// Purpose: 
//   Creates the extra buttons for when the window is used for the About window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  8 14:14:03 PST 2008
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jul  1 10:33:10 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
SplashScreen::CreateAboutButtons()
{
    // Add a copyright button
    if(copyrightButton == 0)
    {
        copyrightButton = new QPushButton(tr("Copyright..."));
        connect(copyrightButton, SIGNAL(clicked()),
                this, SLOT(emitShowCopyright()));
        rLayout->addWidget(copyrightButton, Qt::AlignRight);
    }

    // Add a contributor button.
    if(contributorButton == 0)
    {
        contributorButton = new QPushButton(tr("Contributors..."));
        connect(contributorButton, SIGNAL(clicked()),
                this, SLOT(emitShowContributors()));
        rLayout->addWidget(contributorButton, Qt::AlignRight);
        rLayout->addStretch(1);
    }

    // Add a dismiss button
    if(dismissButton == 0)
    {
        dismissButton = new QPushButton(tr("Dismiss"));
        connect(dismissButton, SIGNAL(clicked()),
                this, SLOT(hide()));
        topLayout->addWidget(dismissButton, 0, Qt::AlignCenter);
        topLayout->addSpacing(5);
    }
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
//   Brad Whitlock, Wed Apr  9 10:26:34 PDT 2008
//   Use QString instead of const char *.
//
//   Cyrus Harrison, Tue Jul  1 10:33:10 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
SplashScreen::Progress(const QString &msg, int percent)
{
    text->setText(msg);
    progress->setValue(percent);
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
//    Brad Whitlock, Tue Jan  8 13:51:23 PST 2008
//    Hide/Show the new buttons.
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
        CreateAboutButtons();
        dismissButton->show();
        copyrightButton->show();
        contributorButton->show();
    }
    else
    {
        // Go to splashscreen mode.
        progress->show();
        if(dismissButton != 0)
            dismissButton->hide();
        if(copyrightButton != 0)
            copyrightButton->hide();
        if(contributorButton != 0)
            contributorButton->hide();
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
        if (curPicture >= (int)pictures.size())
            curPicture = 0;

        pictureLabel->setPixmap(pictures[curPicture]);
    }
}


void
SplashScreen::emitShowCopyright()
{
    hide();
    emit showCopyright();
}

void
SplashScreen::emitShowContributors()
{
    hide();
    emit showContributors();
}
