#include <VisItInit.h>

#include <DatabasePluginInfo.h>
#include <DatabasePluginManager.h>
#include <GlobalAttributes.h>

#include <avtCallback.h>
#include <avtContourFilter.h>
#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtDatabaseWriter.h>
#include <avtLinearTransformFilter.h>
#include <avtOriginatingSource.h>

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
// 2) If you specify more than one contour level the program will crash.
//
// 3) If you specify the contour levels with NLevels the program will crash.
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
    // Turn on EAVL.
    //
    avtCallback::SetBackendType(GlobalAttributes::EAVL);

    //
    // Create the database plugin manager.
    //
    cerr << "Creating the database plugin manager." << endl;
    DatabasePluginManager *dbmgr = new DatabasePluginManager;

    dbmgr->Initialize(DatabasePluginManager::Engine, false);

    dbmgr->LoadPluginsNow();

    //
    // Instantiate the database.
    //
    cerr << "Opening the file." << endl;
    const char *filename = "/usr/gapps/visit/data/rect2d.silo";
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

    avtOriginatingSource  *src = dob->GetOriginatingSource();
    avtDataRequest_p ds  = src->GetFullDataRequest();
    avtContract_p contract = new avtContract(ds, 0);

    //
    // Apply a linear transform.
    //
    cerr << "Applying a linear transform." << endl;
    avtLinearTransformFilter *filter = new avtLinearTransformFilter;
    filter->SetInput(dob);
    LinearTransformAttributes atts;
    atts.SetM00(2.); atts.SetM01(0.); atts.SetM02(0.); atts.SetM03(0.);
    atts.SetM10(0.); atts.SetM11(4.); atts.SetM12(0.); atts.SetM13(0.);
    atts.SetM20(0.); atts.SetM21(0.); atts.SetM22(1.); atts.SetM23(0.);
    atts.SetM30(0.); atts.SetM31(0.); atts.SetM32(0.); atts.SetM33(1.);
    filter->SetAtts(&atts);
    avtDataObject_p output = filter->GetOutput();

    //
    // Apply a second linear transform.
    //
    cerr << "Applying a second linear transform." << endl;
    avtLinearTransformFilter *filter2 = new avtLinearTransformFilter;
    filter2->SetInput(output);
    LinearTransformAttributes atts2;
    atts2.SetM00(3.); atts2.SetM01(0.); atts2.SetM02(0.); atts2.SetM03(0.);
    atts2.SetM10(0.); atts2.SetM11(2.); atts2.SetM12(0.); atts2.SetM13(0.);
    atts2.SetM20(0.); atts2.SetM21(0.); atts2.SetM22(1.); atts2.SetM23(0.);
    atts2.SetM30(0.); atts2.SetM31(0.); atts2.SetM32(0.); atts2.SetM33(1.);
    filter2->SetAtts(&atts2);
    avtDataObject_p output2 = filter2->GetOutput();

    //
    // Apply a contour filter.
    //
    cerr << "Applying a contour filter." << endl;
    ContourOpAttributes atts3;
    doubleVector levels;
    levels.push_back(0.5);
    atts3.SetContourPercent(levels);
    avtContourFilter *filter3 = new avtContourFilter(atts3);
    filter3->SetInput(output2);
    filter3->Update(contract);
    avtDataObject_p output3 = filter3->GetOutput();

    //
    // Write out the output.
    //
    cerr << "Writing the output." << endl;
    EngineDatabasePluginInfo *edpi = dbmgr->GetEnginePluginInfo("Silo_1.0");

    if (edpi == NULL)
    {
        exit(EXIT_FAILURE);
    }

    avtDatabaseWriter *wrtr = edpi->GetWriter();

    if (wrtr == NULL)
    {
        exit(EXIT_FAILURE);
    }

    const avtDatabaseMetaData *md = db->GetMetaData(0);

    vector<string> vars;
    vars.push_back("d");
    wrtr->SetInput(output3);
    wrtr->Write("", "output", md, vars);

    //
    // Clean up and exit.
    //
    cerr << "Cleaning up." << endl;
    delete dbmgr;

    delete filter;
    delete filter2;
    delete filter3;

    cerr << "Exiting." << endl;

    return 0;
}
