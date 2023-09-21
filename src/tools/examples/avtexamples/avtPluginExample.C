// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItInit.h>

#include <DatabasePluginInfo.h>
#include <DatabasePluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtDatabaseWriter.h>
#include <avtParallel.h>
#include <avtPlot.h>
#include <avtPluginFilter.h>

#include <VisItException.h>
#include <visitstream.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

//
// Things to note:
//
// 1) If you specify VTK_1.0 to the visit writer the program will crash.
//
// 2) The pseudocolor plot is not used.
//
// 3) It is impossible to set the clip attributes since you can't set
//    any attributes of a ClipAttributes without linking that code into
//    this example.
//
// 4) Parallel has never been tested and probably doesn't work.
//
// 5) The program crashes after calling return. It is probably in the
//    destructor for the avtPlot, since commenting out the code that
//    allocates the avtPlot for the pseudocolor plot fixes the crash.
//

int
main(int argc, char *argv[])
{
    string dataDir("/usr/gapps/visit/data");
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-datadir") == 0 && (i+1) < argc)
        {
            dataDir = string(argv[i+1]);
            break;
        }
    }

    //
    // Initialize VisIt.
    //
    cerr << "Initializing VisIt." << endl;
    bool parallel = false;
#ifdef PARALLEL
    parallel = true;
#endif

    PAR_Init(argc, argv);

    VisItInit::SetComponentName("engine");
    VisItInit::Initialize(argc, argv);

    //
    // Initialize the plugin readers.
    //
    cerr << "Creating the plugin managers." << endl;
    DatabasePluginManager *dbmgr = new DatabasePluginManager;
    OperatorPluginManager  *omgr = new OperatorPluginManager;
    PlotPluginManager      *pmgr = new PlotPluginManager;

    dbmgr->Initialize(DatabasePluginManager::Engine, parallel);
    omgr->Initialize(DatabasePluginManager::Engine, parallel);
    pmgr->Initialize(DatabasePluginManager::Engine, parallel);

    dbmgr->LoadPluginsNow();
    omgr->LoadPluginsOnDemand();
    pmgr->LoadPluginsOnDemand();

    //
    // Load the Pseudocolor plot.
    //
    cerr << "Loading the pseudocolor plot." << endl;
    const string plotName = "Pseudocolor_1.0";
    bool success = pmgr->LoadSinglePluginNow(plotName);
    if (!success)
    {
        cerr << "Unable to load the " << plotName << " plugin." << endl;
        PAR_Exit();
        exit(EXIT_FAILURE);
    }
    avtPlot_p plot = pmgr->GetEnginePluginInfo(plotName)->AllocAvtPlot();
    if (*plot == NULL)
    {
        cerr << "Unable to allocate the " << plotName << " plugin." << endl;
        PAR_Exit();
        exit(EXIT_FAILURE);
    }

    //
    // Load the Clip filter.
    //
    cerr << "Loading the clip filter." << endl;
    const string filterName = "Clip_1.0";
    success = omgr->LoadSinglePluginNow(filterName);
    if (!success)
    {
        cerr << "Unable to load the " << filterName << " plugin." << endl;
        PAR_Exit();
        exit(EXIT_FAILURE);
    }
    avtPluginFilter *filter = omgr->GetEnginePluginInfo(filterName)->AllocAvtPluginFilter();
    if (filter == NULL)
    {
        cerr << "Unable to allocate the " << filterName << " plugin." << endl;
        PAR_Exit();
        exit(EXIT_FAILURE);
    }

    //
    // Instantiate the database.
    //
    cerr << "Opening the file." << endl;
    string fn = dataDir + "/curv2d.silo";
    const char*filename(fn.c_str());
    avtDatabase *db = NULL;
    vector<string> pluginList;

    TRY
    {
        db = avtDatabaseFactory::FileList(dbmgr, &filename, 1, 0, pluginList);
    }
    CATCHALL
    {
        if (PAR_Rank() == 0)
            cerr << "The file " << filename << " does not exist or could "
                 << "not be opened." << endl;
        PAR_Exit();
        exit(EXIT_FAILURE);
    }
    ENDTRY

    if (db == NULL)
    {
        if (PAR_Rank() == 0)
            cerr << "Could not open file " << filename << ".  Tried using plugins ";
        for (size_t i = 0 ; i < pluginList.size() ; i++)
        {
            if (PAR_Rank() == 0)
            {
                cerr << pluginList[i];
                if (i != pluginList.size()-1)
                    cerr << ", ";
                else
                    cerr << endl;
            }
        }

        PAR_Exit();
        exit(EXIT_FAILURE);
    }

    //
    // Get the database object.
    //
    avtDataObject_p dob = db->GetOutput("d", 0);

    //
    // Apply the operator.
    //
    cerr << "Applying a clip operator." << endl;
    filter->SetInput(dob);
    avtDataObject_p output = filter->GetOutput();

    //
    // Write out the output.
    //
    cerr << "Writing the output." << endl;
    EngineDatabasePluginInfo *edpi = dbmgr->GetEnginePluginInfo("Silo_1.0");

    if (edpi == NULL)
    {
        PAR_Exit();
        exit(EXIT_FAILURE);
    }

    avtDatabaseWriter *wrtr = edpi->GetWriter();

    if (wrtr == NULL)
    {
        PAR_Exit();
        exit(EXIT_FAILURE);
    }

    const avtDatabaseMetaData *md = db->GetMetaData(0);

    vector<string> vars;
    vars.push_back("d");
    wrtr->SetInput(output);
    wrtr->Write("", "output", md, vars, false);

    // Clean up and exit.
    //
    cerr << "Cleaning up." << endl;
    delete dbmgr;
    delete omgr;
    delete pmgr;

    cerr << "Exiting." << endl;
    PAR_Exit();

    return 0;
}
