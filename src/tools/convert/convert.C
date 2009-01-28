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

// ************************************************************************* //
//                                  convert.C                                //
// ************************************************************************* //

#include <DatabasePluginManager.h>

#include <DatabasePluginInfo.h>
#include <VisItInit.h>

#ifndef DISABLE_EXPRESSIONS
#include <ExprParser.h>
#include <ParsingExprList.h>
#endif
#include <Utility.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtDatabaseMetaData.h>
#include <avtDatabaseWriter.h>
#ifndef DISABLE_EXPRESSIONS
#include <avtExpressionEvaluatorFilter.h>
#include <avtExprNodeFactory.h>
#endif
#include <DBOptionsAttributes.h>

#include <VisItException.h>
#include <visitstream.h>
using std::cin;

static void UsageAndExit(DatabasePluginManager *, const char *);

// ****************************************************************************
//  Method:  FillOptionsFromCommandline
//
//  Purpose:
//    Fill database writer options from the user interactively.
//
//  Arguments:
//    opts       the options to fill
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  3, 2008
//
//  Modifications:
//    Eric Brugger, Fri Aug  8 12:22:32 PDT 2008
//    Made to use strtod instead of strtof if on Windows or HAVE_STRTOF is
//    not defined or HAVE_STRTOF_PROTOTYPE is not defined.
//
// ****************************************************************************
void
FillOptionsFromCommandline(DBOptionsAttributes *opts)
{
    if (!opts)
        return;

    for (int j=0; j<opts->GetNumberOfOptions(); j++)
    {
        const std::string &name = opts->GetName(j);
        cerr << endl << "Enter value for option '"<<name<<"'";
        switch (opts->GetType(j))
        {
          case DBOptionsAttributes::Bool:
            cerr << " (boolean, default="<<opts->GetBool(name)<<"):\n";
            break;
          case DBOptionsAttributes::Int:
            cerr << " (integer, default="<<opts->GetInt(name)<<"):\n";
            break;
          case DBOptionsAttributes::Float:
            cerr << " (float, default="<<opts->GetFloat(name)<<"):\n";
            break;
          case DBOptionsAttributes::Double:
            cerr << " (double, default="<<opts->GetDouble(name)<<"):\n";
            break;
          case DBOptionsAttributes::String:
            cerr << " (string, default='"<<opts->GetString(name)<<"'):\n";
            break;
          case DBOptionsAttributes::Enum:
            {
            cerr << endl;
            stringVector enumValues = opts->GetEnumStrings(name);
            for (int k=0; k<enumValues.size(); k++)
                cerr << "    ("<<k<<" = "<<enumValues[k]<<")\n";
            cerr << " (integer, default="<<opts->GetEnum(name)<<"):\n";
            }
            break;
        }
        char buff[1024];
        cin.getline(buff,1024);
        if (strlen(buff) == 0)
        {
            cerr << "Accepted default value for '"<<name<<"'\n";
            continue;
        }
        switch (opts->GetType(j))
        {
          case DBOptionsAttributes::Bool:
            opts->SetBool(name, strtol(buff, NULL, 10));
            cerr << "Set to new value "<<opts->GetBool(name) << endl;
            break;
          case DBOptionsAttributes::Int:
            opts->SetInt(name, strtol(buff, NULL, 10));
            cerr << "Set to new value "<<opts->GetInt(name) << endl;
            break;
          case DBOptionsAttributes::Float:
#if defined(_WIN32) || !defined(HAVE_STRTOF) || !defined(HAVE_STRTOF_PROTOTYPE)
            opts->SetFloat(name, (float) strtod(buff, NULL));
#else
            opts->SetFloat(name, strtof(buff, NULL));
#endif
            cerr << "Set to new value "<<opts->GetFloat(name) << endl;
            break;
          case DBOptionsAttributes::Double:
            opts->SetDouble(name, strtod(buff, NULL));
            cerr << "Set to new value "<<opts->GetDouble(name) << endl;
            break;
          case DBOptionsAttributes::String:
            opts->SetString(name, buff);            
            cerr << "Set to new value "<<opts->GetString(name) << endl;
            break;
          case DBOptionsAttributes::Enum:
            opts->SetEnum(name, strtol(buff, NULL, 10));
            cerr << "Set to new value "<<opts->GetEnum(name) << endl;
            break;
        }
    }
}


