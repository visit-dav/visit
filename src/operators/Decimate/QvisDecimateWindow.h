#ifndef QVISDECIMATEWINDOW_H
#define QVISDECIMATEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class DecimateAttributes;
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

// ****************************************************************************
// Class: QvisDecimateWindow
//
// Purpose: 
//   Defines QvisDecimateWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Sun Aug 11 08:39:31 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisDecimateWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisDecimateWindow(const int type,
                         DecimateAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisDecimateWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void targetProcessText();
  private:
    QLineEdit *target;

    DecimateAttributes *atts;
};



#endif
