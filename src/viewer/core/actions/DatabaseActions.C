/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#include <DatabaseActions.h>

#include <ViewerDatabaseCorrelationMethods.h>
#include <ViewerEngineManagerInterface.h>
#include <ViewerFileServerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerPlot.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerState.h>
#include <ViewerStateManager.h>
#include <ViewerText.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <avtDatabaseMetaData.h>
#include <PlotPluginManager.h>

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////
void (*DatabaseActionBase::simConnectCB)(EngineKey &, void *) = NULL;
void *DatabaseActionBase::simConnectCBData = NULL;
void (*DatabaseActionBase::updateExpressionCB)(const avtDatabaseMetaData *, void*) = NULL;
void *DatabaseActionBase::updateExpressionCBData = NULL;

DatabaseActionBase::DatabaseActionBase(ViewerWindow *win) : ViewerActionLogic(win)
{
}

DatabaseActionBase::~DatabaseActionBase()
{
}

void
DatabaseActionBase::SetSimConnectCallback(void (*cb)(EngineKey&,void*), void *cbdata)
{
    simConnectCB = cb;
    simConnectCBData = cbdata;
}

void
DatabaseActionBase::SetUpdateExpressionCallback(
    void (*cb)(const avtDatabaseMetaData *, void *),
    void *cbdata)
{
    updateExpressionCB = cb;
    updateExpressionCBData = cbdata;
}

// ****************************************************************************
//  Method: DatabaseActionBase::OpenDatabaseHelper
//
//  Purpose:
//    Opens a database.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 20 10:33:42 PDT 2001
//    Added code to pass "other" options to the engine when starting.
//
//    Brad Whitlock, Wed Nov 14 17:08:07 PST 2001
//    Added code to set the number of time steps in the animation.
//
//    Brad Whitlock, Wed Sep 11 16:29:27 PST 2002
//    I changed the code so an engine is only launched when the file can
//    be opened.
//
//    Brad Whitlock, Tue Dec 10 15:34:17 PST 2002
//    I added code to tell the engine to open the database.
//
//    Brad Whitlock, Mon Dec 30 14:59:24 PST 2002
//    I changed how nFrames and nStates are set.
//
//    Brad Whitlock, Fri Jan 17 11:35:29 PDT 2003
//    I added code to reset nFrames if there are no plots in the plot list.
//
//    Brad Whitlock, Tue Feb 11 11:56:34 PDT 2003
//    I made it use STL strings.
//
//    Brad Whitlock, Tue Mar 25 14:23:16 PST 2003
//    I made it capable of defining a virtual database.
//
//    Brad Whitlock, Fri Apr 4 11:10:08 PDT 2003
//    I changed how the number of frames in an animation is updated.
//
//    Brad Whitlock, Thu May 15 13:34:19 PST 2003
//    I added the timeState argument and renamed the method.
//
//    Hank Childs, Thu Aug 14 09:10:00 PDT 2003
//    Added code to manage expressions from databases.
//
//    Walter Herrera, Thu Sep 04 16:13:43 PST 2003
//    I made it capable of creating default plots
//
//    Brad Whitlock, Fri Oct 3 10:40:49 PDT 2003
//    I prevented the addition of default plots if the plot list already
//    contains plots from the new database.
//
//    Brad Whitlock, Wed Oct 22 12:27:30 PDT 2003
//    I made the method actually use the addDefaultPlots argument.
//
//    Brad Whitlock, Fri Oct 24 17:07:52 PST 2003
//    I moved the code to update the expression list into the plot list.
//
//    Hank Childs, Fri Mar  5 11:39:22 PST 2004
//    Send the file format type to the engine.
//
//    Jeremy Meredith, Mon Mar 22 17:12:22 PST 2004
//    I made use of the "success" result flag from CreateEngine.
//
//    Brad Whitlock, Tue Mar 23 17:41:57 PST 2004
//    I added support for database correlations. I also prevented the default
//    plot from being realized if the engine was not launched.
//
//    Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//    Added an engine key used to index (and restart) engines.
//    Added support for connecting to running simulations.
//
//    Jeremy Meredith, Fri Apr  2 14:14:54 PST 2004
//    Made it re-use command line arguments if we had some, as long as 
//    we were in nowin mode.
//
//    Brad Whitlock, Mon Apr 19 10:04:47 PDT 2004
//    I added the updateWindowInfo argument so we don't always have to update
//    the window information since that can cause extra updates in the gui.
//    I also added code to make the plot list check its active source vs the
//    active time slider so it can reset the active time slider if it no
//    longer makes sense to have one.
//
//    Jeremy Meredith, Wed Aug 25 10:34:53 PDT 2004
//    Made simulations connect the write socket from the engine to a new
//    socket notifier, which signals a method to read and process data from
//    the engine.  Hook up a new metadata and SIL atts observer to the 
//    metadata and SIL atts from the corresponding engine proxy, and have
//    those observers call callbacks when they get new information.
//
//    Brad Whitlock, Thu Feb 3 10:34:17 PDT 2005
//    Added a little more code to validate the timeState so if it's out of
//    range, we update the time slider to a valid value and we tell the
//    compute engine a valid time state at which to open the database. I also
//    made the routine return the time state in case it needs to be used
//    by the caller.
//
//    Jeremy Meredith, Tue Feb  8 08:58:49 PST 2005
//    Added a query for errors detected during plugin initialization.
//
//    Jeremy Meredith, Wed May 11 09:04:52 PDT 2005
//    Added security key to simulation connection.
//
//    Jeremy Meredith, Mon Aug 28 16:55:01 EDT 2006
//    Added ability to force using a specific plugin when opening a file.
//
//    Hank Childs, Thu Jan 11 15:33:07 PST 2007
//    Return an invalid time state when the file open fails.
//
//    Brad Whitlock, Thu Jan 25 18:48:15 PST 2007
//    Hooked up code to handle requests from the simulation.
//
//    Brad Whitlock, Thu Jan 24 12:00:29 PDT 2008
//    Added argument to ViewerPlotList::AddPlot().
//
//    Brad Whitlock, Fri Feb 15 14:54:34 PST 2008
//    Delete the adn from the default plot.
//
//    Hank Childs, Tue Feb 19 10:28:15 PST 2008
//    Fix bug introduced by Klocwork fix.
//
//    Brad Whitlock, Fri May  9 14:52:00 PDT 2008
//    Qt 4.
//
//    Cyrus Harrison,  Mon Aug  4 16:21:04 PDT 2008
//    Moved set of active host database until after we have obtained valid
//    meta data. 
//
//    Brad Whitlock, Tue Apr 14 13:38:10 PDT 2009
//    Use ViewerProperties.
//
//    Brad Whitlock, Tue Mar 16 11:56:53 PDT 2010
//    I added a call to ClearCache for simulations to force this method to
//    block until metadata and SIL have come back from the simulation.
//
//    Hank Childs, Fri Nov 26 10:31:34 PST 2010
//    Set up expressions from operators when we open a file.
//
//    Brad Whitlock, Thu Feb 24 01:55:07 PST 2011
//    Make a copy of the metadata since something along the way has a 
//    side-effect of invalidating it, causing a crash when we use -o from the
//    command line.
//
// ****************************************************************************

