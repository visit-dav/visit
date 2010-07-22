/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//   Cconstructor for the QvisSaveWindow class.
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
// ****************************************************************************

QvisSaveWindow::QvisSaveWindow(
    SaveWindowAttributes *subj, const QString &caption, const QString &shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    saveWindowAtts = subj;
    currentWindow = 0;
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
//   
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
// ****************************************************************************

void
QvisSaveWindow::CreateWindowContents()
{
    QTabWidget *propertyTabs = new QTabWidget(central);
    topLayout->addWidget(propertyTabs);

    // ------------------------------------------------------------------------
    // First tab
    // ------------------------------------------------------------------------
    QWidget *firstTab = new QWidget(central);
    propertyTabs->addTab(firstTab, tr("Standard Save"));

    QGridLayout *mainLayout = new QGridLayout(firstTab);

    // Create a group box for the image resolution.
    resolutionBox = new QGroupBox(central);
    mainLayout->addWidget(resolutionBox);

    QGridLayout *resolutionLayout = new QGridLayout(resolutionBox);

    resConstraintButtonGroup = new QButtonGroup(resolutionBox);
    screenResButton   = new QRadioButton(tr("Screen ratio"),             
                                         resolutionBox);
    oneToOneResButton = new QRadioButton(tr("1:1 aspect ratio"),         
                                         resolutionBox);
    noResButton       = new QRadioButton(tr("No resolution constraint"), 
                                         resolutionBox);
    screenResButton->setChecked(true);

    resConstraintButtonGroup->addButton(screenResButton,0);
    resConstraintButtonGroup->addButton(oneToOneResButton,1);
    resConstraintButtonGroup->addButton(noResButton,2);

    connect(screenResButton, SIGNAL(toggled(bool)),
            this, SLOT(resConstraintToggled(bool)));
    connect(oneToOneResButton, SIGNAL(toggled(bool)),
            this, SLOT(resConstraintToggled(bool)));
    connect(noResButton, SIGNAL(toggled(bool)),
            this, SLOT(resConstraintToggled(bool)));

    resolutionLayout->addWidget(screenResButton,   1, 0, 1, 4);
    resolutionLayout->addWidget(oneToOneResButton, 2, 0, 1, 4);
    resolutionLayout->addWidget(noResButton,       3, 0, 1, 4);

    // Create the width lineedit and label.
    widthLineEdit = new QLineEdit(resolutionBox);
    connect(widthLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processWidthText()));
    QLabel *widthLabel = new QLabel(tr("Width"), resolutionBox);
    resolutionLayout->addWidget(widthLabel, 4, 0);
    resolutionLayout->addWidget(widthLineEdit, 4, 1);

    // Create the height lineedit and label.
    heightLineEdit = new QLineEdit(resolutionBox);
    connect(heightLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processHeightText()));
    QLabel *heightLabel = new QLabel(tr("Height"), resolutionBox);
    resolutionLayout->addWidget(heightLabel, 4, 2);
    resolutionLayout->addWidget(heightLineEdit, 4, 3);

    // The stereo toggle.
    stereoCheckBox = new QCheckBox(tr("Stereo"), central);
    connect(stereoCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    resolutionLayout->addWidget(stereoCheckBox, 5, 0);

    // The screen capture toggle.
    screenCaptureCheckBox = new QCheckBox(tr("Screen capture"), central);
    connect(screenCaptureCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(screenCaptureToggled(bool)));
    resolutionLayout->addWidget(screenCaptureCheckBox, 5, 1);

    // The tiled toggle.
    saveTiledCheckBox = new QCheckBox(tr("Save tiled"), central);
    connect(saveTiledCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(saveTiledToggled(bool)));
    resolutionLayout->addWidget(saveTiledCheckBox, 5, 2);

    // ------------------------------------------------------------------------
    // Second tab
    // ------------------------------------------------------------------------
    QWidget *secondTab = new QWidget(central);
    propertyTabs->addTab(secondTab, tr("Advanced Multi-Window Save"));

    QGridLayout *mainLayout2 = new QGridLayout(secondTab);

    // Create a group box for the resolution
    QGroupBox *mwsTopBox = new QGroupBox(central);
    mainLayout2->addWidget(mwsTopBox);
    QGridLayout *mwsTopLayout = new QGridLayout(mwsTopBox);

    // Create the width lineedit and label.
    // Create a group box for the file information.
    // The stereo toggle.
    advancedMultiWinSaveCheckBox = new QCheckBox(
                          tr("Do Advanced Multi-Window Save"), central);
    connect(advancedMultiWinSaveCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(advancedMultiWinSaveToggled(bool)));
    mwsTopLayout->addWidget(advancedMultiWinSaveCheckBox, 0, 0);

    // The stereo toggle.
    stereoCheckBox2 = new QCheckBox(tr("Stereo"), central);
    connect(stereoCheckBox2, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    mwsTopLayout->addWidget(stereoCheckBox2, 0, 1);

    // Create a group box for the resolution
    QGroupBox *mwsSizeBox = new QGroupBox(central);
    mwsSizeBox->setTitle(tr("Total Window Size"));
    mainLayout2->addWidget(mwsSizeBox);
    QGridLayout *mwsResolutionLayout = new QGridLayout(mwsSizeBox);

    // Create the width lineedit and label.
    mwsWidthLineEdit = new QLineEdit(mwsSizeBox);
    connect(mwsWidthLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processmwsWidthText()));
    QLabel *mwsWidthLabel = new QLabel(tr("Width"), resolutionBox);
    mwsResolutionLayout->addWidget(mwsWidthLabel, 0, 0);
    mwsResolutionLayout->addWidget(mwsWidthLineEdit, 0, 1);

    // Create the height lineedit and label.
    mwsHeightLineEdit = new QLineEdit(mwsSizeBox);
    connect(mwsHeightLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processHeightText()));
    QLabel *mwsHeightLabel = new QLabel(tr("Height"), resolutionBox);
    mwsResolutionLayout->addWidget(mwsHeightLabel, 0, 2);
    mwsResolutionLayout->addWidget(mwsHeightLineEdit, 0, 3);

    // Create a group box for the individual window controls
    QGroupBox *mwsControlsBox = new QGroupBox(central);
    mwsControlsBox->setTitle(tr("Individual Window Controls"));
    mainLayout2->addWidget(mwsControlsBox);
    QGridLayout *mwsControlsLayout = new QGridLayout(mwsControlsBox);

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
    QLabel *mwsWindowLabel = new QLabel(tr("Window"),central);
    mwsControlsLayout->addWidget(mwsWindowLabel, 0, 0, Qt::AlignRight);
    mwsControlsLayout->addWidget(mwsWindowComboBox, 0, 1, Qt::AlignLeft);

    omitWindowCheckBox = new QCheckBox(
                          tr("Omit Window"), central);
    connect(omitWindowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(omitWindowCheckBoxToggled(bool)));
    mwsControlsLayout->addWidget(omitWindowCheckBox, 0, 3);

    // Create the width lineedit and label.
    mwsIndWidthLineEdit = new QLineEdit(mwsControlsBox);
    connect(mwsIndWidthLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processmwsIndWidthText()));
    QLabel *mwsIndWidthLabel = new QLabel(tr("Width"), mwsControlsBox);
    mwsControlsLayout->addWidget(mwsIndWidthLabel, 1, 0, Qt::AlignRight);
    mwsControlsLayout->addWidget(mwsIndWidthLineEdit, 1, 1, Qt::AlignLeft);

    // Create the height lineedit and label.
    mwsIndHeightLineEdit = new QLineEdit(mwsControlsBox);
    connect(mwsIndHeightLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processmwsIndHeightText()));
    QLabel *mwsIndHeightLabel = new QLabel(tr("Height"), mwsControlsBox);
    mwsControlsLayout->addWidget(mwsIndHeightLabel, 1, 2, Qt::AlignRight);
    mwsControlsLayout->addWidget(mwsIndHeightLineEdit, 1, 3, Qt::AlignLeft);

    // Create the positionX lineedit and label.
    mwsPosXLineEdit = new QLineEdit(mwsControlsBox);
    connect(mwsPosXLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processmwsPosXText()));
    QLabel *mwsPosXLabel = new QLabel(tr("Position (X)"), mwsControlsBox);
    mwsControlsLayout->addWidget(mwsPosXLabel, 2, 0, Qt::AlignRight);
    mwsControlsLayout->addWidget(mwsPosXLineEdit, 2, 1, Qt::AlignLeft);

    // Create the height lineedit and label.
    mwsPosYLineEdit = new QLineEdit(mwsControlsBox);
    connect(mwsPosYLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processmwsPosYText()));
    QLabel *mwsPosYLabel = new QLabel(tr("Position (Y)"), mwsControlsBox);
    mwsControlsLayout->addWidget(mwsPosYLabel, 2, 2, Qt::AlignRight);
    mwsControlsLayout->addWidget(mwsPosYLineEdit, 2, 3, Qt::AlignLeft);

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
    QLabel *mwsLayerLabel = new QLabel(tr("Layer"),central);
    mwsControlsLayout->addWidget(mwsLayerLabel, 3, 0, Qt::AlignRight);
    mwsControlsLayout->addWidget(mwsLayerComboBox, 3, 1, Qt::AlignLeft);

    imageTransparency = new QvisOpacitySlider(mwsControlsBox);
    imageTransparency->setMinimum(0);
    imageTransparency->setMaximum(100);
    connect(imageTransparency, SIGNAL(valueChanged(int)),
            this, SLOT(imageTransparencyChanged(int)));
    QLabel *imageTransparencyLabel = new QLabel(tr("Transparency"),mwsControlsBox);
    mwsControlsLayout->addWidget(imageTransparencyLabel, 3, 2, Qt::AlignRight);
    mwsControlsLayout->addWidget(imageTransparency, 3, 3);

    // Create a group box for the file information.
    QGroupBox *nameBox = new QGroupBox(central);
    nameBox->setTitle(tr("File name"));
    topLayout->addWidget(nameBox);

    QGridLayout *nameLayout = new QGridLayout(nameBox);

    filenameLineEdit = new QLineEdit(nameBox);
    connect(filenameLineEdit, SIGNAL(returnPressed()), this, SLOT(processFilenameText()));
    QLabel *filenameLabel = new QLabel(tr("Filename"), nameBox);
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
    outputDirectoryLineEdit     = new QLineEdit(nameBox);
    outputDirectorySelectButton = new QPushButton("...", nameBox);
    
    outputDirectoryLayout->addWidget(outputDirectoryLineEdit);
    outputDirectoryLayout->addWidget(outputDirectorySelectButton);
    
    connect(outputDirectoryLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOutputDirectoryText()));


