#ifndef QVISREFLECTWINDOW_H
#define QVISREFLECTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ReflectAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QVBox;
class QButtonGroup;
class QComboBox;
class QRadioButton;
class QvisReflectWidget;

// ****************************************************************************
//  Class:  QvisReflectWindow
//
//  Purpose:
//    The attribute window for the Reflect operator
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Apr 12 13:12:17 PST 2002
//    Made it inherit from QvisOperatorWindow.
//
//    Brad Whitlock, Mon Mar 3 11:40:37 PDT 2003
//    I spruced up the window.
//
// ****************************************************************************

class QvisReflectWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisReflectWindow(const int type,
                         ReflectAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisReflectWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void octantChanged(int val);
    void xBoundaryChanged(int val);
    void yBoundaryChanged(int val);
    void zBoundaryChanged(int val);
    void specifiedXProcessText();
    void specifiedYProcessText();
    void specifiedZProcessText();
    void selectOctants(bool *octants);
  private:
    QComboBox     *octant;
    QvisReflectWidget *reflect;
    QButtonGroup  *xBound;
    QRadioButton  *xUseData;
    QRadioButton  *xSpecify;
    QLineEdit     *specifiedX;
    QButtonGroup  *yBound;
    QRadioButton  *yUseData;
    QRadioButton  *ySpecify;
    QLineEdit     *specifiedY;
    QButtonGroup  *zBound;
    QRadioButton  *zUseData;
    QRadioButton  *zSpecify;
    QLineEdit     *specifiedZ;

    ReflectAttributes *atts;
};



#endif
