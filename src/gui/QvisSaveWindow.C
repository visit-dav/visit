/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <stdio.h> // for sscanf

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QNarrowLineEdit.h>
#include <QPushButton>
#include <QSlider>
#include <QButtonGroup>
#include <QRadioButton>

#include <QvisSaveWindow.h>
#include <QvisOpacitySlider.h>
#include <SaveWindowAttributes.h>
#include <ViewerProxy.h>

#include <visit-config.h>


// ****************************************************************************
// Method: QvisSaveWindow::QvisSaveWindow
//
// Purpose: 
//   Constructor for the QvisSaveWindow class.
//
// Arguments:
//   subj      : The SaveWindowAttributes object that the window observes.
//   caption   : The string that appears in the window decoration.
//   shortName : The name to use when the window is posted.
//   notepad   : The notepad widget to which the window posts.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 16:52:00 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed May 2 11:38:31 PDT 2001
//   Modified the last argument to the constructor.
//
//   Hank Childs, Fri May 24 07:49:28 PDT 2002
//   Renamed SaveImageAtts to SaveWindowAtts.
//
//   Brad Whitlock, Wed Apr  9 10:56:38 PDT 2008
//   QString for caption, shortName.
//
//   Kathleen Biagas, Wed Jan  7 12:39:12 PST 2015
//   Added dismissOnSave.
//
//   Eric Brugger, Mon Aug 31 10:38:12 PDT 2015
//   I overhauled the window.
//
// ****************************************************************************

QvisSaveWindow::QvisSaveWindow(
    SaveWindowAttributes *subj, const QString &caption, const QString &shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    saveWindowAtts = subj;
    currentWindow = 0;
    dismissOnSave = true;
    multiWindowSaveMode = Tiled;
    multiWindowSaveEnabled = false;
    if (saveWindowAtts->GetSaveTiled())
    {
        multiWindowSaveEnabled = true;
    }
    else if (saveWindowAtts->GetAdvancedMultiWindowSave())
    {
        multiWindowSaveMode = Advanced;
        multiWindowSaveEnabled = true;
    }
    ignoreNextMultiWindowSaveChange = false;
}

// ****************************************************************************
// Method: QvisSaveWindow::~QvisSaveWindow
//
// Purpose: 
//   This is the destructor for the QvisSaveWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:10:02 PST 2001
//
// Modifications:
//   
// ****************************************************************************

QvisSaveWindow::~QvisSaveWindow()
{
}

// ****************************************************************************
// Method: QvisSaveWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the Aslice operator window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:09:50 PST 2001
//
// Modifications:
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Added STL.
//
//   Hank Childs, Sun May 26 17:31:18 PDT 2002
//   Added OBJ, VTK, and Curve.
//
//   Brad Whitlock, Wed Dec 4 16:45:15 PST 2002
//   Added Postscript.
//
//   Jeremy Meredith, Sat Apr 12 15:09:03 PDT 2003
//   Added Ultra format.
//
//   Hank Childs, Wed Oct 15 08:58:16 PDT 2003
//   Added stereo button.
//
//   Kathleen Bonnell, Thu Nov 13 12:15:25 PST 2003 
//   Added compression type combo box.
//
//   Mark C. Miller, Mon Mar 29 16:21:14 PST 2004
//   Elminated code that disabled parts of GUI controlling off-screen
//   toggle and image size
//
//   Hank Childs, Wed Apr  7 07:50:57 PDT 2004
//   Re-disable tiled windows and host information, since that is not working
//   yet.
//
//   Brad Whitlock, Fri Jul 30 15:25:59 PST 2004
//   I replaced the host line edit with an output directory line edit. I also
//   added a save button.
//
//   Kathleen Bonnell, Wed Dec 15 08:20:11 PST 2004 
//   Changed SLOT for saveButton from 'saveWindow' to 'saveButtonClicked' so
//   that 'apply' can be called before the save.
//
//   Brad Whitlock, Thu Jul 14 13:50:15 PST 2005
//   Prevented "..." button max width from being set on MacOS X.
//
//   Jeremy Meredith, Thu Apr  5 17:25:40 EDT 2007
//   Added the ".pov" (POVRay) file type.
//   Added button to force a merge of parallel geometry.
//
//   Dave Bremer, Fri Sep 28 17:18:41 PDT 2007
//   Changed the "maintain aspect" checkbox to radio buttons that either
//   constrain to a 1:1 ratio, constrain to the screen proportions, or
//   use unconstrained resolution.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Dave Pugmire, Thu Jul  8 08:30:11 EDT 2010
//   Added PLY writer.
//
//   Hank Childs, Thu Jul 22 09:55:03 PDT 2010
//   Added widgets for advanced multi-window saves.
//
//   Brad Whitlock, Wed Aug 25 13:33:08 PDT 2010
//   I moved some code into helper methods.
//
//   Kathleen Biagas, Wed Jan  7 12:43:15 PST 2015
//   Added 'Save and Dismiss' button.
//
//   Eric Brugger, Mon Aug 31 10:38:12 PDT 2015
//   I overhauled the window.
//
// ****************************************************************************

void
QvisSaveWindow::CreateWindowContents()
{
    // Create a group box for the file information.
    QGroupBox *nameBox = new QGroupBox(central);
    nameBox->setTitle(tr("Filename"));
    topLayout->addWidget(nameBox);

    QGridLayout *nameLayout = new QGridLayout(nameBox);

    QLabel *filenameLabel = new QLabel(tr("Filename"), nameBox);
    filenameLineEdit = new QLineEdit(nameBox);
    connect(filenameLineEdit, SIGNAL(returnPressed()), this, SLOT(processFilenameText()));
    nameLayout->addWidget(filenameLabel, 0, 0);
    nameLayout->addWidget(filenameLineEdit, 0, 1);

    familyCheckBox = new QCheckBox(tr("Family"), central);
    connect(familyCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(familyToggled(bool)));
    nameLayout->addWidget(familyCheckBox, 0, 3);

    // The binary toggle.
    outputToCurrentDirectoryCheckBox = new QCheckBox(tr("Output files to current directory"),
                                                     nameBox);
    connect(outputToCurrentDirectoryCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(outputToCurrentDirectoryToggled(bool)));
    nameLayout->addWidget(outputToCurrentDirectoryCheckBox, 1, 0, 1, 2);

    outputDirectoryLabel    = new QLabel(tr("Output directory"), nameBox);
    nameLayout->addWidget(outputDirectoryLabel, 2, 0, 1, 2);
    
    QHBoxLayout *outputDirectoryLayout = new QHBoxLayout();
    outputDirectoryLayout->setMargin(0);
    outputDirectoryLayout->setSpacing(0);
    nameLayout->addLayout(outputDirectoryLayout, 3, 0, 1, 4);

    outputDirectoryLineEdit     = new QLineEdit(nameBox);
    outputDirectorySelectButton = new QPushButton("...", nameBox);
    outputDirectoryLayout->addWidget(outputDirectoryLineEdit);
    outputDirectoryLayout->addWidget(outputDirectorySelectButton);
    
    connect(outputDirectoryLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOutputDirectoryText()));