#ifndef Q_WS_MACX
    outputDirectorySelectButton->setMaximumWidth(
         fontMetrics().boundingRect("...").width() + 6);
#endif
    outputDirectorySelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                               QSizePolicy::Minimum));

    connect(outputDirectorySelectButton, SIGNAL(clicked()),
            this, SLOT(selectOutputDirectory()));
    
    outputDirectoryLayout->setSpacing(0);
    

    nameLayout->addLayout(outputDirectoryLayout, 3, 0, 1, 2);
    
    // Create a group box for the file format.
    QGroupBox *formatBox = new QGroupBox(central);
    formatBox->setTitle(tr("Format Options"));
    topLayout->addWidget(formatBox);

    QGridLayout *formatLayout = new QGridLayout(formatBox);

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
    QLabel *formatLabel = new QLabel(tr("File type"),formatBox);
    formatLayout->addWidget(formatLabel, 5, 0);
    formatLayout->addWidget(fileFormatComboBox, 5, 1);

    // The quality slider.
    qualitySlider = new QSlider(Qt::Horizontal, formatBox);
    qualitySlider->setMinimum(0);
    qualitySlider->setMaximum(100);
    
    connect(qualitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(qualityChanged(int)));
    formatLayout->addWidget(qualitySlider, 6, 1);
    qualityLabel = new QLabel(tr("Quality"),formatBox);
    formatLayout->addWidget(qualityLabel, 6, 0);

    // The progressive toggle.
    progressiveCheckBox = new QCheckBox(tr("Progressive"), formatBox);
    connect(progressiveCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(progressiveToggled(bool)));
    formatLayout->addWidget(progressiveCheckBox, 7, 1, Qt::AlignRight);

    QHBoxLayout *compressionLayout = new QHBoxLayout();
    
    compressionTypeLabel = new QLabel(tr("Compression type"),formatBox);
    compressionTypeComboBox = new QComboBox(formatBox);
    compressionTypeComboBox->addItem(tr("None"));
    compressionTypeComboBox->addItem(tr("PackBits"));
    compressionTypeComboBox->addItem(tr("JPEG"));
    compressionTypeComboBox->addItem(tr("Deflate"));
    //compressionTypeComboBox->addItem("LZW");
    compressionLayout->addWidget(compressionTypeLabel);
    compressionLayout->addWidget(compressionTypeComboBox);
    
    connect(compressionTypeComboBox, SIGNAL(activated(int)),
            this, SLOT(compressionTypeChanged(int)));
    compressionTypeLabel->setBuddy(compressionTypeComboBox);
    formatLayout->addLayout(compressionLayout, 8,0, 1,2);

    binaryCheckBox = new QCheckBox(tr("Binary"), central);
    connect(binaryCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(binaryToggled(bool)));
    formatLayout->addWidget(binaryCheckBox, 9, 0);

    // The stereo toggle.
    forceMergeCheckBox = new QCheckBox(tr("Force parallel merge"), central);
    connect(forceMergeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(forceMergeToggled(bool)));
    formatLayout->addWidget(forceMergeCheckBox, 9, 1, Qt::AlignRight);

    // The save button.
    QHBoxLayout *saveButtonLayout = new QHBoxLayout();
    topLayout->addLayout(saveButtonLayout);
    
    //saveButtonLayout->setSpacing(5);
    QPushButton *saveButton = new QPushButton(tr("Save"), central);
    connect(saveButton, SIGNAL(clicked()),
            this, SLOT(saveButtonClicked()));
    saveButtonLayout->addWidget(saveButton);
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
//   
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
// ****************************************************************************

