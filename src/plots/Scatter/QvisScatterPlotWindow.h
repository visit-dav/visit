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

#ifndef QVISSCATTERPLOTWINDOW_H
#define QVISSCATTERPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class ScatterAttributes;
class QComboBox;
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
// Class: QvisScatterPlotWindow
//
// Purpose: 
//   Defines QvisScatterPlotWindow class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 1 16:16:10 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Jul 20 15:26:06 PST 2005
//   Added pointSizeLabel.
//
// ****************************************************************************

class QvisScatterPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisScatterPlotWindow(const int type,
                         ScatterAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisScatterPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
    void EnsureUniqueRole(int mask, int val, const char *var);
private slots:
    void var1RoleChanged(int val);
    void var1MinFlagChanged(bool val);
    void var1MaxFlagChanged(bool val);
    void var1MinProcessText();
    void var1MaxProcessText();
    void var1ScalingChanged(int val);
    void var1SkewFactorProcessText();
    void var2RoleChanged(int val);
    void var2Selected(const QString &var);
    void var2MinFlagChanged(bool val);
    void var2MaxFlagChanged(bool val);
    void var2MinProcessText();
    void var2MaxProcessText();
    void var2ScalingChanged(int val);
    void var2SkewFactorProcessText();
    void var3RoleChanged(int val);
    void var3Selected(const QString &var);
    void var3MinFlagChanged(bool val);
    void var3MaxFlagChanged(bool val);
    void var3MinProcessText();
    void var3MaxProcessText();
    void var3ScalingChanged(int val);
    void var3SkewFactorProcessText();
    void var4RoleChanged(int val);
    void var4Selected(const QString &var);
    void var4MinFlagChanged(bool val);
    void var4MaxFlagChanged(bool val);
    void var4MinProcessText();
    void var4MaxProcessText();
    void var4ScalingChanged(int val);
    void var4SkewFactorProcessText();
    void pointSizeProcessText();
    void pointTypeChanged(int val);
    void scaleCubeChanged(bool val);
    void colorTableNameChanged(bool useDefault, const QString &ctName);
    void singleColorChanged(const QColor &color);
    void foregroundFlagChanged(bool val);
    void legendFlagChanged(bool val);
private:
    int plotType;
    static const char *roleNames[5];

    QComboBox *var1Role;
    QCheckBox *var1MinFlag;
    QCheckBox *var1MaxFlag;
    QLineEdit *var1Min;
    QLineEdit *var1Max;
    QButtonGroup *var1Scaling;
    QLineEdit *var1SkewFactor;

    QComboBox *var2Role;
    QvisVariableButton *var2;
    QCheckBox *var2MinFlag;
    QCheckBox *var2MaxFlag;
    QLineEdit *var2Min;
    QLineEdit *var2Max;
    QButtonGroup *var2Scaling;
    QLineEdit *var2SkewFactor;

    QComboBox *var3Role;
    QvisVariableButton *var3;
    QCheckBox *var3MinFlag;
    QCheckBox *var3MaxFlag;
    QLineEdit *var3Min;
    QLineEdit *var3Max;
    QButtonGroup *var3Scaling;
    QLineEdit *var3SkewFactor;

    QComboBox *var4Role;
    QvisVariableButton *var4;
    QCheckBox *var4MinFlag;
    QCheckBox *var4MaxFlag;
    QLineEdit *var4Min;
    QLineEdit *var4Max;
    QButtonGroup *var4Scaling;
    QLineEdit *var4SkewFactor;

    QLabel    *xCoordRoleLabel;
    QLabel    *yCoordRoleLabel;
    QLabel    *zCoordRoleLabel;
    QLabel    *colorRoleLabel;

    QLineEdit *pointSize;
    QLabel    *pointSizeLabel;
    QComboBox *pointType;
    QCheckBox *scaleCube;
    QvisColorTableButton *colorTableName;
    QLabel *singleColorLabel;
    QvisColorButton *singleColor;
    QCheckBox *foregroundFlag;
    QCheckBox *legendFlag;

    QLabel *var1ScalingLabel;
    QLabel *var1SkewFactorLabel;

    QLabel *var2Label;
    QLabel *var2ScalingLabel;
    QLabel *var2SkewFactorLabel;

    QLabel *var3Label;
    QLabel *var3ScalingLabel;
    QLabel *var3SkewFactorLabel;

    QLabel *var4Label;
    QLabel *var4ScalingLabel;
    QLabel *var4SkewFactorLabel;

    ScatterAttributes *atts;
};



#endif