#if !(defined(Q_WS_MACX) || defined(Q_OS_MAC))
    outputDirectorySelectButton->setMaximumWidth(
         fontMetrics().boundingRect("...").width() + 6);
#endif
    outputDirectorySelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                               QSizePolicy::Minimum));
    connect(outputDirectorySelectButton, SIGNAL(clicked()),
            this, SLOT(selectOutputDirectory()));
    
    // Create a group box for the file format.
    QGroupBox *formatBox = new QGroupBox(central);
    formatBox->setTitle(tr("Format options"));
    topLayout->addWidget(formatBox);

    QGridLayout *formatLayout = new QGridLayout(formatBox);

    QLabel *formatLabel = new QLabel(tr("File type"),formatBox);
    fileFormatComboBox = new QComboBox(formatBox);
    fileFormatComboBox->addItem("bmp");
    fileFormatComboBox->addItem("curve");
    fileFormatComboBox->addItem("jpeg");
    fileFormatComboBox->addItem("obj");
    fileFormatComboBox->addItem("png");
    fileFormatComboBox->addItem("postscript");
    fileFormatComboBox->addItem("pov");
    fileFormatComboBox->addItem("ppm");
    fileFormatComboBox->addItem("rgb");
    fileFormatComboBox->addItem("stl");
    fileFormatComboBox->addItem("tiff");
    fileFormatComboBox->addItem("ultra");
    fileFormatComboBox->addItem("vtk");
    fileFormatComboBox->addItem("ply");
    connect(fileFormatComboBox, SIGNAL(activated(int)),
           this, SLOT(fileFormatChanged(int)));
    formatLayout->addWidget(formatLabel, 0, 0);
    formatLayout->addWidget(fileFormatComboBox, 0, 1, 1, 2);

    // The quality slider.
    qualityLabel = new QLabel(tr("Quality"),formatBox);
    qualitySlider = new QSlider(Qt::Horizontal, formatBox);
    qualitySlider->setMinimum(0);
    qualitySlider->setMaximum(100);
    
    connect(qualitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(qualityChanged(int)));
    formatLayout->addWidget(qualitySlider, 1, 1);
    formatLayout->addWidget(qualityLabel, 1, 0);

    // The progressive toggle.
    progressiveCheckBox = new QCheckBox(tr("Progressive"), formatBox);
    connect(progressiveCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(progressiveToggled(bool)));
    formatLayout->addWidget(progressiveCheckBox, 1, 2, Qt::AlignRight);
   
    compressionTypeLabel = new QLabel(tr("Compression type"),formatBox);
    compressionTypeComboBox = new QComboBox(formatBox);
    compressionTypeComboBox->addItem(tr("None"));
    compressionTypeComboBox->addItem(tr("PackBits"));
    compressionTypeComboBox->addItem(tr("JPEG"));
    compressionTypeComboBox->addItem(tr("Deflate"));
    //compressionTypeComboBox->addItem("LZW");
    formatLayout->addWidget(compressionTypeLabel, 2, 0);
    formatLayout->addWidget(compressionTypeComboBox, 2, 1, 1, 2);
    
    connect(compressionTypeComboBox, SIGNAL(activated(int)),
            this, SLOT(compressionTypeChanged(int)));
    compressionTypeLabel->setBuddy(compressionTypeComboBox);

    binaryCheckBox = new QCheckBox(tr("Binary"), central);
    connect(binaryCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(binaryToggled(bool)));
    formatLayout->addWidget(binaryCheckBox, 3, 0);

    // The stereo toggle.
    stereoCheckBox = new QCheckBox(tr("Stereo"), central);
    connect(stereoCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    formatLayout->addWidget(stereoCheckBox, 3, 1);

    // The force parallel merge toggle.
    forceMergeCheckBox = new QCheckBox(tr("Force parallel merge"), central);
    connect(forceMergeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(forceMergeToggled(bool)));
    formatLayout->addWidget(forceMergeCheckBox, 3, 2, 1, 2);

    // Create a group box for the image resolution.
    aspectAndResolutionBox = new QGroupBox(central);
    aspectAndResolutionBox->setTitle(tr("Aspect ratio and resolution"));
    topLayout->addWidget(aspectAndResolutionBox);

    QGridLayout *resolutionLayout = new QGridLayout(aspectAndResolutionBox);

    aspectRatioLabel = new QLabel(tr("Aspect ratio"),central);
    aspectRatioComboBox = new QComboBox(central);
    aspectRatioComboBox->addItem("screen ratio");
    aspectRatioComboBox->addItem("1:1 aspect ratio");
    aspectRatioComboBox->addItem("no constraint");
    connect(aspectRatioComboBox, SIGNAL(activated(int)),
           this, SLOT(aspectRatioChanged(int)));
    resolutionLayout->addWidget(aspectRatioLabel, 0, 0);
    resolutionLayout->addWidget(aspectRatioComboBox, 0, 1, 1, 2);

    // Create the width lineedit and label.
    widthLabel = new QLabel(tr("Width"), aspectAndResolutionBox);
    widthLineEdit = new QSixCharLineEdit(aspectAndResolutionBox);
    connect(widthLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processWidthText()));
    resolutionLayout->addWidget(widthLabel, 1, 0);
    resolutionLayout->addWidget(widthLineEdit, 1, 1);

    // Create the height lineedit and label.
    heightLabel = new QLabel(tr("Height"), aspectAndResolutionBox);
    heightLineEdit = new QSixCharLineEdit(aspectAndResolutionBox);
    connect(heightLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processHeightText()));
    resolutionLayout->addWidget(heightLabel, 1, 2);
    resolutionLayout->addWidget(heightLineEdit, 1, 3);

    // The screen capture toggle.
    screenCaptureCheckBox = new QCheckBox(tr("Screen capture"), central);
    connect(screenCaptureCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(screenCaptureToggled(bool)));
    resolutionLayout->addWidget(screenCaptureCheckBox, 2, 0, 1, 2);

    // Create a group box for the multi window save.
    multiWindowSaveBox = new QGroupBox(central);
    multiWindowSaveBox->setTitle(tr("Multi-window save"));
    multiWindowSaveBox->setCheckable(true);
    connect(multiWindowSaveBox, SIGNAL(toggled(bool)),
            this, SLOT(multiWindowSaveToggled(bool)));
    topLayout->addWidget(multiWindowSaveBox);

    QGridLayout *multiWindowSaveLayout = new QGridLayout(multiWindowSaveBox);

    multiWindowSaveTypeButtonGroup = new QButtonGroup(multiWindowSaveBox);
    tiledButton    = new QRadioButton(tr("Tiled"),             
                                      multiWindowSaveBox);
    advancedButton = new QRadioButton(tr("Advanced"),         
                                      multiWindowSaveBox);

    multiWindowSaveTypeButtonGroup->addButton(tiledButton,0);
    multiWindowSaveTypeButtonGroup->addButton(advancedButton,1);

    connect(tiledButton, SIGNAL(toggled(bool)),
            this, SLOT(multiWindowSaveTypeToggled(bool)));
    connect(advancedButton, SIGNAL(toggled(bool)),
            this, SLOT(multiWindowSaveTypeToggled(bool)));

    multiWindowSaveLayout->addWidget(tiledButton,    0, 0, 1, 2);
    multiWindowSaveLayout->addWidget(advancedButton, 0, 2, 1, 2);

    mwsWindowLabel = new QLabel(tr("Window"),central);
    mwsWindowComboBox = new QComboBox(central);
    mwsWindowComboBox->addItem("1");
    mwsWindowComboBox->addItem("2");
    mwsWindowComboBox->addItem("3");
    mwsWindowComboBox->addItem("4");
    mwsWindowComboBox->addItem("5");
    mwsWindowComboBox->addItem("6");
    mwsWindowComboBox->addItem("7");
    mwsWindowComboBox->addItem("8");
    mwsWindowComboBox->addItem("9");
    mwsWindowComboBox->addItem("10");
    mwsWindowComboBox->addItem("11");
    mwsWindowComboBox->addItem("12");
    mwsWindowComboBox->addItem("13");
    mwsWindowComboBox->addItem("14");
    mwsWindowComboBox->addItem("15");
    mwsWindowComboBox->addItem("16");
    connect(mwsWindowComboBox, SIGNAL(activated(int)),
            this, SLOT(mwsWindowComboBoxChanged(int)));
    multiWindowSaveLayout->addWidget(mwsWindowLabel, 1, 0, Qt::AlignRight);
    multiWindowSaveLayout->addWidget(mwsWindowComboBox, 1, 1, Qt::AlignLeft);

    omitWindowCheckBox = new QCheckBox(
                          tr("Omit window"), central);
    connect(omitWindowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(omitWindowCheckBoxToggled(bool)));
    multiWindowSaveLayout->addWidget(omitWindowCheckBox, 1, 3);

    // Create the width lineedit and label.
    mwsIndWidthLabel = new QLabel(tr("Width"), multiWindowSaveBox);
    mwsIndWidthLineEdit = new QSixCharLineEdit(multiWindowSaveBox);
    connect(mwsIndWidthLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processmwsIndWidthText()));
    multiWindowSaveLayout->addWidget(mwsIndWidthLabel, 2, 0, Qt::AlignRight);
    multiWindowSaveLayout->addWidget(mwsIndWidthLineEdit, 2, 1, Qt::AlignLeft);

    // Create the height lineedit and label.
    mwsIndHeightLabel = new QLabel(tr("Height"), multiWindowSaveBox);
    mwsIndHeightLineEdit = new QSixCharLineEdit(multiWindowSaveBox);
    connect(mwsIndHeightLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processmwsIndHeightText()));
    multiWindowSaveLayout->addWidget(mwsIndHeightLabel, 2, 2, Qt::AlignRight);
    multiWindowSaveLayout->addWidget(mwsIndHeightLineEdit, 2, 3, Qt::AlignLeft);

    // Create the positionX lineedit and label.
    mwsPosXLabel = new QLabel(tr("Position (X)"), multiWindowSaveBox);
    mwsPosXLineEdit = new QSixCharLineEdit(multiWindowSaveBox);
    connect(mwsPosXLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processmwsPosXText()));
    multiWindowSaveLayout->addWidget(mwsPosXLabel, 3, 0, Qt::AlignRight);
    multiWindowSaveLayout->addWidget(mwsPosXLineEdit, 3, 1, Qt::AlignLeft);

    // Create the height lineedit and label.
    mwsPosYLabel = new QLabel(tr("Position (Y)"), multiWindowSaveBox);
    mwsPosYLineEdit = new QSixCharLineEdit(multiWindowSaveBox);
    connect(mwsPosYLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processmwsPosYText()));
    multiWindowSaveLayout->addWidget(mwsPosYLabel, 3, 2, Qt::AlignRight);
    multiWindowSaveLayout->addWidget(mwsPosYLineEdit, 3, 3, Qt::AlignLeft);

    mwsLayerLabel = new QLabel(tr("Layer"),central);
    mwsLayerComboBox = new QComboBox(central);
    mwsLayerComboBox->addItem("1");
    mwsLayerComboBox->addItem("2");
    mwsLayerComboBox->addItem("3");
    mwsLayerComboBox->addItem("4");
    mwsLayerComboBox->addItem("5");
    mwsLayerComboBox->addItem("6");
    mwsLayerComboBox->addItem("7");
    mwsLayerComboBox->addItem("8");
    mwsLayerComboBox->addItem("9");
    mwsLayerComboBox->addItem("10");
    mwsLayerComboBox->addItem("11");
    mwsLayerComboBox->addItem("12");
    mwsLayerComboBox->addItem("13");
    mwsLayerComboBox->addItem("14");
    mwsLayerComboBox->addItem("15");
    mwsLayerComboBox->addItem("16");
    connect(mwsLayerComboBox, SIGNAL(activated(int)),
            this, SLOT(mwsLayerComboBoxChanged(int)));
    multiWindowSaveLayout->addWidget(mwsLayerLabel, 4, 0, Qt::AlignRight);
    multiWindowSaveLayout->addWidget(mwsLayerComboBox, 4, 1, Qt::AlignLeft);

    imageTransparencyLabel = new QLabel(tr("Transparency"),multiWindowSaveBox);
    imageTransparency = new QvisOpacitySlider(multiWindowSaveBox);
    imageTransparency->setMinimum(0);
    imageTransparency->setMaximum(100);
    connect(imageTransparency, SIGNAL(valueChanged(int)),
            this, SLOT(imageTransparencyChanged(int)));
    multiWindowSaveLayout->addWidget(imageTransparencyLabel, 4, 2, Qt::AlignRight);
    multiWindowSaveLayout->addWidget(imageTransparency, 4, 3);

    // The save button.
    QHBoxLayout *saveButtonLayout = new QHBoxLayout();
    topLayout->addLayout(saveButtonLayout);
    
    //saveButtonLayout->setSpacing(5);
    QPushButton *saveButton = new QPushButton(tr("Save"), central);
    connect(saveButton, SIGNAL(clicked()),
            this, SLOT(saveButtonClicked()));
    saveButtonLayout->addWidget(saveButton);

    QPushButton *saveAndDismissButton = new QPushButton(tr("Save and Dismiss"), central);
    connect(saveAndDismissButton, SIGNAL(clicked()),
            this, SLOT(saveAndDismissButtonClicked()));
    saveButtonLayout->addWidget(saveAndDismissButton);
    saveButtonLayout->addStretch(50);
}