int
DatabaseActionBase::OpenDatabaseHelper(const std::string &entireDBName,
    int timeState, bool addDefaultPlots, bool updateWindowInfo,
    const std::string &forcedFileType)
{
    int  i;
    const char *mName = "DatabaseActionBase::OpenDatabaseHelper: ";
    debug1 << mName << "Opening database " << entireDBName.c_str()
           << ", timeState=" << timeState
           << ", addDefaultPlots=" << (addDefaultPlots?"true":"false")
           << ", updateWindowInfo=" << (updateWindowInfo?"true":"false")
           << ", forcedFileType=\"" << forcedFileType.c_str() << "\"" << endl;

    //
    // Associate the database with the currently active animation (window).
    //
    ViewerPlotList *plotList = GetWindow()->GetPlotList();

    //
    // Expand the new database name and then set it into the plot list.
    //
    std::string hdb(entireDBName), host, db;
    ViewerFileServerInterface *fs = GetViewerFileServer();
    fs->ExpandDatabaseName(hdb, host, db);
    debug1 << mName << "ExpandDatabaseName(" << hdb << ") -> host=" << host
           << ", db=" << db << endl;

    //
    // Get the number of time states and set that information into the
    // active animation. The mdserver will clamp the time state that it
    // uses to open the database if timeState is out of range at this point.
    //
    debug1 << mName << "Calling GetMetaDataForState(" << host << ", " << db
           << ", " << timeState << ", " << forcedFileType << ")" << endl;
    const avtDatabaseMetaData *mdptr = fs->GetMetaDataForState(host, db,
                                                               timeState,
                                                               forcedFileType);
    if (mdptr != NULL)
    {
        avtDatabaseMetaData md(*mdptr);

        // set the active host database name now that we have valid metadata.
        plotList->SetHostDatabaseName(hdb.c_str());
        
        //
        // If the database has more than one time state then we should
        // add it to the list of database correlations so we have a trivial
        // correlation for this database.
        //
        bool nStatesDecreased = false;
        if(md.GetNumStates() > 1)
        {
            //
            // Get the name of the database so we can use that for the name
            // of a new trivial database correlation.
            //
            const std::string &correlationName = plotList->GetHostDatabaseName();

            debug3 << mName << "Correlation for " << hdb.c_str() << " is "
                   << correlationName.c_str() << endl;

            //
            // In the case where we're reopening a database that now has
            // fewer time states, clamp the timeState value to be in the new
            // range of time states so we set the time slider to a valid
            // value and we use a valid time state when telling the compute
            // engine to open the database.
            //
            if(timeState > md.GetNumStates() - 1)
            {
                debug3 << mName << "There are " << md.GetNumStates()
                       << " time states in the database but timeState was "
                       << "set to "<< timeState <<". Clamping timeState to ";
                timeState = md.GetNumStates() - 1;
                debug3 << timeState << "." << endl;
                nStatesDecreased = true;
            }

            //
            // Tell the window manager to create the correlation. We could
            // use the file server but this way also creates time sliders
            // for the new correlation in each window and makes the active
            // window's active time slider be the new correlation.
            //
            stringVector dbs; dbs.push_back(correlationName);
            int timeSliderState = (timeState >= 0) ? timeState : 0;
            windowMgr->CreateDatabaseCorrelation(correlationName, dbs, 0,
                timeSliderState, md.GetNumStates());
        }
        else if(timeState > 0)
        {
            debug3 << mName << "There is only 1 time state in the database "
                   << "but timeState was set to "<< timeState <<". Clamping "
                   << "timeState to 0.";
            timeState = 0;
            nStatesDecreased = true;
        }

        //
        // Make sure that it is appropriate to have the time slider that
        // is currently used in the plot list.
        //
        if(!windowMgr->GetActiveWindow()->GetTimeLock())
            plotList->ValidateTimeSlider();

        // Alter the time slider for the database that we opened.
        if(nStatesDecreased)
            windowMgr->AlterTimeSlider(hdb);

        //
        // Update the global atts since that has the list of sources.
        //
        windowMgr->UpdateGlobalAtts();

        //
        // Since we updated the source and we made have also updated the time
        // slider and time slider states when the new database was opened, send
        // back the source, time sliders, and animation information.
        //
        if(updateWindowInfo || nStatesDecreased)
        {
            windowMgr->UpdateWindowInformation(WINDOWINFO_SOURCE |
                WINDOWINFO_TIMESLIDERS | WINDOWINFO_ANIMATION);
        }

        //
        // Update the expression list.
        //
        plotList->UpdateExpressionList(false);
        if(updateExpressionCB != NULL)
        {
            (*updateExpressionCB)(&md, updateExpressionCBData);
        }

        //
        // Determine the name of the simulation
        //
        std::string sim = "";
        if (md.GetIsSimulation())
            sim = db;

        //
        // Create an engine key, used to index and start engines
        //
        debug1 << mName << "Creating engine key from host=" << host << ", sim=" << sim << endl;
        EngineKey ek(host, sim);

        //
        // Tell the plot list the new engine key
        //
        plotList->SetEngineKey(ek);

        //
        // Create a compute engine to use with the database.
        //
        stringVector noArgs;
        bool success;
        if (md.GetIsSimulation())
        {
            success = GetViewerEngineManager()->ConnectSim(ek, noArgs,
                                      md.GetSimInfo().GetHost(),
                                      md.GetSimInfo().GetPort(),
                                      md.GetSimInfo().GetSecurityKey());

            // Call a callback function for the class that will help in
            // setting up socket notifiers, etc.
            if (success && simConnectCB != NULL)
            {
                (*simConnectCB)(ek, simConnectCBData);
            }
        }
        else
        {
            if (GetViewerProperties()->GetNowin())
            {
                success = GetViewerEngineManager()->
                    CreateEngine(ek,
                                 GetViewerProperties()->GetEngineParallelArguments(),
                                 false,
                                 GetViewerProperties()->GetNumEngineRestarts());
            }
            else
            {
                success = GetViewerEngineManager()->
                    CreateEngine(ek,
                                 noArgs,
                                 false,
                                 GetViewerProperties()->GetNumEngineRestarts());
            }
        }

        if (success)
        {
            //
            // Tell the new engine to open the specified database.
            // Don't bother if you couldn't even start an engine.
            //
            if(md.GetIsVirtualDatabase() && md.GetNumStates() > 1)
            {
                GetViewerEngineManager()->DefineVirtualDatabase(ek,
                    md.GetFileFormat(),
                    db,
                    md.GetTimeStepPath(),
                    md.GetTimeStepNames(),
                    timeState);
            }
            else
            {
                GetViewerEngineManager()->OpenDatabase(ek, md.GetFileFormat(),
                    db, timeState);

                // If we're opening a simulation, send ClearCache to it since that
                // is a harmless blocking RPC that will prevent OpenDatabaase from
                // returning until we get simulation metadata and SIL back. This is
                // a synchronize operation.
                if(md.GetIsSimulation())
                    GetViewerEngineManager()->ClearCache(ek, "invalid name");
            }
        }
        
        //
        // Create default plots if there are no plots from the database
        // already in the plot list.
        //
        if(addDefaultPlots && !plotList->FileInUse(host, db))
        {
            bool defaultPlotsAdded = false;

            for(i=0; i<md.GetNumDefaultPlots(); i++)
            {
                const avtDefaultPlotMetaData *dp = md.GetDefaultPlot(i);
                DataNode *adn = CreateAttributesDataNode(dp);

                //
                // Use the plot plugin manager to get the plot type index from
                // the plugin id.
                //
                int type = GetPlotPluginManager()->GetEnabledIndex(dp->pluginID);

                if(type != -1)
                {
                    debug4 << "Adding default plot: type=" << type
                           << " var=" << dp->plotVar.c_str() << endl;
                    plotList->AddPlot(type, dp->plotVar, false, false, true, 
                        false, adn);
                    defaultPlotsAdded = true;
                }

                if (adn != NULL)
                {
                    delete adn;
                    adn = NULL;
                }
            }

            //
            // Only realize the plots if we added some default plots *and*
            // the engine was successfully launched above.
            //
            if (defaultPlotsAdded && success)
            {
                plotList->RealizePlots();
            } 
        }
        else
        {
            debug4 << "Default plots were not added because the plot list "
                      "already contains plots from "
                   << host.c_str() << ":" << db.c_str() << endl;
        }
    }
    else
    {
        // We had a problem opening the file ... indicate that with the
        // return value (which is timeState).
        timeState = -1;
    }

    //
    // Check to see if there were errors in the mdserver
    //
    std::string err = GetViewerFileServer()->GetPluginErrors(host);
    if (!err.empty())
    {
        GetViewerMessaging()->Warning(err);
    }

    return timeState;
}

