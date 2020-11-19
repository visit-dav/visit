// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISREFLECTWINDOW_H
#define QVISREFLECTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ReflectAttributes;
class QLabel;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QRadioButton;
class QvisReflectWidget;
class QTabWidget;

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
//    Brad Whitlock, Wed Jun 25 09:22:58 PDT 2003
//    I added a 2D view of the window.
//
//    Alister Maguire, Wed Apr 11 09:29:49 PDT 2018
//    I added several variables needed to integrate reflecting
//    over an arbitrary plane. 
//
// ****************************************************************************

class QvisReflectWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisReflectWindow(const int type,
                         ReflectAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisReflectWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
    void UpdateOctantMenuContents();
  private slots:
    void octantChanged(int val);
    void xBoundaryChanged(int val);
    void yBoundaryChanged(int val);
    void zBoundaryChanged(int val);
    void specifiedXProcessText();
    void specifiedYProcessText();
    void specifiedZProcessText();
    void selectOctants(bool *octants);
    void selectMode(int);
    void reflectTabsChangedIndex(int);
    void planePointProcessText();
    void planeNormProcessText();
  private:
    bool               userSetMode;
    bool               mode2D;

    QButtonGroup      *modeButtons;
    QLabel            *originalDataLabel;
    QComboBox         *octant;
    QLabel            *reflectionLabel;
    QvisReflectWidget *reflect;
    QButtonGroup      *xBound;
    QRadioButton      *xUseData;
    QRadioButton      *xSpecify;
    QLineEdit         *specifiedX;
    QButtonGroup      *yBound;
    QRadioButton      *yUseData;
    QRadioButton      *ySpecify;
    QLineEdit         *specifiedY;
    QButtonGroup      *zBound;
    QRadioButton      *zUseData;
    QRadioButton      *zSpecify;
    QLineEdit         *specifiedZ;
    QLineEdit         *planePoint;
    QLineEdit         *planeNorm;
    QTabWidget        *reflectTabs;
    QWidget           *planeTab;
    QWidget           *axisTab;

    ReflectAttributes *atts;
};



#endif
