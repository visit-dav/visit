#ifndef QVISINVERSEGHOSTZONEWINDOW_H
#define QVISINVERSEGHOSTZONEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class InverseGhostZoneAttributes;
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
// Class: QvisInverseGhostZoneWindow
//
// Purpose: 
//   Defines QvisInverseGhostZoneWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Thu Jan 8 09:57:12 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class QvisInverseGhostZoneWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisInverseGhostZoneWindow(const int type,
                         InverseGhostZoneAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisInverseGhostZoneWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void showTypeChanged(int val);
  private:
    QButtonGroup *showType;
    QLabel *showTypeLabel;

    InverseGhostZoneAttributes *atts;
};



#endif
