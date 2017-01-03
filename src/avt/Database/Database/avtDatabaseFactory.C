/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                          avtDatabaseFactory.C                             //
// ************************************************************************* //

#include <avtDatabaseFactory.h>
#include <visit-config.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

#include <visitstream.h>
#include <sys/stat.h>

#include <visitstream.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtGenericDatabase.h>

#include <DatabasePluginManager.h>
#include <DatabasePluginInfo.h>
#include <FileFunctions.h>
#include <DBOptionsAttributes.h>
#include <StringHelpers.h>

#include <BadPermissionException.h>
#include <DBYieldedNoDataException.h>
#include <FileDoesNotExistException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidDBTypeException.h>
#include <NonCompliantException.h>
#include <NonCompliantFileException.h>
#include <TimingsManager.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <snprintf.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

using std::string;
using std::vector;
using StringHelpers::Plural;
using StringHelpers::HumanReadableList;


//
// Static data members
//
bool    avtDatabaseFactory::createMeshQualityExpressions = true;
bool    avtDatabaseFactory::createTimeDerivativeExpressions = true;
bool    avtDatabaseFactory::createVectorMagnitudeExpressions = true;
bool    avtDatabaseFactory::checkFilePermissions = true;
bool    avtDatabaseFactory::removeDuplicateNodes = false;
FileOpenOptions avtDatabaseFactory::defaultFileOpenOptions;
avtPrecisionType avtDatabaseFactory::precisionType = AVT_PRECISION_NATIVE;
avtBackendType avtDatabaseFactory::backendType = AVT_BACKEND_VTK;

//
// Function Prototypes
//
static void CheckPermissions(const char *);


// ****************************************************************************
//  Method:  avtDatabaseFactory::SetDefaultFileOpenOptions
//
//  Purpose:
//    Store off the default file opening options.  We use this when we
//    initialize database plugin readers.
//
//  Arguments:
//    opts       the new default options
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
// ****************************************************************************

void
avtDatabaseFactory::SetDefaultFileOpenOptions(const FileOpenOptions &opts)
{
    defaultFileOpenOptions = opts;
}

// ****************************************************************************
//  Method:  avtDatabaseFactory::SetPrecisionType
//
//  Purpose:
//    Store off the precisionType.  We use this when XForm Manager decides
//    whether or not to tranform data.
//
//  Arguments:
//    pType      the new precision type.
//
//  Programmer:  Kathleen Biagas
//  Creation:    August 1, 2013
//
// ****************************************************************************

void
avtDatabaseFactory::SetPrecisionType(const int pType)
{
    if (pType >= 0 && pType < 3)
        precisionType = avtPrecisionType(pType);
}

// ****************************************************************************
//  Method:  avtDatabaseFactory::SetBackendType
//
//  Purpose:
//    Store off the backendType.
//
//  Arguments:
//    bType      the new backend type.
//
//  Programmer:  Cameron Christensen
//  Creation:    June 11, 2014
//
// ****************************************************************************

void
avtDatabaseFactory::SetBackendType(const int bType)
{
    if (bType >= 0 && bType < 4)
        backendType = avtBackendType(bType);
}