void
QvisSaveWindow::UpdateWindow(bool doAll)
{
    QString temp;

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
                stereoCheckBox->setEnabled(true);
                stereoCheckBox2->setEnabled(true);
                advancedMultiWinSaveCheckBox->setEnabled(true);
                screenCaptureCheckBox->setEnabled(true);
                forceMergeCheckBox->setEnabled(false);
                saveTiledCheckBox->setEnabled(true);
            }
            else
            {
                stereoCheckBox->setEnabled(false);
                stereoCheckBox2->setEnabled(false);
                advancedMultiWinSaveCheckBox->setEnabled(false);
                screenCaptureCheckBox->setEnabled(false);
                forceMergeCheckBox->setEnabled(true);
                saveTiledCheckBox->setEnabled(false);
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

            // Set the enabled state of the resolution widgets.
            resolutionBox->setEnabled(!saveWindowAtts->GetScreenCapture());
            break;
        case SaveWindowAttributes::ID_saveTiled:
            saveTiledCheckBox->blockSignals(true);
            saveTiledCheckBox->setChecked(saveWindowAtts->GetSaveTiled());
            saveTiledCheckBox->blockSignals(false);
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
            noResButton->blockSignals(true);
            oneToOneResButton->blockSignals(true);
            screenResButton->blockSignals(true);
            heightLineEdit->blockSignals(true);
            if (saveWindowAtts->GetResConstraint() == SaveWindowAttributes::NoConstraint)
            {
                noResButton->setChecked(true);
                oneToOneResButton->setChecked(false);
                screenResButton->setChecked(false);
                heightLineEdit->setEnabled(true);
            }
            else if (saveWindowAtts->GetResConstraint() == SaveWindowAttributes::EqualWidthHeight)
            {
                noResButton->setChecked(false);
                oneToOneResButton->setChecked(true);
                screenResButton->setChecked(false);
                heightLineEdit->setEnabled(false);
            }
            else if (saveWindowAtts->GetResConstraint() == SaveWindowAttributes::ScreenProportions)
            {
                noResButton->setChecked(false);
                oneToOneResButton->setChecked(false);
                screenResButton->setChecked(true);
                heightLineEdit->setEnabled(false);
            }
            noResButton->blockSignals(false);
            oneToOneResButton->blockSignals(false);
            screenResButton->blockSignals(false);
            heightLineEdit->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_advancedMultiWindowSave:
            advancedMultiWinSaveCheckBox->blockSignals(true);
            advancedMultiWinSaveCheckBox->setChecked(saveWindowAtts->GetAdvancedMultiWindowSave());
            advancedMultiWinSaveCheckBox->blockSignals(false);
            break;
        case SaveWindowAttributes::ID_subWindowAtts:
            mwsWindowComboBox->blockSignals(true);
            mwsWindowComboBox->setCurrentIndex(currentWindow);
            mwsWindowComboBox->blockSignals(false);
            mwsWidthLineEdit->blockSignals(true);
            temp.sprintf("%d", saveWindowAtts->GetWidth());
            mwsWidthLineEdit->setText(temp);
            mwsWidthLineEdit->blockSignals(false);
            mwsHeightLineEdit->blockSignals(true);
            temp.sprintf("%d", saveWindowAtts->GetHeight());
            mwsHeightLineEdit->setText(temp);
            mwsHeightLineEdit->blockSignals(false);
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

    // Make sure that the height text field is not enabled when we're saving
    // a tiled image.
    bool shouldBeEnabled = !saveWindowAtts->GetSaveTiled();
    if (noResButton->isEnabled() != shouldBeEnabled)
    {
        noResButton->setEnabled(shouldBeEnabled);
        oneToOneResButton->setEnabled(shouldBeEnabled);
        screenResButton->setEnabled(shouldBeEnabled);
        heightLineEdit->setEnabled(shouldBeEnabled);
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
    bool setWidth = false;
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
                    setWidth = (saveWindowAtts->GetWidth() != w);
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
    bool setHeight = false;
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
                    setHeight = (saveWindowAtts->GetHeight() != h);
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

    //
    // If doAll is true then this method is probably getting called by clicking
    // the Apply button. In that case, and we're enforcing 1:1 aspect then we
    // should update the width or height with the value that was set last in
    // case the user never hit the Enter key after typing a new width or height.
    //
    if(doAll && saveWindowAtts->GetResConstraint() == SaveWindowAttributes::EqualWidthHeight &&
       saveWindowAtts->GetWidth() != saveWindowAtts->GetHeight())
    {
        if(setWidth)
            saveWindowAtts->SetHeight(saveWindowAtts->GetWidth());
        else if(setHeight)
            saveWindowAtts->SetWidth(saveWindowAtts->GetHeight());
    }

    if (which_widget == SaveWindowAttributes::ID_subWindowAtts || doAll)
    {
        if (saveWindowAtts->GetAdvancedMultiWindowSave())
        {
            SaveSubWindowAttributes &atts = saveWindowAtts->GetSubWindowAtts().GetAttsForWindow(currentWindow+1);

            // width & height.  Note this is a bit tricky, since it shares with widgets in the other tab.
            temp = mwsWidthLineEdit->displayText().simplified();
            okay = !temp.isEmpty();
            if(okay)
            {
                int w;
                okay = (sscanf(temp.toStdString().c_str(), "%d", &w) == 1);
                if(okay)
                    saveWindowAtts->SetWidth(w);
            }
            if(!okay)
            {
                msg = tr("The width was invalid. "
                         "Resetting to the last good value %1.").
                      arg(saveWindowAtts->GetWidth());
                Message(msg);
                saveWindowAtts->SetWidth(saveWindowAtts->GetWidth());
            }
            temp = mwsHeightLineEdit->displayText().simplified();
            okay = !temp.isEmpty();
            if(okay)
            {
                int h;
                okay = (sscanf(temp.toStdString().c_str(), "%d", &h) == 1);
                if(okay)
                    saveWindowAtts->SetHeight(h);
            }
            if(!okay)
            {
                msg = tr("The height was invalid. "
                         "Resetting to the last good value %1.").
                      arg(saveWindowAtts->GetHeight());
                Message(msg);
                saveWindowAtts->SetHeight(saveWindowAtts->GetHeight());
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
//            aslice attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:25:04 PST 2001
//
// Modifications:
//   
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
// ****************************************************************************

void
QvisSaveWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
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

void
QvisSaveWindow::compressionTypeChanged(int index)
{
    saveWindowAtts->SetCompression(SaveWindowAttributes::CompressionType(index));
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::resConstraintToggled
//
// Purpose: 
//   This is a Qt slot function that sets an enum indicating whether the
//   resolution should be constrained to have 1:1 proportions, to match
//   the current screen proportions, or be unconstrained.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Dave Bremer
// Creation:   Thu Sep 27 19:35:42 PDT 2007
//
// Modifications:
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisSaveWindow::resConstraintToggled(bool val)
{
    // Ignore the "toggled off" signal
    if (!val)
        return;

    if (noResButton->isChecked())
    {
        saveWindowAtts->SetResConstraint(SaveWindowAttributes::NoConstraint);
        heightLineEdit->setEnabled(true);
    }
    else if (oneToOneResButton->isChecked())
    {
        saveWindowAtts->SetResConstraint(SaveWindowAttributes::EqualWidthHeight);
        heightLineEdit->setEnabled(false);

        saveWindowAtts->SetHeight(saveWindowAtts->GetWidth());
        QString temp;
        temp.sprintf("%d", saveWindowAtts->GetHeight());
        heightLineEdit->setText(temp);
    }
    else if (screenResButton->isChecked())
    {
        saveWindowAtts->SetResConstraint(SaveWindowAttributes::ScreenProportions);
        heightLineEdit->setEnabled(false);
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
// ****************************************************************************

void
QvisSaveWindow::processWidthText()
{
    GetCurrentValues(SaveWindowAttributes::ID_width);
    // If we're maintaining the 1:1 aspect ratio, udpate
    // the height too.
    if (saveWindowAtts->GetResConstraint() == SaveWindowAttributes::EqualWidthHeight)
        saveWindowAtts->SetHeight(saveWindowAtts->GetWidth());
    QString temp;
    temp.sprintf("%d", saveWindowAtts->GetHeight());
    heightLineEdit->setText(temp);

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
// ****************************************************************************

void
QvisSaveWindow::processHeightText()
{
    GetCurrentValues(SaveWindowAttributes::ID_height);
    // If we're maintaining the 1:1 aspect ratio, udpate
    // the width too.
    if (saveWindowAtts->GetResConstraint() == SaveWindowAttributes::EqualWidthHeight)
        saveWindowAtts->SetWidth(saveWindowAtts->GetHeight());
    QString temp;
    temp.sprintf("%d", saveWindowAtts->GetWidth());
    widthLineEdit->setText(temp);

    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::progressiveToggled
//
// Purpose: 
//   This is a QWt slot function that is called when the progressive toggle
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

// ****************************************************************************
// Method: QvisSaveWindow::binaryToggled
//
// Purpose: 
//   This is a QWt slot function that is called when the binary toggle
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
//   
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
// Method: QvisSaveWindow::screenCaptureToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not the image is saved through screen capture.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:27:07 PST 2001
//
// Modifications:
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
//   Brad Whitlock, Thu Jul 15 16:10:25 PST 2004
//   Removed some code to prevent saving tiled images.
//
// ****************************************************************************

void
QvisSaveWindow::screenCaptureToggled(bool val)
{
    saveWindowAtts->SetScreenCapture(val);
    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::saveTiledToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not the image is saved in tiled format.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:27:07 PST 2001
//
// Modifications:
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
//   Brad Whitlock, Thu Jul 15 16:10:25 PST 2004
//   Removed some code to prevent screen capture.
//
// ****************************************************************************

void
QvisSaveWindow::saveTiledToggled(bool val)
{
    saveWindowAtts->SetSaveTiled(val);
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
//
// ****************************************************************************

void
QvisSaveWindow::forceMergeToggled(bool val)
{
    saveWindowAtts->SetSaveTiled(val);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::advancedMultiWinSaveToggled
//
//  Purpose:
//    This is a Qt slot function that is called when the advanced multi-window
//    save button is clicked.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void 
QvisSaveWindow::advancedMultiWinSaveToggled(bool val)
{
    saveWindowAtts->SetAdvancedMultiWindowSave(val);
    saveWindowAtts->SetResConstraint(SaveWindowAttributes::NoConstraint);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::processmwsWidthText
//
//  Purpose:
//    This is a Qt slot function that is called when the width field for
//    advanced multi-window saves should be processed.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void
QvisSaveWindow::processmwsWidthText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::processmwsHeightText
//
//  Purpose:
//    This is a Qt slot function that is called when the height field for
//    advanced multi-window saves should be processed.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void
QvisSaveWindow::processmwsHeightText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::mwsWindowComboBoxChanged
//
//  Purpose:
//    This is a Qt slot function that is called when the combo box that selects
//    the current window changes.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void 
QvisSaveWindow::mwsWindowComboBoxChanged(int v)
{
    // process any changes in the line edits
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
   
    currentWindow = v;
    UpdateWindow(true);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::processmwsIndWidthText
//
//  Purpose:
//    This is a Qt slot function that is called when the width field for
//    an individual window in an advanced multi-window saves should be processed.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void
QvisSaveWindow::processmwsIndWidthText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::processmwsIndHeightText
//
//  Purpose:
//    This is a Qt slot function that is called when the height field for
//    an individual window in an advanced multi-window saves should be processed.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void
QvisSaveWindow::processmwsIndHeightText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::processmwsPosXText
//
//  Purpose:
//    This is a Qt slot function that is called when the x-position for
//    an individual window in an advanced multi-window saves should be processed.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void
QvisSaveWindow::processmwsPosXText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::processmwsPosYText
//
//  Purpose:
//    This is a Qt slot function that is called when the y-position for
//    an individual window in an advanced multi-window saves should be processed.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void
QvisSaveWindow::processmwsPosYText()
{
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::mwsLayerComboBoxChanged
//
//  Purpose:
//    This is a Qt slot function that is called when the layer for
//    an individual window in an advanced multi-window saves has changed.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void QvisSaveWindow::mwsLayerComboBoxChanged(int val)
{
    // read line edits 
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);

    SaveSubWindowAttributes &atts = saveWindowAtts->GetSubWindowAtts().GetAttsForWindow(currentWindow+1);
    atts.SetLayer(val+1);
    Apply();
}


// ****************************************************************************
//  Method: QvisSaveWindow::omitWindowCheckBoxToggled
//
//  Purpose:
//    This is a Qt slot function that is called when omit status for
//    an individual window in an advanced multi-window saves has changed.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void QvisSaveWindow::omitWindowCheckBoxToggled(bool val)
{
    // read line edits 
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);

    SaveSubWindowAttributes &atts = saveWindowAtts->GetSubWindowAtts().GetAttsForWindow(currentWindow+1);
    atts.SetOmitWindow(val);
    Apply();
}

// ****************************************************************************
//  Method: QvisSaveWindow::imageTransparencyChanged
//
//  Purpose:
//    This is a Qt slot function that is called when the transparency for
//    an individual window in an advanced multi-window saves has changed.
//
//  Programmer: Hank Childs
//  Creation:   July 22, 2010
//
// ****************************************************************************

void QvisSaveWindow::imageTransparencyChanged(int val)
{
    // read line edits 
    GetCurrentValues(SaveWindowAttributes::ID_subWindowAtts);

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
// Note:       Hides the window and saves the active vis window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 30 15:51:57 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::saveWindow()
{
    Apply();
    if(isVisible() && !posted())
        hide();
    GetViewerMethods()->SaveWindow();
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
// Method: QvisSaveWindow::saveButtonClicked
//
// Purpose: 
//   This is Qt slot function that is called when the save button is clicked. 
//
// Programmer: Kathleen Bonnell 
// Creation:   December 15, 2004 
//
// Modifications:
//
// ****************************************************************************

void
QvisSaveWindow::saveButtonClicked()
{
    apply();
    saveWindow();
}


