#ifndef QVIS_POINT_CONTROLS_H
#define QVIS_POINT_CONTROLS_H
#include <gui_exports.h>
#include <qwidget.h>

class QCheckBox;
class QLabel;
class QLineEdit;
class QButtonGroup;
class QvisVariableButton;

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
// ****************************************************************************

class GUI_API QvisPointControl : public QWidget
{
    Q_OBJECT

public:
    QvisPointControl(QWidget * parent=0, const char * name=0);
    ~QvisPointControl();

    void SetPointSize(double);
    void SetPointSizeVarChecked(bool);
    void SetPointSizeVar(QString &);
    void SetPointType(int);

    double   GetPointSize();
    bool     GetPointSizeVarChecked() const;
    QString  &GetPointSizeVar();
    int      GetPointType() const;

signals:
    void pointSizeChanged(double);
    void pointSizeVarToggled(bool val);
    void pointSizeVarChanged(const QString &);
    void pointTypeChanged(int);

private slots:
    void processSizeText();
    void sizeVarChanged(const QString &);
    void sizeVarToggled(bool on);
    void typeButtonChanged(int);

private:
    QLabel                 *sizeLabel;
    QLineEdit              *sizeLineEdit;
    QCheckBox              *sizeVarToggle;
    QvisVariableButton     *sizeVarButton;
    QButtonGroup           *typeButtons;

    double                  lastGoodSize;
    QString                 lastGoodVar;
};

#endif
