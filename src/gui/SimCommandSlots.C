/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "SimCommandSlots.h"

#include <QAction>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QString>
#include <QTableWidget>

#include <EngineList.h>
#include <StatusAttributes.h>
#include <ViewerProxy.h>
#include <string>
#include <DebugStream.h>

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
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Jul  8 11:16:09 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Sun Feb 27 14:08:32 PST 2011
//   Reorder command string.
//
// ****************************************************************************

int SimCommandSlots::SendCMD(QString sig, const QObject *ui, QString value)
{
    if (ui)
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
    else
        QMessageBox::warning(0, "VisIt", tr("Invalid ui component"),
                             tr("Ok"), 0, 0, 0, 1 );
    if (simIndex < 0)
    {
        QMessageBox::warning(0, "VisIt",
                             tr("Invalid index encountered for Sim engine access"),
                             tr("Ok"), 0, 0, 0, 1 );
        return -1;
    }

    std::string host = engines->GetEngineName()[simIndex];
    std::string sim  = engines->GetSimulationName()[simIndex];

    QString cmd = QString("UI;%1;%2;%3;%4").arg(ui->objectName())
                                           .arg(ui->metaObject()->className())
                                           .arg(sig)
                                           .arg(value);

    viewer->GetViewerMethods()->SendSimulationCommand(host, sim, cmd.toStdString());

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
    if (ui)
    {
      debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
      debug5 << "Clicked" << endl;
    }
    else
      debug5 << "unknown signaler" << endl;
    SendCMD("clicked()", ui, "NONE");
}

// ****************************************************************************
// Method: SimCommandSlots::ToggledHandler
//
// Purpose:
//   This is the ToggledHandler signal handler function. It is generated
//   when the users changes a value on a custom UI widget.
//
// Programmer: Allen Sanderson
// Creation:   Feb 6, 2016
//
// ****************************************************************************

void SimCommandSlots::ToggledHandler(bool on)
{
    const QObject *ui = sender();
    debug5 << "inside ToggledHandler signal" << endl;
    if (ui)
    {
      debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
      debug5 << "New Value = " << on << endl;
    }
    else
      debug5 << "unknown signaler" << endl;
    QString value = QString::number(on);
    SendCMD("valueChanged(int)", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::TriggeredHandler
//
// Purpose:
//   This is the TriggeredHandler signal handler function. It is generated
//   when the users changes a value on a custom UI widget.
//
// Programmer: Allen Sanderson
// Creation:   Feb 6, 2016
//
// ****************************************************************************

void SimCommandSlots::TriggeredHandler(QAction *action)
{
    const QObject *ui = sender();
    debug5 << "inside TriggeredHandler signal" << endl;
    if (ui)
    {
      debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
      debug5 << "Trigger text = " << action->text().toStdString() << endl;
    }
    else
      debug5 << "unknown signaler" << endl;
    QString text = action->text();
    SendCMD("textChanged(char *)", ui, text);
}

// ****************************************************************************
// Method: SimCommandSlots::CurrentIndexChangedHandler
//
// Purpose:
//   This is the CurrentIndexChangedHandler signal handler function. It is
//   generated when the users changes a value on a custom UI widget.
//
// Programmer: Allen Sanderson
// Creation:   Feb 6, 2016
//
// ****************************************************************************

void SimCommandSlots::CurrentIndexChangedHandler(int index)
{
    const QObject *ui = sender();
    debug5 << "inside CurrentIndexChangedHandler signal" << endl;
    if (ui)
    {
      debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
      debug5 << "New Value = " << index << endl;
    }
    else
      debug5 << "unknown signaler" << endl;
    QString value = QString::number(index);
    SendCMD("valueChanged(int)", ui, value);
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
//   Brad Whitlock, Tue Jul  8 11:17:38 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void SimCommandSlots::ValueChangedHandler(int index)
{
    const QObject *ui = sender();
    debug5 << "inside ValueChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
        debug5 << "New Value = " << index << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = QString::number(index);
    SendCMD("valueChanged(int)", ui, value);
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
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = theTime.toString();
    SendCMD("valueChanged(QTime)", ui, value);
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
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = theDate.toString();
    SendCMD("valueChanged(QDate)", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::StateChangedHandler
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
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value(QString::number(OnOff ? 1 : 0));
    SendCMD("stateChanged(int)", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::CellChangedHandler
//
// Purpose:
//   This is the CellChangedHandler signal handler function. It is generated
//   when the users changes a text value inside a text custom UI widget.
//
// Programmer: Allen Sanderson
// Creation:   Feb 6, 2016
//
// ****************************************************************************

void SimCommandSlots::CellChangedHandler(int row, int col)
{
    const QObject *ui = sender();
    debug5 << "inside CellChangedHandler signal" << endl;
    QString tvalue;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
        
        tvalue = ((QTableWidget *)ui)->item(row, col)->text();
        debug5 << "New Value row,col,text = " << row << ", " << col << ", "
               << tvalue.toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;

    QString value =
      QString::number(row) + " | " + QString::number(col) + " | " + tvalue;
    
    SendCMD("cellChanged(int,int)", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::ItemChangedHandler
//
// Purpose:
//   This is the ItemChangedHandler signal handler function. It is
//   generated when the user changes a text value inside a list
//   widget item in custom UI widget.
//
// Programmer: Allen Sanderson
// Creation:   Nov 6, 2018
//
// ****************************************************************************

void SimCommandSlots::ItemChangedHandler(QListWidgetItem *item)
{
    const QObject *ui = sender();
    debug5 << "inside CellChangedHandler signal" << endl;
    QString tvalue;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
        
        tvalue = item->text();
        debug5 << "New Value text = " << tvalue.toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;

    SendCMD("textChanged(char *)", ui, tvalue);
}

// ****************************************************************************
// Method: SimCommandSlots::CurrentRowChangedHandler
//
// Purpose:
//   This is the CurrentRowChangedHandler signal handler function. It
//   is generated when the user selects a new row inside a list
//   widget item in custom UI widget.
//
// Programmer: Allen Sanderson
// Creation:   Nov 6, 2018
//
// ****************************************************************************

void SimCommandSlots::CurrentRowChangedHandler(int row)
{
    const QObject *ui = sender();
    debug5 << "inside ValueChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
        debug5 << "New Value = " << row << endl;
    // QString tvalue = ((QListWidget *)ui)->item(row)->text();
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = QString::number(row);
    SendCMD("valueChanged(int)", ui, value);
}

// ****************************************************************************
// Method: SimCommandSlots::CurrentTextChangedHandler
//
// Purpose:
//   This is the CurrentTextChangedHandler signal handler function. It
//   is generated when the users selects a new text inside a list
//   widget item in custom UI widget.
//
// Programmer: Allen Sanderson
// Creation:   Nov 6, 2018
//
// ****************************************************************************

void SimCommandSlots::CurrentTextChangedHandler(const QString &newText)
{
    const QObject *ui = sender();
    debug5 << "inside CurrentTextChangedHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
        debug5 << "New Text Value = " << newText.toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    SendCMD("textChanged(char *)", ui, newText);
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
    debug5 << "inside ActivateHandler signal" << endl;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
        debug5 << "New Value = " << index << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = QString::number(index);
    SendCMD("activated(int)", ui, value);
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
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString() << " parent " << ui->parent()->objectName().toStdString() << endl;
        debug5 << "New Text Value = " << newText.toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    SendCMD("textChanged(char *)", ui, newText);
}
