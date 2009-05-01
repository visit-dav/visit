/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <MultiCurveViewer.h>

#include <QTabWidget>
#include <QTimer>
#include <QWidget>

#include <QvisAnnotationWidget.h>
#include <QvisAppearanceWidget.h>
#include <QvisGeneralWidget.h>
#include <QvisMessageDialog.h>

// State objects that we use.
#include <avtDatabaseMetaData.h>
#include <AnnotationAttributes.h>
#include <MessageAttributes.h>
#include <SaveWindowAttributes.h>
#include <ViewAxisArrayAttributes.h>

#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <DataNode.h>
#include <VisItViewer.h>
#include <ViewerMethods.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

// Include this last since something about it on X11 systems seems to
// interfere with some of our other enums. X11 headers must have some
// naughty #define directives.
#include <vtkQtRenderWindow.h>

vtkQtRenderWindow *
MultiCurveViewer::ReturnVisWin(void *data)
{
    MultiCurveViewer *This = (MultiCurveViewer *)data;
    return This->viswin;
}

// ****************************************************************************
// Method: MultiCurveViewer::MultiCurveViewer
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   v         The VisItViewer object that we'll use to control the viewer.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

MultiCurveViewer::MultiCurveViewer(VisItViewer *v) : QMainWindow()
{
    viewer = v;
    setWindowTitle(tr("Multi Curve Viewer"));

    // Create the main window.
    CreateWindow();

    // Create the message window.
    messageDialog = new QvisMessageDialog();
    messageDialog->setModal(true);

    //
    // Register a window creation function (before Setup) that will
    // return the vtkQtRenderWindow objects that we've already
    // parented into our interface.
    //
    viewer->SetWindowCreationCallback(ReturnVisWin, (void *)this);

    // Set the initial widget sensitivity.
    ResetWindow();
}

// ****************************************************************************
// Method: MultiCurveViewer::~MultiCurveViewer
//
// Purpose: 
//   Destructor.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

MultiCurveViewer::~MultiCurveViewer()
{
}

// ****************************************************************************
// Method: MultiCurveViewer::Update
//
// Purpose: 
//   Display a message when the message attributes change.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveViewer::Update(Subject *subject)
{
    if (subject == messageAtts)
        messageDialog->DisplayMessage(messageAtts);
}

// ****************************************************************************
// Method: MultiCurveViewer::AttachSubjects
//
// Purpose: 
//   Attach the attribute subjects to observe.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveViewer::AttachSubjects()
{
    // Connect the message attributes.
    messageAtts = viewer->State()->GetMessageAttributes();
    messageAtts->Attach(this);

    // Connect the index select attributes.
    int IndexSelect = viewer->GetOperatorIndex("IndexSelect");
    AttributeSubject *indexSelectAtts = viewer->State()->GetOperatorAttributes(IndexSelect);
    generalWidget->ConnectIndexSelectAttributes(indexSelectAtts);

    // Connect the multi curve attributes.
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    AttributeSubject *multiCurveAtts = viewer->State()->GetPlotAttributes(MultiCurve);
    appearanceWidget->ConnectMultiCurveAttributes(multiCurveAtts);

    // Connect the annotation attributes.
    AnnotationAttributes *annotAtts = viewer->State()->GetAnnotationAttributes();
    annotationWidget->ConnectAnnotationAttributes(annotAtts);

    // Connect the view axis array attributes.
    ViewAxisArrayAttributes *axisArrayAtts = viewer->State()->GetViewAxisArrayAttributes();
    annotationWidget->ConnectViewAxisArrayAttributes(axisArrayAtts);

    // Connect the annotation object list.
    AnnotationObjectList *annotationObjectList = viewer->State()->GetAnnotationObjectList();
    annotationWidget->ConnectAnnotationObjectList(annotationObjectList);
}

