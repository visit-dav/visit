// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QtVisWindow.h>

#include <QApplication>

#include <QVTKOpenGLWidget.h>

#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

#include <vtkDebugStream.h>
#include <vtkObjectFactory.h>
#include <vtkVersion.h>

#include <vtkVisItCellDataToPointData.h>
#include <vtkVisItRectilinearGrid.h>
#include <vtkVisItStructuredGrid.h>

#include <vtkVisItDataSetMapper.h>
#include <vtkVisItRectilinearGrid.h>
#include <vtkVisItStructuredGrid.h>
#include <vtkOpenGLPointMapper.h>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkRenderingOpenGL2)

#include <TimingsManager.h>

#include <DatabasePluginInfo.h>
#include <DatabasePluginManager.h>

#include <avtContourFilter.h>
#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtLinearTransformFilter.h>
#include <avtLookupTable.h>
#include <avtOriginatingSource.h>
#include <avtVariablePointGlyphMapper.h>

//
// A factory that will allow VisIt to override any vtkObject
// with a sub-class of that object.
// This combines the factories from InitVTK and InitVTKRendering.
//
class vtkVisItGraphicsFactory : public vtkObjectFactory
{
  public:
    static vtkVisItGraphicsFactory * New();
    vtkTypeMacro(vtkVisItGraphicsFactory, vtkObjectFactory)

    const char* GetDescription() override { return "vtkVisItGraphicsFactory"; };
    const char* GetVTKSourceVersion() override;

  protected:
    vtkVisItGraphicsFactory();

  private:
    vtkVisItGraphicsFactory(const vtkVisItGraphicsFactory&);
    void operator=(const vtkVisItGraphicsFactory&);
};

vtkStandardNewMacro(vtkVisItGraphicsFactory)

//
// Necessary for each object that will override a vtkObject.
//
VTK_CREATE_CREATE_FUNCTION(vtkVisItDataSetMapper);
VTK_CREATE_CREATE_FUNCTION(vtkVisItRectilinearGrid);
VTK_CREATE_CREATE_FUNCTION(vtkVisItStructuredGrid);
VTK_CREATE_CREATE_FUNCTION(vtkOpenGLPointMapper);
VTK_CREATE_CREATE_FUNCTION(vtkVisItCellDataToPointData);

const char*
vtkVisItGraphicsFactory::GetVTKSourceVersion()
{
    return VTK_SOURCE_VERSION;
}

vtkVisItGraphicsFactory::vtkVisItGraphicsFactory()
{
  this->RegisterOverride("vtkDataSetMapper", "vtkVisItDataSetMapper",
                         "vtkVisItDataSetMapper override vtkDataSetMapper",
                         1,
                         vtkObjectFactoryCreatevtkVisItDataSetMapper);
  this->RegisterOverride("vtkRectilinearGrid", "vtkVisItRectilinearGrid",
                         "vtkVisItRectilinearGrid override vtkRectilinearGrid",
                         1,
                         vtkObjectFactoryCreatevtkVisItRectilinearGrid);
  this->RegisterOverride("vtkStructuredGrid", "vtkVisItStructuredGrid",
                         "vtkVisItStructuredGrid override vtkStructuredGrid",
                         1,
                         vtkObjectFactoryCreatevtkVisItStructuredGrid);
  this->RegisterOverride("vtkPointMapper", "vtkOpenGLPointMapper",
                         "vtkOpenGLPointMapper override vtkPointMapper",
                         1,
                         vtkObjectFactoryCreatevtkOpenGLPointMapper);
  this->RegisterOverride("vtkCellDataToPointData", "vtkVisItCellDataToPointData",
                         "vtkVisItCellDataToPointData override vtkCellDataToPointData",
                         1,
                         vtkObjectFactoryCreatevtkVisItCellDataToPointData);
}

void
AddPlot(VisWindow *visWindow, string dataDir)
{
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
    string fn = dataDir + "/rect2d.silo";
    const char*filename(fn.c_str());
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
        cerr << "Could not open file " << filename << ".  Tried using plugins ";
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
    levels.push_back(0.1);
    levels.push_back(0.2);
    levels.push_back(0.3);
    levels.push_back(0.4);
    levels.push_back(0.5);
    levels.push_back(0.6);
    levels.push_back(0.7);
    levels.push_back(0.8);
    atts2.SetContourPercent(levels);
    avtContourFilter *filter2 = new avtContourFilter(atts2);
    filter2->SetInput(output);
    avtDataObject_p output2 = filter2->GetOutput();

    //
    // Create the actor.
    //
    cerr << "Creating the actor." << endl;
    avtVariableMapper *mapper = new avtVariableMapper;
    avtLookupTable *LUT = new avtLookupTable;
    mapper->SetLookupTable(LUT->GetLookupTable());
    mapper->SetMin(0.);
    mapper->SetMax(1.);
    mapper->SetLineWidth(LW_0);
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
    // Add the actor to the window.
    //
    visWindow->AddPlot(actor);
}

int
main(int argc, char **argv)
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

    int retval = 0;

    // Initialize VTK debugstream. From InitVTKLite.
    vtkDebugStream::Initialize();

    // Register the factory that allows VisIt objects to override vtk objects.
    // This combines the factories from InitVTK and InitVTKRendering.
    vtkVisItGraphicsFactory *graphicsFactory = vtkVisItGraphicsFactory::New();
    vtkObjectFactory::RegisterFactory(graphicsFactory);
    graphicsFactory->Delete();

    //
    // Setting the default QSurfaceFormat required with QVTKOpenGLwidget.
    // This causes Qt to create an OpenGL 3.2 context.
    //
    auto surfaceFormat = QVTKOpenGLWidget::defaultFormat();
    surfaceFormat.setSamples(0);
    surfaceFormat.setAlphaBufferSize(0);
    QSurfaceFormat::setDefaultFormat(surfaceFormat);

    //
    // Initialize the timing manager.
    //
    TimingsManager::Initialize("qtviswinExample");

    //
    // Create the QApplication. This sets the qApp pointer.
    //
    QApplication *mainApp = new QApplication(argc, argv, true);

    VisWindow *visWindow = NULL;
    visWindow = new QtVisWindow(false);

    visWindow->Realize();
    visWindow->Show();
    visWindow->Raise();

    AddPlot(visWindow, dataDir);

    retval = mainApp->exec();

    return retval;
}
