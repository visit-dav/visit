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

    //
    // Print a usage statement.
    //
    bool doClean = false;
    if (argc == 5)
    {
        if (strcmp(argv[4], "-clean") == 0)
            doClean = true;
        else
            UsageAndExit(argv[0]);
    }
    else if (argc != 4)
    {
        UsageAndExit(argv[0]);
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
/* The EEF is currently only available on the engine.
         avtExpressionEvaluatorFilter eef;
         eef.SetInput(dob);
         dob = eef.GetOutput();
 */
         wrtr->SetInput(dob);

         char filename[1024];
         if (strstr(argv[2], "%") != NULL)
             sprintf(filename, argv[2], i);
         else if (md->GetNumStates() == 1)
             strcpy(filename, argv[2]);
         else
             sprintf(filename, "%04d.%s", i, argv[2]);
        
         wrtr->Write(filename, md);
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
// ****************************************************************************

static void
UsageAndExit(const char *progname)
{
    cerr << "Usage: " << progname << " <input-file-name> "
         << "<output-file-name>\\\n\t\t <output-file-type> [-clean]" << endl;
    cerr << "\t-clean should be specified when all clean zones are desired.\n"
         << "\tIn this case material interface reconstruction will be "
         << "performed." << endl;
    cerr << "Example (one timestep): " << progname 
         << " run1.exodus run1.silo Silo" << endl;
    cerr << "Example (multi-timestep): " << progname
         << " run1.exodus run%04d.silo Silo" << endl;
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


