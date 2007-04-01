#include <QvisVisItUpdate.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qtimer.h>
#include <qurlinfo.h>
    
#include <stdlib.h>
#include <visit-config.h>
#include <DebugStream.h>
#include <Utility.h>

#define STAGE_CONNECT               0
#define STAGE_LOGIN                 1
#define STAGE_DETERMINE_VERSION     2
#define STAGE_GET_FILES_FOR_VERSION 3
#define STAGE_GET_FILES             4
#define STAGE_INSTALL               5
#define STAGE_CLEAN_UP              6
#define STAGE_ERROR                 10

//#define DEBUGGING
#ifdef DEBUGGING
#define CURRENT_VERSION "1.3.4"
#define VISITARCHHOME   "/home/whitlocb/visitinstall/1.4.1/sun4-sunos5-sparc"
#else
#define CURRENT_VERSION VERSION
#define VISITARCHHOME   getenv("VISITARCHHOME")
#endif

//
// Architecture names that match VisIt's architecture directories.
//
const char *archNames[] = {
"darwin-ppc",
"dec-osf1-alpha",
"ibm-aix-pwr",
"ibm-aix-pwr64",
"linux-ia64",
"linux-intel",
"linux-x86_64",
"sgi-irix6-mips2",
"sun4-sunos5-sparc"
};

#define NARCH (sizeof(archNames) / sizeof(const char *))

//
// Names that match VisIt's distribution names.
//
const char *distNames[] = {
"darwin",
"osf1",
"aix",
"aix",
"linux-ia64",
"linux_chaos",
"linux-x86_64",
"irix6",
"sunos5"
};

// ****************************************************************************
// Method: QvisVisItUpdate::QvisVisItUpdate
//
// Purpose: 
//   Constructor for the QvisVisItUpdate class.
//
// Arguments:
//   parent : The object's parent.
//   name   : The name of the object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:22:32 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisVisItUpdate::QvisVisItUpdate(QObject *parent, const char *name) :
    QObject(parent, name), GUIBase(), 
    latestVersion(CURRENT_VERSION), files(), downloads()
{
    stage = STAGE_CONNECT;
    architecture = -1;
    ftp = 0;
    installProcess = 0;
}

// ****************************************************************************
// Method: QvisVisItUpdate::~QvisVisItUpdate
//
// Purpose: 
//   The destructor for the QvisVisItUpdate class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:23:13 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:13:28 PDT 2005
//   I made it use QvisFtp.
//
// ****************************************************************************

QvisVisItUpdate::~QvisVisItUpdate()
{
    if(ftp)
    {
        if(ftp->State() != QvisFtp::Unconnected)
            ftp->Close();
    }
}

// ****************************************************************************
// Method: QvisVisItUpdate::provideLogin
//
// Purpose: 
//   This class provides the ftp login for the VisIt FTP site.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:23:38 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:13:28 PDT 2005
//   I made it use QvisFtp.
//   
// ****************************************************************************

void
QvisVisItUpdate::provideLogin()
{
#if defined(_WIN32)
    const char *platform = "win32";
#else
    const char *platform = distNames[architecture];
#endif

    // Get the number of startups.
    ConfigStateEnum code;
    int nStartups = ConfigStateGetRunCount(code);
    QString password;
    password.sprintf("visit_update_%s_%s_%d", CURRENT_VERSION,
        platform, nStartups);

    ftp->Login("anonymous", password);
}

// ****************************************************************************
// Method: QvisVisItUpdate::latestDirectory
//
// Purpose: 
//   This method returns the most recent version directory on the FTP site.
//
// Returns:    The most recent version directory.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:24:03 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QString
QvisVisItUpdate::latestDirectory() const
{
    return QString("/pub/visit/visit") + latestVersion + "/";
}

// ****************************************************************************
// Method: QvisVisItUpdate::localTempDirectory
//
// Purpose: 
//   Returns a local temporary directory.
//
// Returns:    A local temporary directory.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:24:44 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QString
QvisVisItUpdate::localTempDirectory() const
{
#if defined(_WIN32)
    return "C:\\";
#else
    return "./";
#endif
}

// ****************************************************************************
// Method: QvisVisItUpdate::getRequiredFiles
//
// Purpose: 
//   Sets up the list of files that must be downloaded and starts downloading
//   the files.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:25:22 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisVisItUpdate::getRequiredFiles()
{
    downloads.clear();
#if defined(_WIN32)
    QString dist("visit" + latestVersion + ".exe");
    downloads += dist;
#else
    downloads += "visit-install";

    //
    // Add files that are appropriate for the architecture.
    //
    if(architecture != -1)
    {
        for(int j = 0; j < files.count(); ++j)
        {
            if(files[j].find(distNames[architecture]) != -1)
            {
                downloads += files[j];
                break;
            }
        }
    }
#endif

    // Make sure that all of the requested files are in the files list.
    // If they are then initiate the download, otherwise issue an error.

    files.clear();

    initiateDownload();
}