// ****************************************************************************
// Method: MultiCurveViewer::CreateWindow
//
// Purpose: 
//   Create the contents of the window.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveViewer::CreateWindow()
{
    //
    // Create the window.
    //
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *hLayout = new QHBoxLayout(central);
    hLayout->setMargin(10);
    hLayout->setSpacing(10);
    QVBoxLayout *leftLayout = new QVBoxLayout(0);
    leftLayout->setSpacing(10);
    hLayout->addLayout(leftLayout);

    //
    // Create the tabs on the left side of the window.
    //
    tabs = new QTabWidget(central);
    leftLayout->addWidget(tabs);

    //
    // Create the general tab.
    //
    pageGeneral = new QWidget(central);
    tabs->addTab(pageGeneral, tr("General"));

    QVBoxLayout *gLayout = new QVBoxLayout(pageGeneral);

    generalWidget = new QvisGeneralWidget(pageGeneral, viewer);
    connect(generalWidget, SIGNAL(variableTextChanged(const QString &)),
            this, SLOT(variableChanged(const QString &)));
    connect(generalWidget, SIGNAL(indexSelectChanged(const AttributeSubject *)),
            this, SLOT(indexSelectChanged(const AttributeSubject *)));
    gLayout->addWidget(generalWidget);

    //
    // Create the appearance tab.
    //
    pageAppearance = new QWidget(central);
    tabs->addTab(pageAppearance, tr("Appearance"));

    QVBoxLayout *aLayout = new QVBoxLayout(pageAppearance);

    appearanceWidget = new QvisAppearanceWidget(pageAppearance, viewer);
    connect(appearanceWidget, SIGNAL(multiCurveChanged(const AttributeSubject *)),
            this, SLOT(multiCurveChanged(const AttributeSubject *)));
    aLayout->addWidget(appearanceWidget);

    //
    // Create the annotation tab.
    //
    pageAnnotation = new QWidget(central);
    tabs->addTab(pageAnnotation, tr("Annotation"));

    QVBoxLayout *annotLayout = new QVBoxLayout(pageAnnotation);

    annotationWidget = new QvisAnnotationWidget(pageAnnotation, viewer);
    connect(annotationWidget, SIGNAL(annotationChanged(const AnnotationAttributes *)),
            this, SLOT(annotationChanged(const AnnotationAttributes *)));
    connect(annotationWidget, SIGNAL(viewAxisArrayChanged(const ViewAxisArrayAttributes *)),
            this, SLOT(viewAxisArrayChanged(const ViewAxisArrayAttributes *)));
    connect(annotationWidget, SIGNAL(annotationObjectListChanged(const AnnotationObjectList *)),
            this, SLOT(annotationObjectListChanged(const AnnotationObjectList *)));
    annotLayout->addWidget(annotationWidget);

    annotLayout->addStretch(1000);

    //
    // Create the visualization window directly.
    //
    viswin = new vtkQtRenderWindow(central);
    viswin->setMinimumSize(QSize(500,500));
    hLayout->addWidget(viswin, 100);

    //
    // Create the menus.
    //
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("Open . . ."), this, SLOT(selectFile()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Save window"), this, SLOT(saveWindow()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Quit"), qApp, SLOT(quit()));
    
    QMenu *controlsMenu = menuBar()->addMenu(tr("Controls"));
    controlsMenu->addAction(tr("Open GUI"), this, SLOT(openGUI()));
}

// ****************************************************************************
// Method: MultiCurveViewer::CreatePlot
//
// Purpose: 
//   Create a multi curve plot with an index select operator applied to
//   its variable.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveViewer::CreatePlot()
{
    if(generalWidget->GetCurrentVariable() == -1)
        return;

    // Determine the variable.
    std::string var(generalWidget->GetCurrentVariableName().toStdString());

    // Delete the active plots.
    viewer->DelayedMethods()->DeleteActivePlots();

    // Create a title.
    viewer->DelayedMethods()->AddAnnotationObject(AnnotationObject::Text2D, "title0");
    viewer->DelayedMethods()->AddAnnotationObject(AnnotationObject::Text2D, "title1");

    // Create a multi curve plot.
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    viewer->DelayedMethods()->AddPlot(MultiCurve, var);
    AttributeSubject *multiCurveAtts = viewer->DelayedState()->GetPlotAttributes(MultiCurve);
    if(multiCurveAtts != 0)
    {
        DataNode root("root");
        multiCurveAtts->CreateNode(&root, true, false);
        DataNode *multiCurveNode = root.GetNode("MultiCurveAttributes");

        DataNode *multiColorNode = multiCurveNode->GetNode("multiColor");
        DataNode *calNode = multiColorNode->GetNode("ColorAttributeList");
        DataNode **children = calNode->GetChildren();

        unsigned char color[4];
        color[0] = 0; color[1] = 100; color[2] = 0; color[3] = 255;
        children[0]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 85; color[1] = 107; color[2] = 47;
        children[1]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 0; color[1] = 205; color[2] = 208;
        children[2]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 0; color[1] = 0; color[2] = 254;
        children[3]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 209; color[1] = 105; color[2] = 30;
        children[4]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 147; color[1] = 0; color[2] = 210;
        children[5]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 83; color[1] = 133; color[2] = 138;
        children[6]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 204; color[1] = 38; color[2] = 38;
        children[7]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 0; color[1] = 0; color[2] = 0;
        children[8]->GetNode("color")->SetUnsignedCharArray(color, 4);
        color[0] = 220; color[1] = 20; color[2] = 60;
        children[9]->GetNode("color")->SetUnsignedCharArray(color, 4);

        multiCurveNode->GetNode("yAxisTitleFormat")->SetString("%4.2f");
        multiCurveNode->GetNode("useYAxisTickSpacing")->SetBool(true);
        multiCurveNode->GetNode("yAxisTickSpacing")->SetDouble(0.25);
        multiCurveNode->GetNode("markerVariable")->SetString("v");

        multiCurveAtts->SetFromNode(&root);
        multiCurveAtts->Notify();
        viewer->DelayedMethods()->SetPlotOptions(MultiCurve);
    }

    // Add an index select operator.
    int IndexSelect = viewer->GetOperatorIndex("IndexSelect");
    viewer->DelayedMethods()->AddOperator(IndexSelect);
    AttributeSubject *indexSelectAtts = viewer->DelayedState()->GetOperatorAttributes(IndexSelect);
    if(indexSelectAtts != 0)
    {
        indexSelectAtts->SetValue("xMin", 0);
        indexSelectAtts->SetValue("xMax", -1);
        indexSelectAtts->SetValue("yMin", 16);
        indexSelectAtts->SetValue("yMax", 25);
        indexSelectAtts->Notify();
        viewer->DelayedMethods()->SetOperatorOptions(IndexSelect);
    }

    //
    // Issue a timer to finish the plot.
    //
    QTimer::singleShot(20, this, SLOT(finishPlot()));
}

