#include <SimpleVisApp.h>
#include <VisItViewer.h>
#include <ViewerMethods.h>

#include <qbuttongroup.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qwidget.h>

// State objects that we use.
#include <avtDatabaseMetaData.h>
#include <SaveWindowAttributes.h>

// Include this last since something about it on X11 systems seems to
// interfere with some of our other enums. X11 headers must have some
// naughty #define directives.
#include <vtkQtRenderWindow.h>

vtkQtRenderWindow *
SimpleVisApp::ReturnVisWin(void *data)
{
    SimpleVisApp *This = (SimpleVisApp *)data;
    return This->viswin;
}

SimpleVisApp::SimpleVisApp(VisItViewer *v) : QMainWindow()
{
    viewer = v;
    setCaption(tr("Simple visualization"));
    plotType = 0;

    // Create the window.
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *hLayout = new QHBoxLayout(central);
    hLayout->setMargin(10);
    hLayout->setSpacing(10);
    QVBoxLayout *leftLayout = new QVBoxLayout(0);
    leftLayout->setSpacing(10);
    hLayout->addLayout(leftLayout);

    scalarLabel = new QLabel(tr("Scalar variables"), central);
    leftLayout->addWidget(scalarLabel);

    variables = new QListBox(central);
    leftLayout->addWidget(variables);
    connect(variables, SIGNAL(selected(const QString &)),
            this, SLOT(changeVariable(const QString &)));

    plotTypeWidget = new QWidget(central);
    leftLayout->addWidget(plotTypeWidget);
    QHBoxLayout *ptLayout = new QHBoxLayout(plotTypeWidget);
    ptLayout->setSpacing(10);
    ptLayout->addWidget(new QLabel(tr("Plot type"), plotTypeWidget));
    plotType = new QButtonGroup(0);
    QRadioButton *rb = new QRadioButton(tr("Pseudocolor"), plotTypeWidget);
    plotType->insert(rb, 0);
    ptLayout->addWidget(rb);
    rb = new QRadioButton(tr("Contour"), plotTypeWidget);
    plotType->insert(rb, 1);
    ptLayout->addWidget(rb);
    connect(plotType, SIGNAL(clicked(int)),
            this, SLOT(changePlotType(int)));

    contourWidget = new QWidget(central);
    leftLayout->addWidget(contourWidget);
    QHBoxLayout *cLayout = new QHBoxLayout(contourWidget);
    cLayout->setSpacing(10);
    nContours = new QSpinBox(contourWidget);
    nContours->setMinValue(1);
    nContours->setMaxValue(40);
    nContours->setValue(10);
    connect(nContours, SIGNAL(valueChanged(int)),
            this, SLOT(setNContours(int)));
    cLayout->addWidget(new QLabel(tr("Number of contours"), contourWidget));
    cLayout->addWidget(nContours);

    // Create the vis window directly.
    viswin = new vtkQtRenderWindow(central);
    viswin->setMinimumSize(QSize(500,500));
    hLayout->addWidget(viswin, 100);

    // Create menus
    QPopupMenu *fileMenu = new QPopupMenu();
    fileMenu->insertItem(tr("Open . . ."), this, SLOT(selectFile()));
    fileMenu->insertSeparator();
    fileMenu->insertItem(tr("Save window"), this, SLOT(saveWindow()));
    fileMenu->insertSeparator();
    fileMenu->insertItem(tr("Quit"), qApp, SLOT(quit()));
    menuBar()->insertItem(tr("File"), fileMenu);

    QPopupMenu *controlsMenu = new QPopupMenu();
    controlsMenu->insertItem(tr("Open GUI"), this, SLOT(openGUI()));
    menuBar()->insertItem(tr("Controls"), controlsMenu);

    //
    // Register a window creation function (before Setup) that will
    // return the vtkQtRenderWindow objects that we've already
    // parented into our interface.
    //
    viewer->SetWindowCreationCallback(ReturnVisWin, (void *)this);

    // Set the initial widget sensitivity.
    resetWindow();
}