// ****************************************************************************
//  Method: avtDatabaseFactory::FileList
//
//  Purpose:
//      Looks through the file list and tries to guess what the appropriate
//      derived type of database the files correspond to.
//
//  Arguments:
//      filelist     A list of files.
//      filelistN    The number of files in `filelist'.
//      timestep     The timestep to open to.
//      plugins      The plugins types that were tried (output argument)
//      format       The format to try first.
//
//  Returns:    An avtDatabase object that is correctly typed for the filelist.
//
//  Note:       It is assumed that the file list is homogeneous.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Aug 27 14:30:22 PDT 2002
//    Made databases be plugins.
//
//    Hank Childs, Mon Oct 14 08:48:40 PDT 2002
//    Allow for multiple formats to support the same extension.  Blew away
//    outdated comments.
//
//    Hank Childs, Thu Feb 19 08:00:35 PST 2004
//    Allow for exact filenames in addition to default extensions.
//
//    Hank Childs, Mon Mar  1 08:56:52 PST 2004
//    Allow for the time to be specified.
//
//    Hank Childs, Fri Mar  5 11:25:47 PST 2004
//    Set the file format type with the database meta data.
//
//    Mark C. Miller, Tue Mar 16 09:38:19 PST 2004
//    Just prior to calls to GetMetaData, added calls to ActivateTimestep.
//
//    Hank Childs, Mon Mar 22 09:43:33 PST 2004
//    Added the ability for a format to be specified.
//
//    Brad Whitlock, Fri Apr 30 16:05:42 PST 2004
//    I made extension comparison be case insensitive on Windows.
//
//    Hank Childs, Sun May  9 11:41:45 PDT 2004
//    Use the user-settable default format, rather than a hard-coded "Silo".
//
//    Brad Whitlock, Thu May 27 18:12:45 PST 2004
//    I fixed a bug that I introduced into the Windows extension comparison.
//
//    Jeremy Meredith, Tue Aug 24 17:59:24 PDT 2004
//    Vastly improved the error messages when failing to open a file.
//    The extra error detection prevented crashes in some common situations.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool arg to forceReadAllCyclesTimes
//
//    Hank Childs, Thu Jan 11 15:56:53 PST 2007
//    Added argument for which plugin types were tried.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Replaced sprintfs with SNPRINTFs. Adjusted to accomodate possible
//    null return from GetCommonPluginInfo. Moved bulk of code to match
//    on file extensions and filenames to DatabasePluginManager
//
//    Mark C. Miller, Thu Aug  9 09:16:01 PDT 2007
//    Made GetMatchingPluginIds return a vector of possible ids
//
//    Hank Childs, Fri Oct  5 16:28:42 PDT 2007
//    Catch any type of exception that a file format may throw.
//
//    Jeremy Meredith, Wed Jan 23 16:15:41 EST 2008
//    Set the options from the current set of default options before
//    calling SetupDatabase, no matter how we determined which plugin
//    we're using.
//
//    Jeremy Meredith, Wed Mar 19 14:09:36 EDT 2008
//    Renamed defaultFormat to fallbackFormat for clarity.
//    Also, allow an assumed format to fail and fall back on other readers.
//
//    Jeremy Meredith, Wed Apr  2 12:46:52 EDT 2008
//    Adding a little debug info for watching format attempts.
//
//    Brad Whitlock, Tue Jun 24 15:33:58 PDT 2008
//    Pass in the database plugin manager since it's no longer a singleton.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
//
//    Hank Childs, Thu Dec 17 17:49:54 PST 2009
//    If a plugin was extended to include read options, and if the user
//    previously had saved their settings, then ignore the empty options
//    and get the options directly from the plugin.
//
//    Brad Whitlock, Tue Dec 22 10:14:23 PDT 2009
//    I added a guard against using options that could be NULL.
//
//    Jeremy Meredith, Tue Dec 29 17:03:57 EST 2009
//    Attempt to open using *all* plugins which match a given file's name.
//    If more than one can set up the database without error, present a warning
//    to the user.  The first one found still wins, however.
//
//    Jeremy Meredith, Wed Dec 30 11:41:46 EST 2009
//    Improved the warning about multiple successful plugins.
//    Also, use *all* remaining file format reader plugins as fallback formats.
//
//    Jeremy Meredith, Wed Jan  6 15:36:13 EST 2010
//    Small fix to warning messages.
//
//    Jeremy Meredith, Thu Jan  7 15:36:03 EST 2010
//    Fixed a typo.
//
//    Jeremy Meredith, Fri Jan  8 16:12:42 EST 2010
//    Added a new strict file reading mode used when we're trying to
//    determine what type of file something is.
//
//    Jeremy Meredith, Fri Jan 15 17:25:43 EST 2010
//    Became concerned that too many people rely on there being a small
//    set of fallback formats.  (Reference the lack of Silo file
//    formatting consistency even among its primary users.)  So I changed
//    it so that preferred plugins are not "assumed", but instead "fallbacks".
//    Also decided it was smart to make all formats do strict parsing for
//    now, even when their filename matches, to make sure that we get to
//    a user's "preferred" plugins when we should.  There is currently no
//    warning when we fall back to a preferred format, because I expect this
//    to happen quite often now.
//
//    Jeremy Meredith, Mon Feb  1 09:52:40 EST 2010
//    Added try/catch block for when using a specific, forced format.
//    Don't know if it's necessary, but all the other SetupDatabase calls
//    were wrapped with try/catches, so it seemed the logical thing.
//
//    Brad Whitlock, Tue Feb 2 14:37:34 PDT 2010
//    Delete a database pointer in the event that a 2nd reader could open
//    the data file too.
//
//    Brad Whitlock, Wed Mar 10 10:50:00 PST 2010
//    Rethrow a caught ImproperUseException so its message won't be changed.
//
//    Hank Childs, Thu May 20 14:32:06 PDT 2010
//    Add support for NonCompliantFileExceptions, a new type which should 
//    immediately issue their warnings to the user.  You use these when
//    you know that you have a file of a certain type, but that type is 
//    a non-compliant file.
//
//    Jeremy Meredith, Wed Aug 18 14:32:11 EDT 2010
//    Added another pass, where we try preferred plugins which match
//    the target filename before any others which match the filename.
//    We still try preferred plugins before giving up, even if they
//    don't match the filename.
//
//    Hank Childs, Sun Sep 19 09:02:05 PDT 2010
//    Add parsing for explicit setting of times by user.
//
//    Jeremy Meredith, Thu Sep 30 12:16:00 EDT 2010
//    Allow specified file type to look for a name match, now allowing
//    "VTK" (as well as "VTK_1.0" which previously worked), for example.
//
//    Jeremy Meredith, Thu Oct 28 12:50:53 EDT 2010
//    When using "Open As", report the error given from the reader even if 
//    it throws a standard InvalidFilesException, since the details of the
//    failure will at least be relevant (unlike the general case when we
//    get that failure when guessing which plugin to use).
//
//    Mark C. Miller, Fri Oct 29 09:57:32 PDT 2010
//    Added catch/rethrow of DBYieldedNoDataException
//
//    Mark C. Miller, Mon Nov  1 11:28:33 PDT 2010
//    Fixed handling of DBYieldedNoDataException. Re-throws only when format
//    is specified. Otherwise, handled like any other exception. This ensures
//    desired behavior, where empty db moves on to next candidate, occurs.
//    Also, removed whole TRY/CATCH construct from the format!=NULL case as
//    per Jeremy's advice that none of the exceptions in that case require
//    special handling here.
//
//    Jeremy Meredith, Tue Mar 29 14:30:23 EDT 2011
//    NonCompliant exceptions are now no longer "fatal" -- i.e. the no longer
//    abort the file opening process.  However, we still take these types
//    of exceptions as special indications that their error message is
//    meaningful and should be reported to the user, so we now collect any
//    occurrences of them and report them prominently, even if a different
//    file format was able to successfully open the file.
//
//    Dave Pugmire, Thu Feb 14 13:56:46 EST 2013
//    Support for ensemble .visit files (files with identical time states)
//
//    Mark C. Miller, Wed Jan  8 18:15:01 PST 2014
//    Added some error checking for !NBLOCKS, !TIMES declarations and adjusted
//    the debug output to be a little more helpful.
//
//    Brad Whitlock, Thu Sep 18 23:02:07 PDT 2014
//    Allow file permission check to be bypassed. We don't want it for batch
//    in situ.
//
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::FileList(DatabasePluginManager *dbmgr,
                             const char * const * filelist, int filelistN,
                             int timestep, vector<string> &plugins,
                             const char *format,
                             bool forceReadAllCyclesAndTimes,
                             bool treatAllDBsAsTimeVarying,
                             int bang_nblocks)
{
    vector<string> noncompliantPlugins;
    vector<string> noncompliantErrors;

    if (filelistN <= 0)
    {
        EXCEPTION1(InvalidFilesException, filelistN);
    }

    avtDatabase *rv = NULL;
    int fileIndex = 0;

    int nBlocks = bang_nblocks > 0 ? bang_nblocks : 1;
    bool filesAreEnsemble = false;
    vector<double> times;
    for (int f = 0 ; f < filelistN ; f++)
    {
         //
         // MCM-28Aug14: All the logic for handling these special values should probably
         // be re-factored to GetFileListFromTextFile. There is already logic there
         // for !NBLOCKS and it is being duplicated here. Furthermore, the
         // list of strings passed in to FileList here is called a 'filelist' but nonetheless
         // is expected/allowed to contain strings that are certainly not files. OTOH,
         // the set of !KEYWORDS being processed here has grown on a few occasions and
         // instead of continuing to add args to the method itself for each such keyword
         // setting, maybe its ok to use the list of strings passed here as a sort of
         // catchall container for all such possible !KEYWORD settings.
         //
         if (bang_nblocks < 0 && strstr(filelist[fileIndex], "!NBLOCKS ") != NULL)
         {
             errno = 0;
             nBlocks = strtol(filelist[fileIndex] + strlen("!NBLOCKS "), 0, 10);
             if (errno != 0 || nBlocks <= 0)
             {
                 debug1 << "BAD SYNTAX FOR !NBLOCKS, \"" << filelist[fileIndex] << "\", RESETTING TO 1"  << endl;
                 nBlocks = 1;
             }
             else
             {
                 debug1 << "Found a multi-block file with " << nBlocks << " blocks." << endl;
             }
             fileIndex++;
         }
         else if (strstr(filelist[fileIndex], "!TIME ") != NULL)
         {
             char *endptr = 0;
             errno = 0;
             double time = strtod(filelist[fileIndex] + strlen("!TIME "), &endptr);
             if (errno != 0 || (time == 0.0 || endptr == filelist[fileIndex] + strlen("!TIME ")))
             {
                 debug1 << "BAD SYNTAX FOR !TIME, \"" << filelist[fileIndex] << "\", RESETTING TO ";
                 if (times.size())
                     time = times[times.size()-1];
                 else
                     time = 0.0;
                 debug1 << time << endl;
             }
             times.push_back(time);
             fileIndex++;
         }
         else if (strstr(filelist[fileIndex], "!ENSEMBLE") != NULL)
         {
             debug1 << "!ENSEMBLE FLAG ENCOUNTERED" << endl;
             filesAreEnsemble = true;
             fileIndex++;
         }
         else
         {
             // The whole file index thing mandates that the '!'s go first in the file.
             break;
         }
    }

    //
    // Make sure we can read the file before we proceed.
    //
    if(checkFilePermissions)
        CheckPermissions(filelist[fileIndex]);

    //
    // If we were specifically told which format to use, then try that now.
    //
    if (format != NULL)
    {
        debug3 << "avtDatabaseFactory: specifically told to use "
               << format << endl;
        int formatindex = dbmgr->GetAllIndex(format);
        if (formatindex < 0)
        {
            formatindex = dbmgr->GetAllIndexFromName(format);
            debug3 << "avtDatabaseFactory: couldn't find "
                   << format << " by ID, attempt at a name match "
                   << ((formatindex>=0) ? "succeeded" : "failed") << endl;
        }
        if (formatindex < 0)
        {
            char msg[1000];
            SNPRINTF(msg, sizeof(msg),
                    "The DB factory was told to open a file of type %s, "
                    "but the engine had no plugin of that type.",
                    format);
            EXCEPTION1(ImproperUseException, msg);
        }

        string formatid = dbmgr->GetAllID(formatindex);
        if (!dbmgr->PluginAvailable(formatid))
        {
            char msg[1000];
            SNPRINTF(msg, sizeof(msg),
                    "The DB factory was told to open a file of type %s, "
                    "but that format's plugin could not be loaded.",
                    format);
            EXCEPTION1(ImproperUseException, msg);
        }
        CommonDatabasePluginInfo *info = 
            dbmgr->GetCommonPluginInfo(formatid);
        if (info)
        {
            // Set the opening options
            const DBOptionsAttributes *opts = 
                defaultFileOpenOptions.GetOpenOptionsForID(formatid);
            if (opts == 0 || (opts != 0 && opts->GetNames().size() == 0))
            {
                // The options aren't in the default options.  Maybe defaults
                // have been added to the plugin since they saved their settings.
                // Try to get it from the plugin.
                DBOptionsAttributes *opts = info->GetReadOptions();
                if (opts)
                    info->SetReadOptions(opts);
            }
            else
            {
                info->SetReadOptions(new DBOptionsAttributes(*opts));
            }
        }
        plugins.push_back(info ? info->GetName(): "");
        rv = SetupDatabase(info, filelist, filelistN, timestep, fileIndex,
                           nBlocks, forceReadAllCyclesAndTimes,
                           treatAllDBsAsTimeVarying, false, times, filesAreEnsemble);
        
        if (rv == NULL)
        {
            char msg[1000];
            SNPRINTF(msg, sizeof(msg),
                    "The DB factory was told to open a file of type %s, but "
                    "that format is not a match for file %s",
                    format, filelist[0]);
            EXCEPTION1(ImproperUseException, msg);
        }
    }
 
    //
    // Check to see if there is an extension that matches.
    //
    vector<string> fileMatchedIds =
        dbmgr->GetMatchingPluginIds(filelist[fileIndex]);

    //
    // First try any preferred plugins which match, in the order given,
    // to see if they work.  If you encounter one that does, stop and
    // succeed immediately.
    //
    vector<string> &preferred = defaultFileOpenOptions.GetPreferredIDs();
    for (size_t i = 0 ; i < preferred.size() && rv == NULL ; i++)
    {
        string formatid = preferred[i];
        if (!dbmgr->PluginAvailable(formatid))
        {
            debug3 << "avtDatabaseFactory: 'assumed' pass, skipping "
                   << "unavailable preferred format " << formatid << endl;
            continue;
        }

        // For some reason, disabling of db plugins at load time is
        // apparently not working at the moment.  That's okay, we can
        // double check here that the current ID is enabled with no harm.
        if (!defaultFileOpenOptions.IsIDEnabled(formatid))
        {
            debug1 << "Warning: plugin "<<formatid<<" claims to be "
                   << "disabled but wasn't.\n";
            continue;
        }

        // don't try any which don't match the file pattern here
        if (std::find(fileMatchedIds.begin(),fileMatchedIds.end(), formatid) ==
                                                          fileMatchedIds.end())
            continue;

        debug3 << "avtDatabaseFactory: 'assumed' pass, trying "
               << "filename-matched preferred format " << formatid << endl;
        CommonDatabasePluginInfo *info = 
            dbmgr->GetCommonPluginInfo(formatid);
        if (info)
        {
            // if this is strict about filename matching, don't try it either
            if (info->AreDefaultFilePatternsStrict())
            {
                debug3 << "avtDatabaseFactory: no, skip it, since it's "
                       << "strict about file patterns.\n";
                continue;
            }

            // Set the opening options
            const DBOptionsAttributes *opts = 
                defaultFileOpenOptions.GetOpenOptionsForID(formatid);
            if (opts == 0 || (opts != 0 && opts->GetNames().size() == 0))
            {
                // The options aren't in the default options.  Maybe
                // defaults have been added to the plugin since they saved 
                // their settings. Try to get it from the plugin.
                DBOptionsAttributes *opts = info->GetReadOptions();
                if (opts)
                    info->SetReadOptions(opts);
            }
            else
            {
                info->SetReadOptions(new DBOptionsAttributes(*opts));
            }
        }
        TRY
        {
            plugins.push_back(info ? info->GetName(): "");
            rv = SetupDatabase(info, filelist, filelistN,
                               timestep, fileIndex,
                               nBlocks, forceReadAllCyclesAndTimes,
                               treatAllDBsAsTimeVarying, true, times, filesAreEnsemble);
            // If some file reader claimed this file, but couldn't open it,
            // but another one could, then report a warning.
            if (rv != NULL && noncompliantPlugins.size() > 0)
            {
                int n = (int)noncompliantPlugins.size();
                string warning = "While we were able to open the file "
                    "with the " + string(info ? info->GetName() : "") + " reader, "+
                    (n==1?"an":"") + "other file format "+Plural(n,"reader")+
                    " believed "+(n==1?"it":"they")+" should be "
                    "able to open it ("+HumanReadableList(noncompliantPlugins)+
                    ") but found the following errors, which may be worth "
                    "investigating:\n";
                for (int i=0; i<n; i++)
                    warning += noncompliantErrors[i] + "\n";
                dbmgr->ReportWarning(warning);                
            }
        }
        CATCH2(NonCompliantException, e)
        {
            rv = NULL;
            noncompliantPlugins.push_back(info ? info->GetName(): "");
            noncompliantErrors.push_back(e.Message());
        }
        CATCH2(NonCompliantFileException, e)
        {
            rv = NULL;
            noncompliantPlugins.push_back(info ? info->GetName(): "");
            noncompliantErrors.push_back(e.Message());
        }
        CATCHALL
        {
            rv = NULL;
        }
        ENDTRY
    }

    //
    // If none of the preferred plugins which matched the filename worked,
    // try any others which match the filename.  Try *all* of these, because
    // since we're just guessing with no input from the user, we want to warn
    // the user if more than one would have worked.
    //
    if (rv == NULL)
    {
        vector<string> succeeded;
        for (size_t i = 0; i < fileMatchedIds.size(); i++)
        {
            // Skip this one if it was preferred -- if we got here, it
            // already failed on the previous pass
            if (std::find(preferred.begin(),preferred.end(),fileMatchedIds[i])
                                                            != preferred.end())
                continue;

            // For some reason, disabling of db plugins at load time is
            // apparently not working at the moment.  That's okay, we can
            // double check here that the current ID is enabled with no harm.
            if (!defaultFileOpenOptions.IsIDEnabled(fileMatchedIds[i]))
            {
                debug1 << "Warning: plugin "<<fileMatchedIds[i]<<" claims to "
                       << "be disabled but wasn't.\n";
                continue;
            }

            CommonDatabasePluginInfo *info = dbmgr->GetCommonPluginInfo(fileMatchedIds[i]);
            if (info)
            {
                // Set the opening options
                const DBOptionsAttributes *opts = 
                    defaultFileOpenOptions.GetOpenOptionsForID(fileMatchedIds[i]);
                if (opts == 0 || (opts != 0 && opts->GetNames().size() == 0))
                {
                    // The options aren't in the default options.  Maybe
                    // defaults have been added to the plugin since they saved 
                    // their settings. Try to get it from the plugin.
                    DBOptionsAttributes *opts = info->GetReadOptions();
                    if (opts)
                        info->SetReadOptions(opts);
                }
                else
                {
                    info->SetReadOptions(new DBOptionsAttributes(*opts));
                }
            }
            debug3 << "avtDatabaseFactory: trying extension-matched format "
                   << fileMatchedIds[i] << endl;
            TRY
            {
                plugins.push_back(info ? info->GetName() : "");
                avtDatabase *dbtmp =
                    SetupDatabase(info, filelist, filelistN, timestep,
                                  fileIndex, nBlocks, forceReadAllCyclesAndTimes,
                                  treatAllDBsAsTimeVarying, true, times, filesAreEnsemble);
                if (dbtmp)
                {
                    succeeded.push_back(info->GetName());
                    if (rv==NULL)
                        rv = dbtmp;
                    else
                        delete dbtmp;
                }
            }
            CATCH2(NonCompliantException, e)
            {
                rv = NULL;
                noncompliantPlugins.push_back(info ? info->GetName(): "");
                noncompliantErrors.push_back(e.Message());
            }
            CATCH2(NonCompliantFileException, e)
            {
                rv = NULL;
                noncompliantPlugins.push_back(info ? info->GetName(): "");
                noncompliantErrors.push_back(e.Message());
            }
            CATCHALL
            {
            }
            ENDTRY
        }
        // If some file reader claimed this file, but couldn't open it,
        // but another one could, then report a warning.
        if (succeeded.size() > 0 && noncompliantPlugins.size() > 0)
        {

            int n = (int)noncompliantPlugins.size();
            string warning = "While we were able to open the file "
                "with the " + succeeded[0] + " reader, " + 
                (n==1?"an":"") + "other file format "+Plural(n,"reader")+
                " believed "+(n==1?"it":"they")+" should be "
                "able to open it ("+HumanReadableList(noncompliantPlugins)+
                ") but found the following errors, which may be worth "
                "investigating:\n";
            for (int i=0; i<n; i++)
                warning += noncompliantErrors[i] + "\n";
            dbmgr->ReportWarning(warning);
        }
        // And if more than one could open it, also report a warning.
        else if (succeeded.size() > 1)
        {
            string used = succeeded[0];
            string warning = "Multiple file format reader plugins (";
            warning += HumanReadableList(succeeded);
            warning += ") matched the filename and would have been able to "
                       "successfully open the file.\n\n";
            warning += "We will try the first successful one: "+used+".\n\n";
            warning += "If this is not a " + used + " file, you can "
                       "manually select the correct file format reader "
                       "when opening the file, set the correct reader as "
                       "a preferred plugin, disable the " + used +
                       " plugin, or request that the developers of the " +
                       used + " plugin add code to distinguish your file "
                       "from one of theirs.\n\n";
            warning += "If this *is* a "+used+" file, "
                       "making the "+used+" reader a preferred plugin, or "
                       "ensuring the other file reader plugins listed above "
                       "do not successfully open this type of file will "
                       "prevent this warning in the future.\n";
            dbmgr->ReportWarning(warning);
        }
    }

    //
    // If no file extension matched, then try the "preferred" ones as 
    // fallbacks, unless they are strictly unable to open the given
    // file without a matching filename.
    //
    for (size_t i = 0 ; i < preferred.size() && rv == NULL ; i++)
    {
        string formatid = preferred[i];
        if (!dbmgr->PluginAvailable(formatid))
        {
            debug3 << "avtDatabaseFactory: fallback pass, skipping "
                   << "unavailable preferred format " << formatid << endl;
            continue;
        }

        // For some reason, disabling of db plugins at load time is
        // apparently not working at the moment.  That's okay, we can
        // double check here that the current ID is enabled with no harm.
        if (!defaultFileOpenOptions.IsIDEnabled(formatid))
        {
            debug1 << "Warning: plugin "<<formatid<<" claims to be "
                   << "disabled but wasn't.\n";
            continue;
        }

        // don't try any which matched file pattern again; they failed above
        if (std::find(fileMatchedIds.begin(),fileMatchedIds.end(), formatid) !=
                                                          fileMatchedIds.end())
            continue;

        debug3 << "avtDatabaseFactory: fallback pass, "
               << "trying preferred format " << formatid << endl;
        CommonDatabasePluginInfo *info = 
            dbmgr->GetCommonPluginInfo(formatid);
        if (info)
        {
            // if this is strict about filename matching, don't try it either
            if (info->AreDefaultFilePatternsStrict())
            {
                debug3 << "avtDatabaseFactory: no, skip it, since it's "
                       << "strict about file patterns.\n";
                continue;
            }

            // Set the opening options
            const DBOptionsAttributes *opts = 
                defaultFileOpenOptions.GetOpenOptionsForID(formatid);
            if (opts == 0 || (opts != 0 && opts->GetNames().size() == 0))
            {
                // The options aren't in the default options.  Maybe
                // defaults have been added to the plugin since they saved 
                // their settings. Try to get it from the plugin.
                DBOptionsAttributes *opts = info->GetReadOptions();
                if (opts)
                    info->SetReadOptions(opts);
            }
            else
            {
                info->SetReadOptions(new DBOptionsAttributes(*opts));
            }
        }
        TRY
        {
            plugins.push_back(info ? info->GetName(): "");
            rv = SetupDatabase(info, filelist, filelistN,
                               timestep, fileIndex,
                               nBlocks, forceReadAllCyclesAndTimes,
                               treatAllDBsAsTimeVarying, true, times, filesAreEnsemble);
            // If some file reader claimed this file, but couldn't open it,
            // but another one could, then report a warning.
            if (rv != NULL && noncompliantPlugins.size() > 0)
            {
                int n = (int)noncompliantPlugins.size();
                string warning = "While we were able to open the file "
                    "with the " + string(info ? info->GetName() : "") + " reader, "+
                    (n==1?"an":"") + "other file format "+Plural(n,"reader")+
                    " believed "+(n==1?"it":"they")+" should be "
                    "able to open it ("+HumanReadableList(noncompliantPlugins)+
                    ") but found the following errors, which may be worth "
                    "investigating:\n";
                for (int i=0; i<n; i++)
                    warning += noncompliantErrors[i] + "\n";
                dbmgr->ReportWarning(warning);                
            }
        }
        CATCH2(NonCompliantException, e)
        {
            rv = NULL;
            noncompliantPlugins.push_back(info ? info->GetName(): "");
            noncompliantErrors.push_back(e.Message());
        }
        CATCH2(NonCompliantFileException, e)
        {
            rv = NULL;
            noncompliantPlugins.push_back(info ? info->GetName(): "");
            noncompliantErrors.push_back(e.Message());
        }
        CATCHALL
        {
            rv = NULL;
        }
        ENDTRY
    }

    // If we couldn't open the file, always report details occurring
    // from any NonCompliant exceptions.
    if (rv == NULL && noncompliantPlugins.size() > 0)
    {
        size_t nNonCompl = noncompliantPlugins.size();
        string error;
        if (nNonCompl > 1)
        {
            error += "At least one file format reader claimed this file, "
                     " but could not open it:\n";
        }
        for (size_t i=0; i<nNonCompl; i++)
            error += noncompliantErrors[i] + "\n";
        EXCEPTION2(InvalidFilesException,"the file",error);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtDatabaseFactory::SetupDatabase
//
//  Purpose:
//      Setups the avtDatabase.  There are several spots where we want to do
//      this, so this provides a "single point of source" that all routines
//      can call.
//
//  Arguments:
//      info         The CommonDatabasePluginInfo that can instantiate a DB.
//      filelist     A list of files.
//      filelistN    The number of files in filelist.
//      timestep     The current timestep.
//
//  Returns:         A database.  This may very well be NULL.
//
//  Notes:           If the format type is not correct, there is a strong
//                   possibility that the exception InvalidDBTypeException
//                   will be thrown.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2004
//
//  Modifications:
//    Jeremy Meredith/Hank Childs, Tue Mar 23 12:26:55 PST 2004
//    Set the file format with the database, not the meta-data.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool arg to forceReadAllCyclesTimes
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Mon Aug  6 13:36:16 PDT 2007
//    Added logic to accomodate possible null info.
//
//    Mark C. Miller, Mon Aug 20 18:20:37 PDT 2007
//    Added logic to support a timestep specification of -2 indicating that
//    methods to get information from the format (e.g. ActivateTimestep and
//    GetMetaData) should NOT be called. The rationale for '-2' instead of
//    '-1' is that a timestep of '-1' has typically been used throughout
//    VisIt to mean 'any' timestep. This is primarily to support ZipWrapper.
//
//    Hank Childs, Fri Oct  5 16:35:54 PDT 2007
//    Added debug statements.
//
//    Hank Childs, Mon Oct  8 11:17:56 PDT 2007
//    Correct default argument usage.  A missing argument caused 
//    treatAllDBsAsTimeVarying to be interpreted as forceReadThisStateCycleTime
//
//    Jeremy Meredith, Fri Jan  8 16:12:42 EST 2010
//    Added a new strict file reading mode used when we're trying to
//    determine what type of file something is.
//
//    Hank Childs, Sun Sep 19 09:02:05 PDT 2010
//    Add argument for explicit setting of times by user.
//
//    Kathleen Biagas, Thu Feb 14 10:36:09 PST 2013
//    Only set isEnsemble if rv not null.
//
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::SetupDatabase(CommonDatabasePluginInfo *info,
                                  const char * const *filelist, int filelistN, 
                                  int timestep, int fileIndex, int nBlocks,
                                  bool forceReadAllCyclesAndTimes,
                                  bool treatAllDBsAsTimeVarying, 
                                  bool strictMode,
                                  const std::vector<double> &times,
                                  bool isEnsemble)
{
    if (info == 0)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "Attempted to setup a database with null info object");
        EXCEPTION1(ImproperUseException, msg);
    }

    debug4 << "Trying to open the file with the "
           << info->GetName() << " file format, "
           << "strict mode is "<<(strictMode?"on":"off") << endl;

    int t0 = visitTimer->StartTimer();
    avtDatabase *rv = info->SetupDatabase(filelist+fileIndex,
                                          filelistN-fileIndex, nBlocks);
    visitTimer->StopTimer(t0, "Calling file format's SetupDatabase");

    //
    // By policy, the plugin doesn't do much work to set up the
    // database.  So there is a chance that a format did not
    // throw an exception, but is the wrong type.  To make
    // sure we have the correct plugin, force it to read
    // in its metadata.  This does not cause extra work,
    // because the metadata is cached and we are going to
    // ask for it in a bit anyway.
    //
    if (rv != NULL)
    {
        int t0 = visitTimer->StartTimer();
        rv->SetIsEnsemble(isEnsemble);
        rv->SetStrictMode(strictMode);
        if (timestep != -2)
            rv->ActivateTimestep(timestep);
        rv->SetFileFormat(info->GetID());
        bool forceReadThisStateCycleTime = false;
        if (timestep != -2)
        {
            avtDatabaseMetaData *md = rv->GetMetaData(timestep, 
                            forceReadAllCyclesAndTimes, 
                            forceReadThisStateCycleTime,
                            treatAllDBsAsTimeVarying);
            int nStates = md->GetNumStates();
            // Expectation is that nStates == times.size() or times.size() == 0
            int nToDo = (nStates < (int)times.size() ? nStates : (int)times.size());
            for (int i = 0 ; i < nToDo ; i++)
            {
                md->SetTime(i, times[i]);
                md->SetTimeIsAccurate(true, i);
            }
        }
       
        visitTimer->StopTimer(t0, "Forcing file format to do initialization");
        debug4 << "File open appears to be successful." << endl;
    }
    else
        debug4 << "File open resulted in NULL database" << endl;
    return rv;
}


