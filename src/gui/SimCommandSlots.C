#include "SimCommandSlots.h"
#include <qtable.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>

#include <EngineList.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>
#include <string>
#include <DebugStream.h>


using std::string;

// ****************************************************************************
// Method: SimCommandSlots::SimCommandSlots
//
// Purpose:
//   This is the constructor for the SimCommandSlots class.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

SimCommandSlots::SimCommandSlots(ViewerProxy *theViewer,
    EngineList *engineList, int index)
{
    viewer = theViewer;
    simIndex = index;
    engines = engineList;
}

// ****************************************************************************
// Method: SimCommandSlots::SendCMD
//
// Purpose:
//   This is the main function called to send a Visit UI signal to
//   the remote running simulation program.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

int SimCommandSlots::SendCMD(QString sig, const QObject *ui, QString value)
{
    if (ui)
        debug5 << "signal sender is type " << ui->className() << " named "
               << ui->name() << " parent " << ui->parent()->name() << endl;
    else
        QMessageBox::warning(0, "VisIt", "Invalid ui component",
                             "Ok", 0, 0, 0, 1 );
    if (simIndex < 0)
    {
        QMessageBox::warning(0, "VisIt",
                             "Invalid index encountered for Sim engine access",
                             "Ok", 0, 0, 0, 1 );
        return -1;
    }

    string host = engines->GetEngines()[simIndex];
    string sim  = engines->GetSimulationName()[simIndex];

    QString cmd = sig + ";" + ui->name() + ";" + ui->className() + ";" +
                  ui->parent()->name() + ";" + value;
    viewer->SendSimulationCommand(host, sim, cmd.latin1());

    return 0;
}

// ****************************************************************************
// Method: SimCommandSlots::ClickedHandler
//
// Purpose:
//   This is the clicked signal handler function. It is generated when
//   the users clicks on a custom UI widget.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

void SimCommandSlots::ClickedHandler()
{
    const QObject *ui = sender();
    debug5 << "inside clicked signal" << endl;
    SendCMD("clicked()", ui, "NONE");
}

// ****************************************************************************
// Method: SimCommandSlots::ValueChangedHandler
//
// Purpose:
//   This is the ValueChangedHandler signal handler function. It is generated
//   when the users changes a value on a custom UI widget.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

void SimCommandSlots::ValueChangedHandler(int index)
{
    const QObject *ui = sender();
    debug5 << "inside ValueChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->className() << " named "
               << ui->name() << " parent " << ui->parent()->name() << endl;
        debug5 << "New Value = " << index << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = QString::number(index);
    SendCMD("valueChanged()", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::ValueChangedHandler
//
// Purpose:
//   This is the ValueChangedHandler signal handler function. It is generated
//   when the users changes a value on a custom UI widget.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

void SimCommandSlots::ValueChangedHandler(const QTime &theTime)
{
    const QObject *ui = sender();
    debug5 << "inside ValueChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->className() << " named "
               << ui->name() << " parent " << ui->parent()->name() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = theTime.toString();
    SendCMD("valueChanged()", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::ValueChangedHandler
//
// Purpose:
//   This is the ValueChangedHandler signal handler function. It is generated
//   when the users changes a value on a custom UI widget.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

void SimCommandSlots::ValueChangedHandler(const QDate &theDate)
{
    const QObject *ui = sender();
    debug5 << "inside ValueChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->className() << " named "
               << ui->name() << " parent " << ui->parent()->name() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = theDate.toString();
    SendCMD("valueChanged()", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::SimCommandSlots
//
// Purpose:
//   This is the StateChangedHandler signal handler function. It is generated
//   when the users changes a value on a custom UI widget.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

void SimCommandSlots::StateChangedHandler(int OnOff)
{                    
    const QObject *ui = sender();
    debug5 << "inside StateChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->className() << " named "
               << ui->name() << " parent " << ui->parent()->name() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value;
    value.number(OnOff);
    SendCMD("valueChanged()", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::CurrentChangedHandler
//
// Purpose:
//   This is the CurrentChangedHandler signal handler function. It is generated
//   when the users changes a text value inside a text custom UI widget.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

void SimCommandSlots::CurrentChangedHandler(int row, int col)
{
    const QObject *ui = sender();
    debug5 << "inside ValueChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->className() << " named "
               << ui->name() << " parent " << ui->parent()->name() << endl;
        debug5 << "New Value row,col,text = " << row << ", " << col << ", "
               << ((QTable *)ui)->text( row, col ).latin1() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = QString::number(row) + ";" + QString::number(col);
    SendCMD("currentChanged()", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::ActivatedHandler
//
// Purpose:
//   This is the ActivatedHandler signal handler function. It is generated when
//   the users action cause a value change on a custom UI widget.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

void SimCommandSlots::ActivatedHandler(int index)
{
    const QObject *ui = sender();
    debug5 << "inside ValueChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->className() << " named "
               << ui->name() << " parent " << ui->parent()->name() << endl;
        debug5 << "New Value = " << index << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = QString::number(index);
    SendCMD("activate()", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::TextChangedHandler
//
// Purpose:
//   This is the TextChangedHandler signal handler function. It is generated
//   when the users changes the text in a line edit type custom UI widget.
//
// Programmer: Shelly Prevost
// Creation:   Jan 6, 2006
//
// Modifications:
//
// ****************************************************************************

void SimCommandSlots::TextChangedHandler(const QString &newText)
{
    const QObject *ui = sender();
    debug5 << "inside TextChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->className() << " named "
               << ui->name() << " parent " << ui->parent()->name() << endl;
        debug5 << "New Text Value = " << newText.latin1() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    SendCMD("textChanged()", ui, newText);
}