// ****************************************************************************
// Method: QvisSaveWindow::UpdateWindow
//
// Purpose: 
//   This method updates the window's widgets to reflect changes made
//   in the SaveWindowAttributes object that the window watches.
//
// Arguments:
//   doAll : A flag indicating whether to update all of the widgets
//           regardless of the SaveWindowAttribute object's selected
//           states.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:09:40 PST 2001
//
// Modifications:
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
//   Hank Childs, Sun May 26 17:31:18 PDT 2002
//   Add support for file formats that can be saved out as binary or ASCII
//   files.
//
//   Jeremy Meredith, Thu Jul 25 11:50:10 PDT 2002
//   Made file format be a true enum.
//
//   Hank Childs, Wed Oct 15 09:03:08 PDT 2003
//   Only allow the stereo button to be enabled when we have an image format.
//
//   Kathleen Bonnell, Thu Nov 13 12:16:06 PST 2003 
//   Only allow the compression type combo box to be enabled when we 
//   have a tiff format.
//
//   Hank Childs, Thu Jun 17 11:39:35 PDT 2004
//   Disable "Screen capture" button when saving out polygonal formats.
//
//   Brad Whitlock, Fri Jul 16 12:30:42 PDT 2004
//   I added code to disable the height line edit when we're saving a tiled
//   image.
//
//   Brad Whitlock, Fri Jul 30 15:29:52 PST 2004
//   Added support for an output directory.
//
//   Jeremy Meredith, Thu Apr  5 17:25:40 EDT 2007
//   Added button to force a merge of parallel geometry.
//
//   Dave Bremer, Fri Sep 28 17:18:41 PDT 2007
//   bool field for "maintain aspect" was changed to an enum 
//   to constrain resolution.
//
//   Brad Whitlock, Mon Dec 17 10:38:20 PST 2007
//   Made it use ids.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Dave Pugmire, Thu Jul  8 08:30:11 EDT 2010
//   Added PLY writer.
//
//   Hank Childs, Thu Jul 22 09:55:03 PDT 2010
//   Added support for advanced multi-window saves.
//
//   Eric Brugger, Mon Aug 31 10:38:12 PDT 2015
//   I overhauled the window.
//
// ****************************************************************************

