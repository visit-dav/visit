#ifndef QVISCRACKSCLIPPERWINDOW_H
#define QVISCRACKSCLIPPERWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class CracksClipperAttributes;
class QCheckBox;
class QComboBox;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisCracksClipperWindow
//
// Purpose: 
//   Defines QvisCracksClipperWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Aug 22 09:10:02 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisCracksClipperWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisCracksClipperWindow(const int type,
                         CracksClipperAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisCracksClipperWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void crack1VarChanged(const QString &);
    void crack2VarChanged(const QString &);
    void crack3VarChanged(const QString &);
    void strainVarChanged(const QString &);
    void useCrack1Changed(bool val);
    void useCrack2Changed(bool val);
    void useCrack3Changed(bool val);
  private:
    QvisVariableButton *crack1Var;
    QvisVariableButton *crack2Var;
    QvisVariableButton *crack3Var;
    QvisVariableButton *strainVar;
    QCheckBox *useCrack1;
    QCheckBox *useCrack2;
    QCheckBox *useCrack3;
    CracksClipperAttributes *atts;
};



#endif