// ****************************************************************************
// Method: MultiCurveViewer::ResetWindow
//
// Purpose: 
//   Reset the window to its initial state.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:43:04 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
MultiCurveViewer::ResetWindow()
{
    viewer->Methods()->DeleteActivePlots();
}

//
// Qt slots
//

void
MultiCurveViewer::show()
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
MultiCurveViewer::selectFile()
{
    // Get a filename from the file dialog.
    QString filename = QFileDialog::getOpenFileName(this,
               tr("Open File"),
               QDir::current().path(),
               tr("Data files (*.silo)"));
        
    if(!filename.isEmpty())
    {
        // Open the file.
        viewer->Methods()->OpenDatabase(filename.toStdString());

        // Get the file's metadata and populate the variable list.
        const avtDatabaseMetaData *md = viewer->GetMetaData(filename.toStdString());
        if(md != 0)
        {
            generalWidget->blockSignals(true);
            generalWidget->ClearVariableList();
            int nVars = 0;
            for (int i = 0; i < md->GetNumScalars(); ++i)
            {
                if (md->GetScalar(i)->centering == AVT_NODECENT)
                {
                    nVars++;
                    generalWidget->AddVariable(md->GetScalar(i)->name.c_str());
                }
            }
            generalWidget->SetCurrentVariable(0);
            generalWidget->blockSignals(false);
            if(nVars > 0)
            {
                // Add a multi curve plot of the first variable
                CreatePlot();
            }
        }
        else
            ResetWindow();
    }
    else
        ResetWindow();
}

void
MultiCurveViewer::variableChanged(const QString &var)
{
    viewer->DelayedMethods()->ChangeActivePlotsVar(var.toStdString());
}

void
MultiCurveViewer::indexSelectChanged(const AttributeSubject *atts)
{
    int IndexSelect = viewer->GetOperatorIndex("IndexSelect");
    viewer->DelayedMethods()->SetOperatorOptions(IndexSelect);
}

void
MultiCurveViewer::multiCurveChanged(const AttributeSubject *atts)
{
    int MultiCurve = viewer->GetPlotIndex("MultiCurve");
    viewer->DelayedMethods()->SetPlotOptions(MultiCurve);
}

void
MultiCurveViewer::annotationChanged(const AnnotationAttributes *atts)
{
    viewer->DelayedMethods()->SetAnnotationAttributes();
}

void
MultiCurveViewer::viewAxisArrayChanged(const ViewAxisArrayAttributes *atts)
{
    viewer->DelayedMethods()->SetViewAxisArray();
}

void
MultiCurveViewer::annotationObjectListChanged(const AnnotationObjectList *atts)
{
    viewer->DelayedMethods()->SetAnnotationObjectOptions();
}

void
MultiCurveViewer::saveWindow()
{
    // Set the output format to JPEG
    SaveWindowAttributes *swa = viewer->State()->GetSaveWindowAttributes();
    swa->SetFormat(SaveWindowAttributes::PNG);
    swa->SetResConstraint(SaveWindowAttributes::NoConstraint);
    swa->SetWidth(1024);
    swa->SetHeight(1024);
    swa->SetScreenCapture(false);

    // Save the window
    viewer->Methods()->SaveWindow();
}

void
MultiCurveViewer::openGUI()
{
    stringVector args;
    viewer->DelayedMethods()->OpenClient("GUI", viewer->GetVisItCommand(), args);
}

void
MultiCurveViewer::finishPlot()
{
    if (viewer->DelayedState()->GetAnnotationObjectList()->GetNumAnnotations() > 0)
    {
        // Set the first title.
        vector<string> text;
        text.push_back("Multi Curve Plot");
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetText(text);
        double position[2];
        position[0] = 0.3; position[1] = 0.92;
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetPosition(position);
        position[0] = 0.4;  // The text width
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetPosition2(position);
        ColorAttribute red;
        red.SetRgba(255, 0, 0, 255);
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetTextColor(red);
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(0).SetUseForegroundForTextColor(false);

        // Set the second title.
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetText(text);
        position[0] = 0.3; position[1] = 0.02;
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetPosition(position);
        position[0] = 0.4;  // The text width
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetPosition2(position);
        red.SetRgba(0, 255, 0, 255);
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetTextColor(red);
        viewer->DelayedState()->GetAnnotationObjectList()->GetAnnotation(1).SetUseForegroundForTextColor(false);

        viewer->DelayedState()->GetAnnotationObjectList()->Notify();
        viewer->DelayedMethods()->SetAnnotationObjectOptions();
    }

    // Draw the plot.
    viewer->DelayedMethods()->DrawPlots();
}