SimpleVisApp::~SimpleVisApp()
{
    delete plotType;
}

void
SimpleVisApp::resetWindow()
{
    variables->blockSignals(true);
    variables->clear();
    variables->blockSignals(false);

    plotType->blockSignals(true);
    plotType->setButton(0);
    plotType->blockSignals(false);

    scalarLabel->setEnabled(false);
    variables->setEnabled(false);
    plotTypeWidget->setEnabled(false);
    contourWidget->setEnabled(false);

    viewer->Methods()->DeleteActivePlots();
}

//
// Qt slots
//

void
SimpleVisApp::show()
{
    // Tell the viewer to hide toolbars in all windows.
    viewer->Methods()->HideToolbars(true);
    // Tell the viewer to show all windows. This does not show our windows
    // since our windows are embedded but it does do some extra setup for
    // the windows and thus needs to be called.
    viewer->Methods()->ShowAllWindows();

    QMainWindow::show();
}

void
SimpleVisApp::selectFile()
{
    // Get a filename from the file dialog.
    QString filename = QFileDialog::getOpenFileName(
        QDir::current().path(),
        tr("Data files (*.silo *.vtk *.cgns *.nc *.h5 *.pdb *.visit)"),
        this, 
        tr("Open data file"));

    if(!filename.isEmpty())
    {
        // Open the file.
        viewer->Methods()->OpenDatabase(filename.latin1());

        // Get the file's metadata and populate the variable list.
        const avtDatabaseMetaData *md = viewer->GetMetaData(filename);
        if(md != 0)
        {
            variables->blockSignals(true);
            for(int i = 0; i < md->GetNumScalars(); ++i)
                variables->insertItem(md->GetScalar(i)->name.c_str());
            variables->setCurrentItem(0);
            variables->blockSignals(false);
            if(md->GetNumScalars() > 0)
            {
                variables->setEnabled(true);
                plotTypeWidget->setEnabled(true);
                contourWidget->setEnabled(plotType->selectedId() == 1);
                // Add a plot of the first variable
                changePlotType(plotType->selectedId());
            }
            scalarLabel->setEnabled(true);
            variables->setEnabled(true);
        }
        else
            resetWindow();
    }
    else
        resetWindow();
}

void
SimpleVisApp::changeVariable(const QString &var)
{
    viewer->Methods()->ChangeActivePlotsVar(var.latin1());
}

void
SimpleVisApp::changePlotType(int val)
{
    if(variables->currentItem() == -1)
        return;

    // Determine the variable.
    std::string var(variables->currentText().latin1());

    // Delete the active plots.
    viewer->Methods()->DeleteActivePlots();
    if(val == 0)
    {
        int Pseudocolor = viewer->GetPlotIndex("Pseudocolor");
        viewer->Methods()->AddPlot(Pseudocolor, var);
    }
    else
    {
        int Contour = viewer->GetPlotIndex("Contour");
        viewer->Methods()->AddPlot(Contour, var);
    }
    contourWidget->setEnabled(val == 1);
    viewer->Methods()->DrawPlots();
}

void
SimpleVisApp::setNContours(int nc)
{
    int Contour = viewer->GetPlotIndex("Contour");
    AttributeSubject *contourAtts = viewer->DelayedState()->GetPlotAttributes(Contour);
    if(contourAtts != 0)
    {
        contourAtts->SetValue("contourNLevels", nc);
        contourAtts->Notify();
        viewer->DelayedMethods()->SetPlotOptions(Contour);
    }
}

void
SimpleVisApp::saveWindow()
{
    // Set the output format to JPEG
    SaveWindowAttributes *swa = viewer->State()->GetSaveWindowAttributes();
    swa->SetFormat(SaveWindowAttributes::JPEG);
    swa->SetScreenCapture(true);

    // Save the window
    viewer->Methods()->SaveWindow();
}

void
SimpleVisApp::openGUI()
{
    stringVector args;
    viewer->DelayedMethods()->OpenClient("GUI", viewer->GetVisItCommand(), args);
}
