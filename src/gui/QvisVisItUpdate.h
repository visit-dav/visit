/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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
#include <qobject.h>
#include <qstringlist.h>
#include <QvisFtp.h>

class QProcess;
class QUrlInfo;

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
// ****************************************************************************

class QvisVisItUpdate : public QObject, GUIBase
{
    Q_OBJECT
public:
    QvisVisItUpdate(QObject *parent = 0, const char *name = 0);
    virtual ~QvisVisItUpdate();

public slots:
    void startUpdate();
signals:
    void updateNotAllowed();
    void installationComplete(const QString &);
private slots:
    void initiateStage();
    void initiateDownload();
    void ftp_commandStarted();
    void ftp_commandFinished();
    void ftp_done(bool);
    void ftp_stateChanged(int);
    void ftp_listInfo(const QUrlInfo &);
    void ftp_reportDownloadProgress(int,int);

    void readInstallerStdout();
    void readInstallerStderr();
    void emitInstallationComplete();
private:
    void    provideLogin();
    void    nextStage();
    QString latestDirectory() const;
    QString localTempDirectory() const;
    QString getInstallationDir() const;
    void    getRequiredFiles();
    void    installVisIt();
    void    cleanUp();

    QvisFtp     *ftp;
    QProcess    *installProcess;
    int          stage;
    QString      distName;
    QString      configName;
    QString      bankName;
    QString      latestVersion;
    QStringList  files;
    QStringList  downloads;
};

#endif
