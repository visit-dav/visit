#include <VisItInit.h>

#include <DatabasePluginInfo.h>
#include <DatabasePluginManager.h>
#include <SaveWindowAttributes.h>

#include <avtContourFilter.h>
#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtFileWriter.h>
#include <avtImage.h>
#include <avtLinearTransformFilter.h>
#include <avtLookupTable.h>
#include <avtOriginatingSource.h>
#include <avtVariablePointGlyphMapper.h>
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

    //
    // Apply a linear transform.
    //
    cerr << "Applying a linear transform." << endl;
    avtLinearTransformFilter *filter = new avtLinearTransformFilter;
    filter->SetInput(dob);
    LinearTransformAttributes atts;
    atts.SetM00(0.5); atts.SetM01(0.);  atts.SetM02(0.); atts.SetM03(0.);
    atts.SetM10(0.);  atts.SetM11(0.5); atts.SetM12(0.); atts.SetM13(0.);
    atts.SetM20(0.);  atts.SetM21(0.);  atts.SetM22(1.); atts.SetM23(0.);
    atts.SetM30(0.);  atts.SetM31(0.);  atts.SetM32(0.); atts.SetM33(1.);
    filter->SetAtts(&atts);
    avtDataObject_p output = filter->GetOutput();

    //
    // Apply a contour filter.
    //
    cerr << "Applying a contour filter." << endl;
    ContourOpAttributes atts2;
    doubleVector levels;
    levels.push_back(0.5);
    atts2.SetContourPercent(levels);
    avtContourFilter *filter2 = new avtContourFilter(atts2);
    filter2->SetInput(output);
    avtDataObject_p output2 = filter2->GetOutput();

    //
    // Create the actor.
    //
    cerr << "Creating the actor." << endl;
    avtVariablePointGlyphMapper *mapper = new avtVariablePointGlyphMapper;
    avtLookupTable *LUT = new avtLookupTable;
    mapper->SetLookupTable(LUT->GetLookupTable());
    mapper->SetMin(0.);
    mapper->SetMax(1.);
    mapper->SetLineWidth(LW_0);
    mapper->SetLineStyle(SOLID);
    mapper->SetPointSize(5.);
    // The following 2 lines are necessary to get an image rendered on
    // LLNL clusters because of nvidia driver bug that affects texture
    // mapping.
    mapper->SetColorTexturingFlag(false);
    mapper->SetColorTexturingFlagAllowed(false);

    avtOriginatingSource *src = output2->GetOriginatingSource();
    avtContract_p contract = new avtContract(src->GetFullDataRequest(), 0);

    mapper->SetInput(output2);
    mapper->Execute(contract);

    avtDrawable_p drawable = mapper->GetDrawable();
    avtDrawable_p decorations = NULL;

    avtDataObjectInformation info;
    info.Copy(output2->GetInfo());
    avtBehavior *behavior = new avtBehavior();
    behavior->SetInfo(info);
    behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_2D);

    avtActor_p actor = new avtActor();
    actor->SetDrawable(drawable);
    actor->SetDecorations(decorations);
    actor->SetBehavior(behavior);

    //
    // Create the window and add the actor.
    //
    cerr << "Creating the window and adding the actor." << endl;
    VisWindow *window = new VisWindow();
    window->Realize();
    window->AddPlot(actor);

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

    delete filter;
    delete filter2;
    delete mapper;
    delete LUT;
    delete behavior;
    delete window;

    cerr << "Exiting." << endl;

    return 0;
}