void
QvisSaveWindow::UpdateWindow(bool doAll)
{
    QString temp;

    // Handle changes related to multi window saving at the end.
    bool multiWindowSaveChanged = false;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    SaveSubWindowAttributes &atts = saveWindowAtts->GetSubWindowAtts().GetAttsForWindow(currentWindow+1);
    for(int i = 0; i < saveWindowAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!saveWindowAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case SaveWindowAttributes::ID_outputToCurrentDirectory:
            outputToCurrentDirectoryCheckBox->blockSignals(true);
            outputToCurrentDirectoryCheckBox->setChecked(
                saveWindowAtts->GetOutputToCurrentDirectory());
            outputToCurrentDirectoryCheckBox->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_outputDirectory:
            outputDirectoryLineEdit->setText(saveWindowAtts->GetOutputDirectory().c_str());
            break;
        case SaveWindowAttributes::ID_fileName:
            filenameLineEdit->setText(saveWindowAtts->GetFileName().c_str());
            break;
        case SaveWindowAttributes::ID_family:
            familyCheckBox->blockSignals(true);
            familyCheckBox->setChecked(saveWindowAtts->GetFamily());
            familyCheckBox->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_format:
            fileFormatComboBox->blockSignals(true);
            fileFormatComboBox->setCurrentIndex(saveWindowAtts->GetFormat());
            fileFormatComboBox->blockSignals(false);

            qualityLabel->setEnabled(saveWindowAtts->GetFormat() ==
                                     SaveWindowAttributes::JPEG);
            qualitySlider->setEnabled(saveWindowAtts->GetFormat() ==
                                      SaveWindowAttributes::JPEG);
            progressiveCheckBox->setEnabled(saveWindowAtts->GetFormat() ==
                                      SaveWindowAttributes::JPEG);
            compressionTypeLabel->setEnabled(saveWindowAtts->GetFormat() ==
                                      SaveWindowAttributes::TIFF);
            compressionTypeComboBox->setEnabled(saveWindowAtts->GetFormat() ==
                                      SaveWindowAttributes::TIFF);
            if (saveWindowAtts->GetFormat() 
                     == SaveWindowAttributes::VTK
                || saveWindowAtts->GetFormat() 
                     == SaveWindowAttributes::STL
                || saveWindowAtts->GetFormat() 
                     == SaveWindowAttributes::PLY)
            {
                binaryCheckBox->setEnabled(true);
            }
            else
            {
                binaryCheckBox->setEnabled(false);
            }
            if (saveWindowAtts->CurrentFormatIsImageFormat())
            {
                forceMergeCheckBox->setEnabled(false);
                stereoCheckBox->setEnabled(true);
                aspectAndResolutionBox->setEnabled(true);
                multiWindowSaveBox->setEnabled(true);
            }
            else
            {
                forceMergeCheckBox->setEnabled(true);
                stereoCheckBox->setEnabled(false);
                aspectAndResolutionBox->setEnabled(false);
                multiWindowSaveBox->setEnabled(false);
            }
            break;
        case SaveWindowAttributes::ID_width:
            temp.sprintf("%d", saveWindowAtts->GetWidth());
            widthLineEdit->setText(temp);
            break;
        case SaveWindowAttributes::ID_height:
            temp.sprintf("%d", saveWindowAtts->GetHeight());
            heightLineEdit->setText(temp);
            break;
        case SaveWindowAttributes::ID_screenCapture:
            screenCaptureCheckBox->blockSignals(true);
            screenCaptureCheckBox->setChecked(saveWindowAtts->GetScreenCapture());
            screenCaptureCheckBox->blockSignals(false);
            multiWindowSaveChanged = true;
            break;
        case SaveWindowAttributes::ID_saveTiled:
            multiWindowSaveChanged = true;
            break;
        case SaveWindowAttributes::ID_quality:
            qualitySlider->blockSignals(true);
            qualitySlider->setValue(saveWindowAtts->GetQuality());
            qualitySlider->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_progressive:
            progressiveCheckBox->blockSignals(true);
            progressiveCheckBox->setChecked(saveWindowAtts->GetProgressive());
            progressiveCheckBox->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_binary:
            binaryCheckBox->blockSignals(true);
            binaryCheckBox->setChecked(saveWindowAtts->GetBinary());
            binaryCheckBox->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_stereo:
            stereoCheckBox->blockSignals(true);
            stereoCheckBox->setChecked(saveWindowAtts->GetStereo());
            stereoCheckBox->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_compression:
            compressionTypeComboBox->blockSignals(true);
            compressionTypeComboBox->setCurrentIndex(saveWindowAtts->GetCompression());
            compressionTypeComboBox->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_forceMerge:
            forceMergeCheckBox->blockSignals(true);
            forceMergeCheckBox->setChecked(saveWindowAtts->GetForceMerge());
            forceMergeCheckBox->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_resConstraint:
            aspectRatioComboBox->blockSignals(true);
            if (saveWindowAtts->GetResConstraint() == SaveWindowAttributes::NoConstraint)
                aspectRatioComboBox->setCurrentIndex(2);
            else if (saveWindowAtts->GetResConstraint() == SaveWindowAttributes::EqualWidthHeight)
                aspectRatioComboBox->setCurrentIndex(1);
            else if (saveWindowAtts->GetResConstraint() == SaveWindowAttributes::ScreenProportions)
                aspectRatioComboBox->setCurrentIndex(0);
            aspectRatioComboBox->blockSignals(false);
            multiWindowSaveChanged = true;
            break;
        case SaveWindowAttributes::ID_advancedMultiWindowSave:
            multiWindowSaveChanged = true;
            break;
        case SaveWindowAttributes::ID_subWindowAtts:
            mwsWindowComboBox->blockSignals(true);
            mwsWindowComboBox->setCurrentIndex(currentWindow);
            mwsWindowComboBox->blockSignals(false);
            mwsIndWidthLineEdit->blockSignals(true);
            temp.sprintf("%d", atts.GetSize()[0]);
            mwsIndWidthLineEdit->setText(temp);
            mwsIndWidthLineEdit->blockSignals(false);
            mwsIndHeightLineEdit->blockSignals(true);
            temp.sprintf("%d", atts.GetSize()[1]);
            mwsIndHeightLineEdit->setText(temp);
            mwsIndHeightLineEdit->blockSignals(false);
            mwsPosXLineEdit->blockSignals(true);
            temp.sprintf("%d", atts.GetPosition()[0]);
            mwsPosXLineEdit->setText(temp);
            mwsPosXLineEdit->blockSignals(false);
            mwsPosYLineEdit->blockSignals(true);
            temp.sprintf("%d", atts.GetPosition()[1]);
            mwsPosYLineEdit->setText(temp);
            mwsPosYLineEdit->blockSignals(false);
            mwsLayerComboBox->blockSignals(true);
            mwsLayerComboBox->setCurrentIndex((atts.GetLayer() < 1 ? 0 : (atts.GetLayer() > 16 ? 15 : atts.GetLayer()-1)));
            mwsLayerComboBox->blockSignals(false);
            omitWindowCheckBox->blockSignals(true);
            omitWindowCheckBox->setChecked(atts.GetOmitWindow());
            omitWindowCheckBox->blockSignals(false);
            imageTransparency->blockSignals(true);
            imageTransparency->setValue( (atts.GetTransparency() < 0 ? 0 : (atts.GetTransparency() > 1 ? 100 : 100*atts.GetTransparency())));
            imageTransparency->blockSignals(false);
            break;
        }
    } // end for

    // Handle changes related to multi window saving now.
    if (multiWindowSaveChanged)
    {
        if (!ignoreNextMultiWindowSaveChange)
        {
            // Determine the window state from the attributes.
            multiWindowSaveMode = Tiled;
            multiWindowSaveEnabled = false;
            if (saveWindowAtts->GetSaveTiled())
            {
                multiWindowSaveEnabled = true;
            }
            else if (saveWindowAtts->GetAdvancedMultiWindowSave())
            {
                multiWindowSaveMode = Advanced;
                multiWindowSaveEnabled = true;
            }
        }
        ignoreNextMultiWindowSaveChange = false;

        // Set the window.
        if (multiWindowSaveEnabled)
        {
            aspectRatioLabel->setEnabled(false);
            aspectRatioComboBox->setEnabled(false);
            if (multiWindowSaveMode == Tiled)
            {
                widthLabel->setEnabled(!saveWindowAtts->GetScreenCapture());
                widthLineEdit->setEnabled(!saveWindowAtts->GetScreenCapture());
                heightLabel->setEnabled(false);
                heightLineEdit->setEnabled(false);
                screenCaptureCheckBox->setEnabled(true);
            }
            else
            {
                widthLabel->setEnabled(true);
                widthLineEdit->setEnabled(true);
                heightLabel->setEnabled(multiWindowSaveMode == Advanced);
                heightLineEdit->setEnabled(multiWindowSaveMode == Advanced);
                screenCaptureCheckBox->setEnabled(false);
            }
            mwsWindowLabel->setEnabled(multiWindowSaveMode == Advanced);
            mwsWindowComboBox->setEnabled(multiWindowSaveMode == Advanced);
            omitWindowCheckBox->setEnabled(multiWindowSaveMode == Advanced);
            mwsIndWidthLabel->setEnabled(multiWindowSaveMode == Advanced);
            mwsIndWidthLineEdit->setEnabled(multiWindowSaveMode == Advanced);
            mwsIndHeightLabel->setEnabled(multiWindowSaveMode == Advanced);
            mwsIndHeightLineEdit->setEnabled(multiWindowSaveMode == Advanced);
            mwsPosXLabel->setEnabled(multiWindowSaveMode == Advanced);
            mwsPosXLineEdit->setEnabled(multiWindowSaveMode == Advanced);
            mwsPosYLabel->setEnabled(multiWindowSaveMode == Advanced);
            mwsPosYLineEdit->setEnabled(multiWindowSaveMode == Advanced);
            mwsLayerLabel->setEnabled(multiWindowSaveMode == Advanced);
            mwsLayerComboBox->setEnabled(multiWindowSaveMode == Advanced);
            imageTransparencyLabel->setEnabled(multiWindowSaveMode == Advanced);
            imageTransparency->setEnabled(multiWindowSaveMode == Advanced);
        }
        else
        {
            if (saveWindowAtts->GetScreenCapture())
            {
                aspectRatioLabel->setEnabled(false);
                aspectRatioComboBox->setEnabled(false);
                widthLabel->setEnabled(false);
                widthLineEdit->setEnabled(false);
                heightLabel->setEnabled(false);
                heightLineEdit->setEnabled(false);
            }
            else
            {
                aspectRatioLabel->setEnabled(true);
                aspectRatioComboBox->setEnabled(true);
                widthLabel->setEnabled(true);
                widthLineEdit->setEnabled(true);
                if (saveWindowAtts->GetResConstraint() == SaveWindowAttributes::NoConstraint)
                {
                    heightLabel->setEnabled(true);
                    heightLineEdit->setEnabled(true);
                }
                else
                {
                    heightLabel->setEnabled(false);
                    heightLineEdit->setEnabled(false);
                }
            }
            screenCaptureCheckBox->setEnabled(true);
        }

        tiledButton->blockSignals(true);
        advancedButton->blockSignals(true);
        multiWindowSaveBox->blockSignals(true);
        tiledButton->setChecked(multiWindowSaveMode == Tiled);
        advancedButton->setChecked(multiWindowSaveMode == Advanced);
        multiWindowSaveBox->setChecked(multiWindowSaveEnabled);
        tiledButton->blockSignals(false);
        advancedButton->blockSignals(false);
        multiWindowSaveBox->blockSignals(false);
    }

    // Make sure that the output directory text field is not enabled if we are
    // outputting to the current directory.
    bool outputDirEnabled = !saveWindowAtts->GetOutputToCurrentDirectory();
    if(outputDirEnabled != outputDirectoryLabel->isEnabled())
    {
        outputDirectoryLabel->setEnabled(outputDirEnabled);
        outputDirectoryLineEdit->setEnabled(outputDirEnabled);
        outputDirectorySelectButton->setEnabled(outputDirEnabled);
    }
}

