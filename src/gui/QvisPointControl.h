// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_POINT_CONTROLS_H
#define QVIS_POINT_CONTROLS_H
#include <gui_exports.h>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QNarrowLineEdit;
class QvisVariableButton;
class PointGlyphAttributes;

// ****************************************************************************
// Class: QvisPointControl
//
// Purpose:
//   This is a widget that encapsulates the individual point control
//   buttons into a Point controls widget.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:01:35 PST 2004
//   I replaced the line edit for the variable with a variable button.
//
//   Brad Whitlock, Wed Jul 20 13:44:39 PST 2005
//   Added a control for setting the point size in terms of pixels for
//   when we render the points as points.
//
//   Brad Whitlock, Thu Aug 25 09:29:56 PDT 2005
//   I changed the point size from a button group to a combo box.
//
//   Cyrus Harrison, Tue Jul  8 09:58:45 PDT 2008
//   Initial Qt4 Port
//
// ****************************************************************************

class GUI_API QvisPointControl : public QWidget
{
    Q_OBJECT

public:
    QvisPointControl(QWidget * parent=0,
                     bool enableScaleByVar=true);
    ~QvisPointControl();

    void SetPointAtts(PointGlyphAttributes *p);


    void SetAutoSizeChecked(bool);
    void SetPointSize(double);
    void SetPointSizePixels(int);
    void SetPointSizeVarChecked(bool);
    void SetPointSizeVar(QString &);
    void SetPointType(int);

    bool     GetAutoSizeEnabled() const;
    double   GetPointSize();
    int      GetPointSizePixels();
    bool     GetPointSizeVarChecked() const;
    QString  &GetPointSizeVar();
    int      GetPointType() const;

signals:
    void autoSizeToggled(bool);
    void pointSizeChanged(double);
    void pointSizePixelsChanged(int);
    void pointSizeVarToggled(bool val);
    void pointSizeVarChanged(const QString &);
    void pointTypeChanged(int);

private slots:
    void autoSizeChanged(bool);
    void processSizeText();
    void sizeVarChanged(const QString &);
    void sizeVarToggled(bool on);
    void typeComboBoxChanged(int);

private:
    void UpdateSizeText();
    void UpdatePointType();
    bool ProcessSizeText(int pointType);

    QCheckBox              *autoSizeToggle;
    QLabel                 *sizeLabel;
    QNarrowLineEdit        *sizeLineEdit;
    QCheckBox              *sizeVarToggle;
    QvisVariableButton     *sizeVarButton;
    QComboBox              *typeComboBox;

    int                     lastGoodPointType;
    double                  lastGoodSize;
    int                     lastGoodSizePixels;
    QString                 lastGoodVar;
};

#endif
