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

#ifndef QVISISOSURFACEWINDOW_H
#define QVISISOSURFACEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class IsosurfaceAttributes;
class QLabel;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisIsosurfaceWindow
//
// Purpose: 
//   Defines QvisIsosurfaceWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Apr 16 17:41:29 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:44:32 PST 2004
//   I made the window use a variable button.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Moved StringToDoubleList to QvisWindowBase 
// ****************************************************************************

class QvisIsosurfaceWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisIsosurfaceWindow(const int type,
                         IsosurfaceAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisIsosurfaceWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
    void UpdateSelectByText();
 
    QString LevelString(int i);
    void ProcessSelectByText();
  private slots:
    void scaleClicked(int scale);
    void selectByChanged(int);
    void processSelectByText();
    void minToggled(bool);
    void processMinLimitText();
    void maxToggled(bool);
    void processMaxLimitText();
    void variableChanged(const QString &);
  private:
    QComboBox              *selectByComboBox;
    QLineEdit              *selectByLineEdit;
    QButtonGroup           *scalingButtons;
    QCheckBox              *minToggle;
    QLineEdit              *minLineEdit;
    QCheckBox              *maxToggle;
    QLineEdit              *maxLineEdit;
    QvisVariableButton     *variable;

    IsosurfaceAttributes *atts;
};



#endif
