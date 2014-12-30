#include <VisItInit.h>

#include <AnnotationAttributes.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <AttributeGroup.h>
#include <DatabasePluginInfo.h>
#include <DatabasePluginManager.h>
#include <LoadBalancer.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <NetworkManager.h>
#include <SaveWindowAttributes.h>
#include <View2DAttributes.h>
#include <VisualCueInfo.h>
#include <VisualCueList.h>
#include <WindowAttributes.h>

#include <avtFileWriter.h>

#include <VisItException.h>
#include <visitstream.h>

#include <string>
#include <vector>

#include <stdlib.h>

using std::string;
using std::vector;

//
// Things to note:
//
// 1) The saved image is only the actual plot in the viewport without
//    any annotations. Hence, you only get the pseudocolor plot in an
//    800 by 400 image.
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

    dbmgr->LoadPluginsOnDemand();
    omgr->LoadPluginsOnDemand();
    pmgr->LoadPluginsOnDemand();

    //
    // Create the network manager.
    //
    NetworkManager *netmgr = new NetworkManager;

    netmgr->SetDatabasePluginManager(dbmgr);
    netmgr->SetOperatorPluginManager(omgr);
    netmgr->SetPlotPluginManager(pmgr);

    LoadBalancer *loadBalancer = new LoadBalancer(1, 0);
    netmgr->SetLoadBalancer(loadBalancer);

    //
    // Open the file.
    //
    cerr << "Opening a silo file." << endl; 
    netmgr->StartNetwork("Silo_1.0", "/usr/gapps/visit/data/curv2d.silo",
                         "d", 0);

    //
    // Add an operator.
    //
    cerr << "Adding the clip filter." << endl;
    const string operatorName = "Clip_1.0";
    if (!omgr->PluginAvailable(operatorName))
    {
        cerr << "Operator " << operatorName << " does not exist for the engine." << endl;
        exit(EXIT_FAILURE);
    }

    AttributeGroup *operatorAtts = omgr->GetEnginePluginInfo(operatorName)->AllocAttributes();

    netmgr->AddFilter(operatorName, operatorAtts, 1);
 
    //
    // Add a plot.
    //
    cerr << "Adding a pseudocolor plot." << endl; 
    const string plotName = "Pseudocolor_1.0";
    if (!pmgr->PluginAvailable(plotName))
    {
        cerr << "Plot " << plotName << " does not exist for the engine." << endl;
        exit(EXIT_FAILURE);
    }

    std::vector<double> extents;
    extents.push_back(-5.0);
    extents.push_back(5.0);
    extents.push_back(0.0);
    extents.push_back(5.0);
    extents.push_back(0.0);
    extents.push_back(0.0);

    AttributeGroup *plotAtts = pmgr->GetEnginePluginInfo(plotName)->AllocAttributes();
    
    netmgr->MakePlot(plotName, plotName, plotAtts, extents);

    //
    // Finalize the network.
    //
    cerr << "Finalizing the network." << endl;
    netmgr->EndNetwork(0);

    //
    // Set the window attributes.
    //
    cerr << "Setting the window attributes." << endl;
    WindowAttributes windowAtts;
    int size[2] = {1000, 1000};
    windowAtts.SetSize(size);
    View2DAttributes view2D;
    double window[4] = {-5., 5., 0., 5.};
    view2D.SetWindowCoords(window);
    double viewport[4] = {0.1, 0.9, 0.1, 0.9};
    view2D.SetViewportCoords(viewport);
    view2D.SetWindowValid(true);
    windowAtts.SetView2D(view2D);
    std::string extstr = "AVT_ORIGINAL_EXTENTS";
    double extents2[4] = {-5., 5., 0., 5.};
    netmgr->SetWindowAttributes(windowAtts, extstr, extents2, "Default", 0);

    //
    // Set the annotation attributes.
    //
    cerr << "Setting the annotation attributes." << endl;
    AnnotationAttributes annotAtts;
    annotAtts.SetUserInfoFlag(true);
    annotAtts.SetDatabaseInfoFlag(true);
    annotAtts.SetLegendInfoFlag(true);
    annotAtts.GetAxes2D().SetVisible(true);
    AnnotationObjectList annotObjList;
    VisualCueList visCues;
    int frameAndState[7] = {1, 0, 0, 0, 0, 0, 0};
    netmgr->SetAnnotationAttributes(annotAtts, annotObjList, visCues,
                                    frameAndState, 0, 2);

    //
    // Render the image.
    //
    cerr << "Rendering the image." << endl;
    intVector plotIds;
    plotIds.push_back(0);
    avtDataObject_p image = netmgr->Render(false, plotIds, false, 0, 0, false);

    //
    // Save the image.
    //
    cerr << "Saving the image." << endl;
    avtFileWriter *fileWriter = new avtFileWriter();
    fileWriter->SetFormat(SaveWindowAttributes::PNG);
    fileWriter->Write("output.png", image, 100, false, 1, false);
    delete fileWriter;

    //
    // Clean up and exit. The plugin managers get deleted by the network
    // manager.
    //
    cerr << "Cleaning up." << endl;
    delete netmgr;

    cerr << "Exiting." << endl;

    return 0;
}
