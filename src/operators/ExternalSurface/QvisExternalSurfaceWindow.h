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
class QvisVariableButton;

// ****************************************************************************
// Class: QvisExternalSurfaceWindow
//
// Purpose: 
//   Defines QvisExternalSurfaceWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Sep 26 09:31:48 PDT 2005
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
    void removeGhostsChanged(bool val);
    void edgesIn2DChanged(bool val);
  private:
    QCheckBox *removeGhosts;
    QCheckBox *edgesIn2D;
    QLabel *removeGhostsLabel;
    QLabel *edgesIn2DLabel;

    ExternalSurfaceAttributes *atts;
};



#endif
