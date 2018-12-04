#include <VisItInit.h>

#include <DatabasePluginInfo.h>
#include <DatabasePluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <SaveWindowAttributes.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtFileWriter.h>
#include <avtImage.h>
#include <avtPlot.h>
#include <avtPluginFilter.h>
#include <avtView2D.h>
#include <VisWindow.h>

#include <VisItException.h>
#include <visitstream.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

//
// Things to note:
//
// 1) The program crashes after calling return.
//
//

int
main(int argc, char *argv[])
{
    //
    // Initialize VisIt.
    //
    cerr << "Initializing VisIt." << endl;
    VisItInit::SetComponentName("engine");
    VisItInit::Initialize(argc, argv);

    //
    // Initialize the plugin readers.
    //
    cerr << "Creating the plugin managers." << endl;
    DatabasePluginManager *dbmgr = new DatabasePluginManager;
    OperatorPluginManager  *omgr = new OperatorPluginManager;
    PlotPluginManager      *pmgr = new PlotPluginManager;

    dbmgr->Initialize(DatabasePluginManager::Engine, false);
    omgr->Initialize(DatabasePluginManager::Engine, false);
    pmgr->Initialize(DatabasePluginManager::Engine, false);

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
        exit(EXIT_FAILURE);
    }
    avtPlot_p plot = pmgr->GetEnginePluginInfo(plotName)->AllocAvtPlot();
    if (*plot == NULL)
    {
        cerr << "Unable to allocate the " << plotName << " plugin." << endl;
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
        exit(EXIT_FAILURE);
    }
    avtPluginFilter *filter = omgr->GetEnginePluginInfo(filterName)->AllocAvtPluginFilter();
    if (filter == NULL)
    {
        cerr << "Unable to allocate the " << filterName << " plugin." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // Instantiate the database.
    //
    cerr << "Opening the file." << endl;
    const char *filename = "/usr/gapps/visit/data/curv2d.silo";
    avtDatabase *db = NULL;
    vector<string> pluginList;
    TRY
    {
        db = avtDatabaseFactory::FileList(dbmgr, &filename, 1, 0, pluginList);
    }
    CATCHALL
    {
        cerr << "The file " << filename << " does not exist or could "
             << "not be opened." << endl;
        exit(EXIT_FAILURE);
    }
    ENDTRY

    if (db == NULL)
    {
        cerr << "Could not open file " << argv[1] << ".  Tried using plugins ";
        for (size_t i = 0 ; i < pluginList.size() ; i++)
        {
            cerr << pluginList[i];
            if (i != pluginList.size()-1)
                cerr << ", ";
            else
                cerr << endl;
        }

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

    avtOriginatingSource *src = dob->GetOriginatingSource();
    avtContract_p contract = new avtContract(src->GetFullDataRequest(), 0);

    output->Update(contract);

    //
    // Create the actor from the plot.
    //
    cerr << "Creating the actor from the pseudocolor plot." << endl;
    plot->SetColorTable("Default");
    avtActor_p actor = plot->Execute(NULL, output);

    //
    // Create the window and add the actor.
    //
    cerr << "Creating the window and adding the actor." << endl;
    VisWindow *window = new VisWindow();
    window->Realize();
    window->AddPlot(actor);

    //
    // Set the view.
    //
    avtView2D view2D;
    view2D.viewport[0] = 0.1;
    view2D.viewport[1] = 0.9;
    view2D.viewport[2] = 0.1;
    view2D.viewport[3] = 0.9;
    view2D.window[0] = -5.;
    view2D.window[1] =  5.;
    view2D.window[2] =  0.;
    view2D.window[3] =  5.;
    view2D.windowValid = true;
    window->SetView2D(view2D);

    //
    // Save the image.
    //
    cerr << "Saving the image." << endl;
    avtImage_p image = window->ScreenCapture();
    avtDataObject_p tmpImage;
    CopyTo(tmpImage, image);

    avtFileWriter *fileWriter = new avtFileWriter();
    fileWriter->SetFormat(SaveWindowAttributes::PNG);
    fileWriter->Write("output.png", tmpImage, 100, false, 1, false);
    delete fileWriter;

    //
    // Clean up and exit.
    //
    cerr << "Cleaning up." << endl;
    delete dbmgr;
    delete omgr;
    delete pmgr;

    delete window;

    cerr << "Exiting." << endl;

    return 0;
}
