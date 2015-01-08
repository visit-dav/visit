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
#include <ViewerDatabaseCorrelationMethods.h>

#include <ViewerFileServerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerState.h>
#include <ViewerText.h>

#include <avtDatabaseMetaData.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DataNode.h>
#include <DebugStream.h>

#include <snprintf.h>

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::ViewerDatabaseCorrelationMethods
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 11:00:20 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerDatabaseCorrelationMethods::ViewerDatabaseCorrelationMethods() : ViewerBase(),
    declinedFiles(), declinedFilesLength()
{
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::~ViewerDatabaseCorrelationMethods
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 11:00:35 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerDatabaseCorrelationMethods::~ViewerDatabaseCorrelationMethods()
{
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::CreateDatabaseCorrelation
//
// Purpose: 
//   Creates and returns a database correlation that contains the specified
//   databases.
//
// Arguments:
//   name    : The name of the new correlation.
//   dbs     : The databases to include in the correlation.
//   method  : The correlation method.
//   nStates : The number of states (optional)
//
// Returns:    A pointer to a new database correlation or 0 if one could
//             not be created.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 22:58:13 PST 2004
//
// Modifications:
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added logic to decide if we need to force reading of all cycles
//   and times when getting meta data
//
//   Brad Whitlock, Tue Apr 29 15:00:26 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

DatabaseCorrelation *
ViewerDatabaseCorrelationMethods::CreateDatabaseCorrelation(const std::string &name,
    const stringVector &dbs, int method, int nStates)
{
    if(dbs.size() < 1)
    {
        GetViewerMessaging()->Error(
            TR("VisIt cannot create a database correlation that does "
               "not use any databases."));
        return 0;
    }

    //
    // Create a new correlation and set its basic attributes.
    //
    DatabaseCorrelation *correlation = new DatabaseCorrelation;
    correlation->SetName(name);
    DatabaseCorrelation::CorrelationMethod m =
        (DatabaseCorrelation::CorrelationMethod)method;
    correlation->SetMethod(m);
    if(nStates != -1 &&
       (m == DatabaseCorrelation::IndexForIndexCorrelation ||
        m == DatabaseCorrelation::StretchedIndexCorrelation))
    {
        correlation->SetNumStates(nStates);
    }

    // Add the different databases to the correlation.
    for(size_t i = 0; i < dbs.size(); ++i)
    {
        //
        // Split the database name into host and database components
        // and expand it too.
        //
        std::string host, db;
        std::string correlationDB(dbs[i]);
        GetViewerFileServer()->ExpandDatabaseName(correlationDB, host, db);

        //
        // Get the metadata for the database.
        //
        const avtDatabaseMetaData *md = GetViewerFileServer()->GetMetaData(host, db);

        //
        // We might need to re-acqure metadata if we are doing
        // time or cycle correlations and times or cycles are not
        // all accurate and valid
        //
        if(md)
        {
            const bool forceReadAllCyclesAndTimes = true;

            if ((m == DatabaseCorrelation::TimeCorrelation &&
                 md->AreAllTimesAccurateAndValid() == false) ||
                (m == DatabaseCorrelation::CycleCorrelation &&
                 md->AreAllCyclesAccurateAndValid() == false))
            {
                md = GetViewerFileServer()->GetMetaData(host, db, forceReadAllCyclesAndTimes);
            }
        }

        if(md)
        {
            //
            // Issue warning messages if we're doing time or cycle
            // correlations and the metadata cannot be trusted.
            //
            if(m == DatabaseCorrelation::TimeCorrelation)
            {
                bool accurate = true;
                for(int j = 0; j < md->GetNumStates() && accurate; ++j)
                    accurate &= md->IsTimeAccurate(j);

                if(!accurate)
                {
                    GetViewerMessaging()->Warning(
                        TR("The times for %1 may not be accurate so the new "
                           "correlation %2 might not work as expected.").
                          arg(correlationDB).
                          arg(name));
                }
            }
            else if(m == DatabaseCorrelation::CycleCorrelation)
            {
                bool accurate = true;
                for(int j = 0; j < md->GetNumStates() && accurate; ++j)
                    accurate &= md->IsCycleAccurate(j);

                if(!accurate)
                {
                    GetViewerMessaging()->Warning(
                        TR("The cycles for %1 may not be accurate so the new "
                           "correlation %2 might not work as expected.").
                        arg(correlationDB).
                        arg(name));
                }
            }

            //
            // Add the database to the new correlation.
            //
            correlation->AddDatabase(correlationDB, md->GetNumStates(),
                md->GetTimes(), md->GetCycles()); 
        }
        else
        {
            delete correlation; correlation = 0;
            GetViewerMessaging()->Error(
                TR("VisIt could not retrieve metadata for %1 so the "
                   "correlation %2 could not be created.").
                  arg(correlationDB).
                  arg(name));
            break;
        }
    }

    return correlation;
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::RemoveDatabaseCorrelation
//
// Purpose:
//   Removes a database correlation and notifies.
//
// Arguments:
//   cName : The name of the database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 11:05:10 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerDatabaseCorrelationMethods::RemoveDatabaseCorrelation(const std::string &cName)
{
    // Remove the correlation
    if(GetViewerState()->GetDatabaseCorrelationList()->RemoveCorrelation(cName))
    {
        debug4 << "\tDeleted database correlation for " 
               << cName.c_str() << endl;
        GetViewerState()->GetDatabaseCorrelationList()->Notify();
    }
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::UpdateDatabaseCorrelation
//
// Purpose: 
//   Updates a the named database correlation by recreating it using its
//   input databases and storing the result over the old database correlation.
//
// Arguments:
//   cName : The name of the database correlation to update.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 27 10:36:35 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerDatabaseCorrelationMethods::UpdateDatabaseCorrelation(const std::string &cName)
{
    DatabaseCorrelation *c = GetViewerState()->GetDatabaseCorrelationList()->FindCorrelation(cName);
    if(c != 0)
    {
        DatabaseCorrelation *replacementCorrelation = CreateDatabaseCorrelation(
            c->GetName(), c->GetDatabaseNames(), (int)c->GetMethod());
        if(replacementCorrelation != 0)
        {
            // Copy over the old database correlation.
            *c = *replacementCorrelation;
        }
    }
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::GetMostSuitableCorrelation
//
// Purpose: 
//   Returns a pointer to the correlation that most matches the list of 
//   databases.
//
// Arguments:
//   dbs : The list of databases for which we want a correlation.
//
// Returns:    A pointer to the most suitable correlation or 0 if there is
//             no suitable correlation.
//
// Note:       This method will not return a pointer to a trivial correlation
//             unless that is the only database in the dbs list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 08:57:49 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

DatabaseCorrelation *
ViewerDatabaseCorrelationMethods::GetMostSuitableCorrelation(const stringVector &dbs) const
{
    //
    // Score the correlations as to how many of the input databases they
    // contain.
    //
    std::map<std::string,int> scores;
    for(int i = 0; i < GetViewerState()->GetDatabaseCorrelationList()->GetNumCorrelations();
        ++i)
    {
        const DatabaseCorrelation &c = GetViewerState()->GetDatabaseCorrelationList()->
            GetCorrelations(i);
        scores[c.GetName()] = 0;
        for(size_t j = 0; j < dbs.size(); ++j)
        {
            if(c.UsesDatabase(dbs[j]))
                ++scores[c.GetName()];
        }
    }

#if 0
    debug3 << "Scores: " << endl;
    for(std::map<std::string,int>::const_iterator sIt = scores.begin(); sIt != scores.end(); ++sIt)
        debug3 << "\t" << sIt->first << ", score= " << sIt->second << endl;
#endif

    //
    // Look for any databases with a score of dbs.size() and then go down
    // to the correlation with the next highest score.
    //
    std::string correlationName;
    int score = 0;
    for(int desiredScore = (int)dbs.size(); desiredScore > 1 && score == 0;
        --desiredScore)
    {
        for(std::map<std::string,int>::const_iterator pos = scores.begin();
            pos != scores.end(); ++pos)
        {
            if(pos->second == desiredScore)
            {
                score = desiredScore;
                correlationName = pos->first;
                break;
            }
        }
    }

    //
    // If the score matches the number of databases then return that
    // correlation. It's okay to return that correlation if score > 1
    // since it means that it is not a trivial correlation.
    //
    DatabaseCorrelation *retval = 0;
    if((int)dbs.size() == score || score > 1)
        retval = GetViewerState()->GetDatabaseCorrelationList()->FindCorrelation(correlationName);

    return retval;
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::CreateNewCorrelationName
//
// Purpose: 
//   Gets the next correlation name in the series.
//
// Returns:    The next automatically generated name for a database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 08:56:09 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

std::string
ViewerDatabaseCorrelationMethods::CreateNewCorrelationName() const
{
    int index = 0;
    char newName[100];
    do
    {
        index++;
        SNPRINTF(newName, 100, "Correlation%02d", index);
    } while(GetViewerState()->GetDatabaseCorrelationList()->FindCorrelation(newName) != 0);

    return std::string(newName);
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::PreviouslyDeclinedCorrelationCreation
//
// Purpose: 
//   Returns whether the user previously declined to create a database 
//   correlation for a specified set of files.
//
// Arguments:
//   dbs : The list of files that are in the correlation.
//
// Returns:    True if the user did not want to create a correlation; false
//             if they never declined correlation creation.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 13 23:27:53 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerDatabaseCorrelationMethods::PreviouslyDeclinedCorrelationCreation(
    const stringVector &dbs) const
{
    int index = 0;
    for(size_t fileSet = 0; fileSet < declinedFilesLength.size();
        ++fileSet)
    {
        if(declinedFilesLength[fileSet] == (int)dbs.size())
        {
            bool same = true;
            for(int i = 0; i < declinedFilesLength[fileSet] && same; ++i, ++index)
                same &= (std::find(dbs.begin(), dbs.end(), declinedFiles[index])
                         != dbs.end());

            if(same)
                return true;
        }
    }

    return false;
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::DeclineCorrelationCreation
//
// Purpose: 
//   Tell VisIt that it should not allow automatic correlations containing
//   the files in the passed in list of files.
//
// Arguments:
//   dbs : The list of files.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 13 23:29:32 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerDatabaseCorrelationMethods::DeclineCorrelationCreation(const stringVector &dbs)
{
    if(dbs.size() > 0)
    {
        for(size_t i = 0; i < dbs.size(); ++i)
            declinedFiles.push_back(dbs[i]);
        declinedFilesLength.push_back((int)dbs.size());
    }
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::CreateNode
//
// Purpose: 
//   Tells the database correlation list to save its settings to the data
//   node that was passed in.
//
// Arguments:
//   parentNode : the node on which to save the settings.
//   detailed   : Whether we're saving a session file.
//
// Note:       Correlations are only saved in session files.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 16:50:05 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Nov 9 16:36:16 PST 2006
//   I added code to make sure that database correlations use the source
//   ids instead of databases when their information is saved. We will fill
//   in the blanks for the correlation when we read it back in.
//
//   Jeremy Meredith, Fri Jan 29 11:34:09 EST 2010
//   Save the database plugin map to the node.
//
// ****************************************************************************

void
ViewerDatabaseCorrelationMethods::CreateNode(DataNode *parentNode, 
    const std::map<std::string, std::string> &dbToSource, bool detailed)
{
    // Create a copy of the database correlation list.
    DatabaseCorrelationList dbcl(*GetViewerState()->GetDatabaseCorrelationList());
    dbcl.ClearCorrelations();
    for(int i = 0; 
        i < GetViewerState()->GetDatabaseCorrelationList()->GetNumCorrelations(); ++i)
    {
        const DatabaseCorrelation &corr = 
            GetViewerState()->GetDatabaseCorrelationList()->GetCorrelations(i);
        // Let's only save out correlations that have more than 1 db.
        if(corr.GetNumDatabases() > 1)
        {
            // Map database names to source names.
            const stringVector &dbNames = corr.GetDatabaseNames();
            stringVector sourceIds;
            for(size_t j = 0; j < dbNames.size(); ++j)
            {
                std::map<std::string, std::string>::const_iterator pos =
                    dbToSource.find(dbNames[j]);
                if(pos == dbToSource.end())
                    sourceIds.push_back(dbNames[j]);
                else
                    sourceIds.push_back(pos->second);
            }

            // Create a copy of the correlation but override its database
            // names with source ids. Also, note that we're not using a
            // copy constructor because we want most of the correlation
            // information to be absent so we can repopulate it on session read
            // in case a new database is chosen.
            DatabaseCorrelation modCorr;                       
            if(corr.GetMethod() != DatabaseCorrelation::UserDefinedCorrelation)
            {
                modCorr.SetName(corr.GetName());
                modCorr.SetNumStates(corr.GetNumStates());
                modCorr.SetMethod(corr.GetMethod());
            }
            else
            {
               // A user-defined correlation should be saved mostly as-is.
               modCorr = corr;
            }
            modCorr.SetDatabaseNames(sourceIds);

            // Add the modified correlation to the list.
            dbcl.AddCorrelations(modCorr);
        }
    }

    // Add the database correlation list information to the session.
    dbcl.CreateNode(parentNode, detailed, false);
}

// ****************************************************************************
// Method: ViewerDatabaseCorrelationMethods::SetFromNode
//
// Purpose: 
//   Initializes the database correlation list.
//
// Arguments:
//   parentNode : The node on which to look for attribute nodes.
//   sourceToDB : The map of source ids -> database names.
//   configVersion : The version from the config file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 25 16:49:01 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Nov 10 13:44:40 PST 2006
//   I rewrote the method to handle creation of database correlations 
//   using replacement sources.
//
//   Brad Whitlock, Fri Oct 23 16:11:33 PDT 2009
//   I added code to remove all metadata, SILs, and database correlations.
//
//   Jeremy Meredith, Fri Jan 29 10:27:29 EST 2010
//   Added knowledge of what plugin was used to open a file.  Clear it here.
//
//   Jeremy Meredith, Fri Jan 29 11:33:45 EST 2010
//   Load the database->plugin map from the node.
//
// ****************************************************************************

void
ViewerDatabaseCorrelationMethods::SetFromNode(DataNode *parentNode,
    const std::map<std::string,std::string> &sourceToDB,
    const std::string &configVersion)
{
    GetViewerState()->GetDatabaseCorrelationList()->ClearCorrelations();

    // Load the correlations
    DataNode *cLNode = parentNode->GetNode("DatabaseCorrelationList");
    if(cLNode != 0)
    {
        // Read the database correlation list into a temporary.
        DatabaseCorrelationList dbcl;
        dbcl.ProcessOldVersions(parentNode, configVersion.c_str());
        dbcl.SetFromNode(parentNode);

        // Since the database correlations in the temporary may
        // not have complete information, let's recreate the 
        // database correlations.
        GetViewerState()->GetDatabaseCorrelationList()->ClearCorrelations();
        for(int i = 0; i < dbcl.GetNumCorrelations(); ++i)
        {
            const DatabaseCorrelation &corr = dbcl[i];
            if(corr.GetMethod() != DatabaseCorrelation::UserDefinedCorrelation)
            {
                // Translate the source names in the database correlation into
                // database names using the sourceToDB map.
                stringVector dbNames;
                const stringVector &sourceNames = corr.GetDatabaseNames();
                for(size_t j = 0; j < sourceNames.size(); ++j)
                {
                    std::map<std::string,std::string>::const_iterator pos =
                        sourceToDB.find(sourceNames[j]);
                    if(pos != sourceToDB.end())
                        dbNames.push_back(pos->second);
                    else
                        dbNames.push_back(sourceNames[j]);
                }

                // Create a new database correlation based on the inputs for the
                // one that we read from the session file.
                DatabaseCorrelation *newCorr = CreateDatabaseCorrelation(
                    corr.GetName(), dbNames, corr.GetMethod(), -1);

                // If we were able to create a database correlation then add it
                // to the database correlation list.
                if(newCorr != 0)
                {
                    GetViewerState()->GetDatabaseCorrelationList()->AddCorrelations(*newCorr);
                    delete newCorr;
                }
            }
            else
            {
                // We read in a user-defined database correlation. 
                // Don't mess with it.
                GetViewerState()->GetDatabaseCorrelationList()->AddCorrelations(corr);
            }
        }
              
        //
        // Now that the correlation list contains valid correlations,
        // notify clients.
        //
        GetViewerState()->GetDatabaseCorrelationList()->Notify();
    }
}
