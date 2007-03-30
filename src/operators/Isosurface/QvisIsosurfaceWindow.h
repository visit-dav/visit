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
//   
// ****************************************************************************

class QvisIsosurfaceWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisIsosurfaceWindow(const int type,
                         IsosurfaceAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisIsosurfaceWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
    void UpdateSelectByText();
 
    QString LevelString(int i);
    void ProcessSelectByText();
    void StringToDoubleList(const char *str, doubleVector &dv);
  private slots:
    void scaleClicked(int scale);
    void selectByChanged(int);
    void processSelectByText();
    void minToggled(bool);
    void processMinLimitText();
    void maxToggled(bool);
    void processMaxLimitText();
    void variableProcessText();
  private:
    QComboBox              *selectByComboBox;
    QLineEdit              *selectByLineEdit;
    QButtonGroup           *scalingButtons;
    QCheckBox              *minToggle;
    QLineEdit              *minLineEdit;
    QCheckBox              *maxToggle;
    QLineEdit              *maxLineEdit;
    QLineEdit              *variable;

    IsosurfaceAttributes *atts;
};



#endif
