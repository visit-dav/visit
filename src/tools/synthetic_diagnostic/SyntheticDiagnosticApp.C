#include <SyntheticDiagnosticApp.h>
#include <VisItViewer.h>
#include <ViewerMethods.h>
#include <GlobalAttributes.h>

#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QTabWidget>
#include <QMenuBar>
#include <QMenu>
#include <QRadioButton>
#include <QSpinBox>
#include <QWidget>

// State objects that we use.
#include <avtDatabaseMetaData.h>
#include <PlotList.h>

// Include this last since something about it on X11 systems seems to
// interfere with some of our other enums. X11 headers must have some
// naughty #define directives.
#include <vtkQtRenderWindow.h>

QWidget *
create_application_main_window(VisItViewer *v)
{
    return new SyntheticDiagnosticApp(v);
}

void
show_application_main_window(QWidget *w)
{
    // Do it like this since virtual show isn't working...
    SyntheticDiagnosticApp *app = (SyntheticDiagnosticApp *)w;
    app->show();
    app->raise();
}

// Window creation callback. Returns the vis window pointers to VisIt.
vtkQtRenderWindow *
SyntheticDiagnosticApp::ReturnVisWin(void *data)
{
    SyntheticDiagnosticApp *This = (SyntheticDiagnosticApp *)data;
    // Return vis window 1, 2, ...
    int idx = ++This->currentWindow;
    return (idx <= 3) ? This->viswindows[idx] : NULL;
}

