#ifndef QVISEXTERNALSURFACEWINDOW_H
#define QVISEXTERNALSURFACEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ExternalSurfaceAttributes;
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
// Class: QvisExternalSurfaceWindow
//
// Purpose: 
//   Defines QvisExternalSurfaceWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Sat Aug 3 12:12:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisExternalSurfaceWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisExternalSurfaceWindow(const int type,
                         ExternalSurfaceAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisExternalSurfaceWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void dummyChanged(bool val);
  private:
    QCheckBox *dummy;

    ExternalSurfaceAttributes *atts;
};



#endif
