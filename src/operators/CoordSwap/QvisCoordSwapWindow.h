#ifndef QVISCOORDSWAPWINDOW_H
#define QVISCOORDSWAPWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class CoordSwapAttributes;
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
// Class: QvisCoordSwapWindow
//
// Purpose: 
//   Defines QvisCoordSwapWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Wed Feb 2 16:09:10 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisCoordSwapWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisCoordSwapWindow(const int type,
                         CoordSwapAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisCoordSwapWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void newCoord1Changed(int val);
    void newCoord2Changed(int val);
    void newCoord3Changed(int val);
  private:
    QButtonGroup *newCoord1;
    QButtonGroup *newCoord2;
    QButtonGroup *newCoord3;
    QLabel *newCoord1Label;
    QLabel *newCoord2Label;
    QLabel *newCoord3Label;

    CoordSwapAttributes *atts;
};



#endif
