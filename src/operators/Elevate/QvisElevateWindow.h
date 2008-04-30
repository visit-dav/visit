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

#ifndef QVISELEVATEWINDOW_H
#define QVISELEVATEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ElevateAttributes;
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
class QvisVariableButton;

// ****************************************************************************
// Class: QvisElevateWindow
//
// Purpose: 
//   Defines QvisElevateWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Aug 14 12:09:26 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class QvisElevateWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisElevateWindow(const int type,
                         ElevateAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisElevateWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void useXYLimitsChanged(bool val);
    void limitsModeChanged(int val);
    void scalingChanged(int val);
    void skewFactorProcessText();
    void minFlagChanged(bool val);
    void minProcessText();
    void maxFlagChanged(bool val);
    void maxProcessText();
    void zeroFlagChanged(bool val);
    void variableChanged(const QString &varName);
  private:
    QCheckBox *useXYLimits;
    QButtonGroup *limitsMode;
    QButtonGroup *scaling;
    QLineEdit *skewFactor;
    QCheckBox *minFlag;
    QLineEdit *min;
    QCheckBox *maxFlag;
    QLineEdit *max;
    QCheckBox *zeroFlag;
    QvisVariableButton *variable;
    QLabel *useXYLimitsLabel;
    QLabel *limitsModeLabel;
    QLabel *scalingLabel;
    QLabel *skewFactorLabel;
    QLabel *minFlagLabel;
    QLabel *minLabel;
    QLabel *maxFlagLabel;
    QLabel *maxLabel;
    QLabel *zeroFlagLabel;
    QLabel *variableLabel;

    ElevateAttributes *atts;
};



#endif
