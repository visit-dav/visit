// ************************************************************************* //
//                                  convert.C                                //
// ************************************************************************* //

#include <DatabasePluginManager.h>

#include <DatabasePluginInfo.h>
#include <Init.h>

#include <ExprParser.h>
#include <ParsingExprList.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtDatabaseMetaData.h>
#include <avtDatabaseWriter.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtExprNodeFactory.h>

#include <VisItException.h>

static void UsageAndExit(const char *);


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
// ****************************************************************************

int main(int argc, char *argv[])
{
    int  i;

    Init::Initialize(argc, argv);

    //
    // Initialize the plugin readers.
    //
    bool parallel = false;
#ifdef PARALLEL
    parallel = true;
#endif
    DatabasePluginManager::Initialize(DatabasePluginManager::Engine, parallel);
    DatabasePluginManager::Instance()->LoadPluginsNow();

    if (argc < 4)
    {
        UsageAndExit(argv[0]);
    }

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
                    UsageAndExit(argv[0]);
                doSpecificVariable = true;
                i++;
                var = argv[i];
            }
            else if (strcmp(argv[i], "-target_chunks") == 0)
            {
                if ((i+1) >= argc)
                    UsageAndExit(argv[0]);
                i++;
                target_chunks = atoi(argv[i]);
            }
            else if (strcmp(argv[i], "-target_zones") == 0)
            {
                if ((i+1) >= argc)
                    UsageAndExit(argv[0]);
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
            else
                UsageAndExit(argv[0]);
        }
    }

    //
    // Users want to enter formats like "Silo", not "Silo_1.0".  Make that
    // conversion for them now.
    //
    DatabasePluginManager *dbmgr = DatabasePluginManager::Instance();
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
        cerr << "Was not able to load file type " << argv[3]<<"\n\n"<<endl;
        UsageAndExit(argv[0]);
    }

    //
    // Set the write options as the default.  If we were clever, we would
    // walk through the write options and have the user iterate over them
    // from the command line.
    //
    edpi->SetWriteOptions(edpi->GetWriteOptions());

    //
    // Make sure this format has a writer.
    //
    avtDatabaseWriter *wrtr = edpi->GetWriter();
    if (wrtr == NULL)
    {
        cerr << "No writer defined for file type " << argv[3] << ".\n"
             << "Please see a VisIt developer." << endl;
        UsageAndExit(argv[0]);
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
            UsageAndExit(argv[0]);
        }
    }
    if (target_chunks > 0)
    {
        bool canDoIt = wrtr->SetTargetChunks(target_chunks);
        if (!canDoIt)
        {
            cerr << "This writer does not support the \"-target_chunks\" "
                 << "option" << endl;
            UsageAndExit(argv[0]);
        }
    }

    //
    // Instantiate the database.
    //
    avtDatabase *db = NULL;
    TRY
    {
        if (strstr(argv[1], ".visit") != NULL)
            db = avtDatabaseFactory::VisitFile(argv[1], 0);
        else
            db = avtDatabaseFactory::FileList(argv+1, 1, 0);
    }
    CATCH(...)
    {
        cerr << "The file " << argv[1] << " does not exist." << endl;
        exit(EXIT_FAILURE);
    }
    ENDTRY

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
        list->AddExpression(*e);
    }

    cerr << "Operating on " << md->GetNumStates() << " timestep(s)." << endl;
    for (i = 0 ; i < md->GetNumStates() ; i++)
    {
         avtDataObject_p dob = db->GetOutput(meshname.c_str(), i);
         avtExpressionEvaluatorFilter eef;
         eef.SetInput(dob);
         dob = eef.GetOutput();
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
// ****************************************************************************

static void
UsageAndExit(const char *progname)
{
    cerr << "Usage: " << progname << " <input-file-name> "
         << "<output-file-name>\\\n\t\t <output-file-type> [-clean] "
         << "[-target_chunks #] \\\n\t\t[-target_zones #] "
         << "[-variable var]" << endl;
    cerr << "\t-clean should be specified when all clean zones are desired.\n"
         << "\tIn this case material interface reconstruction will be "
         << "performed." << endl;
    cerr << "\t-target_zones specifies what the total number of zones in the "
         << "output \n\tshould be.\n";
    cerr << "\t-target_chunks should be specified when the chunks should "
         << "be \n\trepartitioned.  This is often used in conjunction with "
         << "-target_zones.\n";
    cerr << "\t-variable specifies which variable should be processed.\n";
    cerr << "Example (one timestep):\n " << progname 
         << " run1.exodus run1.silo Silo" << endl;
    cerr << "Example (multi-timestep):\n " << progname
         << " run1.exodus run%04d.silo Silo" << endl;
    cerr << "Example (scaling study):\n " << progname
         << " rect.silo rect BOV -target_chunks 512 \\\n\t\t -target_zones "
         << "512000000 -variable var1" << endl;
    cerr << "Acceptable output types are: " << endl;
    DatabasePluginManager *dbmgr = DatabasePluginManager::Instance();
    for (int i = 0 ; i < dbmgr->GetNAllPlugins() ; i++)
    {
        string plugin = dbmgr->GetAllID(i);
        if (dbmgr->PluginHasWriter(plugin))
            cerr << "\t" << dbmgr->GetPluginName(plugin) << endl;
    }

    exit(EXIT_FAILURE);
}