// ****************************************************************************
//  Method: avtDatabaseFactory::VisitFile
//
//  Purpose:
//      Gets an avtDatabase from just the .visit file.
//
//  Arguments:
//      visitFile   The name of the .visit file.
//      timestep    The timestep to open at.
//      plugins     The plugins types that were tried (output argument)
//      format      The file format type (optional).
//
//  Returns:    An avtDatabase object that is correctly typed for the .visit
//              file.
//
//  Programmer: Hank Childs
//  Creation:   September 18, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Mar  2 11:22:58 PST 2001
//    Modified routine to just read in the file list and pass it
//    to the file list routine.
//
//    Hank Childs, Wed Nov  7 15:09:08 PST 2001
//    Check permissions of the first file before starting.
//
//    Jeremy Meredith, Tue Aug 27 15:17:57 PDT 2002
//    Moved GetFileListFromTextFile to be a static avtDatabase function.
//
//    Brad Whitlock, Wed Mar 26 14:24:11 PST 2003
//    I added code to prepend the path to the VisIt file onto all of the
//    timestep files if they are relative. The engine was crashing on Windows
//    because it did not know where to get the timestep files.
//
//    Hank Childs, Mon Mar  1 08:56:52 PST 2004
//    Allow for the time to be specified.
//
//    Hank Childs, Mon Mar 22 11:01:05 PST 2004
//    Added format argument.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool arg to forceReadAllCyclesTimes
//
//    Brad Whitlock, Tue Jan 31 14:55:35 PST 2006
//    Fixed a problem that lead to !NBLOCKS failing on win32.
//
//    Hank Childs, Thu Jan 11 15:56:53 PST 2007
//    Added an argument for which plugins were tried.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Brad Whitlock, Tue Jun 24 15:33:58 PDT 2008
//    Pass in the database plugin manager since it's no longer a singleton.
//
// ****************************************************************************