// ****************************************************************************
//  Function: main
//
//  Purpose:
//      This program reads in files and then writes them out to a specified
//      format.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Mar  1 09:08:11 PST 2004
//    Send timestep to database factory.
//
//    Hank Childs, Sat Sep 11 12:10:53 PDT 2004
//    Add support for target chunks, target zones, -variable.
//
//    Jeremy Meredith, Wed Nov 24 15:39:02 PST 2004
//    Added expression support back in.  ('3769)
//
//    Hank Childs, Wed Dec 22 11:33:30 PST 2004
//    Make expressions work ('5701), also add better error handling ('5771).
//
//    Jeremy Meredith, Tue Feb 22 18:16:46 PST 2005
//    Added better argument error checking.  Added ability to turn off
//    expression conversion.  Added ability to turn off MIR even when
//    the writer doesn't support mixed materials.  Moved database writer
//    creation to the engine plugin, and moved the test to see if a database
//    plugin has a writer to the general plugin info.
//
//    Mark C. Miller, Tue Feb  7 11:16:18 PST 2006
//    Added return statement to main
//
//    Hank Childs, Thu Mar 30 11:57:05 PST 2006
//    Add basic (i.e. non-crashing) support for write options.
//
//    Hank Childs, Tue Jan 16 08:37:36 PST 2007
//    Add support for logging which plugins were attempted when a database
//    open fails.
//
//    Jeremy Meredith, Tue Jun  3 14:12:23 EDT 2008
//    Added support for fallback_format and default_format.  Added
//    writer options input.  Put attempt to create a writer after the
//    attempt to open the input file.
//
//    Brad Whitlock, Tue Jun 24 16:58:35 PDT 2008
//    The plugin manager is no longer a singleton.
//
// ****************************************************************************