// ****************************************************************************
// Method: QvisVisItUpdate::getInstallationDir
//
// Purpose: 
//   This method returns the directory where VisIt is installed.
//
// Returns:    The directory where VisIt is installed.
//
// Note:       On UNIX, the directory is where the current version is running
//             from. On Windows, this is not the case since the Windows version
//             can be installed elsewhere. Check the registry on Windows.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:26:07 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 12:12:43 PDT 2005
//   I made it use a new Utility.C function: GetVisItInstallationDirectory.
//
// ****************************************************************************

QString
QvisVisItUpdate::getInstallationDir() const
{
    return QString(GetVisItInstallationDirectory(latestVersion.latin1()).c_str());
}

// ****************************************************************************
// Method: QvisVisItUpdate::installVisIt
//
// Purpose: 
//   This method installs VisIt.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:27:30 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisVisItUpdate::installVisIt()
{
    if(files.count() > 0)
    {
        // If we've created the installer before, delete it.
        if(installProcess != 0)
        {
#if !defined(_WIN32)
            disconnect(installProcess, SIGNAL(readyReadStdout()),
                       this, SLOT(readInstallerStdout()));
            disconnect(installProcess, SIGNAL(readyReadStderr()),
                       this, SLOT(readInstallerStderr()));
#endif
            disconnect(installProcess, SIGNAL(processExited()),
                       this, SLOT(emitInstallationComplete()));
            delete installProcess;
            installProcess = 0;
        }

        // Set the cursor to busy and tell the user that we're installing.
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        Status("Installing...", 120000);

#if defined(_WIN32)
        // Install VisIt the WIN32 way by running the visit installer.
        QString visitexe(files.front());
        installProcess = new QProcess(visitexe, this, visitexe.latin1());
#else
        // Install VisIt the UNIX way.
        QString visit_install(files.front());
        QString command;
        command.sprintf("chmod 700 %s", visit_install.latin1());
        system(command.latin1());

        QString platform(distNames[architecture]);

        // Get the VisIt installation directory.
        QString installDir(getInstallationDir());

        // Create an installation process that will run visit-install.
        if(installProcess == 0)
        {
            installProcess = new QProcess(visit_install, this, "visit-install");
            installProcess->addArgument("-c");
            installProcess->addArgument("none");
            installProcess->addArgument(latestVersion);
            installProcess->addArgument(platform);
            installProcess->addArgument(installDir);
            connect(installProcess, SIGNAL(readyReadStdout()),
                    this, SLOT(readInstallerStdout()));
            connect(installProcess, SIGNAL(readyReadStderr()),
                    this, SLOT(readInstallerStderr()));
            debug1 << "Going to run: visit_install -c none "
                   << latestVersion.latin1() << " "
                   << platform.latin1() << " " 
                   << installDir.latin1() << endl;
        }
#endif
        // We want to know when the installer completes.
        connect(installProcess, SIGNAL(processExited()),
                this, SLOT(emitInstallationComplete()));

        // Start the visit installer.
        installProcess->start();
    }
}

