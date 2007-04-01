#ifndef QVISPROJECTWINDOW_H
#define QVISPROJECTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ProjectAttributes;
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
// Class: QvisProjectWindow
//
// Purpose: 
//   Defines QvisProjectWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue May 18 14:35:37 PST 2004
//
// Modifications:
//   
// ****************************************************************************

class QvisProjectWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisProjectWindow(const int type,
                         ProjectAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisProjectWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void projectionTypeChanged(int val);
  private:
    QButtonGroup *projectionType;
    QLabel *projectionTypeLabel;

    ProjectAttributes *atts;
};



#endif
