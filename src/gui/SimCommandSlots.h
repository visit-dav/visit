#ifndef SIMCOMMANDSLOTS_H
#define SIMCOMMANDSLOTS_H

#include <qdialog.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>

#include <EngineList.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>

class QEvent;

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
    void TextChangedHandler(const QString &);
    void ValueChangedHandler(int);
    void ValueChangedHandler(const QTime &theTime);
    void ValueChangedHandler(const QDate &theDate);
    void ActivatedHandler(int);
    void StateChangedHandler(int);
    void CurrentChangedHandler(int, int);
    int  SendCMD(QString sig, const  QObject *ui,QString value);

private:
    bool eventFilter(QObject *, QEvent *) {printf("inside eventFilter\n");return false;};

    QDialog     *p;
    EngineList  *engines;
    ViewerProxy *viewer;
    int          simIndex;
};

#endif
