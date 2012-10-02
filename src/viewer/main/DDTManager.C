/*****************************************************************************
*
* Copyright (c) 2011, Allinea
* All rights reserved.
*
* This file has been contributed to the VisIt project, which is
* Copyright (c) Lawrence Livermore National Security, LLC. For  details, see
* https://visit.llnl.gov/.  The full copyright notice is contained in the 
* file COPYRIGHT located at the root of the VisIt distribution or at 
* http://www.llnl.gov/visit/copyright.html.
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

#include <DDTManager.h>
#include <DDTSession.h>
#include <QFile>
#include <QDir>

#include <ViewerWindow.h>
#include <ViewerPlotList.h>
#include <ViewerWindowManager.h>

DDTManager* DDTManager::instance = NULL;

// ****************************************************************************
// Method: DDTManager::getInstance
//
// Purpose:
//   Obtains the DDTManager singleton
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

DDTManager* DDTManager::getInstance()
{
    if (instance==NULL)
        instance = new DDTManager();
    return instance;
}

// ****************************************************************************
// Method: DDTManager::DDTManager
//
// Purpose:
//   Constructs the DDTManager singleton object
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

DDTManager::DDTManager() : mSession(NULL)
{
}

// ****************************************************************************
// Method: DDTManager::getSession
//
// Purpose:
//   Obtains the DDTSession representing a connection to a DDT application.
//   Will attempt to connect to DDT if a connection does not already exist.
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

DDTSession*
DDTManager::getSession()
{
    if (mSession!=NULL && mSession->disconnected())
        disconnect(); // Will delete mSession

    if (mSession==NULL)
        mSession = makeConnection(); // If not already connected, make default connection

        return mSession;
}

// ****************************************************************************
// Method: DDTManager::getSessionNC
//
// Purpose:
//   Obtains the DDTSession representing a connection to a DDT application.
//   Will NOT attempt to connect to DDT if a connection does not exist.
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

DDTSession*
DDTManager::getSessionNC()
{
    return mSession;
}

// ****************************************************************************
// Method: DDTManager::makeConnection
//
// Purpose:
//   Attempts to extablish a connection to DDT
//
// Returns:
//   The DDTSession object created, or NULL if a connection already existed
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

DDTSession*
DDTManager::makeConnection()
{
        // For now only support having the one DDT session
    if (mSession!=NULL)
    {
        if (mSession->connected())
            return NULL;            // Do nothing, use the already connected mSession
        else
            delete mSession;        // No longer connected to DDT. Delete mSession and try again
    }

    QString filename = QFile::decodeName(getenv("DDT_SOCKET"));
    if(filename.isEmpty())
        filename = QDir::homePath() + "/.ddt/ddt.socket.tmp";
    mSession = new DDTSession(filename);

    if (mSession->connected())
        updateWindowInfo();
    return mSession;
}

// ****************************************************************************
// Method: DDTManager::disconnect
//
// Purpose:
//   Break the connect to DDT, if one exists
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

void
DDTManager::disconnect()
{
    if (mSession!=NULL)
    {
        mSession->disconnect();
        delete mSession;
        mSession = NULL;
    }
    updateWindowInfo();
}

// ****************************************************************************
// Method: DDTManager::isDatabaseDDTSim
//
// Purpose:
//   Determines if a database filename is likely to have been generated
//   by ddtsim.
//
// Returns:
//   true if the filename suffix matches that expected from a ddtsim
//   session, false otherwise
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

bool
DDTManager::isDatabaseDDTSim(std::string db)
{
    QFileInfo info(QString(db.c_str()));
    return info.fileName().endsWith("-ddt.sim2");
}

// ****************************************************************************
// Method: DDTManager::isDDTSim
//
// Purpose:
//   Determines if a ViewerWindow is likely to be displaying data for a
//   ddtsim-based simulation.
//
// Notes: Checks the database filename and whether the engine is a simulation
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

bool
DDTManager::isDDTSim(ViewerWindow* window)
{
    return window->GetPlotList()->GetEngineKey().IsSimulation() &&
           isDatabaseDDTSim(window->GetPlotList()->GetDatabaseName());
}

// ****************************************************************************
// Method: DDTManager::statusChanged
//
// Purpose:
//   Qt slot called when the connection status to DDT changes.
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

void
DDTManager::statusChanged()
{
    updateWindowInfo();
}

// ****************************************************************************
// Method: DDTManager::updateWindowInfo
//
// Purpose:
//   Triggers an update of the ViewerWindowManager WindowInfo source
//   attributes. Used to inform other VisIt windows when DDT connects
//   or disconnects.
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

void
DDTManager::updateWindowInfo()
{
    ViewerWindowManager::Instance()->UpdateWindowInformation(WINDOWINFO_SOURCE, -1);
}