// ****************************************************************************
// Method: QvisVisItUpdate::cleanUp
//
// Purpose: 
//   This method removes the files that were downloaded.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:32:11 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisVisItUpdate::cleanUp()
{
    // Delete all of the files in the files list.
    QDir dir;
    for(int i = 0; i < files.count(); ++i)
    {
        dir.remove(files[i]);
        debug1 << "Removed " << files[i].latin1() << endl;
    }
    files.clear();
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisVisItUpdate::startUpdate
//
// Purpose: 
//   This is a Qt slot function that starts updating VisIt.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:48:42 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:09:39 PDT 2005
//   I made it use QvisFtp.
//
// ****************************************************************************

void
QvisVisItUpdate::startUpdate()
{
#if !defined(_WIN32)
    //
    // Try and determine the platform that should be downloaded.
    //
    const char *archHome = VISITARCHHOME;
    bool platformDetermined = false;
    if(archHome != 0)
    {
        QString arch(archHome);
        int lastSlash = arch.findRev('/');
        if(lastSlash != -1)
        {
            arch = arch.right(arch.length() - lastSlash - 1);
            for(int i = 0; i < NARCH; ++i)
            {
                if(arch == archNames[i])
                {
                    platformDetermined = true;
                    architecture = i;
                    break;
                }
            }

            if(!platformDetermined)
                debug1 << "Unknown plaform: " << arch.latin1() << endl;
        }
    }
    else
        debug1 << "VISITARCHHOME was not set." << endl;

    if(!platformDetermined)
    {
        Error("VisIt could not determine the platform that "
              "you are running on so VisIt cannot automatically "
              "update. You should browse to "
              "ftp://ftp.llnl.gov/pub/visit and download "
              "the latest binary distribution for your platform.");
        emit updateNotAllowed();
        return;
    }

    // Get the installation directory and make sure that the user can
    // write to it.
    QString installDir(getInstallationDir());
    QFileInfo info(installDir);
    if(!info.isWritable())
    {
        QString msg;
        QString g(info.group());
        if(g.isEmpty())
            g.sprintf("id %d", info.groupId());

        msg.sprintf("VisIt determined that you do not have write permission "
            "to the %s directory where VisIt is installed. You must have "
            "group %s write access to update VisIt.", installDir.latin1(),
            g.latin1());
        Error(msg);
        emit updateNotAllowed();
        return;
    }
#endif

    if(ftp == 0)
    {
        ftp = new QvisFtp(this);
        connect( ftp, SIGNAL(CommandStarted()),
            SLOT(ftp_commandStarted()) );
        connect( ftp, SIGNAL(CommandFinished()),
            SLOT(ftp_commandFinished()) );
        connect( ftp, SIGNAL(Done(bool)),
            SLOT(ftp_done(bool)) );
        connect( ftp, SIGNAL(StateChanged(int)),
            SLOT(ftp_stateChanged(int)) );
        connect( ftp, SIGNAL(ListInfo(const QUrlInfo &)),
            SLOT(ftp_listInfo(const QUrlInfo &)) );
    }

    stage = STAGE_CONNECT;
    initiateStage();
}

// ****************************************************************************
// Method: QvisVisItUpdate::initiateStage
//
// Purpose: 
//   This is a Qt slot function that is called repeatedly to handle stages
//   of the download and installation procedure for upgrading VisIt.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:34:27 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:10:08 PDT 2005
//   I made it use QvisFtp.
//
// ****************************************************************************

void
QvisVisItUpdate::initiateStage()
{
    debug1 << "initiateStage: Start stage: " << stage << endl;

    switch(stage)
    {
    case STAGE_CONNECT:
         ftp->ConnectToHost("ftp.llnl.gov", 21);
         break;
    case STAGE_LOGIN:
         provideLogin();
         break;
    case STAGE_DETERMINE_VERSION:
         ftp->List("/pub/visit");
         break;
    case STAGE_GET_FILES_FOR_VERSION:
         ftp->List(latestDirectory());
         break;
    case STAGE_GET_FILES:
         connect(ftp, SIGNAL(DataTransferProgress(int,int)),
             this, SLOT(ftp_reportDownloadProgress(int,int)));
         getRequiredFiles();
         break;
    case STAGE_INSTALL:
         disconnect(ftp, SIGNAL(DataTransferProgress(int,int)),
                this, SLOT(ftp_reportDownloadProgress(int,int)));
         installVisIt();
         break;
    case STAGE_CLEAN_UP:
    case STAGE_ERROR:
         cleanUp();
         if(ftp->State() != QvisFtp::Unconnected)
             ftp->Close();
         break;
    }

    debug1 << "initiateStage: End stage: " << stage << endl;
}

// ****************************************************************************
// Method: QvisVisItUpdate::nextStage
//
// Purpose: 
//   This is a Qt slot function that advances the procedure to the next stage.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:35:34 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisVisItUpdate::nextStage()
{
    // Move on to the next stage now that we've completed the
    // current stage.
    ++stage;
    QTimer::singleShot(100, this, SLOT(initiateStage()));
}

// ****************************************************************************
// Method: QvisVisItUpdate::initiateDownload
//
// Purpose: 
//   This is a Qt slot function that downloads a file from the VisIt FTP site
//   and adds it to the files list, which contains the local names of the
//   files that have been downloaded.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:36:11 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:13:55 PDT 2005
//   I made it use QvisFtp.
//
// ****************************************************************************

void
QvisVisItUpdate::initiateDownload()
{
    if(downloads.count() > 0)
    {
        QString current(downloads.first());
        downloads.pop_front();
        QString localName(localTempDirectory() + current);
        QString remoteName(latestDirectory() + current);

        debug1 << "Going to download " <<  remoteName.latin1()
               << " into local file " << localName.latin1() << endl;

        QFile *file = new QFile(localName);
        if(!file->open(IO_WriteOnly))
        {
            QString msg;
            msg.sprintf("Could not download %s! Can't finish updating VisIt.",
                        localName.latin1());
            Error(msg);

            delete file;
            ++stage; // skip installation and go to cleanup.
            nextStage();
        }
        files += localName;

        ftp->Get(remoteName, file);
    }
    else
        nextStage();
}


void
QvisVisItUpdate::readInstallerStdout()
{
    // Add the output to the output window.
    Message(installProcess->readLineStdout());
}

void
QvisVisItUpdate::readInstallerStderr()
{
    // Add the output to the output window.
    Message(installProcess->readLineStderr());
}

// ****************************************************************************
// Method: QvisVisItUpdate::emitInstallationComplete
//
// Purpose: 
//   This is a Qt slot function that is called when the VisIt installation
//   program has completed. We use this method to emit a signal that can
//   tell other objects that the installation is complete.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:41:28 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:31:03 PDT 2005
//   Added a check to make sure that the new executable is available before
//   emitting installationComplete in case the user aborted the installation.
//
// ****************************************************************************

void
QvisVisItUpdate::emitInstallationComplete()
{
    // Restore the cursor since we're done installing.
    QApplication::restoreOverrideCursor();
    Status("Installation complete.", 1000);

    // prevent updates again just in case.
    emit updateNotAllowed();

    // Move on to the cleanup stage.
    nextStage();

    // Now that cleanup is done, finalize the installation process.
#if defined(_WIN32)
    QString visitDir(getInstallationDir() + "\\");
    QString filename("visit.exe");
#else
    QString visitDir(getInstallationDir() + "/bin/");
    QString filename("visit");
#endif

    // Make sure that the VisIt executable exists
    QDir dir(visitDir);
    if(dir.exists() && dir.isReadable() && dir.exists(filename))
    {
        emit installationComplete(visitDir + filename);
    }
    else
    {
        QString err("The new version of VisIt could not be located in ");
        err += visitDir;
        err += filename;
        Error(err);
    }
}

// ****************************************************************************
// Method: QvisVisItUpdate::ftp_reportDownloadProgress
//
// Purpose: 
//   This is a Qt slot function that we hook up while we download the 
//   new version of VisIt so we can see the progress in the status bar.
//
// Arguments:
//   done  : Number of bytes read.
//   total : Number of bytes expected.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:42:30 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisVisItUpdate::ftp_reportDownloadProgress(int done, int total)
{
    QString msg;
    msg.sprintf("Downloaded %d/%d bytes.", done, total);
    Status(msg);
}

// ****************************************************************************
// Method: QvisVisItUpdate::ftp_commandStarted
//
// Purpose: 
//   This is a Qt slot function that is called when ftp commands start.
//
// Note:       We use this method to help determine state transitions.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:39:15 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisVisItUpdate::ftp_commandStarted()
{
    debug1 << "ftp_commandStarted: stage=" << stage << endl;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if(stage == STAGE_GET_FILES_FOR_VERSION)
        files.clear();
}

// ****************************************************************************
// Method: QvisVisItUpdate::ftp_commandFinished
//
// Purpose: 
//   This is a Qt slot function that is called when ftp commands complete.
//
// Note:       We use this method to help determine state transitions.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:39:15 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:17:24 PDT 2005
//   I made it use QvisFtp.
//
// ****************************************************************************

void
QvisVisItUpdate::ftp_commandFinished()
{
    debug1 << "ftp_commandFinished: stage=" << stage << endl;
    QApplication::restoreOverrideCursor();

    ftp->DeleteCurrentDevice();

    if(stage == STAGE_CONNECT || stage == STAGE_LOGIN)
    {
         nextStage();
    }
    else if(stage == STAGE_DETERMINE_VERSION)
    {
        debug1 << "The latest version of VisIt that was found is: "
               << latestVersion.latin1() << endl;

        QWidget *p = 0;
        if(parent() != 0 && parent()->isWidgetType())
            p = (QWidget *)parent();

        if(latestVersion == CURRENT_VERSION)
        {
            // Inform the user.
            QMessageBox::information(p, "VisIt",
                "Your version of VisIt is up to date.",
                QMessageBox::Ok);
            emit updateNotAllowed();
        }
        else
        {
            // Ask the user whether or not VisIt should be installed.
            QString msg;
            msg.sprintf("VisIt %s is available for download. Would you "
                        "like to install it?", latestVersion.latin1());
            if(QMessageBox::information(p, "VisIt", msg,
               QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
            {
                // Make the next stage be the clean up stage.
                stage = STAGE_CLEAN_UP-1;
                debug1 << "User chose not to install VisIt "
                       << latestVersion.latin1() << endl;
                emit updateNotAllowed();
            }

            nextStage();
        }
    }
    else if(stage == STAGE_GET_FILES_FOR_VERSION)
    {
        debug1 << "Files for version " << latestVersion.latin1() << ":\n";
        for(int i = 0; i < files.count(); ++i) 
            debug1 << "\t" << files[i].latin1() << endl;
        nextStage();
    }
    else if (stage == STAGE_GET_FILES)
    {
        // Get the next file.
        QTimer::singleShot(100, this, SLOT(initiateDownload()));
    }
}

// ****************************************************************************
// Method: QvisVisItUpdate::ftp_done
//
// Purpose: 
//   This method is called when there is an FTP error.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:47:58 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:10:40 PDT 2005
//   I made it use QvisFtp.
//
// ****************************************************************************

void
QvisVisItUpdate::ftp_done(bool error)
{
    if(error)
    {
        QString msg("VisIt could not complete the update.\n");
        msg += "FTP Error: ";
        msg += ftp->ErrorString();
        Error(msg);

        // If we are connected, but not logged in, it is not meaningful to stay
        // connected to the server since the error is a really fatal one (login
        // failed).
        if(ftp->State() == QvisFtp::Connected)
            ftp->Close();
        stage = STAGE_ERROR;
    }
}

void
QvisVisItUpdate::ftp_stateChanged(int state)
{
#if 0
    switch((QvisFtp::State)state)
    {
    case QvisFtp::Unconnected:
        qDebug("Unconnected");
        break;
    case QvisFtp::HostLookup:
        qDebug("Host lookup");
        break;
    case QvisFtp::Connecting:
        qDebug("Connecting");
        break;
    case QvisFtp::Connected:
        qDebug("Connected");
        break;
    case QvisFtp::LoggedIn:
        qDebug("Logged in");
        break;
    case QvisFtp::Closing:
        qDebug("Closing");
        break;
    }
#endif
}

// ****************************************************************************
// Method: QvisVisItUpdate::ftp_listInfo
//
// Purpose: 
//   This is a Qt slot function that is called in response to an FTP list
//   operation.
//
// Arguments:
//   item : Information about the i'th item in the directory.
//
// Note:       This method is called for each item in a directory. We use
//             this method to gather information about the version and the
//             files that are available for download.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:43:43 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Mar 7 14:48:50 PST 2005
//   I fixed some bad logic that prevented it from determining the correct
//   new version.
//
// ****************************************************************************

void
QvisVisItUpdate::ftp_listInfo(const QUrlInfo &item)
{
#if 0
    QString itemType;
    if(item.isDir())
        itemType = "dir";
    else
        itemType = "file";

    qDebug("ftp_listInfo: %s %s %d %s", itemType.latin1(),
           item.name().latin1(), item.size(), 
           item.lastModified().toString().latin1());
#endif

    if(stage == STAGE_DETERMINE_VERSION)
    {
        // If it's a directory with "visit" at the start then
        // look for a version number in the directory. Compare
        // the version number to see if it's greater than the
        // version compiled into the application.
        int index = -1;
        if(item.isDir() && 
           ((index = item.name().find("visit")) == 0))
        {
            // Split the VisIt version into major, minor, patch.
            QStringList visitVersion(QStringList::split(".", CURRENT_VERSION));

            // Split the file version into major, minor, patch.
            QString version(item.name().right(item.name().length() - 5));
            QStringList fileVersion(QStringList::split(".", version));

            // Make sure both versions have the same number of numbers.
            int n = visitVersion.count();
            if(n != fileVersion.count())
            {
                 n = (fileVersion.count() > n) ? fileVersion.count() : n;

                 while(visitVersion.count() < n)
                     visitVersion += "0";

                 while(fileVersion.count() < n)
                     fileVersion += "0";
            }                      

            for(int i = 0; i < n; ++i)
            {
                bool okay1 = true, okay2 = true;

                int vv = visitVersion[i].toInt(&okay1);
                int fv = fileVersion[i].toInt(&okay2);
                if(okay1 && okay2)
                {
                    if(vv < fv)
                    {
                        latestVersion = version;
                        break;
                    }
                    else if(fv != vv)
                        break;
                }
                else
                    break;
            }
        }
    }
    else if (stage == STAGE_GET_FILES_FOR_VERSION)
    {
        files += item.name();
    }
}
