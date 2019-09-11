// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMCOMMANDSLOTS_H
#define SIMCOMMANDSLOTS_H

#include <QDialog>
#include <QDateTime>
#include <QDateTimeEdit>

#include <EngineList.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>

class QEvent;
class QAction;
class QListWidgetItem;

// ****************************************************************************
// Class: SimCommandSlots
//
// Purpose:
//   This class implements the slots defined for the qt ui that is loaded
//   dynamically.
//
// Notes:      
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

class SimCommandSlots:  public QObject
{
    Q_OBJECT
public:
    SimCommandSlots(ViewerProxy *theViewer, EngineList *engineList,
                    int index );
    void SetParent(QDialog * parent ) {p = parent;};

public slots:
    void SimCommands() {printf("command Slot Fired\n");};
    void ClickedHandler();
    void ToggledHandler(bool);
    void TriggeredHandler(QAction *);
    void CurrentIndexChangedHandler(int);
    void TextChangedHandler(const QString &);
    void ValueChangedHandler(int);
    void ValueChangedHandler(const QTime &theTime);
    void ValueChangedHandler(const QDate &theDate);
    void ActivatedHandler(int);
    void StateChangedHandler(int);
    void CellChangedHandler(int, int);
    void ItemChangedHandler(QListWidgetItem *item);
    void CurrentRowChangedHandler(int);
    void CurrentTextChangedHandler(const QString &);
  
    int  SendCMD(QString sig, const  QObject *ui,QString value);

private:
    bool eventFilter(QObject *, QEvent *) {printf("inside eventFilter\n");return false;};

    QDialog     *p;
    EngineList  *engines;
    ViewerProxy *viewer;
    int          simIndex;
};

#endif
