#ifndef QVISMOLECULEPLOTWINDOW_H
#define QVISMOLECULEPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class MoleculeAttributes;
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
class QComboBox;

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
//   
// ****************************************************************************

class QvisMoleculePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisMoleculePlotWindow(const int type,
                         MoleculeAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
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
    void bondLineStyleChanged(int style);
    void elementColorTableChanged(bool useDefault, const QString &ctName);
    void residueTypeColorTableChanged(bool useDefault, const QString &ctName);
    void residueSequenceColorTableChanged(bool useDefault, const QString &ctName);
    void continuousColorTableChanged(bool useDefault, const QString &ctName);
    void legendFlagChanged(bool val);
    void minFlagChanged(bool val);
    void scalarMinProcessText();
    void maxFlagChanged(bool val);
    void scalarMaxProcessText();
  private:
    int plotType;
    QComboBox *drawAtomsAs;
    QComboBox *scaleRadiusBy;
    QComboBox *drawBondsAs;
    QButtonGroup *colorBonds;
    QvisColorButton *bondSingleColor;
    QvisVariableButton *radiusVariable;
    QLineEdit *radiusScaleFactor;
    QLineEdit *radiusFixed;
    QComboBox *atomSphereQuality;
    QComboBox *bondCylinderQuality;
    QLineEdit *bondRadius;
    QvisLineWidthWidget *bondLineWidth;
    QvisLineStyleWidget *bondLineStyle;
    QvisColorTableButton *elementColorTable;
    QvisColorTableButton *residueTypeColorTable;
    QvisColorTableButton *residueSequenceColorTable;
    QvisColorTableButton *continuousColorTable;
    QCheckBox *legendFlag;
    QCheckBox *minFlag;
    QLineEdit *scalarMin;
    QCheckBox *maxFlag;
    QLineEdit *scalarMax;
    QLabel *drawAtomsAsLabel;
    QLabel *scaleRadiusByLabel;
    QLabel *drawBondsAsLabel;
    QLabel *colorBondsLabel;
    QLabel *radiusVariableLabel;
    QLabel *radiusScaleFactorLabel;
    QLabel *radiusFixedLabel;
    QLabel *atomSphereQualityLabel;
    QLabel *bondCylinderQualityLabel;
    QLabel *bondRadiusLabel;
    QLabel *bondLineWidthLabel;
    QLabel *bondLineStyleLabel;
    QLabel *elementColorTableLabel;
    QLabel *residueTypeColorTableLabel;
    QLabel *residueSequenceColorTableLabel;
    QLabel *continuousColorTableLabel;
    QLabel *legendFlagLabel;
    QLabel *minFlagLabel;
    QLabel *maxFlagLabel;

    MoleculeAttributes *atts;
};



#endif