avtDatabase *
avtDatabaseFactory::VisitFile(DatabasePluginManager *dbmgr,
                              const char *visitFile, int timestep,
                              vector<string> &plugins,
                              const char *format, 
                              bool forceReadAllCyclesAndTimes,
                              bool treatAllDBsAsTimeVarying)
{
    //
    // Make sure we can read the file before we proceed.
    //
    CheckPermissions(visitFile);

    //
    // Get the list of files from the .visit file.
    //
    char  **reallist  = NULL;
    int     listcount = 0;
    int     bang_nblocks = -1;
    avtDatabase::GetFileListFromTextFile(visitFile, reallist, listcount, &bang_nblocks);

#if defined(_WIN32)
    //
    // Get the path out of the VisIt file and prepend it to all of the
    // timestep filenames.
    //
    string visitPath(visitFile);
    std::string::size_type sepIndex = visitPath.find_last_of("\\/");
    if(sepIndex != std::string::npos)
    {
        visitPath = visitPath.substr(0, sepIndex + 1);

        size_t s = visitPath.size();
        string curDir("./");
        for(int j = 0; j < listcount; ++j)
        {
            string fileName(reallist[j]);

            // If the filename contains a colon, assume that it contains
            // the whole path to the file. We don't need to prepend a 
            // path if it already contains one.
            if((fileName.find(":") != std::string::npos) ||
               (fileName.size() > 0 && fileName[0] == '!'))
                continue;

            // If the filename begins with "./", remove it.
            if(fileName.substr(0,2) == curDir)
                fileName = fileName.substr(2, fileName.size() - 2);

            // Create a new filename that has the path prepended to it.
            size_t len = fileName.size() + s + 2;
            char *name = new char[len];
            SNPRINTF(name, len, "%s%s", visitPath.c_str(), fileName.c_str());
            delete [] reallist[j];
            reallist[j] = name;
        }
    }
#endif

    //
    // Create a database using the list of files.
    //
    avtDatabase *rv = FileList(dbmgr, reallist, listcount, timestep, plugins,
        format, forceReadAllCyclesAndTimes, treatAllDBsAsTimeVarying, bang_nblocks);

    //
    // Clean up memory
    //
    for (int i = 0 ; i < listcount ; i++)
    {
        delete [] reallist[i];
    }
    delete [] reallist;

    return rv;
}


