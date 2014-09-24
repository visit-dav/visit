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

#include <ViewerBase.h>
#include <ViewerEngineManagerInterface.h>
#include <ViewerPlotList.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <WindowInformation.h>
#include <avtDatabaseMetaData.h>


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

DDTManager::DDTManager() : ViewerBaseUI(), mSession(NULL)
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
//   Jonathan Byrd, Fri Feb 1, 2013
//   Update default location of DDT's socket (for DDT >= 4.0)
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
    {
        const char *userName = NULL;
        userName = getenv("USER");          // Linux/Mac
        if (userName == NULL)
            userName = getenv("LOGNAME");
        if (userName == NULL)
            userName = getenv("USERNAME");  // Windows
        if (userName == NULL)
            return NULL;                    // Unable to get username
        filename = QString("%0/allinea-%1/ddt.socket.tmp").arg(QDir::tempPath(),QString::fromLocal8Bit(userName));
    }

    if (!QFile::exists(filename))
        return NULL;                // Named socket does not exist, cannot connect to it

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

// ****************************************************************************
// Method: DDTPickCallback
//
// Purpose:
//   Supports DDT pick support for domain & element
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

#include <PickAttributes.h>
#include <PickVarInfo.h>
#include <ViewerMessaging.h>
#include <ViewerQueryManager.h>
#include <ViewerText.h>
#include <snprintf.h>

static void
DDTPickCallback(PickAttributes *pickAtts, void *)
{
    const int targetDomain           = pickAtts->GetDomain();
    const int targetElement          = pickAtts->GetElementNumber();
    const std::string targetVariable = pickAtts->GetActiveVariable();

    char buff[256];
    buff[0] = '\0';

    for (int i=0; i<pickAtts->GetNumVarInfos(); ++i)
    {
        const PickVarInfo  &info = pickAtts->GetVarInfo(i);
        const doubleVector &values = info.GetValues();

        // For the active variable only
        if (info.GetVariableName() == targetVariable)
        {
            if (info.GetVariableType() == "scalar" && values.size()==1)
            {
                SNPRINTF(buff, 256, info.GetFloatFormat().c_str(), values[0]);
            }
        }
    }
    std::string targetValue(buff);

    DDTSession* ddt = DDTManager::getInstance()->getSession();
    if (ddt!=NULL)
        ddt->setFocusOnElement(targetDomain, targetVariable, targetElement, targetValue);
    else
    {
        ViewerBase::GetViewerMessaging()->Error(
            TR("Cannot focus on domain %1, element %2 of %3: unable to "
               "connect to DDT").
            arg(targetDomain).
            arg(targetElement).
            arg(targetVariable));
    }
}

// ****************************************************************************
// Method: DDTUpdateWindowInformation
//
// Purpose:
//   Gets called by VWM when window information is updated.
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

static void
DDTUpdateWindowInformation(WindowInformation *windowInfo, int flags, void *)
{ 
    ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
    if(win == NULL)
        return;

    if((flags & WINDOWINFO_SOURCE) != 0)
    {
        if(win->GetPlotList()->GetHostDatabaseName().empty())
            windowInfo->SetActiveSource("notset");
        else
        {
            windowInfo->SetActiveSource(win->GetPlotList()->GetHostDatabaseName());

            // Not using DDTManager::isDDTSim() here, as for some reason at this
            // point the EngineKey is not flagged as a simulation
            windowInfo->SetDDTSim(DDTManager::isDatabaseDDTSim(win->GetPlotList()->GetDatabaseName().c_str()));
        }
        DDTSession *session = DDTManager::getInstance()->getSessionNC();
        windowInfo->SetDDTConnected(session && session->connected());
    }

    if((flags & WINDOWINFO_ANIMATION) != 0 ||
       (flags & WINDOWINFO_TIMESLIDERS) != 0)
    {
        if (DDTManager::isDDTSim(win))
        {
            const EngineKey &key = win->GetPlotList()->GetEngineKey();
            if (key.IsSimulation())
            {
                const avtDatabaseMetaData *md = ViewerBase::GetViewerEngineManager()->GetSimulationMetaData(key);
                if (md->GetSimInfo().GetMode()==avtSimulationInformation::Running)
                    windowInfo->SetAnimationMode(3);
                else
                    windowInfo->SetAnimationMode(2);
            }
        }
    }
}

// ****************************************************************************
// Method: DDTInitialize
//
// Purpose:
//   Initialization function for DDT.
//
// Note:       Installs a DDT pick callback on VQM.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 09:20:51 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
DDTInitialize()
{
    ViewerQueryManager::Instance()->SetDDTPickCallback(DDTPickCallback, NULL);
    ViewerWindowManager::Instance()->SetUpdateWindowInformationCallback(
        DDTUpdateWindowInformation, NULL);
}