SyntheticDiagnosticApp::SyntheticDiagnosticApp(VisItViewer *v) : QMainWindow()
{
    viewer = v;
    windowsAdded = false;
    currentWindow = 0;
    for(int i = 0; i < NWINDOWS + 1; ++i)
        viswindows[i] = 0;

    setWindowTitle(tr("Synthetic Diagnostic for Fusion"));

    // Create the window.
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout *Layout = new QHBoxLayout(central);
    Layout->setMargin(10);
    Layout->setSpacing(10);

    //Create the left side controls.
    QVBoxLayout *controlsLayout = new QVBoxLayout(0);
    controlsLayout->setSpacing(10);
    Layout->addLayout(controlsLayout);

    controlsLayout->addWidget(new QLabel(tr("Diagnostics"), central));

    diagnostics = new QListWidget(central);
    controlsLayout->addWidget(diagnostics);
    connect(diagnostics, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(onSelectDiagnostic(const QString &)));

    QHBoxLayout *buttonsLayout = new QHBoxLayout(0);
    controlsLayout->addLayout(buttonsLayout);
    buttonsLayout->addWidget(new QPushButton(tr("New Diagnostic"), central));
    buttonsLayout->addWidget(new QPushButton(tr("Delete"), central));

    QTabWidget *controlTabs = new QTabWidget(central);
    controlsLayout->addWidget(controlTabs);
    QWidget *diagnosticTab = new QWidget(central);
    controlTabs->addTab(diagnosticTab, tr("Diagnostic Data"));
    QWidget *simulationTab = new QWidget(central);
    controlTabs->addTab(simulationTab, tr("Simulation"));
    QWidget *miscTab = new QWidget(central);
    controlTabs->addTab(miscTab, tr("Miscellaneous"));

    //Diagnostic tab.
    QVBoxLayout *diagnosticLayout = new QVBoxLayout(diagnosticTab);
    QGroupBox *diagnosticSourceGroup = new QGroupBox(diagnosticTab);
    diagnosticLayout->addWidget(diagnosticSourceGroup);
    QGridLayout *diagnosticSourceLayout = new QGridLayout(diagnosticSourceGroup);
    diagnosticSourceLayout->addWidget(new QLabel("Diagnostic Source: ", diagnosticSourceGroup), 0, 0);
    QComboBox *diagnosticSource = new QComboBox(diagnosticSourceGroup);
    diagnosticSourceLayout->addWidget(diagnosticSource, 0, 1);
    diagnosticSource->addItem(tr("Red Barchetta"));
    diagnosticSource->addItem(tr("The Pass"));
    diagnosticSource->addItem(tr("La Villa Strangiato"));
    diagnosticSourceLayout->addWidget(new QLabel(tr("Diagnostic information"), diagnosticSourceGroup), 1, 0);
    
    QListWidget *diagnosticInfoList = new QListWidget(diagnosticSourceGroup);
    diagnosticSourceLayout->addWidget(diagnosticInfoList, 2,0, 1,2);
    
    diagnosticLayout->addWidget(new QLabel(tr("Post processing"), diagnosticTab));

    diagnosticLayout->addWidget(new QLabel(tr("View"), diagnosticTab));
    QHBoxLayout *diagViewLayout = new QHBoxLayout(0);
    diagnosticLayout->addLayout(diagViewLayout);
    diagViewLayout->addWidget(new QCheckBox(tr("1D"), diagnosticTab));
    diagViewLayout->addWidget(new QCheckBox(tr("2D"), diagnosticTab));
    diagViewLayout->addWidget(new QCheckBox(tr("3D"), diagnosticTab));
    
    diagnosticLayout->addWidget(new QLabel(tr("Options"), diagnosticTab));

    ////////////////////////////////////////////////////////////////////
    //Simulation tab.
    QGroupBox *simulationSourceGroup = new QGroupBox(simulationTab);
    QGridLayout *simulationLayout = new QGridLayout(simulationTab);
    simulationSourceGroup->setTitle("Source");
    simulationLayout->addWidget(simulationSourceGroup);
    QGridLayout *simulationSourceLayout = new QGridLayout(simulationSourceGroup);
    simulationSourceLayout->addWidget(new QLabel(tr("Active source"), simulationSourceGroup), 0,0);
    QComboBox *simulationSource = new QComboBox(simulationSourceGroup);
    simulationSource->addItem(tr("YYZ"));
    simulationSource->addItem(tr("Limelight"));
    simulationSource->addItem(tr("Witch Hunt"));
    simulationSource->addItem(tr("New World Man"));
    simulationSourceLayout->addWidget(simulationSource, 0, 1);

    QGroupBox *simulationTransformGroup = new QGroupBox(simulationSourceGroup);
    QGridLayout *simulationTransformLayout = new QGridLayout(simulationTransformGroup);
    simulationTransformGroup->setTitle(tr("Transformation"));
    simulationSourceLayout->addWidget(simulationTransformGroup, 1,0, 1,2);

    simulationTransformLayout->addWidget(new QLabel(tr("Coordinate system stuff"), simulationTransformGroup), 0, 0);
    simulationTransformLayout->addWidget(new QLabel(tr("Coordinate system stuff"), simulationTransformGroup), 0, 1);
    simulationTransformLayout->addWidget(new QLabel(tr("Other stuf...."), simulationTransformGroup), 1, 0);
    simulationTransformLayout->addWidget(new QLabel(tr("Other stuf...."), simulationTransformGroup), 1, 1);

    QGroupBox *syntheticDiagnosticGroup = new QGroupBox(simulationTab);
    QGridLayout *syntheticDiagnosticLayout = new QGridLayout(syntheticDiagnosticGroup);
    syntheticDiagnosticGroup->setTitle(tr("Synthetic diagnostic"));
    simulationLayout->addWidget(syntheticDiagnosticGroup);    
    CreateSimulationGUI(syntheticDiagnosticGroup, syntheticDiagnosticLayout);

#if 0
    QGroupBox *syntheticDiagnosticGroup = new QGroupBox(simulationTab);
    int row = 0;
    QGridLayout *syntheticDiagnosticLayout = new QGridLayout(syntheticDiagnosticGroup);
    syntheticDiagnosticGroup->setTitle(tr("Synthetic diagnostic"));
    simulationLayout->addWidget(syntheticDiagnosticGroup);

    syntheticDiagnosticLayout->addWidget(new QLabel(tr("Beam type"), syntheticDiagnosticGroup), row, 0);
    QComboBox *beamType = new QComboBox(syntheticDiagnosticGroup);
    beamType->addItem(tr("Parallel"));
    beamType->addItem(tr("Fan"));
    syntheticDiagnosticLayout->addWidget(beamType, row, 1);
    syntheticDiagnosticLayout->addWidget(new QLabel(tr("Beam shape"), syntheticDiagnosticGroup), row, 2);
    QComboBox *beamShape = new QComboBox(syntheticDiagnosticGroup);
    beamShape->addItem(tr("Line"));
    beamShape->addItem(tr("Cylinder"));
    beamShape->addItem(tr("Cone"));
    syntheticDiagnosticLayout->addWidget(beamShape, row, 3);
    row++;
    
    syntheticDiagnosticLayout->addWidget(new QLabel(tr("Cylinder radius"), syntheticDiagnosticGroup), row, 0);
    QLineEdit *cylinderRadius = new QLineEdit(syntheticDiagnosticGroup);
    syntheticDiagnosticLayout->addWidget(cylinderRadius, row, 1);
    row++;
    
    syntheticDiagnosticLayout->addWidget(new QLabel(tr("Cone divergence"), syntheticDiagnosticGroup), row, 0);
    QLineEdit *coneDivergence = new QLineEdit(syntheticDiagnosticGroup);
    syntheticDiagnosticLayout->addWidget(coneDivergence, row, 1);
    row++;

    syntheticDiagnosticLayout->addWidget(new QLabel(tr("Number of beams"), syntheticDiagnosticGroup), row, 0);
    QSpinBox *numberOfBeams = new QSpinBox(syntheticDiagnosticGroup);
    numberOfBeams->setMinimum(1);
    numberOfBeams->setMaximum(1000);
    syntheticDiagnosticLayout->addWidget(numberOfBeams, row, 1);
    row++;
    
    syntheticDiagnosticLayout->addWidget(new QLabel(tr("Linear sample distance"), syntheticDiagnosticGroup), row, 0);
    QLineEdit *linearSampleDistance = new QLineEdit(syntheticDiagnosticGroup);
    syntheticDiagnosticLayout->addWidget(linearSampleDistance, row, 1);
    row++;
    syntheticDiagnosticLayout->addWidget(new QLabel(tr("Radial sample arc"), syntheticDiagnosticGroup), row, 0);
    QLineEdit *radialSampleArc = new QLineEdit(syntheticDiagnosticGroup);
    syntheticDiagnosticLayout->addWidget(radialSampleArc, row, 1);
    row++;
#endif

    /* NOTES
     * beam type: parallel/divergent(fan)
     * offsetbeams: parallel angle between: divergent
     * origin: from
     * all beam types:
     *   beam direction, etc.
     * View dimension: Appearance stuff.
     * sampling:
     *  beam type: all variations.
     *  std deviation:
     * /
    
    

        
/*
    simulationLayout->addWidget(new QPushButton(tr("Line Probe Properties"), simulationTab));

    simulationLayout->addWidget(new QLabel(tr("View"), simulationTab));
    QHBoxLayout *simViewLayout = new QHBoxLayout(0);
    simulationLayout->addLayout(simViewLayout);
    simViewLayout->addWidget(new QCheckBox(tr("1D"), simulationTab));
    simViewLayout->addWidget(new QCheckBox(tr("2D"), simulationTab));
    simViewLayout->addWidget(new QCheckBox(tr("3D"), simulationTab));
    
    simulationLayout->addWidget(new QLabel(tr("Options"), simulationTab));
*/

    //Misc tab.
    QVBoxLayout *miscLayout = new QVBoxLayout(miscTab);
    miscLayout->addWidget(new QLabel(tr("misc"), miscTab));
    miscLayout->addWidget(new QLabel(tr("misc"), miscTab));
    miscLayout->addWidget(new QLabel(tr("misc"), miscTab));
    
    //Create the viz windows.
    QSplitter *splitter1 = new QSplitter(central);
    splitter1->setOrientation(Qt::Horizontal);
    Layout->addWidget(splitter1, 100);

    viswindows[WINDOW_2D] = new vtkQtRenderWindow(splitter1);
    viswindows[WINDOW_2D]->setMinimumSize(QSize(100,150));

    viswindows[WINDOW_3D] = new vtkQtRenderWindow(splitter1);
    viswindows[WINDOW_3D]->setMinimumSize(QSize(100,150));
    
    viswindows[WINDOW_1D] = new vtkQtRenderWindow(splitter1);
    viswindows[WINDOW_1D]->setMinimumSize(QSize(100,150));
 
    //
    // Register a window creation function (before Setup) that will
    // return the vtkQtRenderWindow objects that we've already
    // parented into our interface.
    //
    viewer->SetWindowCreationCallback(ReturnVisWin, (void *)this);

    // Create menus
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("Open . . ."), this, SLOT(selectFile()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Quit"), qApp, SLOT(quit()));

    QMenu *controlsMenu = menuBar()->addMenu(tr("Controls"));
    controlsMenu->addAction(tr("Open GUI"), this, SLOT(openGUI()));
}

SyntheticDiagnosticApp::~SyntheticDiagnosticApp()
{
}

void
SyntheticDiagnosticApp::show()
{
    // Tell the viewer to hide toolbars in all windows.
    viewer->DelayedMethods()->HideToolbars(true);

    if(!windowsAdded)
    {
        windowsAdded = true;
        // Call AddWindow() N-1 times to make sure that VisIt calls the window
        // creation callback the appropriate number of times.
        viewer->DelayedMethods()->ToggleLockViewMode();
        for(int i = 0; i < NWINDOWS-1; ++i)
        {
            viewer->DelayedMethods()->AddWindow();
            viewer->DelayedMethods()->ToggleLockViewMode();
        }
    }
    viewer->DelayedMethods()->ShowAllWindows();
 
    QMainWindow::show();
}

void
SyntheticDiagnosticApp::CreateSimulationGUI(QWidget *parent, QGridLayout *mainLayout)
{
    coordinateSystemLabel = new QLabel(tr("Coordinate Systems"), parent);
    mainLayout->addWidget(coordinateSystemLabel,0,0);
    coordinateSystem = new QWidget(parent);
    coordinateSystemButtonGroup= new QButtonGroup(coordinateSystem);
    QHBoxLayout *coordinateSystemLayout = new QHBoxLayout(coordinateSystem);
    coordinateSystemLayout->setMargin(0);
    coordinateSystemLayout->setSpacing(10);
    QRadioButton *coordinateSystemCoordinateSystemCartesian = new QRadioButton(tr("Cartesian"), coordinateSystem);
    coordinateSystemButtonGroup->addButton(coordinateSystemCoordinateSystemCartesian,0);
    coordinateSystemLayout->addWidget(coordinateSystemCoordinateSystemCartesian);
    QRadioButton *coordinateSystemCoordinateSystemCylindrical = new QRadioButton(tr("Cylindrical"), coordinateSystem);
    coordinateSystemButtonGroup->addButton(coordinateSystemCoordinateSystemCylindrical,1);
    coordinateSystemLayout->addWidget(coordinateSystemCoordinateSystemCylindrical);
//    connect(coordinateSystemButtonGroup, SIGNAL(buttonClicked(int)),
//            this, SLOT(coordinateSystemChanged(int)));
    mainLayout->addWidget(coordinateSystem, 0,1);

    beamShapeLabel = new QLabel(tr("Beam shape"), parent);
    mainLayout->addWidget(beamShapeLabel,1,0);
    beamShape = new QWidget(parent);
    beamShapeButtonGroup= new QButtonGroup(beamShape);
    QHBoxLayout *beamShapeLayout = new QHBoxLayout(beamShape);
    beamShapeLayout->setMargin(0);
    beamShapeLayout->setSpacing(10);
    QRadioButton *beamShapeBeamShapeLine = new QRadioButton(tr("Line"), beamShape);
    beamShapeButtonGroup->addButton(beamShapeBeamShapeLine,0);
    beamShapeLayout->addWidget(beamShapeBeamShapeLine);
    QRadioButton *beamShapeBeamShapeCylinder = new QRadioButton(tr("Cylinder"), beamShape);
    beamShapeButtonGroup->addButton(beamShapeBeamShapeCylinder,1);
    beamShapeLayout->addWidget(beamShapeBeamShapeCylinder);
    QRadioButton *beamShapeBeamShapeCone = new QRadioButton(tr("Cone"), beamShape);
    beamShapeButtonGroup->addButton(beamShapeBeamShapeCone,2);
    beamShapeLayout->addWidget(beamShapeBeamShapeCone);
//    connect(beamShapeButtonGroup, SIGNAL(buttonClicked(int)),
//            this, SLOT(beamShapeChanged(int)));
    mainLayout->addWidget(beamShape, 1,1);

    radiusLabel = new QLabel(tr("Cylinder radius"), parent);
    mainLayout->addWidget(radiusLabel,2,0);
    radius = new QLineEdit(parent);
//    connect(radius, SIGNAL(returnPressed()),
//            this, SLOT(radiusProcessText()));
    mainLayout->addWidget(radius, 2,1);

    divergenceLabel = new QLabel(tr("Cone divergence (degrees)"), parent);
    mainLayout->addWidget(divergenceLabel,3,0);
    divergence = new QLineEdit(parent);
//    connect(divergence, SIGNAL(returnPressed()),
//            this, SLOT(divergenceProcessText()));
    mainLayout->addWidget(divergence, 3,1);

    nBeamsLabel = new QLabel(tr("Number of beams"), parent);
    mainLayout->addWidget(nBeamsLabel,4,0);
    nBeams = new QLineEdit(parent);
//    connect(nBeams, SIGNAL(returnPressed()),
//            this, SLOT(nBeamsProcessText()));
    mainLayout->addWidget(nBeams, 4,1);

    beamProjectionLabel = new QLabel(tr("Beam group projection"), parent);
    mainLayout->addWidget(beamProjectionLabel,5,0);
    beamProjection = new QWidget(parent);
    beamProjectionButtonGroup= new QButtonGroup(beamProjection);
    QHBoxLayout *beamProjectionLayout = new QHBoxLayout(beamProjection);
    beamProjectionLayout->setMargin(0);
    beamProjectionLayout->setSpacing(10);
    QRadioButton *beamProjectionBeamProjectionParallel = new QRadioButton(tr("Parallel"), beamProjection);
    beamProjectionButtonGroup->addButton(beamProjectionBeamProjectionParallel,0);
    beamProjectionLayout->addWidget(beamProjectionBeamProjectionParallel);
    QRadioButton *beamProjectionBeamProjectionDivergent = new QRadioButton(tr("Divergent"), beamProjection);
    beamProjectionButtonGroup->addButton(beamProjectionBeamProjectionDivergent,1);
    beamProjectionLayout->addWidget(beamProjectionBeamProjectionDivergent);
//    connect(beamProjectionButtonGroup, SIGNAL(buttonClicked(int)),
//            this, SLOT(beamProjectionChanged(int)));
    mainLayout->addWidget(beamProjection, 5,1);

    offsetLabel = new QLabel(tr("Offset between beams"), parent);
    mainLayout->addWidget(offsetLabel,6,0);
    offset = new QLineEdit(parent);
//    connect(offset, SIGNAL(returnPressed()),
//            this, SLOT(offsetProcessText()));
    mainLayout->addWidget(offset, 6,1);

    angleLabel = new QLabel(tr("Angle between beams (degrees)"), parent);
    mainLayout->addWidget(angleLabel,7,0);
    angle = new QLineEdit(parent);
//    connect(angle, SIGNAL(returnPressed()),
//            this, SLOT(angleProcessText()));
    mainLayout->addWidget(angle, 7,1);

    originLabel = new QLabel(tr("Origin"), parent);
    mainLayout->addWidget(originLabel,8,0);
    origin = new QLineEdit(parent);
//    connect(origin, SIGNAL(returnPressed()),
//            this, SLOT(originProcessText()));
    mainLayout->addWidget(origin, 8,1);

    beamAxisLabel = new QLabel(tr("Beam axis direction"), parent);
    mainLayout->addWidget(beamAxisLabel,9,0);
    beamAxis = new QWidget(parent);
    beamAxisButtonGroup= new QButtonGroup(beamAxis);
    QHBoxLayout *beamAxisLayout = new QHBoxLayout(beamAxis);
    beamAxisLayout->setMargin(0);
    beamAxisLayout->setSpacing(10);
    QRadioButton *beamAxisBeamAxisR = new QRadioButton(tr("R"), beamAxis);
    beamAxisButtonGroup->addButton(beamAxisBeamAxisR,0);
    beamAxisLayout->addWidget(beamAxisBeamAxisR);
    QRadioButton *beamAxisBeamAxisZ = new QRadioButton(tr("Z"), beamAxis);
    beamAxisButtonGroup->addButton(beamAxisBeamAxisZ,1);
    beamAxisLayout->addWidget(beamAxisBeamAxisZ);
//    connect(beamAxisButtonGroup, SIGNAL(buttonClicked(int)),
//            this, SLOT(beamAxisChanged(int)));
    mainLayout->addWidget(beamAxis, 9,1);

    poloialAngleLabel = new QLabel(tr("Poloidal angle (degrees)"), parent);
    mainLayout->addWidget(poloialAngleLabel,10,0);
    poloialAngle = new QLineEdit(parent);
//    connect(poloialAngle, SIGNAL(returnPressed()),
//            this, SLOT(poloialAngleProcessText()));
    mainLayout->addWidget(poloialAngle, 10,1);

    poloialRTiltLabel = new QLabel(tr("Poloidal plane R-tilt (degrees)"), parent);
    mainLayout->addWidget(poloialRTiltLabel,11,0);
    poloialRTilt = new QLineEdit(parent);
//    connect(poloialRTilt, SIGNAL(returnPressed()),
//            this, SLOT(poloialRTiltProcessText()));
    mainLayout->addWidget(poloialRTilt, 11,1);

    poloialZTiltLabel = new QLabel(tr("Poloidal plane Z-tilt (degrees)"), parent);
    mainLayout->addWidget(poloialZTiltLabel,12,0);
    poloialZTilt = new QLineEdit(parent);
//    connect(poloialZTilt, SIGNAL(returnPressed()),
//            this, SLOT(poloialZTiltProcessText()));
    mainLayout->addWidget(poloialZTilt, 12,1);

    toroialAngleLabel = new QLabel(tr("Toroidal angle (degrees)"), parent);
    mainLayout->addWidget(toroialAngleLabel,13,0);
    toroialAngle = new QLineEdit(parent);
//    connect(toroialAngle, SIGNAL(returnPressed()),
//            this, SLOT(toroialAngleProcessText()));
    mainLayout->addWidget(toroialAngle, 13,1);

    viewDimensionLabel = new QLabel(tr("View dimension"), parent);
    mainLayout->addWidget(viewDimensionLabel,14,0);
    viewDimension = new QWidget(parent);
    viewDimensionButtonGroup= new QButtonGroup(viewDimension);
    QHBoxLayout *viewDimensionLayout = new QHBoxLayout(viewDimension);
    viewDimensionLayout->setMargin(0);
    viewDimensionLayout->setSpacing(10);
    QRadioButton *viewDimensionViewDimensionOne = new QRadioButton(tr("One"), viewDimension);
    viewDimensionButtonGroup->addButton(viewDimensionViewDimensionOne,0);
    viewDimensionLayout->addWidget(viewDimensionViewDimensionOne);
    QRadioButton *viewDimensionViewDimensionTwo = new QRadioButton(tr("Two"), viewDimension);
    viewDimensionButtonGroup->addButton(viewDimensionViewDimensionTwo,1);
    viewDimensionLayout->addWidget(viewDimensionViewDimensionTwo);
    QRadioButton *viewDimensionViewDimensionThree = new QRadioButton(tr("Three"), viewDimension);
    viewDimensionButtonGroup->addButton(viewDimensionViewDimensionThree,2);
    viewDimensionLayout->addWidget(viewDimensionViewDimensionThree);
//    connect(viewDimensionButtonGroup, SIGNAL(buttonClicked(int)),
//            this, SLOT(viewDimensionChanged(int)));
    mainLayout->addWidget(viewDimension, 14,1);

    beamTypeLabel = new QLabel(tr("Beam Type"), parent);
    mainLayout->addWidget(beamTypeLabel,15,0);
    beamType = new QWidget(parent);
    beamTypeButtonGroup= new QButtonGroup(beamType);
    QHBoxLayout *beamTypeLayout = new QHBoxLayout(beamType);
    beamTypeLayout->setMargin(0);
    beamTypeLayout->setSpacing(10);
    QRadioButton *beamTypeBeamTypeTopHat = new QRadioButton(tr("TopHat"), beamType);
    beamTypeButtonGroup->addButton(beamTypeBeamTypeTopHat,0);
    beamTypeLayout->addWidget(beamTypeBeamTypeTopHat);
    QRadioButton *beamTypeBeamTypeGaussian = new QRadioButton(tr("Gaussian"), beamType);
    beamTypeButtonGroup->addButton(beamTypeBeamTypeGaussian,1);
    beamTypeLayout->addWidget(beamTypeBeamTypeGaussian);
//    connect(beamTypeButtonGroup, SIGNAL(buttonClicked(int)),
//            this, SLOT(beamTypeChanged(int)));
    mainLayout->addWidget(beamType, 15,1);

    standardDeviationLabel = new QLabel(tr("Standard deviation"), parent);
    mainLayout->addWidget(standardDeviationLabel,16,0);
    standardDeviation = new QLineEdit(parent);
//    connect(standardDeviation, SIGNAL(returnPressed()),
//            this, SLOT(standardDeviationProcessText()));
    mainLayout->addWidget(standardDeviation, 16,1);

    sampleDistanceLabel = new QLabel(tr("Linear sample distance"), parent);
    mainLayout->addWidget(sampleDistanceLabel,17,0);
    sampleDistance = new QLineEdit(parent);
//    connect(sampleDistance, SIGNAL(returnPressed()),
//            this, SLOT(sampleDistanceProcessText()));
    mainLayout->addWidget(sampleDistance, 17,1);

    sampleArcLabel = new QLabel(tr("Radial sample arc (degrees)"), parent);
    mainLayout->addWidget(sampleArcLabel,18,0);
    sampleArc = new QLineEdit(parent);
//    connect(sampleArc, SIGNAL(returnPressed()),
//            this, SLOT(sampleArcProcessText()));
    mainLayout->addWidget(sampleArc, 18,1);

}

void
SyntheticDiagnosticApp::selectFile()
{
    // Get a filename from the file dialog.
    QString filename = QFileDialog::getOpenFileName(this,
               tr("Open File"),
               QDir::current().path(),
               tr("Data files (*.silo *.vtk *.cgns *.nc *.h5 *.pdb *.visit)"));

    filename = QDir::toNativeSeparators(filename);

    openFile(filename);
}

void
SyntheticDiagnosticApp::openFile(const QString &filename)
{
    if(!filename.isEmpty())
    {
        activeFile = filename;
        resetWindow();

        // Get the file's metadata and populate the variable list.
        const avtDatabaseMetaData *md = viewer->GetMetaData(filename.toStdString());
        if(md != 0)
        {
            diagnostics->blockSignals(true);
            for(int i = 0; i < md->GetNumScalars(); ++i)
                diagnostics->addItem(md->GetScalar(i)->name.c_str());
            diagnostics->blockSignals(false);
            diagnostics->setEnabled(md->GetNumScalars() > 0);
        }
    }
}

void
SyntheticDiagnosticApp::resetWindow()
{
    diagnostics->blockSignals(true);
    diagnostics->clear();
    diagnostics->blockSignals(false);

    viewer->Methods()->SetActiveWindow(WINDOW_1D);
    viewer->Methods()->DeleteActivePlots();
    viewer->Methods()->SetActiveWindow(WINDOW_2D);
    viewer->Methods()->DeleteActivePlots();
    viewer->Methods()->SetActiveWindow(WINDOW_3D);
    viewer->Methods()->DeleteActivePlots();
}

void
SyntheticDiagnosticApp::onSelectDiagnostic(const QString &var)
{
    /*
    viewer->State()->GetGlobalAttributes()->SetApplyOperator(false);

    if(viewer->State()->GetPlotList()->GetNumPlots() == 1)
    {
        viewer->DelayedMethods()->SetActiveWindow(WINDOW_1D);
        viewer->DelayedMethods()->ChangeActivePlotsVar(var.toStdString());

        viewer->DelayedMethods()->SetActiveWindow(WINDOW_2D);
        viewer->DelayedMethods()->ChangeActivePlotsVar(var.toStdString());

        viewer->DelayedMethods()->SetActiveWindow(WINDOW_3D);
        viewer->DelayedMethods()->ChangeActivePlotsVar(var.toStdString());
    }
    else
    {
        viewer->DelayedMethods()->SetActiveWindow(WINDOW_3D);
        viewer->DelayedMethods()->OpenDatabase(activeFile.toStdString());
        viewer->DelayedMethods()->DeleteActivePlots();
        viewer->DelayedMethods()->AddPlot(viewer->GetPlotIndex("Contour"),
                                          var.toStdString());
        viewer->DelayedMethods()->DrawPlots();
        

        viewer->DelayedMethods()->SetActiveWindow(WINDOW_2D);
        viewer->DelayedMethods()->OpenDatabase(activeFile.toStdString());
        viewer->DelayedMethods()->DeleteActivePlots();
        viewer->DelayedMethods()->AddPlot(viewer->GetPlotIndex("Pseudocolor"),
                                          var.toStdString());
        viewer->DelayedMethods()->AddOperator(viewer->GetOperatorIndex("Slice"));
        viewer->DelayedMethods()->DrawPlots();

        viewer->DelayedMethods()->SetActiveWindow(WINDOW_1D);
        viewer->DelayedMethods()->OpenDatabase(activeFile.toStdString());
        viewer->DelayedMethods()->DeleteActivePlots();
        viewer->DelayedMethods()->AddPlot(viewer->GetPlotIndex("Pseudocolor"),
                                          var.toStdString());
        viewer->DelayedMethods()->DrawPlots();
    }
    */
}

void
SyntheticDiagnosticApp::openGUI()
{
    stringVector args;
    viewer->DelayedMethods()->OpenClient("GUI", viewer->GetVisItCommand(), args);
}

void
SyntheticDiagnosticApp::showTheWindow()
{
    show();
    raise();
}