int main(int argc, char *argv[])
{
    int  i;

    VisItInit::Initialize(argc, argv);

    //
    // Initialize the plugin readers.
    //
    bool parallel = false;
#ifdef PARALLEL
    parallel = true;
#endif
    DatabasePluginManager *dbmgr = new DatabasePluginManager;
    dbmgr->Initialize(DatabasePluginManager::Engine, parallel);
    dbmgr->LoadPluginsNow();
    cerr << endl; // whitespace after some expected plugin loading errors

    if (argc < 4)
    {
        UsageAndExit(dbmgr, argv[0]);
    }

    bool noOptions = false;
    bool doClean = false;
    bool disableMIR = false;
    bool disableExpressions = false;
    bool doSpecificVariable = false;
    const char *var = NULL;
    int target_chunks = -1;
    long long target_zones = -1;
    if (argc > 4)
    {
        for (int i = 4 ; i < argc ; i++)
        {
            if (strcmp(argv[i], "-clean") == 0)
                doClean = true;
            else if (strcmp(argv[i], "-nomir") == 0)
                disableMIR = true;
            else if (strcmp(argv[i], "-noexpr") == 0)
                disableExpressions = true;
            else if (strcmp(argv[i], "-variable") == 0)
            {
                if ((i+1) >= argc)
                    UsageAndExit(dbmgr, argv[0]);
                doSpecificVariable = true;
                i++;
                var = argv[i];
            }
            else if (strcmp(argv[i], "-target_chunks") == 0)
            {
                if ((i+1) >= argc)
                    UsageAndExit(dbmgr, argv[0]);
                i++;
                target_chunks = atoi(argv[i]);
            }
            else if (strcmp(argv[i], "-target_zones") == 0)
            {
                if ((i+1) >= argc)
                    UsageAndExit(dbmgr, argv[0]);
                i++;
                target_zones = 0;
                int nchars = strlen(argv[i]);
                for (int j = 0 ; j < nchars ; j++)
                {
                    if (isdigit(argv[i][j]))
                    {
                        target_zones *= 10;
                        target_zones += argv[i][j] - '0';
                    }
                }
            }
            else if (strcmp(argv[i], "-fallback_format") == 0)
            {
                if ((i+1) >= argc)
                    UsageAndExit(dbmgr, argv[0]);
                i++;

                avtDatabaseFactory::SetFallbackFormat(argv[i]);
            }
            else if (strcmp(argv[i], "-assume_format") == 0)
            {
                if ((i+1) >= argc)
                    UsageAndExit(dbmgr, argv[0]);
                i++;

                avtDatabaseFactory::SetFormatToTryFirst(argv[i]);
            }
            else if (strcmp(argv[i], "-no_options") == 0)
                noOptions = true;
            else
                UsageAndExit(dbmgr, argv[0]);
        }
    }

    //
    // Users want to enter formats like "Silo", not "Silo_1.0".  Make that
    // conversion for them now.
    //
    EngineDatabasePluginInfo *edpi = NULL;
    int index = dbmgr->GetAllIndexFromName(argv[3]);
    if (index >= 0)
    {
        std::string id = dbmgr->GetAllID(index);
        if (dbmgr->PluginAvailable(id))
        {
            edpi = dbmgr->GetEnginePluginInfo(id);
        }
    }

    if (edpi == NULL)
    {
        cerr << "Was not able to create file type " << argv[3]<<"\n\n"<<endl;
        UsageAndExit(dbmgr, argv[0]);
    }

    //
    // Instantiate the database.
    //
    avtDatabase *db = NULL;
    std::vector<std::string> pluginList;
    TRY
    {
        if (strstr(argv[1], ".visit") != NULL)
            db = avtDatabaseFactory::VisitFile(dbmgr, argv[1], 0, pluginList);
        else
            db = avtDatabaseFactory::FileList(dbmgr, argv+1, 1, 0, pluginList);
    }
    CATCH(...)
    {
        cerr << "The file " << argv[1] << " does not exist or could "
             << "not be opened." << endl;
        exit(EXIT_FAILURE);
    }
    ENDTRY

    if (db == NULL)
    {
        cerr << "Could not open file " << argv[1] << ".  Tried using plugins ";
        for (int i = 0 ; i < pluginList.size() ; i++)
        {
            cerr << pluginList[i];
            if (i != pluginList.size()-1)
                cerr << ", ";
            else
                cerr << endl;
        }
    }

    //
    // Set the write options as the default.
    // Walk through the write options and have the user iterate over them
    // from the command line.
    //
    DBOptionsAttributes *opts = edpi->GetWriteOptions();
    if (!noOptions)
        FillOptionsFromCommandline(opts);
    edpi->SetWriteOptions(opts);

    //
    // Make sure this format has a writer.
    //
    avtDatabaseWriter *wrtr = edpi->GetWriter();
    if (wrtr == NULL)
    {
        cerr << "No writer defined for file type " << argv[3] << ".\n"
             << "Please see a VisIt developer." << endl;
        UsageAndExit(dbmgr, argv[0]);
    }
    if (doClean)
        wrtr->SetShouldAlwaysDoMIR(doClean);
    if (disableMIR)
        wrtr->SetShouldNeverDoMIR(disableMIR);
    if (disableExpressions)
        wrtr->SetShouldNeverDoExpressions(disableExpressions);
    if (target_zones > 0)
    {
        bool canDoIt = wrtr->SetTargetZones(target_zones);
        if (!canDoIt)
        {
            cerr << "This writer does not support the \"-target_zones\" option"
                 << endl;
            UsageAndExit(dbmgr, argv[0]);
        }
    }
    if (target_chunks > 0)
    {
        bool canDoIt = wrtr->SetTargetChunks(target_chunks);
        if (!canDoIt)
        {
            cerr << "This writer does not support the \"-target_chunks\" "
                 << "option" << endl;
            UsageAndExit(dbmgr, argv[0]);
        }
    }

    //
    // Figure out which mesh to operate on.
    // Assume MetaData for timestep 0 is sufficient for what we need here
    //
    const avtDatabaseMetaData *md = db->GetMetaData(0);
    const avtMeshMetaData *mmd = NULL;
    std::string meshname = "";
    if (md->GetNumMeshes() >= 1)
    {
        if (md->GetNumMeshes() > 1)
        {
            cerr << "There are multiple meshes in the file.  This program can "
                 << "only\nhandle one mesh at a time.  I am using mesh: ";
            cerr << md->GetMesh(0)->name << endl;
        }
        meshname = md->GetMesh(0)->name;
    }
    else if (md->GetNumMeshes() < 1)
    {
        if (md->GetNumCurves() > 0)
        {
            cerr << "Cannot find any meshes, converting curves." << endl;
            meshname = md->GetCurve(0)->name;
        }
        else
        {
            cerr << "Cannot find any valid meshes or curves to convert.\n"
                 << "Giving up." << endl;
            exit(EXIT_FAILURE);
        }
    }

#ifndef DISABLE_EXPRESSIONS
    //
    // Hook up the expressions we have associated with the database, so
    // we can get those as well.
    //
    Parser *p = new ExprParser(new avtExprNodeFactory());
    ParsingExprList *l = new ParsingExprList(p);
    ExpressionList *list = l->GetList();
    for (i = 0 ; i < md->GetNumberOfExpressions() ; i++)
    {
        const Expression *e = md->GetExpression(i);
        list->AddExpressions(*e);
    }
#endif

    cerr << "Operating on " << md->GetNumStates() << " timestep(s)." << endl;
    for (i = 0 ; i < md->GetNumStates() ; i++)
    {
         avtDataObject_p dob = db->GetOutput(meshname.c_str(), i);
#ifndef DISABLE_EXPRESSIONS
         avtExpressionEvaluatorFilter eef;
         eef.SetInput(dob);
         dob = eef.GetOutput();
#endif
         wrtr->SetInput(dob);

         char filename[1024];
         if (strstr(argv[2], "%") != NULL)
             sprintf(filename, argv[2], i);
         else if (md->GetNumStates() == 1)
             strcpy(filename, argv[2]);
         else
             sprintf(filename, "%04d.%s", i, argv[2]);
        
         TRY
         {
             if (doSpecificVariable)
             {
                 std::vector<std::string> varlist;
                 varlist.push_back(var);
                 wrtr->Write(filename, md, varlist);
             }
             else
             {
                 wrtr->Write(filename, md);
             } 
         }
         CATCH2(VisItException, e)
         {
             cerr << "Error encountered.  Unable to write files." << endl;
             cerr << "Error was: " << endl;
             cerr << e.Message() << endl;
             break;
         }
         ENDTRY
    }

    delete dbmgr;

    return 0;
}


