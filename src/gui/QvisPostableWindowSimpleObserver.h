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

#ifndef QVIS_POSTABLE_WINDOW_SIMPLEOBSERVER_H
#define QVIS_POSTABLE_WINDOW_SIMPLEOBSERVER_H
#include <gui_exports.h>
#include <QvisPostableWindow.h>
#include <SimpleObserver.h>

// ****************************************************************************
// Class: QvisPostableWindowSimpleObserver
//
// Purpose:
//   This is the base class for a postable window that observes multiple
//   subjects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 11:46:00 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 11:26:46 PDT 2002
//   Made CreateEntireWindow public.
//
//   Jeremy Meredith, Wed May  8 10:47:14 PDT 2002
//   Added stretch.
//
//   Brad Whitlock, Mon Sep 9 10:58:32 PDT 2002
//   I made the apply button optional.
//
//   Brad Whitlock, Fri Nov 7 16:12:55 PST 2003
//   I added much of QvisPostableWindowObserver's extra functionality.
//
//   Brad Whitlock, Wed Apr  9 10:49:01 PDT 2008
//   QString for caption and shortName.
//
//   Jeremy Meredith, Fri Jan  2 17:05:57 EST 2009
//   Added Load/Save button support.
//
// ****************************************************************************

class GUI_API QvisPostableWindowSimpleObserver : public QvisPostableWindow, public SimpleObserver
{
    Q_OBJECT
public:
    static const int NoExtraButtons;
    static const int ApplyButton;
    static const int MakeDefaultButton;
    static const int ResetButton;
    static const int SaveButton;
    static const int LoadButton;
    static const int AllExtraButtons;
    static const int AllExtraButtonsAndLoadSave;

    QvisPostableWindowSimpleObserver(const QString &caption = QString::null,
                                     const QString &shortName = QString::null,
                                     QvisNotepadArea *n = 0,
                                     int buttonCombo = AllExtraButtons,
                                     bool stretch = true);
    virtual ~QvisPostableWindowSimpleObserver();

    virtual void Update(Subject *TheChangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);

    virtual void CreateEntireWindow();

public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
    virtual void loadSubject();
    virtual void saveSubject();
protected slots:
    void makeDefaultHelper();
protected:
    virtual void CreateWindowContents() = 0;
    Subject     *SelectedSubject();

    Subject *selectedSubject;
    int      buttonCombination;
    bool     stretchWindow;
};

#endif