// ****************************************************************************
// Function: getToken
//
// Purpose: 
//   Return the first token readed from a buffer string. 
//   If there are no more tokens, it returns an empty string.
//
// Programmer: Walter Herrera
// Creation:   Tue Sep 11 12:07:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************

static std::string
getToken(std::string buff, bool reset = false)
{
    static std::string::size_type pos1 = 0;
    std::string::size_type pos2;
    std::string token;
    
    if (reset)
      pos1 = 0;
    
    if (pos1 == std::string::npos)
        return token;

    pos1 = buff.find_first_not_of(' ', pos1);  
    if (pos1 == std::string::npos)
        return token;

    pos2 = buff.find_first_of(' ', pos1);
    token = buff.substr(pos1, pos2-pos1);
    pos1 = pos2;

    return token;
}

// ****************************************************************************
// Function: getVectorTokens
//
// Purpose: 
//   Return a vector of tokens readed from a buffer string. 
//   The first token tell us the number of tokens that must be readed.
//
// Programmer: Walter Herrera
// Creation:   Tue Sep 11 12:07:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************

static int
getVectorTokens(std::string buff, std::vector<std::string> &tokens, int nodeType)
{
    int length, ival;
    std::string token, numTokens;
    long lval;
    float fval;
    double dval;

    tokens.clear();
    
    numTokens = getToken(buff);  
    if (sscanf(numTokens.c_str(),"%d",&length) != 1)
        return 0;

    for(int j=0; j<length; j++)
    {
        token = getToken(buff);
        if (token.size() != 0)
        {
              switch(nodeType)
            {
            case CHAR_ARRAY_NODE:
            case CHAR_VECTOR_NODE:
            case UNSIGNED_CHAR_ARRAY_NODE:
            case UNSIGNED_CHAR_VECTOR_NODE:
            case INT_ARRAY_NODE:
            case INT_VECTOR_NODE:
                if(sscanf(token.c_str(),"%d",&ival) == 1)
                    tokens.push_back(token);
                break;

            case LONG_ARRAY_NODE:
            case LONG_VECTOR_NODE:
                if(sscanf(token.c_str(),"%ld",&lval) == 1)
                    tokens.push_back(token);
                break;        
            
            case FLOAT_ARRAY_NODE:
            case FLOAT_VECTOR_NODE:
                if(sscanf(token.c_str(),"%f",&fval) == 1)
                    tokens.push_back(token);
                break;
            
            case DOUBLE_ARRAY_NODE:
            case DOUBLE_VECTOR_NODE:
                if(sscanf(token.c_str(),"%lf",&dval) == 1)
                    tokens.push_back(token);
                break;
              
            case STRING_ARRAY_NODE:
            case STRING_VECTOR_NODE:
            case BOOL_ARRAY_NODE:
            case BOOL_VECTOR_NODE:
                if (token.size() > 0)
                    tokens.push_back(token);
                break;
          }
        }
    }

    if (tokens.size() != (size_t)length)
        tokens.clear();

    return (int)tokens.size();
}

