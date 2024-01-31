// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISCREATEBONDSWINDOW_H
#define QVISCREATEBONDSWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class CreateBondsAttributes;
class QLabel;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QTreeWidget;
class QvisElementButton;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisCreateBondsWindow
//
// Purpose: 
//   Defines QvisCreateBondsWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Wed Apr 19 17:23:18 PST 2006
//
// Modifications:
//    Jeremy Meredith, Mon Feb 11 17:03:15 EST 2008
//    Added support for wildcards for atomic numbers.  Necessitated
//    adding an up/down button (since order is now significant).
//   
//    Cyrus Harrison, Wed Aug 20 08:27:03 PDT 2008
//    Qt4 Port.
//
//    Jeremy Meredith, Wed Jan 27 10:39:48 EST 2010
//    Added periodic bond matching support.
//
// ****************************************************************************

class QvisCreateBondsWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisCreateBondsWindow(const int type,
                         CreateBondsAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisCreateBondsWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
    int  GetListLength();
  private slots:
    void UpdateWindowSingleItem();
    void elementVariableChanged(const QString &varName);
    void bondsTreeNew();
    void bondsTreeDel();
    void bondsTreeUp();
    void bondsTreeDown();
    void minDistTextChanged(const QString&);
    void maxDistTextChanged(const QString&);
    void minDistReturnPressed();
    void maxDistReturnPressed();
    void maxBondsReturnPressed();
    void firstElementChanged(int);
    void secondElementChanged(int);
    void addPeriodicBondsToggled(bool);
    void useUnitCellVectorsChanged(bool);
    void xPeriodicToggled(bool);
    void yPeriodicToggled(bool);
    void zPeriodicToggled(bool);
    void xVectorProcessText();
    void yVectorProcessText();
    void zVectorProcessText();
  private:
    QvisVariableButton    *elementVariable;
    QLabel                *elementVariableLabel;

    QLineEdit             *maxBonds;
    QLabel                *maxBondsLabel;

    QPushButton           *newButton;
    QPushButton           *delButton;

    QPushButton           *upButton;
    QPushButton           *downButton;

    QTreeWidget           *bondsTree;
    CreateBondsAttributes *atts;

    QvisElementButton     *firstElement;
    QvisElementButton     *secondElement;
    QLineEdit             *minDist;
    QLineEdit             *maxDist;

    QGroupBox *addPeriodicBonds;
    QCheckBox *useUnitCellVectors;
    QCheckBox *xPeriodic;
    QCheckBox *yPeriodic;
    QCheckBox *zPeriodic;
    QLineEdit *xVector;
    QLineEdit *yVector;
    QLineEdit *zVector;
    QLabel *xVectorLabel;
    QLabel *yVectorLabel;
    QLabel *zVectorLabel;
};



#endif
