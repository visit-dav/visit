// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISMOLECULEPLOTWINDOW_H
#define QVISMOLECULEPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class MoleculeAttributes;
class QLabel;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QButtonGroup;
class QvisColorTableButton;
class QvisColorButton;
class QvisLineWidthWidget;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisMoleculePlotWindow
//
// Purpose: 
//   Defines QvisMoleculePlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: Jeremy Meredith
// Creation:   March 23, 2006
//
// Modifications:
//   Cyrus Harrison, Fri Jul 18 14:38:14 PDT 2008
//   Initial Qt4 Port.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
// ****************************************************************************

class QvisMoleculePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisMoleculePlotWindow(const int type,
                         MoleculeAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisMoleculePlotWindow();
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
    void drawAtomsAsChanged(int val);
    void scaleRadiusByChanged(int val);
    void drawBondsAsChanged(int val);
    void colorBondsChanged(int val);
    void bondSingleColorChanged(const QColor &color);
    void radiusVariableChanged(const QString &varName);
    void radiusScaleFactorProcessText();
    void radiusFixedProcessText();
    void atomSphereQualityChanged(int val);
    void bondCylinderQualityChanged(int val);
    void bondRadiusProcessText();
    void bondLineWidthChanged(int style);
    void elementColorTableChanged(bool useDefault, const QString &ctName);
    void residueTypeColorTableChanged(bool useDefault, const QString &ctName);
    void residueSequenceColorTableChanged(bool useDefault, const QString &ctName);
    void continuousColorTableChanged(bool useDefault, const QString &ctName);
    void legendToggled(bool val);
    void minFlagChanged(bool val);
    void scalarMinProcessText();
    void maxFlagChanged(bool val);
    void scalarMaxProcessText();
  private:
    int                   plotType;
    QComboBox            *drawAtomsAs;
    QComboBox            *scaleRadiusBy;
    QComboBox            *drawBondsAs;
    QButtonGroup         *colorBondsGroup;
    QWidget              *colorBondsWidget;
    QvisColorButton      *bondSingleColor;
    QvisVariableButton   *radiusVariable;
    QLineEdit            *radiusScaleFactor;
    QLineEdit            *radiusFixed;
    QComboBox            *atomSphereQuality;
    QComboBox            *bondCylinderQuality;
    QLineEdit            *bondRadius;
    QvisLineWidthWidget  *bondLineWidth;
    QvisColorTableButton *elementColorTable;
    QvisColorTableButton *residueTypeColorTable;
    QvisColorTableButton *residueSequenceColorTable;
    QvisColorTableButton *continuousColorTable;
    QCheckBox            *legendToggle;
    QCheckBox            *minFlag;
    QLineEdit            *scalarMin;
    QCheckBox            *maxFlag;
    QLineEdit            *scalarMax;
    QLabel               *drawAtomsAsLabel;
    QLabel               *scaleRadiusByLabel;
    QLabel               *drawBondsAsLabel;
    QLabel               *colorBondsLabel;
    QLabel               *radiusVariableLabel;
    QLabel               *radiusScaleFactorLabel;
    QLabel               *radiusFixedLabel;
    QLabel               *atomSphereQualityLabel;
    QLabel               *bondCylinderQualityLabel;
    QLabel               *bondRadiusLabel;
    QLabel               *bondLineWidthLabel;
    QLabel               *elementColorTableLabel;
    QLabel               *residueTypeColorTableLabel;
    QLabel               *residueSequenceColorTableLabel;
    QLabel               *continuousColorTableLabel;
    QLabel               *minFlagLabel;
    QLabel               *maxFlagLabel;

    MoleculeAttributes   *atts;
};



#endif