// ****************************************************************************
// Method: DatabaseActionBase::CreateAttributesDataNode
//
// Purpose: 
//   Create a DataNode with the attributes of one plot.
//
// Programmer: Walter Herrera
// Creation:   Tue Sep 9 10:27:46 PST 2003
//
// Modifications:
//   
// ****************************************************************************

DataNode *
DatabaseActionBase::CreateAttributesDataNode(const avtDefaultPlotMetaData *dp) const
{
    DataNode *node = 0, *fatherNode, *newNode;
    std::string nodeTypeToken, fatherName, attrName, attrValue;
    std::vector<std::string> tokens;
    int nodeType = 0, length, ival;
    char cval;
    unsigned char ucval;
    long lval;
    float fval;
    double dval;

    for(size_t i=0; i < dp->plotAttributes.size(); i++) 
    {
        nodeTypeToken = getToken(dp->plotAttributes[i], true);  
        if(sscanf(nodeTypeToken.c_str(), "%d", &nodeType) != 1)
            continue;

        fatherName = getToken(dp->plotAttributes[i]);  
        attrName = getToken(dp->plotAttributes[i]);  

        fatherNode = 0;
        if (node != 0)
            fatherNode = node->GetNode(fatherName);

        switch(nodeType)
        {
        case INTERNAL_NODE:
            if(fatherName == "NULL")
            {
                if(node != 0)
                    delete node;

                node = new DataNode(attrName);
            }
            else if(fatherNode != 0)
            {
                newNode = new DataNode(attrName);
                fatherNode->AddNode(newNode);
            }
            break;

        case CHAR_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%d",&ival) == 1)
                {                
                    cval = (char)ival;
                    newNode = new DataNode(attrName,cval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case UNSIGNED_CHAR_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%d",&ival) == 1)
                {                
                    ucval = (unsigned char)ival;
                    newNode = new DataNode(attrName,ucval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case INT_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%d",&ival) == 1)
                {                
                    newNode = new DataNode(attrName,ival);
                    fatherNode->AddNode(newNode);
                }
            }
            break;        

        case LONG_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%ld",&lval) == 1)
                {                
                    newNode = new DataNode(attrName,lval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;        

        case FLOAT_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%f",&fval) == 1)
                {                
                    newNode = new DataNode(attrName,fval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case DOUBLE_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%lf",&dval) == 1)
                {                
                    newNode = new DataNode(attrName,dval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case STRING_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  
                if (attrValue.size() != 0)
                {
                    newNode = new DataNode(attrName,attrValue);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case BOOL_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (attrValue.size() != 0)
                {
                    newNode = new DataNode(attrName,(attrValue == "true"));
                    fatherNode->AddNode(newNode);
                } 
            }
            break;        

        case CHAR_ARRAY_NODE:
        case CHAR_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                char *arrayItems = new char[length];
                charVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%d",&ival);
                    arrayItems[j] = (char)ival;
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == CHAR_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case UNSIGNED_CHAR_ARRAY_NODE:
        case UNSIGNED_CHAR_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                unsigned char *arrayItems = new unsigned char[length];
                unsignedCharVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%d",&ival);
                    arrayItems[j] = (unsigned char)ival;
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == UNSIGNED_CHAR_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case INT_ARRAY_NODE:
        case INT_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                int *arrayItems = new int[length];
                intVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%d",&arrayItems[j]);
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == INT_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case LONG_ARRAY_NODE:
        case LONG_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                long *arrayItems = new long[length];
                longVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%ld",&arrayItems[j]);
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == LONG_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;        

        case FLOAT_ARRAY_NODE:
        case FLOAT_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                float *arrayItems = new float[length];
                floatVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%f",&arrayItems[j]);
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == FLOAT_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case DOUBLE_ARRAY_NODE:
        case DOUBLE_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                double *arrayItems = new double[length];
                doubleVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%lf",&arrayItems[j]);
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == DOUBLE_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case STRING_ARRAY_NODE:
        case STRING_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                std::string *arrayItems = new std::string[length];

                for(int j=0; j<length; j++)
                {
                    arrayItems[j] = tokens[j];
                }
                if (nodeType == STRING_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,tokens);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case BOOL_ARRAY_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                bool *arrayItems = new bool[length];

                for(int j=0; j<length; j++)
                {
                    arrayItems[j] = (tokens[j] == "true");
                }

                newNode = new DataNode(attrName,arrayItems,length);
                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;
        }
    }
    
    return node;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ActivateDatabaseAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ActivateDatabaseRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ActivateDatabaseAction::Execute()
{
    const std::string &database = args.GetDatabase();

    //
    // Expand the database name to its full path just in case.
    //
    std::string expandedDB(database), host, db;
    GetViewerFileServer()->ExpandDatabaseName(expandedDB, host, db);

    //
    // If the database has been opened before then we can make it the active
    // plot list's active database. Then we can set the time slider if we
    // need to.
    //
    if(GetViewerFileServer()->IsDatabase(expandedDB))
    {
        const avtDatabaseMetaData *md = GetViewerFileServer()->GetMetaData(host, db);
        EngineKey newEngineKey;
        if (md && md->GetIsSimulation())
            newEngineKey = EngineKey(host, db);
        else
            newEngineKey = EngineKey(host, "");

        GetWindow()->GetPlotList()->ActivateSource(expandedDB, newEngineKey);
    }
    else
    {
        // We have not seen the database before so open it.
        OpenDatabaseHelper(database, 0, true, true, std::string());
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AlterDatabaseCorrelationAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::AlterDatabaseCorrelationRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
AlterDatabaseCorrelationAction::Execute()
{
    windowMgr->AlterDatabaseCorrelation(args.GetDatabase(),
        args.GetProgramOptions(), args.GetIntArg1(), args.GetIntArg2());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CheckForNewStatesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::CheckForNewStatesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
CheckForNewStatesAction::Execute()
{
    windowMgr->CheckForNewStates(args.GetDatabase());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CloseDatabaseAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::CloseDatabaseRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
CloseDatabaseAction::Execute()
{
    windowMgr->CloseDatabase(args.GetDatabase());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CreateDatabaseCorrelationAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::CreateDatabaseCorrelationRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
CreateDatabaseCorrelationAction::Execute()
{
    const std::string &name = args.GetDatabase();

    //
    // Make sure that the correlation does not have the same name as
    // an existing source.
    //
    if(GetViewerFileServer()->IsDatabase(name))
    {
        GetViewerMessaging()->Error(
           TR("You cannot define a database correlation that "
              "has the same name as a source. No database "
              "correlation will be created for %1.").
           arg(name));
    }
    else
    {
        windowMgr->CreateDatabaseCorrelation(
            name, args.GetProgramOptions(),
            args.GetIntArg1(), 0, args.GetIntArg2());
        windowMgr->UpdateWindowInformation(
            WINDOWINFO_TIMESLIDERS | WINDOWINFO_ANIMATION);
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeleteDatabaseCorrelationAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::DeleteDatabaseCorrelationRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
DeleteDatabaseCorrelationAction::Execute()
{
    windowMgr->DeleteDatabaseCorrelation(args.GetDatabase());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: OpenDatabaseAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::OpenDatabaseRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
OpenDatabaseAction::Execute()
{
    int ts = OpenDatabaseHelper(args.GetDatabase(),
                                args.GetIntArg1(),
                                args.GetBoolFlag(),
                                true,
                                args.GetStringArg1());
    debug5 << "OpenDatabaseAction: result=" << ts << endl;
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: OverlayDatabaseAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::OverlayDatabaseRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
OverlayDatabaseAction::Execute()
{
    int state = args.GetIntArg1();
    debug4 << "OverlayDatabase: db=" << args.GetDatabase()
           << ", time=" << state << endl;

    //
    // First open the database.
    //
    OpenDatabaseHelper(args.GetDatabase(), 
                       state, false, true, std::string());

    //
    // Now perform the database replacement.
    //
    ViewerPlotList *plotList = GetWindow()->GetPlotList();
    plotList->OverlayDatabase(plotList->GetEngineKey(),
                              plotList->GetDatabaseName(), state);

    //
    // Recenter the active window's view and redraw.
    //
    windowMgr->RecenterView();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ReOpenDatabaseAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ReOpenDatabaseRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ReOpenDatabaseAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    std::string hostDatabase(args.GetDatabase());
    bool forceClose = (args.GetIntArg1() == 1);

    //
    // Expand the filename.
    //
    std::string host, db;
    GetViewerFileServer()->ExpandDatabaseName(hostDatabase, host, db);
    debug1 << "Reopening " << hostDatabase.c_str() << endl;

    //
    // Clear default SIL restrictions
    //
    ViewerPlotList::ClearDefaultSILRestrictions(host, db);

    //
    // Clear out any previous information about the file on the mdserver.
    //
    if (forceClose)
        GetViewerFileServer()->CloseFile(host);

    //
    // Try to determine the time state at which the file should be
    // reopened. If the plot list has an active time slider, see if
    // the time slider's correlation includes the database that we're
    // reopening. If so, then we can use the active time slider's
    //
    ViewerPlotList *plotList = GetWindow()->GetPlotList();
    DatabaseCorrelationList *cL = GetViewerState()->GetDatabaseCorrelationList();

    int reOpenState = 0;
    if(plotList->HasActiveTimeSlider())
    {
        const std::string &activeTimeSlider = plotList->GetActiveTimeSlider();
        debug3 << "Reopening " << hostDatabase.c_str()
               << " with an active time slider: " << activeTimeSlider.c_str()
               << endl;
        DatabaseCorrelation *correlation = cL->FindCorrelation(activeTimeSlider);
        if(correlation != 0)
        {
            int state = 0, nStates = 0;
            plotList->GetTimeSliderStates(activeTimeSlider, state, nStates);
            reOpenState = correlation->GetCorrelatedTimeState(hostDatabase, state);
            debug3 << "The active time slider was a correlation involving "
                   << hostDatabase.c_str()
                   << " so we're using the correlated state to reopen the file."
                   << " state = " << reOpenState << endl;
        }
    }

    if(reOpenState < 0)
    {
        // There either was no active time slider, no correlation for the
        // active time slider or there was a correlation for the active time
        // slider but it had nothing to do with the database that we want to
        // reopen. We should try and use the active time slider for the
        // database we're trying to open if there is such a time slider.
        int ns;
        plotList->GetTimeSliderStates(hostDatabase, reOpenState, ns);
        debug3 << "Could not use correlation or active time slider to "
               << "get the reopen state for " << hostDatabase.c_str()
               << ". Using state " << reOpenState << endl;
    }

    //
    // Get the flag to determine if this is a simulation before we
    // clear the metadata from the file server.
    //
    bool isSim = false;
    const avtDatabaseMetaData *md = GetViewerFileServer()->GetMetaData(host, db);
    if (md && md->GetIsSimulation())
        isSim = true;

    //
    // Clear out any local information that we've cached about the file. We
    // have to do this after checking for the correlation because this call
    // will remove the correlation for the database.  Do not clear the
    // metadata if it is a simulation because we lose all of our current
    // information by doing so (since the mdserver has almost no information).
    // If it is a simulation, we will get updated metadata indirectly when
    // we open the database again, regardless of if we clear the cached one.
    //
    if (!isSim)
    {
        GetViewerFileServer()->ClearFile(hostDatabase, false);
        GetViewerStateManager()->GetDatabaseCorrelationMethods()->
            RemoveDatabaseCorrelation(hostDatabase);
    }

    //
    // Tell the compute engine to clear any cached information about the
    // database so it forces the networks to re-execute.
    //
    EngineKey key(host, "");
    if (isSim)
        key = EngineKey(host, db);

    //
    // Clear the cache for the database.
    //
    GetViewerEngineManager()->ClearCache(key, db);

    //
    // Open the database. Since reopening a file can result in a different
    // number of time states (potentially fewer), use the time state returned
    // by OpenDatabaseHelper for the replace operation.
    //
    reOpenState = OpenDatabaseHelper(hostDatabase, reOpenState, false, true, std::string());

    //
    // Now perform the database replacement in all windows that use the
    // specified database.
    //
    windowMgr->ReplaceDatabase(key, db, reOpenState, false, true, false);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ReplaceDatabaseAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ReplaceDatabaseRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   Brad Whitlock, Wed Dec 4 16:25:43 PST 2002
//   I changed it so view recentering does not happen unless the it is
//   enabled in the window.
//
//   Brad Whitlock, Tue Feb 11 12:01:37 PDT 2003
//   I made it use STL strings.
//
//   Brad Whitlock, Thu Apr 3 10:32:43 PDT 2003
//   I made some changes that allow for better animation behavior.
//
//   Eric Brugger, Fri Apr 18 12:46:00 PDT 2003
//   I replaced auto center with maintain view.
//
//   Brad Whitlock, Thu May 15 13:30:27 PST 2003
//   I made it use OpenDatabaseHelper.
//
//   Brad Whitlock, Wed Oct 15 15:40:44 PST 2003
//   I made it possible to replace a database at a later time state.
//
//   Brad Whitlock, Mon Nov 3 09:50:21 PDT 2003
//   I changed a flag to false in the call to OpenDatabaseHelper so the
//   animation's number of frames would not be updated because this caused
//   extra work. I passed the time state to the plot list's ReplaceDatabase
//   method instead.
//
//   Eric Brugger, Mon Dec  8 08:09:54 PST 2003
//   I added a call to turn on view limit merging if the new database
//   was the same as the old one.  I also made the test controlling
//   the call to recenter view more restrictive, also requiring the
//   window to be in 3d mode.
//
//   Brad Whitlock, Tue Jan 27 16:52:40 PST 2004
//   Changed for multiple time sliders.
//
//   Brad Whitlock, Mon Apr 19 10:00:13 PDT 2004
//   I added another argument to OpenDatabaseHelper so it won't update the
//   window information since we're already doing that here. I also added
//   a call to validate the plot lists's time slider.
//
//   Brad Whitlock, Mon May 3 13:21:19 PST 2004
//   I made it use the plot list's engine key in the call to ReplaceDatabase.
//
//   Brad Whitlock, Thu Feb 3 11:07:53 PDT 2005
//   I made the time state used for file replacement be the value that is now
//   returned from OpenDatabaseHelper.
//
//   Brad Whitlock, Wed Mar 16 16:32:14 PST 2005
//   I added code to make sure that the active window is still compatible
//   with other time-locked windows after the database was replaced.
//
//    Cyrus Harrison, Tue Apr 14 13:35:54 PDT 2009
//    Changed the interface to ReplaceDatabase to support option for only 
//    replacing active plots.
//   
// ****************************************************************************

void
ReplaceDatabaseAction::Execute()
{
    debug4 << "ReplaceDatabase: db=" 
           << args.GetDatabase()
           << ", time=" << args.GetIntArg1() 
           << ", onlyReplaceActive=" 
           << args.GetIntArg2() << endl;

    int timeState = args.GetIntArg1();
    bool onlyReplaceActive = (bool) args.GetIntArg2();

    //
    // If the replace is merely changing the timestate, then turn on
    // view limit merging.
    //
    if (args.GetDatabase() == GetWindow()->GetPlotList()->GetHostDatabaseName())
        GetWindow()->SetMergeViewLimits(true);

    //
    // First open the database.
    // 
    timeState = OpenDatabaseHelper(args.GetDatabase(), timeState,
                                   false, false, std::string());

    //
    // Now perform the database replacement.
    //
    ViewerPlotList *plotList = GetWindow()->GetPlotList();
    plotList->ReplaceDatabase(plotList->GetEngineKey(),
                              plotList->GetDatabaseName(),
                              timeState,
                              true,
                              false,
                              onlyReplaceActive);

    //
    // If the current window is time-locked then we have to make sure that
    // its new database is compatible with being time-locked with other
    // windows.
    //
    if(GetWindow()->GetTimeLock())
    {
        intVector windowIds;
        windowMgr->GetTimeLockedWindowIndices(windowIds);
        if(!windowIds.empty())
        {
            debug2 << "We have more than 1 time locked window. We have to "
                      "make sure that we have a suitable multi-window "
                      "database correlation."
                   << endl;
            // Create or alter the most suitable correlation to be used for
            // time-locked windows. If we have to create a multi-window
            // database correlation then we'll have set
            DatabaseCorrelation *C = windowMgr->CreateMultiWindowCorrelation(windowIds);
            if(C != 0)
            {
                std::string hdb(GetWindow()->GetPlotList()->GetHostDatabaseName());
                debug2 << "ReplaceDatabase: The active window is time-locked "
                          "and uses the multi-window database correlation: "
                       << C->GetName().c_str() << ". We have to make sure that "
                       << "we display time state " << timeState << " for the "
                       << "database: " << hdb.c_str() << endl;

                // We have to find the time state in C where we find
                // the new database's new state. Then we have to make
                // the other time locked windows go to that time state.
                int cts = C->GetInverseCorrelatedTimeState(hdb, timeState);
                if(cts != -1)
                {
                    debug2 << "Correlation state "<< cts
                           << " will allow us to show time state "
                           << timeState << " for database " << hdb.c_str()
                           << ". We also have to update the other locked "
                              "windows.\n";
                    windowMgr->SetFrameIndex(cts);
                }
            } // C != 0
        }
    }

    //
    // Make sure the time slider is set to something appropriate.
    //
    plotList->ValidateTimeSlider();

    //
    // We have to send back the source and the time sliders since we
    // could have replaced at a later time state.
    //
    windowMgr->UpdateWindowInformation(WINDOWINFO_SOURCE | WINDOWINFO_TIMESLIDERS);

    //
    // Recenter the active window's view and redraw.
    //
    if(!GetWindow()->GetMaintainViewMode() && (GetWindow()->GetWindowMode() == WINMODE_3D))
        windowMgr->RecenterView();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RequestMetaDataAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::RequestMetaDataRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
RequestMetaDataAction::Execute()
{
    const char *mName = "ViewerSubject::HandleRequestMetaData: ";

    //
    // Expand the new database name and then set it into the plot list.
    //
    std::string hdb(args.GetDatabase()), host, db;
    ViewerFileServerInterface *fs = GetViewerFileServer();
    fs->ExpandDatabaseName(hdb, host, db);

    //
    // Get the number of time states and set that information into the
    // active animation. The mdserver will clamp the time state that it
    // uses to open the database if timeState is out of range at this point.
    //
    const avtDatabaseMetaData *md = NULL;
    int ts = args.GetStateNumber();
    if(ts == -1)
    {
        debug4 << mName << "Calling fs->GetMetaData(" << host
               << ", " << db << ", true)" << endl;
        md = fs->GetMetaData(host, db, true);
    }
    else
    {
        // The time state is not "ANY" so we will ask for a particular
        // database state. Some file formats can be time-varying or
        // time-invariant and we don't know which it will be so we
        // need to clear the metadata for the database so we will read
        // it again at the right time state.
        // Don't forget which plugin you used, though.
        fs->ClearFile(hdb, false);
        GetViewerStateManager()->GetDatabaseCorrelationMethods()->
            RemoveDatabaseCorrelation(hdb);

        debug4 << mName << "Calling fs->GetMetaDataForState(" << host
               << ", " << db << ", " << ts << ", \"\")" << endl;
        md = fs->GetMetaDataForState(host, db, ts, true, "");
    }

    if(md != 0)
    {
        // Copy the metadata so we can send it to the client.
        *GetViewerState()->GetDatabaseMetaData() = *md;

        // Print the metadata to the debug logs.
        debug5 << mName << "Metadata contains: " << endl;
        if(DebugStream::Level5())
            md->Print(DebugStream::Stream5(), 1);
    }
    else
    {
        // Empty out the metadata.
        *GetViewerState()->GetDatabaseMetaData() = avtDatabaseMetaData();

        debug5 << mName << "No metadata was found." << endl;
    }
    GetViewerState()->GetDatabaseMetaData()->SelectAll();
    GetViewerState()->GetDatabaseMetaData()->Notify();

    //
    // Check to see if there were errors in the mdserver
    //
    std::string err = fs->GetPluginErrors(host);
    if (!err.empty())
    {
        GetViewerMessaging()->Warning(err);
    }
}