// ****************************************************************************
//  Function: CheckPermissions
//
//  Purpose:
//      Makes sure that we have read permission to the file.
//
//  Arguments:
//      filename   The fully qualified (wrt to path) name of the file.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Apr 2 14:21:07 PST 2002
//    Added a Windows implementation.
//
//    Hank Childs, Mon May  6 16:03:04 PDT 2002
//    Don't read uninitialized memory if the stat failed.
//
//    Brad Whitlock, Fri Jul 26 11:33:07 PDT 2002
//    I changed the code so it gives a different kind of exception if the
//    desired file does not exist.
//
//    Mark C. Miller, Thu Mar 30 16:45:35 PST 2006
//    Made it use VisItStat instead of stat
//
//    Tom Fogal, Sun May  3 15:33:55 MDT 2009
//    Marked the functions static.
//
//    Kathleen Biagas, Thu Feb 6 13:24:01 PST 2014
//    Test for file's existence on Windows. Throwing the exception here can
//    prevent engine crashing elsewhere. (conn_cmfe test with bad_file.silo).
//
//    Brad Whitlock, Mon Sep  8 14:25:23 PDT 2014
//    Moved code to FileFunctions.
//
//    Burlen Loring, Thu Jul 30 11:18:12 PDT 2015
//    For parallel runs let one proc check perms and share
//    the result to avoid mds scalability issues on lustre
//
// ****************************************************************************

static void
CheckPermissions(const char *filename)
{
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
#endif
    int result = FileFunctions::PERMISSION_RESULT_READABLE;
    if (rank == 0)
        result = FileFunctions::CheckPermissions(filename);
#ifdef PARALLEL
    MPI_Bcast(&result, 1, MPI_INT, 0, VISIT_MPI_COMM);
#endif

    if (result == FileFunctions::PERMISSION_RESULT_NOFILE)
    {
        EXCEPTION1(FileDoesNotExistException, filename);
    }
    else if (result == FileFunctions::PERMISSION_RESULT_NONREADABLE)
    {
        EXCEPTION1(BadPermissionException, filename);
    }
}
