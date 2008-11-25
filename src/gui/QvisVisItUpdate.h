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

#ifndef QVIS_VISIT_UPDATE_H
#define QVIS_VISIT_UPDATE_H
#include <GUIBase.h>
#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <QProcess>

class QUrlInfo;
class QvisDownloader;

// ****************************************************************************
// Class: QvisVisItUpdate
//
// Purpose:
//   This class knows how to connect to the VisIt Web site and download new
//   versions of VisIt.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 9 17:34:59 PST 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:08:25 PDT 2005
//   I made it use QvisFtp instead of qftp.
//
//   Brad Whitlock, Thu May 25 12:12:56 PDT 2006
//   I added some new members.
//
//   Hank Childs, Tue Sep 26 14:10:20 PDT 2006
//   Added keyword `public' for inheritance of GUIBase.  This addresses
//   compiler warning on xlC.
//
//   Brad Whitlock, Thu Oct  2 10:45:14 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisVisItUpdate : public QObject, public GUIBase
{
    Q_OBJECT
public:
    QvisVisItUpdate(QObject *parent = 0);
    virtual ~QvisVisItUpdate();

public slots:
    void startUpdate();
signals:
    void updateNotAllowed();
    void installationComplete(const QString &);
private slots:
    void initiateStage();
    void doneSubmittingRunInfo(bool);
    void determineNewVersion(bool error);
    void downloadDone(bool error);
    void getRequiredFiles();
    void reportDownloadProgress(int done, int total);
    void readInstallerStdout();
    void readInstallerStderr();
    void emitInstallationComplete(int);
private:
    QString runInformationString() const;
    void    nextStage();
    QString localTempDirectory() const;
    QString getInstallationDir() const;
    void    installVisIt();
    void    cleanUp();
    QString remoteToLocalName(const QString &remote) const;

    QvisDownloader *downloader;
    QProcess       *installProcess;
    int             stage;
    QString         distName;
    QString         configName;
    QString         bankName;
    QString         latestVersion;
    QStringList     files;
    QStringList     downloads;
    QByteArray      bytes;
};

#endif