// ****************************************************************************
// Method: QvisSaveWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:16:08 PST 2001
//
// Modifications:
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
//   Brad Whitlock, Fri Jul 16 15:46:40 PST 2004
//   Removed some code and added new code to enforce the 1:1 aspect if needed.
//
//   Brad Whitlock, Fri Jul 30 15:50:30 PST 2004
//   I changed the host text field to output directory.
//
//   Dave Bremer, Fri Sep 28 17:18:41 PDT 2007
//   Enum in SaveWindowAttributes changed.
//
//   Dave Bremer, Thu Oct 11 18:56:56 PDT 2007
//   Added a check for windows larger than the max Mesa can handle, and
//   revert the value if it's too high.
//
//   Brad Whitlock, Mon Dec 17 10:38:47 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Jeremy Meredith, Tue Jun 24 12:27:54 EDT 2008
//   Use the actual OSMesa size limit for the window limit.
//
//   Hank Childs, Thu Jul 22 09:55:03 PDT 2010
//   Added support for advanced multi-window saves.
//
//   Eric Brugger, Mon Aug 31 10:38:12 PDT 2015
//   I overhauled the window.
//
// ****************************************************************************

void
QvisSaveWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the host name
    if(which_widget == SaveWindowAttributes::ID_outputDirectory || doAll)
    {
        temp = outputDirectoryLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            saveWindowAtts->SetOutputDirectory(temp.toStdString());
        }
        else
        {
            msg = tr("The output directory was invalid. "
                     "Resetting to the last good value \"%1\".").
                  arg(saveWindowAtts->GetOutputDirectory().c_str());
            Message(msg);
            saveWindowAtts->SetOutputDirectory(saveWindowAtts->GetOutputDirectory());
        }
    }

    // Do the file name
    if(which_widget == SaveWindowAttributes::ID_fileName || doAll)
    {
        temp = filenameLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            saveWindowAtts->SetFileName(temp.toStdString());
        }
        else
        {
            msg = tr("The filename was invalid. "
                     "Resetting to the last good value \"%1\".").
                  arg(saveWindowAtts->GetFileName().c_str());
            Message(msg);
            saveWindowAtts->SetFileName(saveWindowAtts->GetFileName());
        }
    }

    // Do the image width
    if(which_widget == SaveWindowAttributes::ID_width || doAll)
    {
        temp = widthLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int w;
            okay = (sscanf(temp.toStdString().c_str(), "%d", &w) == 1);
            if(okay)
            {
                okay = (w <= VISIT_RENDERING_SIZE_LIMIT);
                
                if(okay)
                {
                    saveWindowAtts->SetWidth(w);
                }
            }
        }

        if(!okay)
        {
            msg = tr("The width was invalid. "
                     "Resetting to the last good value %1.").
                  arg(saveWindowAtts->GetWidth());
            Message(msg);
            saveWindowAtts->SetWidth(saveWindowAtts->GetWidth());
        }
    }

    // Do the image height
    if(which_widget == SaveWindowAttributes::ID_height || doAll)
    {
        temp = heightLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int h;
            okay = (sscanf(temp.toStdString().c_str(), "%d", &h) == 1);
            if(okay)
            {
                okay = (h <= VISIT_RENDERING_SIZE_LIMIT);
                
                if(okay)
                {
                    saveWindowAtts->SetHeight(h);
                }
            }
        }

        if(!okay)
        {
            msg = tr("The height was invalid. "
                     "Resetting to the last good value %1.").
                  arg(saveWindowAtts->GetHeight());
            Message(msg);
            saveWindowAtts->SetHeight(saveWindowAtts->GetHeight());
        }
    }

    if (which_widget == SaveWindowAttributes::ID_subWindowAtts || doAll)
    {
        SaveSubWindowAttributes &atts = saveWindowAtts->GetSubWindowAtts().GetAttsForWindow(currentWindow+1);

        // size (for a given window).
        temp = mwsIndWidthLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int w;
            okay = (sscanf(temp.toStdString().c_str(), "%d", &w) == 1);
            if(okay)
            {
                okay = (w <= VISIT_RENDERING_SIZE_LIMIT);
                if (okay)
                {
                    int s[2] = { w, atts.GetSize()[1] };
                    atts.SetSize(s);
                }
            }
        }
        if(!okay)
        {
            msg = tr("The width was invalid. "
                     "Resetting to the last good value %1.").
                  arg(atts.GetSize()[0]);
            Message(msg);
        }
        temp = mwsIndHeightLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int h;
            okay = (sscanf(temp.toStdString().c_str(), "%d", &h) == 1);
            if(okay)
            {
                okay = (h <= VISIT_RENDERING_SIZE_LIMIT);
            
                if(okay)
                {
                    int s[2] = { atts.GetSize()[0], h };
                    atts.SetSize(s);
                }
            }
        }
        if(!okay)
        {
            msg = tr("The height was invalid. "
                     "Resetting to the last good value %1.").
                  arg(atts.GetSize()[1]);
            Message(msg);
        }

        // position in X & Y (for a given window).
        temp = mwsPosXLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int x;
            okay = (sscanf(temp.toStdString().c_str(), "%d", &x) == 1);
            if(okay)
            {
                int pos[2] = { x, atts.GetPosition()[1] };
                atts.SetPosition(pos);
            }
        }
        if(!okay)
        {
            msg = tr("The x position was invalid. "
                     "Resetting to the last good value %1.").
                  arg(atts.GetPosition()[0]);
            Message(msg);
        }
        temp = mwsPosYLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int y;
            okay = (sscanf(temp.toStdString().c_str(), "%d", &y) == 1);
            if(okay)
            {
                int pos[2] = { atts.GetPosition()[0], y };
                atts.SetPosition(pos);
            }
        }
        if(!okay)
        {
            msg = tr("The y position was invalid. "
                     "Resetting to the last good value %1.").
                  arg(atts.GetPosition()[1]);
            Message(msg);
        }
    }
}

