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

#include <QvisVisItUpdate.h>
#include <QApplication>
#include <QCursor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>

#include <QvisDownloader.h>
    
#include <stdlib.h>
#include <visit-config.h>
#include <DebugStream.h>
#include <InstallationFunctions.h>

#define STAGE_SUBMIT_RUNINFO        0
#define STAGE_DETERMINE_VERSION     1
#define STAGE_GET_FILES             2
#define STAGE_INSTALL               3
#define STAGE_CLEAN_UP              4
#define STAGE_ERROR                 10

#define CURRENT_VERSION VISIT_VERSION

#ifdef QT_NO_OPENSSL
#define EXECUTABLE_HTML       "/codes/visit/executables.html"
#define HTTPS_EXECUTABLE_HTML "https://wci.llnl.gov" EXECUTABLE_HTML
#else
// We don't have SSL so we can't download from our own website. 
// Thanks a lot LLNL. Try a mirror site.
#define EXECUTABLE_HTML       "/visit/executables.html"
#define HTTPS_EXECUTABLE_HTML "http://www.visitusers.org" EXECUTABLE_HTML
#endif

// There seems to be a QProcess bug on Apple so let's try a workaround.
#ifdef __APPLE__
#define QPROCESS_FINISHED_WORKAROUND
#endif

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
//   Brad Whitlock, Thu May 25 12:03:28 PDT 2006
//   Initialized some string members.
//
//   Jeremy Meredith, Thu Aug  7 15:42:23 EDT 2008
//   Fixed initializer order to match true order.
//
//   Brad Whitlock, Thu Oct  2 10:03:03 PDT 2008
//   Made it use VisItDownloader.
//
// ****************************************************************************

QvisVisItUpdate::QvisVisItUpdate(QObject *parent) : QObject(parent), GUIBase(), 
    distName(), configName("none"), bankName("bdivp"),
    latestVersion(CURRENT_VERSION), 
    files(), downloads(), bytes()
{
    stage = STAGE_SUBMIT_RUNINFO;
    downloader = 0;
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
//   Brad Whitlock, Thu Oct  2 14:03:15 PDT 2008
//   Removed ftp.
//
// ****************************************************************************

QvisVisItUpdate::~QvisVisItUpdate()
{
}


// ****************************************************************************
// Method: QvisVisItUpdate::runInformationString
//
// Purpose: 
//   This class provides the ftp login for the VisIt FTP site.
//
// Returns: A bogus filename that contains a little run information.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:23:38 PDT 2005
//
// Modifications:
//
// ****************************************************************************

QString
QvisVisItUpdate::runInformationString() const
{
#if defined(_WIN32)
    const char *platform = "win32";
#else
    const char *platform = distName.isEmpty() ? "?" : distName.toStdString().c_str();
#endif

    // Get the number of startups.
    ConfigStateEnum code;
    int nStartups = ConfigStateGetRunCount(code);
    QString runinfo;
    runinfo.sprintf("/codes/visit/visit_update_%s_%s_%d", CURRENT_VERSION,
        platform, nStartups);

    return runinfo;
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
    return QString(GetVisItInstallationDirectory(latestVersion.toStdString().c_str()).c_str());
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
//   Brad Whitlock, Thu May 25 11:04:34 PDT 2006
//   I made it pass the bank to visit-install and I changed how the platform
//   we use is specified.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Thu Oct  2 14:10:44 PDT 2008
//   Qt 4.
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
            disconnect(installProcess, SIGNAL(readyReadStandardOutput()),
                       this, SLOT(readInstallerStdout()));
            disconnect(installProcess, SIGNAL(readyReadStandardError()),
                       this, SLOT(readInstallerStderr()));
#endif
            disconnect(installProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                       this, SLOT(emitInstallationComplete(int)));
            delete installProcess;
            installProcess = 0;
        }

        // Set the cursor to busy and tell the user that we're installing.
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        Status(tr("Installing..."), 120000);

        installProcess = new QProcess(0);
        QStringList installerArgs;
#if defined(_WIN32)
        // Install VisIt the WIN32 way by running the visit installer.
        QString installer(files.front());
#else
        // Install VisIt the UNIX way.
        QString visit_install(files.front());
        QFile::setPermissions(visit_install, QFile::ReadOwner | 
                              QFile::WriteOwner | QFile::ExeOwner);

        // Get the VisIt installation directory.
        QString installDir(getInstallationDir());

        // Create an installation process that will run visit-install.
        QString installer(visit_install);

        // Add installer arguments.
        installerArgs.append("-c");
        installerArgs.append(configName);
        installerArgs.append("-b");
        installerArgs.append(bankName);
        installerArgs.append(latestVersion);
        installerArgs.append(distName);
        installerArgs.append(installDir);
        connect(installProcess, SIGNAL(readyReadStandardOutput()),
                this, SLOT(readInstallerStdout()));
        connect(installProcess, SIGNAL(readyReadStandardError()),
                this, SLOT(readInstallerStderr()));

        debug1 << "Going to run: visit-install -c "
               << configName.toStdString()
               << " -b " << bankName.toStdString() << " "
               << latestVersion.toStdString() << " "
               << distName.toStdString() << " " 
               << installDir.toStdString() << endl;
#endif
        debug1 << "Connecting finished->emitInstallationComplete" << endl;
        // We want to know when the installer completes.
        connect(installProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(emitInstallationComplete(int)));

        // Start the visit installer.
        installProcess->start(installer, installerArgs);
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
#ifdef QPROCESS_FINISHED_WORKAROUND
    installProcess->kill();
    delete installProcess;
    installProcess = 0;
#endif

    // Delete all of the files in the files list.
    QDir dir;
    for(size_t i = 0; i < files.count(); ++i)
    {
        dir.remove(files[i]);
        debug1 << "Removed " << files[i].toStdString() << endl;
    }
    files.clear();
}

