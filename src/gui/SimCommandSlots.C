/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <QDateTime>
#include <QDateTimeEdit>
#include <QMessageBox>
#include <QString>
#include <QTableWidget>

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
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Jul  8 11:16:09 PDT 2008
//   Qt 4.
//
// ****************************************************************************

int SimCommandSlots::SendCMD(QString sig, const QObject *ui, QString value)
{
    if (ui)
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString().c_str() << " parent " << ui->parent()->objectName().toStdString().c_str() << endl;
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

    string host = engines->GetEngines()[simIndex];
    string sim  = engines->GetSimulationName()[simIndex];

    QString cmd = sig + ";" + ui->objectName().toStdString().c_str() + ";" + ui->metaObject()->className() + ";" +
                  ui->parent()->objectName().toStdString().c_str() + ";" + value;
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
               << ui->objectName().toStdString().c_str() << " parent " << ui->parent()->objectName().toStdString().c_str() << endl;
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
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString().c_str() << " parent " << ui->parent()->objectName().toStdString().c_str() << endl;
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
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString().c_str() << " parent " << ui->parent()->objectName().toStdString().c_str() << endl;
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
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString().c_str() << " parent " << ui->parent()->objectName().toStdString().c_str() << endl;
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
//   Brad Whitlock, Tue Jul  8 11:28:04 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void SimCommandSlots::CurrentChangedHandler(int row, int col)
{
    const QObject *ui = sender();
    debug5 << "inside CurrentChangedHandler signal" << endl;
    QString tvalue;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString().c_str() << " parent " << ui->parent()->objectName().toStdString().c_str() << endl;
        tvalue = ((QTableWidget *)ui)->item(row, col)->text();
        debug5 << "New Value row,col,text = " << row << ", " << col << ", "
               << tvalue.toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = QString::number(row) + ";" + QString::number(col) + ";" + tvalue;
    SendCMD("currentChanged()", ui, value);
}


// ****************************************************************************
// Method: SimCommandSlots::ValueChangedHandler
//
// Purpose:
//   This is the CurrentChangedHandler signal handler function. It is generated
//   when the users changes a text value inside a text custom UI widget.
//
// Programmer: Shelly Prevost
// Creation:   Mon Jun 18 16:56:13 PDT 2007
//
// Modifications:
//   Brad Whitlock, Tue Jul  8 11:28:04 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void SimCommandSlots::ValueChangedHandler(int row, int col)
{
    const QObject *ui = sender();
    debug5 << "inside ValueChangedHandler signal" << endl;
    QString tvalue;
    if (ui)
    {
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString().c_str() << " parent " << ui->parent()->objectName().toStdString().c_str() << endl;
        tvalue = ((QTableWidget *)ui)->item(row, col)->text();
        debug5 << "New Value row,col,text = " << row << ", " << col << ", "
               << tvalue.toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    QString value = QString::number(row) + ";" + QString::number(col) + ";" + tvalue;
    SendCMD("valueChanged()", ui, value);
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
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString().c_str() << " parent " << ui->parent()->objectName().toStdString().c_str() << endl;
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
        debug5 << "signal sender is type " << ui->metaObject()->className() << " named "
               << ui->objectName().toStdString().c_str() << " parent " << ui->parent()->objectName().toStdString().c_str() << endl;
        debug5 << "New Text Value = " << newText.toStdString() << endl;
    }
    else
        debug5 << "unknown signaler" << endl;
    SendCMD("textChanged()", ui, newText);
}
