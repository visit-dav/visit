// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISSCATTERPLOTWINDOW_H
#define QVISSCATTERPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class ScatterAttributes;
class QComboBox;
class QLabel;
class QCheckBox;
class QLineEdit;
class QButtonGroup;
class QRadioButton;
class QvisColorTableWidget;
class QvisColorButton;
class QvisPointControl;
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
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Cyrus Harrison, Thu Aug 19 13:19:11 PDT 2010
//   Added var1 button & slot to capture var1 changes.
//
//   Kathleen Bonnell, Mon Jan 17 18:10:28 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
//   Kathleen Biagas, Tue Sep 20 16:30:47 PDT 2016
//   Use QvisPointControl, for consistency.
//
// ****************************************************************************

class QvisScatterPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisScatterPlotWindow(const int type,
                         ScatterAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
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
    void var1Selected(const QString &var);
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

    void pointSizeChanged(double);
    void pointSizePixelsChanged(int);
    void pointTypeChanged(int val);

    void scaleCubeChanged(bool val);
    void colorModeChanged(int index);
    void colorTableNameChanged(bool useDefault, const QString &ctName);
    void invertColorTableToggled(bool val);
    void singleColorChanged(const QColor &color);
    void legendToggled(bool val);
private:
    int plotType;
    bool haveColorRole;
    static const char *roleNames[5];

    QComboBox *var1Role;
    QvisVariableButton *var1;
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

    QvisPointControl *pointControl;
    QCheckBox *scaleCube;
    QButtonGroup *colorModeButtons;
    QRadioButton *colorTableRadioButton;
    QvisColorButton *singleColor;
    QvisColorTableWidget *colorTableWidget;
    QCheckBox *legendToggle;

    QLabel *var1ScalingLabel;

    QLabel *var2Label;
    QLabel *var2ScalingLabel;

    QLabel *var3Label;
    QLabel *var3ScalingLabel;

    QLabel *var4Label;
    QLabel *var4ScalingLabel;

    ScatterAttributes *atts;
};



#endif
