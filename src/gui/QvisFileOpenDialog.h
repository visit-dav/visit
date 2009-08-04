#ifndef QVIS_FILE_OPEN_DIALOG_H
#define QVIS_FILE_OPEN_DIALOG_H
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
#include <QvisFileOpenWindow.h>

// ****************************************************************************
// Class: QvisFileOpenDialog
//
// Purpose:
//   This is a dialog version of the file open window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 15 13:28:12 PST 2006
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 10:40:07 PDT 2008
//   Made captions use QString.
//
// ****************************************************************************

class QvisFileOpenDialog : public QvisFileOpenWindow
{
    Q_OBJECT
public:
    static const int Accepted;
    static const int Rejected;

    QvisFileOpenDialog(const QString &caption);
    virtual ~QvisFileOpenDialog();

    // Blocking function to use the dialog to get a filename.
    static QString getOpenFileName(const QString &initialFile, 
                                   const QString &caption);

    int exec();
    void setFilename(const QString &);
    QString getFilename() const;
signals:
    void quitloop();
private slots:
    void accept();
    void reject();
    void changeThePath();
    void userSelectedFile(const QString &s);
private:
    void setResult(int);
    int result() const;
    void delayedChangePath(const QString &initialFile);
    void restoreHostPathFilter(const QString &host,
                               const QString &path, const QString &filter);
    QString getOpenFileNameEx(const QString &initialFile);
    void done(int r);

    QString filename;
    bool    in_loop;
    int     _result;
};

#endif
