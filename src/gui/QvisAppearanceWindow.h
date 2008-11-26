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

#ifndef QVISAPPEARANCEWINDOW_H
#define QVISAPPEARANCEWINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations.
class AppearanceAttributes;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QvisColorButton;
class QvisDialogLineEdit;

// ****************************************************************************
// Class: QvisAppearancePlotWindow
//
// Purpose: 
//   This class creates a window that has controls for changing GUI color,
//   style, and font.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:23:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 29 13:15:07 PST 2002
//   Added orientation combo box.
//
//   Brad Whitlock, Fri Oct 3 10:06:22 PDT 2003
//   Added an internal method.
//
//   Brad Whitlock, Thu Mar 15 15:24:58 PST 2007
//   Added font controls.
//
//   Brad Whitlock, Wed Apr  9 11:10:05 PDT 2008
//   QString for caption, shortName.
//
//   Cyrus Harrison, Mon Nov 24 11:57:42 PST 2008
//   Support for default system appearance.
//
// ****************************************************************************

class GUI_API QvisAppearanceWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisAppearanceWindow(AppearanceAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisAppearanceWindow();
    virtual void CreateWindowContents();
signals:
    void changeAppearance(bool);
public slots:
    virtual void apply();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
    bool ColorsNotTooClose(const QColor &c0, const char *c1str);
    void GetCurrentValues(int);
    void UpdateWindowSensitivity();

private slots:
    void useSysDefaultChanged(bool val);
    void backgroundChanged(const QColor &bg);
    void foregroundChanged(const QColor &fg);
    void styleChanged(int index);
    void fontNameChanged(const QString &);
    void orientationChanged(int index);
private:
    QCheckBox          *useSysDefaultCheckBox;
    QvisColorButton    *backgroundColorButton;
    QLabel             *backgroundColorLabel;
    QvisColorButton    *foregroundColorButton;
    QLabel             *foregroundColorLabel;
    QComboBox          *styleComboBox;
    QLabel             *styleLabel;
    QComboBox          *orientationComboBox;
    QLabel             *orientationLabel;
    QvisDialogLineEdit *fontName;
    QLabel             *fontLabel;
};

#endif