// ****************************************************************************
// Method: QvisVisItUpdate::remoteToLocalName
//
// Purpose: 
//   Converts the remote name to the local filename (strips off the end filename).
//
// Arguments:
//   remote : The name of the remote file.
//
// Returns:    The name of the local file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:53:45 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
QvisVisItUpdate::remoteToLocalName(const QString &remote) const
{
    QString localName(remote);
    int slash = remote.lastIndexOf("/");
    if(slash != -1)
        localName = localTempDirectory() + remote.right(remote.size()-slash-1); 
    return localName;
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
//   Brad Whitlock, Thu May 25 12:08:46 PDT 2006
//   I changed the method so it tries to read a new ".installinfo" file if
//   it exists. The ".installinfo" file contains information about how
//   VisIt was installed and it allows us to download the right version of
//   the VisIt distribution when we don't have enough information to tell
//   from the archdir included in the path name.
//
//   Brad Whitlock, Thu Oct  2 09:56:03 PDT 2008
//   Moved the code that reads the installation info into the 
//   ReadInstallationInfo function.
//
// ****************************************************************************

void
QvisVisItUpdate::startUpdate()
{
#if !defined(_WIN32)
    const char *mName = "QvisVisItUpdate::startUpdate: ";

    std::string s1, s2, s3;
    if(ReadInstallationInfo(s1, s2, s3))
    {
        distName   = s1.c_str();
        if(s2.size() > 0)
            configName = s2.c_str();
        if(s3.size() > 0)
            bankName = s3.c_str();

        debug1 << mName << "distName = " << distName.toStdString().c_str() << endl;
        debug1 << mName << "configName = " << configName.toStdString().c_str() << endl;
        debug1 << mName << "bankName = " << bankName.toStdString().c_str() << endl;
    }
    else
    {
        Error(tr("VisIt could not determine the platform that you are running "
                 "on so VisIt cannot automatically update. You should browse "
                 "to %1 and download the latest binary distribution for your "
                 "platform.").arg(HTTPS_EXECUTABLE_HTML));
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

        msg = tr("VisIt determined that you do not have write permission "
                 "to the %1 directory where VisIt is installed. You must have "
                 "group %2 write access to update VisIt.").
              arg(installDir).
              arg(g);
        Error(msg);
        emit updateNotAllowed();
        return;
    }
#endif

    // Create a downloader and start the process of updating by downloading
    // the executables.html page and figuring out which versions are available.
    if(downloader == 0)
    {
        downloader = new QvisDownloader(this);
        connect(downloader, SIGNAL(downloadProgress(int,int)),
                this, SLOT(reportDownloadProgress(int,int)));
    }

    // Start the process by determining the versions that are available.
    stage = STAGE_DETERMINE_VERSION;
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
//   Brad Whitlock, Thu Oct  2 09:36:42 PDT 2008
//   Simplified because we're using http now.
//
// ****************************************************************************

void
QvisVisItUpdate::initiateStage()
{
    debug1 << "initiateStage: Start stage: " << stage << endl;

    switch(stage)
    {
    case STAGE_SUBMIT_RUNINFO:
         connect(downloader, SIGNAL(done(bool)),
                 this, SLOT(doneSubmittingRunInfo(bool)));
         bytes.clear();
         debug1 << "Send run information to server: "
                << runInformationString().toStdString().c_str() << endl;
         downloader->get(runInformationString(), &bytes);
         break;
    case STAGE_DETERMINE_VERSION:
         disconnect(downloader, SIGNAL(done(bool)),
                    this, SLOT(doneSubmittingRunInfo(bool)));
         connect(downloader, SIGNAL(done(bool)),
                 this, SLOT(determineNewVersion(bool)));
         disconnect(downloader, SIGNAL(done(bool)),
                    this, SLOT(downloadDone(bool)));
         debug1 << "Getting " << EXECUTABLE_HTML << endl;
         bytes.clear();
         downloader->get(EXECUTABLE_HTML, &bytes);
         break;
    case STAGE_GET_FILES:
         disconnect(downloader, SIGNAL(done(bool)),
                    this, SLOT(determineNewVersion(bool)));
         connect(downloader, SIGNAL(done(bool)),
                 this, SLOT(downloadDone(bool)));
         getRequiredFiles();
         break;
    case STAGE_INSTALL:
         installVisIt();
         break;
    case STAGE_CLEAN_UP:
    case STAGE_ERROR:
         cleanUp();
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
// Method: QvisVisItUpdate::doneSubmittingRunInfo
//
// Purpose: 
//   Move us onto the next stage after submitting run information.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 12:07:46 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisVisItUpdate::doneSubmittingRunInfo(bool)
{
    nextStage();
}

// ****************************************************************************
// Method: QvisVisItUpdate::determineNewVersion
//
// Purpose: 
//   This Qt slot function is called when we're down downloading the
//   executables.html page from which we discern the available versions.
//
// Arguments:
//   error : True if an error occurred during download of the executables page.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:46:45 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisVisItUpdate::determineNewVersion(bool error)
{
    QWidget *p = qApp->activeWindow();

    if(!error)
    {
        // Now that we have the bytes, let's search them for "href=" and filter
        // to create a map of versions to the installation files that are present.
        int from = 0;
        int index;
        QMap<QString, QStringList> installations;
        while((index = bytes.indexOf("href=\"", from)) != -1)
        {
            int start = index + 6;
            int end = bytes.indexOf("\"", start + 6);
            QString href(bytes.mid(start, end - start));
            if((href.right(3) == ".gz" || href.right(4) == ".exe") && !href.contains("jvisit"))
            {
                int slash = href.indexOf("/");
                if(slash != -1)
                {
                    QString version = href.left(slash);
                    if(installations.find(version) == installations.end())
                    {
                        QStringList ilist;
                        ilist.append(QString("/codes/visit/") + href);
                        installations[version] = ilist;
                    }
                    else
                        installations[version].append(QString("/codes/visit/") + href);
                }
            }
            from = end + 1;
        }
        bytes.clear();

        // Now we have a map of version numbers to string lists that contain hrefs
        // that point to specific installation files. Write the files to the 
        // debug logs.
        for(QMap<QString, QStringList>::const_iterator it = installations.begin();
            it != installations.end(); ++it)
        {
            debug4 << "Version " << it.key().toStdString().c_str() << endl;
            for(int i = 0; i < it.value().size(); ++i)
                debug4 << "\t" << it.value()[i].toStdString().c_str() << endl;
        }

        // Look for the newest version that has the supported distName.
        QString maxVersion(CURRENT_VERSION), maxUsableVersion(CURRENT_VERSION);
        QString installationFile;
        for(QMap<QString, QStringList>::const_iterator it = installations.begin();
            it != installations.end(); ++it)
        {
            if(VersionGreaterThan(it.key().toStdString(), maxVersion.toStdString()))
            {
                maxVersion = it.key();
                for(int i = 0; i < it.value().size(); ++i)
                {
#ifdef WIN32
                    if(it.value()[i].contains(".exe") &&
                       !it.value()[i].contains("visitdev"))
#else
                    if(it.value()[i].contains(distName))
#endif
                    {
                        maxUsableVersion = it.key();
                        installationFile = it.value()[i];
                        break;
                    }
                }
            }
        }

        if(VersionGreaterThan(maxVersion.toStdString(), CURRENT_VERSION))
        {
            if(maxVersion != maxUsableVersion)
            {
                // There's a newer version than the max usable version, probably
                // meaning that the installed distribution type was discontinued
                QMessageBox::information(p, tr("VisIt"),
                    tr("There is a newer version of VisIt available (version %1) "
                       "but your distribution type: %2 was not found in the new "
                       "version. There is likely another distribution type that "
                       "will work for you but you will need to determine that "
                       "and install VisIt yourself. Alternatively, you can "
                       "build VisIt for your platform using the build_visit "
                       "script.").
                    arg(maxVersion).arg(distName));
                emit updateNotAllowed();
                return;
            }

            latestVersion = maxUsableVersion;
            downloads.clear();
#ifndef WIN32
            int slash = installationFile.lastIndexOf("/");
            if(slash != -1)
            {
                QString visitinstall(installationFile.left(slash+1) + "visit-install");
                downloads += visitinstall;
            }
#endif
            downloads += installationFile;

            // Print out the list of downloads.
            debug1 << "Downloads for update:" << endl;
            for(int i = 0; i < downloads.size(); ++i)
                debug1 << "\t" << downloads[i].toStdString().c_str() << endl;

            // Ask the user whether or not VisIt should be installed.
            QString msg(tr("VisIt %1 is available for download. Would you "
                           "like to install it?").arg(latestVersion));
            if(QMessageBox::information(p, "VisIt", msg,
               QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
            {
                debug1 << "User chose not to install VisIt "
                       << latestVersion.toStdString() << endl;
                emit updateNotAllowed();
            }
            else
                nextStage();
        }
        else
        {
            // we're up to date.
            QMessageBox::information(p, tr("VisIt"),
                tr("Your version of VisIt is up to date."));
            emit updateNotAllowed();
        }
    }
    else
    {
        QMessageBox::critical(p, tr("VisIt"),
            tr("VisIt was not able to check for updates."));
    }
}

// ****************************************************************************
// Method: QvisVisItUpdate::getRequiredFiles
//
// Purpose: 
//   This is a Qt slot function that initiates the download of a file from 
//   the VisIt site.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 12:36:11 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:13:55 PDT 2005
//   I made it use QvisFtp.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Thu Oct  2 09:23:31 PDT 2008
//   Rewrote for HTTP and Qt4.
//
// ****************************************************************************

void
QvisVisItUpdate::getRequiredFiles()
{
    QString remoteName(downloads.first());
    QString localName(remoteToLocalName(remoteName));
    debug1 << "Going to download " <<  remoteName.toStdString().c_str()
           << " into local file " << localName.toStdString().c_str() << endl;

    downloader->get(remoteName, localName);
}

// ****************************************************************************
// Method: QvisVisItUpdate::downloadDone
//
// Purpose: 
//   This slot function is called when the downloader has completed downloading
//   a file.
//
// Arguments:
//   error : Teue if an error occurred during the download of one of the
//           required files.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 09:26:51 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisVisItUpdate::downloadDone(bool error)
{
    QString remoteName(downloads.front());
    downloads.pop_front();

    if(error)
    {
        QMessageBox::critical(qApp->activeWindow(), tr("VisIt"),
            tr("VisIt was not able to download %1 so the update will not "
               "proceed. Try again later.").arg(remoteName));
    }
    else
    {
        files.push_back(remoteToLocalName(remoteName));
        if(downloads.count() > 0)
            QTimer::singleShot(10, this, SLOT(getRequiredFiles()));
        else
            nextStage();
    }
}

// ****************************************************************************
// Method: QvisVisItUpdate::readInstallerStdout
//
// Purpose: 
//   This Qt slot function adds the visit-install output to the output window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:52:06 PDT 2008
//
// Modifications:
//   Brad Whitlock, Thu Oct  2 14:35:15 PDT 2008
//   Qt 4.
//   
// ****************************************************************************

void
QvisVisItUpdate::readInstallerStdout()
{
    // Add the output to the output window.
    QString output(installProcess->readAllStandardOutput());
#ifdef QPROCESS_FINISHED_WORKAROUND
    Message(output);
    if(output.contains("visit-install done"))
        emitInstallationComplete(0);
#else
    Message(output);
#endif
}

// ****************************************************************************
// Method: QvisVisItUpdate::readInstallerStderr
//
// Purpose: 
//   This Qt slot function adds the visit-install output to the output window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:52:06 PDT 2008
//
// Modifications:
//   Brad Whitlock, Thu Oct  2 14:35:15 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVisItUpdate::readInstallerStderr()
{
    // Add the output to the output window.
    Message(QString(installProcess->readAllStandardError()));
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
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Oct  6 09:28:59 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVisItUpdate::emitInstallationComplete(int exitCode)
{
    debug1 << "QvisVisItUpdate::emitInstallationComplete: exit=" << exitCode
           << endl;

    // Restore the cursor since we're done installing.
    QApplication::restoreOverrideCursor();
    Status(tr("Installation complete."), 1000);

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
        QString err = tr("The new version of VisIt could not be located in %1.").
                      arg(visitDir + filename);
        Error(err);
    }
}

// ****************************************************************************
// Method: QvisVisItUpdate::reportDownloadProgress
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
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Oct  6 09:47:22 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisVisItUpdate::reportDownloadProgress(int done, int total)
{
    if(done == total)
        ClearStatus();
    else
        Status(tr("Downloaded %1 / %2 bytes.").arg(done).arg(total));
}