// ****************************************************************************
//  Function: UsageAndExit
//
//  Purpose:
//      Issues a usage statement and then exits.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Sep 11 11:22:09 PDT 2004
//    Added "-target_chunks", "-target_zones", and "-variables".
//
//    Jeremy Meredith, Tue Feb 22 18:27:05 PST 2005
//    Changed it to loop over all the plugins, because the test for
//    having a writer is in the general plugin info.
//
//    Jeremy Meredith, Tue Jun  3 14:27:23 EDT 2008
//    Cleaned up greatly.  Added help for input formats and options.
//
//    Brad Whitlock, Tue Jun 24 16:57:45 PDT 2008
//    Pass in the DatabasePluginManager pointer.
//
// ****************************************************************************

static void
UsageAndExit(DatabasePluginManager *dbmgr, const char *argv0)
{
    const char *progname = argv0 + strlen(argv0)-1;
    while (progname > argv0 && *(progname-1) != '/')
        progname--;

    cerr << "Usage: " << progname
         << " <input-file> <output-file> <output-type> [OPTIONS]\n";

    cerr << endl;
    cerr << "Available options:" << endl;
    cerr << "\t-clean" << endl;
    cerr << "\t-target_chunks   <number>" << endl;
    cerr << "\t-target_zones    <number>" << endl;
    cerr << "\t-variable        <variable>" << endl;
    cerr << "\t-no_options" << endl;
    cerr << "\t-assume_format   <input_format>" << endl;
    cerr << "\t-fallback_format <input_format>" << endl;
    cerr << endl;
    cerr << "Option descriptions:" << endl;
    cerr << "  -clean should be specified when all clean zones are desired." << endl;
    cerr << "   In this case material interface reconstruction will be performed." << endl;
    cerr << endl;
    cerr << "  -target_zones specifies what the total number of zones in the output" << endl;
    cerr << "   should be." << endl;
    cerr << endl;
    cerr << "  -target_chunks should be specified when the chunks should be" << endl;
    cerr << "   repartitioned.  This is often used in conjunction with -target_zones." << endl;
    cerr << endl;
    cerr << "  -variable specifies which variable should be processed." << endl;
    cerr << endl;
    cerr << "  -assume_format specifies the plugin format type to try first" << endl;
    cerr << endl;
    cerr << "  -fallback_format specifies a plugin format type to try if the" << endl;
    cerr << "   guessing based on file extension failes" << endl;
    cerr << endl;
    cerr << "  -no_options skips option entry, instead using only default values." << endl;
    cerr << endl;
    cerr << "Example (one timestep):" << endl;
    cerr << "  visitconvert_ser run1.exodus run1.silo Silo" << endl;
    cerr << "Example (multi-timestep):" << endl;
    cerr << "  visitconvert_ser run1.exodus run%04d.silo Silo" << endl;
    cerr << "Example (scaling study):" << endl;
    cerr << "  visitconvert_ser rect.silo rect BOV -target_chunks 512  \\" << endl;
    cerr << "                 -target_zones 512000000 -variable var1" << endl;
    cerr << endl;
    cerr << "Acceptable output types are: " << endl;
    for (int i = 0 ; i < dbmgr->GetNAllPlugins() ; i++)
    {
        string plugin = dbmgr->GetAllID(i);
        if (dbmgr->PluginHasWriter(plugin))
            cerr << "\t" << dbmgr->GetPluginName(plugin) << endl;
    }
    delete dbmgr;

    exit(EXIT_FAILURE);
}


