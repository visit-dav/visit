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

#ifndef QVIS_POSTABLE_WINDOW
#define QVIS_POSTABLE_WINDOW
#include <gui_exports.h>
#include <QString>
#include <QvisWindowBase.h>

class DataNode;
class QvisNotepadArea;
class QPushButton;
class QVBoxLayout;
class QVBoxLayout;

// ****************************************************************************
// Class: QvisPostableWindow
//
// Purpose:
//   This is a base class for postable windows.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:28:28 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 13:40:41 PST 2000
//   Made it inherit from QvisWindowBase.
//
//   Brad Whitlock, Wed May 2 11:09:42 PDT 2001
//   Removed interpreter member.
//
//   Brad Whitlock, Tue Sep 25 12:28:53 PDT 2001
//   Made post() and unpost() virtual.
//
//   Brad Whitlock, Fri Feb 15 11:16:41 PDT 2002
//   Made CreateEntireWindow public.
//
//   Brad Whitlock, Fri Sep 5 15:50:16 PST 2003
//   Added postWhenShown static member.
//
//   Brad Whitlock, Mon Nov 14 10:42:26 PDT 2005
//   Added postEnabled static member.
//
//   Brad Whitlock, Wed Apr  9 10:50:05 PDT 2008
//   QString for caption and shortName.
//
//   Brad Whitlock, Fri May 30 09:59:10 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisPostableWindow : public QvisWindowBase
{
    Q_OBJECT
public:
    QvisPostableWindow(const QString &captionString = QString::null,
                       const QString &shortName = QString::null,
                       QvisNotepadArea *n = 0);
    virtual ~QvisPostableWindow();
    virtual void CreateWindowContents() = 0;
    QWidget *GetCentralWidget();
    const QString &GetShortCaption();
    bool posted();
    virtual void CreateEntireWindow();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);

    static void  SetPostEnabled(bool);
protected:
    virtual void UpdateWindow(bool doAll);
public slots:
    virtual void raise();
    virtual void show();
    virtual void hide();
protected slots:
    virtual void post();
    virtual void unpost();
protected:
    bool               isCreated;
    bool               isPosted;
    bool               addLayoutStretch;
    QString            shortCaption;
    QWidget            *central;
    QVBoxLayout        *topLayout;
    QPushButton        *postButton;
    QPushButton        *dismissButton;
    QvisNotepadArea    *notepad;

    static bool         postWhenShown;
    static bool         postEnabled;
};

#endif