// ****************************************************************************
// Method: QvisSaveWindow::Apply
//
// Purpose: 
//   This method applies the save image attributes and optionally tells
//   the viewer to apply them.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            save window attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:25:04 PST 2001
//
// Modifications:
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
// ****************************************************************************

void
QvisSaveWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current save window attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        saveWindowAtts->Notify();
    }
    else
        saveWindowAtts->Notify();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisSaveWindow::apply
//
// Purpose: 
//   This is a Qt slot function to apply the save image attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:25:59 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisSaveWindow::processFilenameText
//
// Purpose: 
//   This is a Qt slot function that sets the normal vector.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:26:26 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::processFilenameText()
{
    GetCurrentValues(SaveWindowAttributes::ID_fileName);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::familyToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not the file is to be named as a family of files.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:27:07 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::familyToggled(bool val)
{
    saveWindowAtts->SetFamily(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::outputToCurrentDirectoryToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the
//  "Output to current directory" toggle is clicked.
//
// Arguments:
//   val : Whether we should save to the current directory.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 30 15:54:51 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::outputToCurrentDirectoryToggled(bool val)
{
    saveWindowAtts->SetOutputToCurrentDirectory(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::processOutputDirectoryText
//
// Purpose: 
//   This is a Qt slot function that sets the output directory where we'll
//   save the image.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 16:57:34 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 30 15:53:08 PST 2004
//   Changed from processing host to output directory.
//
// ****************************************************************************

void
QvisSaveWindow::processOutputDirectoryText()
{
    GetCurrentValues(SaveWindowAttributes::ID_outputDirectory);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::selectOutputDirectory
//
// Purpose: 
//   This is Qt slot function that selects a new output file directory.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 30 16:56:24 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Nov 4 14:04:11 PST 2004
//   Added code to get current values to ensure that the new directory
//   gets stored in the window attributes.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisSaveWindow::selectOutputDirectory()
{
    //
    // Try and get a directory using a file dialog.
    //
    QString initialDir(saveWindowAtts->GetOutputDirectory().c_str());
    QString dirName = QFileDialog::getExistingDirectory(this,
       tr("Select output directory"),initialDir,QFileDialog::ShowDirsOnly);

    //
    // If a directory was chosen, use it as the output directory.
    //
    if(!dirName.isEmpty())
    {
        outputDirectoryLineEdit->setText(dirName);
        GetCurrentValues(SaveWindowAttributes::ID_outputDirectory);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisSaveWindow::fileFormatChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new file format
//   is selected.
//
// Arguments:
//   index : The index of the file format that was selected.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:28:13 PST 2001
//
// Modifications:
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
//   Jeremy Meredith, Thu Jul 25 11:49:41 PDT 2002
//   Made file format be a true enum.
//
// ****************************************************************************

void
QvisSaveWindow::fileFormatChanged(int index)
{
    saveWindowAtts->SetFormat(SaveWindowAttributes::FileFormat(index));
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::qualityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the quality slider changes.
//
// Arguments:
//   val : The new quality value.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 15:31:56 PST 2002
//
// Modifications:
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
// ****************************************************************************

void
QvisSaveWindow::qualityChanged(int val)
{
    saveWindowAtts->SetQuality(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::progressiveToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the progressive toggle
//   is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 15:30:56 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::progressiveToggled(bool val)
{
    saveWindowAtts->SetProgressive(val);
    Apply();
}

void
QvisSaveWindow::compressionTypeChanged(int index)
{
    saveWindowAtts->SetCompression(SaveWindowAttributes::CompressionType(index));
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::binaryToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the binary toggle
//   is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Hank Childs
// Creation:   Sun May 26 17:31:18 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::binaryToggled(bool val)
{
    saveWindowAtts->SetBinary(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::stereoToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not to save the image in stereo.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Hank Childs
// Creation:   October 15, 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::stereoToggled(bool val)
{
    saveWindowAtts->SetStereo(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::forceMergeToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not it should attempt to merge parallel domains before saving.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Jeremy Meredith
// Creation:   April  5, 2007
//
// Modifications:
//   Eric Brugger, Mon Aug 31 10:38:12 PDT 2015
//   I corrected a bug where toggling Force parallel merge set the
//   SaveImageTiled attribute instead of the SetForceMerge attribute.
//
// ****************************************************************************

void
QvisSaveWindow::forceMergeToggled(bool val)
{
    saveWindowAtts->SetForceMerge(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::aspectRatioChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new aspect ratio
//   is selected.
//
// Arguments:
//   index : The index of the aspect ratio that was selected.
//
// Programmer: Eric Brugger
// Creation:   Mon Aug 31 10:38:12 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisSaveWindow::aspectRatioChanged(int index)
{
    QString temp;

    switch(index)
    {
      case 0:
        saveWindowAtts->SetResConstraint(SaveWindowAttributes::ScreenProportions);
        saveWindowAtts->SetScreenCapture(false);
        widthLabel->setEnabled(true);
        widthLineEdit->setEnabled(true);
        heightLabel->setEnabled(false);
        heightLineEdit->setEnabled(false);
        break;
      case 1:
        saveWindowAtts->SetResConstraint(SaveWindowAttributes::EqualWidthHeight);
        saveWindowAtts->SetScreenCapture(false);
        widthLabel->setEnabled(true);
        widthLineEdit->setEnabled(true);
        heightLabel->setEnabled(false);
        heightLineEdit->setEnabled(false);
        break;
      case 2:
        saveWindowAtts->SetResConstraint(SaveWindowAttributes::NoConstraint);
        saveWindowAtts->SetScreenCapture(false);
        widthLabel->setEnabled(true);
        widthLineEdit->setEnabled(true);
        heightLabel->setEnabled(true);
        heightLineEdit->setEnabled(true);
        break;
    }

    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::processWidthText
//
// Purpose: 
//   This is a Qt slot function that is called when the image width changes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 12 13:00:24 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 16 14:37:13 PST 2004
//   Moved some code out of GetCurrentValues.
//
//   Dave Bremer, Fri Sep 28 17:18:41 PDT 2007
//   Enum in SaveWindowAttributes changed.
//
// ****************************************************************************

void
QvisSaveWindow::processWidthText()
{
    GetCurrentValues(SaveWindowAttributes::ID_width);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::processHeightText
//
// Purpose: 
//   This is a Qt slot function that is called when the image height changes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 12 13:00:24 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Jul 16 14:37:13 PST 2004
//   Moved some code out of GetCurrentValues.
//   
//   Dave Bremer, Fri Sep 28 17:18:41 PDT 2007
//   Enum in SaveWindowAttributes changed.
//
// ****************************************************************************

void
QvisSaveWindow::processHeightText()
{
    GetCurrentValues(SaveWindowAttributes::ID_height);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::screenCaptureToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not to screen capture the image.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Eric Brugger
// Creation:   Mon Aug 31 10:38:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::screenCaptureToggled(bool val)
{
    saveWindowAtts->SetScreenCapture(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::multiWindowSaveToggled
//
// Purpose: 
//   This is a Qt slot function that turns on and off multi window save.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Eric Brugger
// Creation:   Mon Aug 31 10:38:12 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisSaveWindow::multiWindowSaveToggled(bool val)
{
    multiWindowSaveEnabled = val;
    if (val)
    {
        if (multiWindowSaveMode == Tiled)
        {
            saveWindowAtts->SetSaveTiled(true);
            saveWindowAtts->SetAdvancedMultiWindowSave(false);
        }
        else
        {
            saveWindowAtts->SetSaveTiled(false);
            saveWindowAtts->SetAdvancedMultiWindowSave(true);
        }
    }
    else
    {
        saveWindowAtts->SetSaveTiled(false);
        saveWindowAtts->SetAdvancedMultiWindowSave(false);
    }
    ignoreNextMultiWindowSaveChange = true;

    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::multiWindowSaveTypeToggled
//
// Purpose: 
//   This is a Qt slot function that sets the type of multi window save
//   to do.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Eric Brugger
// Creation:   Mon Aug 31 10:38:12 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisSaveWindow::multiWindowSaveTypeToggled(bool val)
{
    // Ignore the "toggled off" signal
    if (!val)
        return;

    if (tiledButton->isChecked())
    {
        saveWindowAtts->SetSaveTiled(true);
        saveWindowAtts->SetAdvancedMultiWindowSave(false);
    }
    else if (advancedButton->isChecked())
    {
        saveWindowAtts->SetSaveTiled(false);
        saveWindowAtts->SetAdvancedMultiWindowSave(true);
    }

    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::mwsWindowComboBoxChanged
//
// Purpose:
//   This is a Qt slot function that is called when the combo box that selects
//   the current window changes.
//
// Programmer: Hank Childs
// Creation:   July 22, 2010
//
// Modifications:
//   Eric Brugger, Mon Aug 31 10:38:12 PDT 2015
//   I overhauled the window.
//
// ****************************************************************************

void QvisSaveWindow::mwsWindowComboBoxChanged(int v)
{
    // process any changes in the line edits
    GetCurrentValues(-1);

    currentWindow = v;
    UpdateWindow(true);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::omitWindowCheckBoxToggled
//
// Purpose:
//   This is a Qt slot function that is called when omit status for
//   an individual window in an advanced multi-window saves has changed.
//
// Programmer: Hank Childs
// Creation:   July 22, 2010
//
// Modifications:
//   Eric Brugger, Mon Aug 31 10:38:12 PDT 2015
//   I overhauled the window.
//
// ****************************************************************************

void QvisSaveWindow::omitWindowCheckBoxToggled(bool val)
{
    SaveSubWindowAttributes &atts = saveWindowAtts->GetSubWindowAtts().GetAttsForWindow(currentWindow+1);
    atts.SetOmitWindow(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::processmwsIndWidthText
//
// Purpose:
//   This is a Qt slot function that is called when the width field for
//   an individual window in an advanced multi-window saves should be processed.
//
// Programmer: Hank Childs
// Creation:   July 22, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSaveWindow::processmwsIndWidthText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::processmwsIndHeightText
//
// Purpose:
//   This is a Qt slot function that is called when the height field for
//   an individual window in an advanced multi-window saves should be processed.
//
// Programmer: Hank Childs
// Creation:   July 22, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSaveWindow::processmwsIndHeightText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::processmwsPosXText
//
// Purpose:
//   This is a Qt slot function that is called when the x-position for
//   an individual window in an advanced multi-window saves should be processed.
//
// Programmer: Hank Childs
// Creation:   July 22, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSaveWindow::processmwsPosXText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::processmwsPosYText
//
// Purpose:
//   This is a Qt slot function that is called when the y-position for
//   an individual window in an advanced multi-window saves should be processed.
//
// Programmer: Hank Childs
// Creation:   July 22, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisSaveWindow::processmwsPosYText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::mwsLayerComboBoxChanged
//
// Purpose:
//   This is a Qt slot function that is called when the layer for
//   an individual window in an advanced multi-window saves has changed.
//
// Programmer: Hank Childs
// Creation:   July 22, 2010
//
// Modifications:
//   Eric Brugger, Mon Aug 31 10:38:12 PDT 2015
//   I overhauled the window.
//
// ****************************************************************************

void QvisSaveWindow::mwsLayerComboBoxChanged(int val)
{
    SaveSubWindowAttributes &atts = saveWindowAtts->GetSubWindowAtts().GetAttsForWindow(currentWindow+1);
    atts.SetLayer(val+1);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::imageTransparencyChanged
//
// Purpose:
//   This is a Qt slot function that is called when the transparency for
//   an individual window in an advanced multi-window saves has changed.
//
// Programmer: Hank Childs
// Creation:   July 22, 2010
//
// Modifications:
//   Eric Brugger, Mon Aug 31 10:38:12 PDT 2015
//   I overhauled the window.
//
// ****************************************************************************

void QvisSaveWindow::imageTransparencyChanged(int val)
{
    SaveSubWindowAttributes &atts = saveWindowAtts->GetSubWindowAtts().GetAttsForWindow(currentWindow+1);
    atts.SetTransparency(val/100.0);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::saveWindow
//
// Purpose: 
//   This is Qt slot function that is called when the Save button is clicked.
//
// Note:       Saves the active vis window and may hide the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 30 15:51:57 PST 2004
//
// Modifications:
//   Kathleen Biagas, Wed Jan  7 12:39:12 PST 2015
//   Added dismissOnSave flag.
//   
// ****************************************************************************

void
QvisSaveWindow::saveWindow()
{
    Apply();
    if(isVisible() && !posted() && dismissOnSave)
        hide();
    GetViewerMethods()->SaveWindow();
}

// ****************************************************************************
// Method: QvisSaveWindow::saveButtonClicked
//
// Purpose: 
//   This is Qt slot function that is called when the save button is clicked. 
//
// Programmer: Kathleen Bonnell 
// Creation:   December 15, 2004 
//
// Modifications:
//   Kathleen Biagas, Wed Jan  7 12:39:12 PST 2015
//   Added dismissOnSave flag.
//
// ****************************************************************************

void
QvisSaveWindow::saveButtonClicked()
{
    apply();
    dismissOnSave = false;
    saveWindow();
}

// ****************************************************************************
// Method: QvisSaveWindow::saveAndDismissButtonClicked
//
// Purpose: 
//   This is Qt slot function that is called when the saveAndDismis button
//   is clicked. 
//
// Programmer: Kathleen Biagas 
// Creation:   January 7, 2015
//
// Modifications:
//
// ****************************************************************************

void
QvisSaveWindow::saveAndDismissButtonClicked()
{
    apply();
    dismissOnSave = true;
    saveWindow();
}
