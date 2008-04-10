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

#ifndef QVISINTERACTORWINDOW_H
#define QVISINTERACTORWINDOW_H

#include <AttributeSubject.h>
#include <QvisPostableWindowObserver.h>

class InteractorAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisInteractorWindow
//
// Purpose: 
//   Defines QvisInteractorWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Aug 16 15:29:28 PST 2004
//
// Modifications:
//   Eric Brugger, Thu Nov 11 11:47:28 PST 2004
//   I added the navigation mode toggle buttons.
//   
//   Eric Brugger, Thu Nov 18 13:01:35 PST 2004
//   I added the fill viewport on zoom toggle button.
//   
//   Jeremy Meredith, Thu Feb  7 17:51:32 EST 2008
//   Added snap-to-horizontal grid support for axis array mode navigation.
//
//   Brad Whitlock, Wed Apr  9 11:31:21 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

class QvisInteractorWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisInteractorWindow(
                         InteractorAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisInteractorWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
  private slots:
    void showGuidelinesChanged(bool val);
    void clampSquareChanged(bool val);
    void fillViewportOnZoomChanged(bool val);
    void navigationModeChanged(int val);
    void axisSnapChanged(bool val);
  private:
    QCheckBox *showGuidelines;
    QCheckBox *clampSquare;
    QCheckBox *fillViewportOnZoom;
    QButtonGroup *navigationMode;
    QCheckBox *axisSnap;

    InteractorAttributes *atts;
};



#endif
