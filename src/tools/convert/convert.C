// ************************************************************************* //
//                                  convert.C                                //
// ************************************************************************* //

#include <DatabasePluginManager.h>

#include <DatabasePluginInfo.h>
#include <Init.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtDatabaseMetaData.h>
#include <avtDatabaseWriter.h>
#include <avtExpressionEvaluatorFilter.h>


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
// ****************************************************************************

int main(int argc, char *argv[])
{
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

    bool doClean = false;
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
    CommonDatabasePluginInfo *cdpi = NULL;
    for (int i = 0 ; i < dbmgr->GetNEnabledPlugins() ; i++)
    {
        std::string name = dbmgr->GetPluginName(dbmgr->GetEnabledID(i));
        if (name == argv[3])
        {
            cdpi = dbmgr->GetCommonPluginInfo(dbmgr->GetEnabledID(i));
            break;
        }
    }

    if (cdpi == NULL)
    {
        cerr << "Was not able to load file type " << argv[3]<<"\n\n"<<endl;
        UsageAndExit(argv[0]);
    }

    //
    // Make sure this format has a writer.
    //
    avtDatabaseWriter *wrtr = cdpi->GetWriter();
    if (wrtr == NULL)
    {
        cerr << "No writer defined for file type " << argv[3] << ".\n"
             << "Please see a VisIt developer." << endl;
        UsageAndExit(argv[0]);
    }
    if (doClean)
        wrtr->SetShouldAlwaysDoMIR(doClean);
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
    if (strstr(argv[1], ".visit") != NULL)
        db = avtDatabaseFactory::VisitFile(argv[1], 0);
    else
        db = avtDatabaseFactory::FileList(argv+1, 1, 0);

    //
    // Figure out which mesh to operate on.
    // Assume MetaData for timestep 0 is sufficient for what we need here
    //
    const avtDatabaseMetaData *md = db->GetMetaData(0);
    if (md->GetNumMeshes() > 1)
    {
        cerr << "There are multiple meshes in the file.  This program can "
             << "only\nhandle one mesh at a time.  I am using mesh: ";
        cerr << md->GetMesh(0)->name << endl;
    }
    const avtMeshMetaData *mmd = md->GetMesh(0);

    cerr << "Operating on " << md->GetNumStates() << " timestep(s)." << endl;
    for (int i = 0 ; i < md->GetNumStates() ; i++)
    {
         avtDataObject_p dob = db->GetOutput(mmd->name.c_str(), i);
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
    for (int i = 0 ; i < dbmgr->GetNEnabledPlugins() ; i++)
    {
         CommonDatabasePluginInfo *cdpi = 
                      dbmgr->GetCommonPluginInfo(dbmgr->GetEnabledID(i));
         avtDatabaseWriter *wrtr = cdpi->GetWriter();
         if (wrtr == NULL)
             continue;
         std::string name = dbmgr->GetPluginName(dbmgr->GetEnabledID(i));
         cerr << "\t" << name << endl;
    }

    exit(EXIT_FAILURE);
}


