/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#ifndef QVISLABELPLOTWINDOW_H
#define QVISLABELPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class LabelAttributes;
class QLabel;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QvisColorButton;

// ****************************************************************************
// Class: QvisLabelPlotWindow
//
// Purpose: 
//   Defines QvisLabelPlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 21 18:18:16 PST 2004
//
// Modifications:
//   Brad Whitlock, Tue Aug 2 14:55:40 PST 2005
//   I removed some controls then I added other controls for setting the
//   colors and heights for node labels and cell labels independently, also
//   for z-buffering.
//
// ****************************************************************************

class QvisLabelPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisLabelPlotWindow(const int type,
                         LabelAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisLabelPlotWindow();
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
    void showNodesToggled(bool val);
    void showCellsToggled(bool val);
    void restrictNumberOfLabelsToggled(bool val);
    void numberOfLabelsChanged(int val);
    void drawLabelsFacingChanged(int val);
    void depthTestButtonGroupChanged(int);
    void labelDisplayFormatChanged(int val);

    void specifyTextColor1Toggled(bool val);
    void textColor1Changed(const QColor &color);
    void textHeight1Changed(int val);
    void specifyTextColor2Toggled(bool val);
    void textColor2Changed(const QColor &color);
    void textHeight2Changed(int val);

    void horizontalJustificationChanged(int val);
    void verticalJustificationChanged(int val);

    void legendToggled(bool val);
private:
    int              plotType;
    LabelAttributes *labelAtts;

    QGroupBox       *selectionGroupBox;
    QCheckBox       *showNodesToggle;
    QCheckBox       *showCellsToggle;
    QCheckBox       *restrictNumberOfLabelsToggle;
    QSpinBox        *numberOfLabelsSpinBox;
    QComboBox       *drawLabelsFacingComboBox;
    QButtonGroup    *depthTestButtonGroup;

    QGroupBox       *formattingGroupBox;
    QComboBox       *labelDisplayFormatComboBox;
    QvisColorButton *textColor1Button;
    QCheckBox       *specifyTextColor1Toggle;
    QvisColorButton *textColor2Button;
    QCheckBox       *specifyTextColor2Toggle;
    QLabel          *textHeight1Label;
    QSpinBox        *textHeight1SpinBox;
    QLabel          *textHeight2Label;
    QSpinBox        *textHeight2SpinBox;
    QComboBox       *horizontalJustificationComboBox;
    QComboBox       *verticalJustificationComboBox;

    QCheckBox       *legendToggle;
};

#endif
