/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <stdio.h> // for sscanf

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qslider.h>

#include <QvisSaveWindow.h>
#include <SaveWindowAttributes.h>
#include <ViewerProxy.h>

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
// ****************************************************************************

QvisSaveWindow::QvisSaveWindow(
    SaveWindowAttributes *subj, const char *caption, const char *shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    saveWindowAtts = subj;
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
// ****************************************************************************

void
QvisSaveWindow::CreateWindowContents()
{
    // Create a group box for the file information.
    QGroupBox *infoBox = new QGroupBox(central, "infoBox");
    infoBox->setTitle("File information");
    topLayout->addWidget(infoBox);

    QGridLayout *infoLayout = new QGridLayout(infoBox, 9, 2);
    infoLayout->setMargin(10);
    infoLayout->setSpacing(5);
    infoLayout->addRowSpacing(0, 10);
    
    outputToCurrentDirectoryCheckBox = new QCheckBox("Output files to current directory",
        infoBox, "outputToCurrentDirectoryCheckBox");
    connect(outputToCurrentDirectoryCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(outputToCurrentDirectoryToggled(bool)));
    infoLayout->addMultiCellWidget(outputToCurrentDirectoryCheckBox, 1, 1, 0, 1);

    QHBox *outputDirectoryParent = new QHBox(infoBox, "outputDirectoryParent");
    outputDirectoryLabel = new QLabel("Output directory",
        infoBox, "outputDirectoryLabel");
    outputDirectoryLineEdit = new QLineEdit(outputDirectoryParent,
        "outputDirectoryLineEdit");
    connect(outputDirectoryLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processOutputDirectoryText()));
    outputDirectorySelectButton = new QPushButton("...", outputDirectoryParent,
        "outputSelectButton");
#ifndef Q_WS_MACX
    outputDirectorySelectButton->setMaximumWidth(
         fontMetrics().boundingRect("...").width() + 6);
#endif
    outputDirectorySelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
         QSizePolicy::Minimum));
    connect(outputDirectorySelectButton, SIGNAL(clicked()),
            this, SLOT(selectOutputDirectory()));
    outputDirectoryParent->setSpacing(0);
    outputDirectoryParent->setStretchFactor(outputDirectoryLineEdit, 100);
    outputDirectoryLabel->setBuddy(outputDirectoryParent);
    infoLayout->addMultiCellWidget(outputDirectoryLabel, 2, 2, 0, 1);
    infoLayout->addMultiCellWidget(outputDirectoryParent, 3, 3, 0, 1);
    
    filenameLineEdit = new QLineEdit(infoBox, "filenameLineEdit");
    connect(filenameLineEdit, SIGNAL(returnPressed()), this, SLOT(processFilenameText()));
    QLabel *filenameLabel = new QLabel(filenameLineEdit, "Filename", infoBox, "filenameLabel");
    infoLayout->addWidget(filenameLabel, 4, 0);
    infoLayout->addWidget(filenameLineEdit, 4, 1);

    fileFormatComboBox = new QComboBox(false, infoBox, "fileFormatComboBox");
    fileFormatComboBox->insertItem("bmp");
    fileFormatComboBox->insertItem("curve");
    fileFormatComboBox->insertItem("jpeg");
    fileFormatComboBox->insertItem("obj");
    fileFormatComboBox->insertItem("png");
    fileFormatComboBox->insertItem("postscript");
    fileFormatComboBox->insertItem("pov");
    fileFormatComboBox->insertItem("ppm");
    fileFormatComboBox->insertItem("rgb");
    fileFormatComboBox->insertItem("stl");
    fileFormatComboBox->insertItem("tiff");
    fileFormatComboBox->insertItem("ultra");
    fileFormatComboBox->insertItem("vtk");
    connect(fileFormatComboBox, SIGNAL(activated(int)),
           this, SLOT(fileFormatChanged(int)));
    QLabel *formatLabel = new QLabel(fileFormatComboBox, "File type",
                                     infoBox, "formatLabel");
    infoLayout->addWidget(formatLabel, 5, 0);
    infoLayout->addWidget(fileFormatComboBox, 5, 1);

    // The quality slider.
    qualitySlider = new QSlider(Qt::Horizontal, infoBox, "qualitySlider");
    qualitySlider->setMinValue(0);
    qualitySlider->setMaxValue(100);
    connect(qualitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(qualityChanged(int)));
    infoLayout->addWidget(qualitySlider, 6, 1);
    qualityLabel = new QLabel(qualitySlider, "Quality",
                              infoBox, "qualityLabel");
    infoLayout->addWidget(qualityLabel, 6, 0);

    // The progressive toggle.
    progressiveCheckBox = new QCheckBox("Progressive", infoBox, "progressiveCheckBox");
    connect(progressiveCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(progressiveToggled(bool)));
    infoLayout->addWidget(progressiveCheckBox, 7, 1, Qt::AlignRight);

    QHBox *compressionParent = new QHBox(infoBox, "compressionParent");
    compressionTypeLabel = new QLabel("Compression type",
                                     compressionParent, "compressionLabel");
    compressionTypeComboBox = new QComboBox(false, compressionParent, "compressionTypeComboBox");
    compressionTypeComboBox->insertItem("None");
    compressionTypeComboBox->insertItem("PackBits");
    compressionTypeComboBox->insertItem("JPEG");
    compressionTypeComboBox->insertItem("Deflate");
    //compressionTypeComboBox->insertItem("LZW");
    connect(compressionTypeComboBox, SIGNAL(activated(int)),
           this, SLOT(compressionTypeChanged(int)));
    compressionTypeLabel->setBuddy(compressionTypeComboBox);
    infoLayout->addMultiCellWidget(compressionParent, 8,8, 0,1);

    // Create a group box for the image resolution.
    resolutionBox = new QGroupBox(central, "resolutionBox");
    resolutionBox->setTitle("Resolution");
    topLayout->addWidget(resolutionBox);

    QGridLayout *resolutionLayout = new QGridLayout(resolutionBox, 3, 4);
    resolutionLayout->setMargin(10);
    resolutionLayout->setSpacing(5);
    resolutionLayout->addRowSpacing(0, 10);

    maintainAspectCheckBox = new QCheckBox("Maintain 1:1 aspect", resolutionBox,
                                           "maintainAspectCheckBox");
    connect(maintainAspectCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(maintainAspectToggled(bool)));
    resolutionLayout->addMultiCellWidget(maintainAspectCheckBox, 1, 1, 0, 3);

    // Create the width lineedit and label.
    widthLineEdit = new QLineEdit(resolutionBox, "widthLineEdit");
    connect(widthLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processWidthText()));
    QLabel *widthLabel = new QLabel(widthLineEdit, "Width", resolutionBox, "widthLabel");
    resolutionLayout->addWidget(widthLabel, 2, 0);
    resolutionLayout->addWidget(widthLineEdit, 2, 1);

    // Create the height lineedit and label.
    heightLineEdit = new QLineEdit(resolutionBox, "heightLineEdit");
    connect(heightLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processHeightText()));
    QLabel *heightLabel = new QLabel(heightLineEdit, "Height", resolutionBox, "heightLabel");
    resolutionLayout->addWidget(heightLabel, 2, 2);
    resolutionLayout->addWidget(heightLineEdit, 2, 3);

    // The family toggle.
    QGridLayout *toggleLayout = new QGridLayout(topLayout, 2, 3);
    toggleLayout->setSpacing(5);
    familyCheckBox = new QCheckBox("Family", central, "familyCheckBox");
    connect(familyCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(familyToggled(bool)));
    toggleLayout->addWidget(familyCheckBox, 0, 0);

    // The screen capture toggle.
    screenCaptureCheckBox = new QCheckBox("Screen capture", central, "screenCaptureCheckBox");
    connect(screenCaptureCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(screenCaptureToggled(bool)));
    toggleLayout->addWidget(screenCaptureCheckBox, 0, 1);

    // The tiled toggle.
    saveTiledCheckBox = new QCheckBox("Save tiled", central, "saveTiledCheckBox");
    connect(saveTiledCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(saveTiledToggled(bool)));
    toggleLayout->addWidget(saveTiledCheckBox, 0, 2);

    // The binary toggle.
    binaryCheckBox = new QCheckBox("Binary", central, "binaryCheckBox");
    connect(binaryCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(binaryToggled(bool)));
    toggleLayout->addWidget(binaryCheckBox, 1, 0);

    // The stereo toggle.
    stereoCheckBox = new QCheckBox("Stereo", central, "stereoCheckBox");
    connect(stereoCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    toggleLayout->addWidget(stereoCheckBox, 1, 1);

    // The stereo toggle.
    forceMergeCheckBox = new QCheckBox("Force parallel merge", central, "forceMergeCheckBox");
    connect(forceMergeCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(forceMergeToggled(bool)));
    toggleLayout->addWidget(forceMergeCheckBox, 1, 2);

    // The save button.
    QHBoxLayout *saveButtonLayout = new QHBoxLayout(topLayout);
    saveButtonLayout->setSpacing(5);
    QPushButton *saveButton = new QPushButton("Save", central, "saveButton");
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
// ****************************************************************************

void
QvisSaveWindow::UpdateWindow(bool doAll)
{
    QString temp;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < saveWindowAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!saveWindowAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 0: // use current directory
            outputToCurrentDirectoryCheckBox->blockSignals(true);
            outputToCurrentDirectoryCheckBox->setChecked(
                saveWindowAtts->GetOutputToCurrentDirectory());
            outputToCurrentDirectoryCheckBox->blockSignals(false);
            break;
        case 1: // output directory
            outputDirectoryLineEdit->setText(saveWindowAtts->GetOutputDirectory().c_str());
            break;
        case 2: // file name
            filenameLineEdit->setText(saveWindowAtts->GetFileName().c_str());
            break;
        case 3: // family
            familyCheckBox->blockSignals(true);
            familyCheckBox->setChecked(saveWindowAtts->GetFamily());
            familyCheckBox->blockSignals(false);
            break;
        case 4: // format
            fileFormatComboBox->blockSignals(true);
            fileFormatComboBox->setCurrentItem(saveWindowAtts->GetFormat());
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
                     == SaveWindowAttributes::STL)
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
                screenCaptureCheckBox->setEnabled(true);
                forceMergeCheckBox->setEnabled(false);
            }
            else
            {
                stereoCheckBox->setEnabled(false);
                screenCaptureCheckBox->setEnabled(false);
                forceMergeCheckBox->setEnabled(true);
            }
            break;
        case 5: // maintain aspect
            maintainAspectCheckBox->blockSignals(true);
            maintainAspectCheckBox->setChecked(saveWindowAtts->GetMaintainAspect());
            maintainAspectCheckBox->blockSignals(false);
            break;
        case 6: // width
            temp.sprintf("%d", saveWindowAtts->GetWidth());
            widthLineEdit->setText(temp);
            break;
        case 7: // height
            temp.sprintf("%d", saveWindowAtts->GetHeight());
            heightLineEdit->setText(temp);
            break;
        case 8: // screen capture
            screenCaptureCheckBox->blockSignals(true);
            screenCaptureCheckBox->setChecked(saveWindowAtts->GetScreenCapture());
            screenCaptureCheckBox->blockSignals(false);

            // Set the enabled state of the resolution widgets.
            resolutionBox->setEnabled(!saveWindowAtts->GetScreenCapture());
            break;
        case 9: // save tiled
            saveTiledCheckBox->blockSignals(true);
            saveTiledCheckBox->setChecked(saveWindowAtts->GetSaveTiled());
            saveTiledCheckBox->blockSignals(false);
            break;
        case 10: // quality
            qualitySlider->blockSignals(true);
            qualitySlider->setValue(saveWindowAtts->GetQuality());
            qualitySlider->blockSignals(false);
            break;
        case 11: // progressive
            progressiveCheckBox->blockSignals(true);
            progressiveCheckBox->setChecked(saveWindowAtts->GetProgressive());
            progressiveCheckBox->blockSignals(false);
            break;
        case 12: // binary
            binaryCheckBox->blockSignals(true);
            binaryCheckBox->setChecked(saveWindowAtts->GetBinary());
            binaryCheckBox->blockSignals(false);
            break;
        case 14: // stereo
            stereoCheckBox->blockSignals(true);
            stereoCheckBox->setChecked(saveWindowAtts->GetStereo());
            stereoCheckBox->blockSignals(false);
            break;
        case 15: // tiffCompression
            compressionTypeComboBox->blockSignals(true);
            compressionTypeComboBox->setCurrentItem(saveWindowAtts->GetCompression());
            compressionTypeComboBox->blockSignals(false);
            break;
        case 16: // forceMerge
            forceMergeCheckBox->blockSignals(true);
            forceMergeCheckBox->setChecked(saveWindowAtts->GetForceMerge());
            forceMergeCheckBox->blockSignals(false);
            break;
        }
    } // end for

    // Make sure that the height text field is not enabled when we're saving
    // a tiled image.
    bool shouldBeEnabled = !saveWindowAtts->GetSaveTiled();
    if(maintainAspectCheckBox->isEnabled() != shouldBeEnabled)
    {
        maintainAspectCheckBox->setEnabled(shouldBeEnabled);
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
// ****************************************************************************

void
QvisSaveWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the host name
    if(which_widget == 0 || doAll)
    {
        temp = outputDirectoryLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            saveWindowAtts->SetOutputDirectory(temp.latin1());
        }
        else
        {
            msg.sprintf("The output directory was invalid. "
                "Resetting to the last good value \"%s\".",
                 saveWindowAtts->GetOutputDirectory().c_str());
            Message(msg);
            saveWindowAtts->SetOutputDirectory(saveWindowAtts->GetOutputDirectory());
        }
    }

    // Do the file name
    if(which_widget == 1 || doAll)
    {
        temp = filenameLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            saveWindowAtts->SetFileName(temp.latin1());
        }
        else
        {
            msg.sprintf("The filename was invalid. "
                "Resetting to the last good value \"%s\".",
                saveWindowAtts->GetFileName().c_str());
            Message(msg);
            saveWindowAtts->SetFileName(saveWindowAtts->GetFileName());
        }
    }

    // Do the image width
    bool setWidth = false;
    if(which_widget == 2 || doAll)
    {
        temp = widthLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int w;
            okay = (sscanf(temp.latin1(), "%d", &w) == 1);
            if(okay)
            {
                setWidth = (saveWindowAtts->GetWidth() != w);
                saveWindowAtts->SetWidth(w);
            }
        }

        if(!okay)
        {
            msg.sprintf("The width was invalid. "
                "Resetting to the last good value %d.",
                saveWindowAtts->GetWidth());
            Message(msg);
            saveWindowAtts->SetWidth(saveWindowAtts->GetWidth());
        }
    }

    // Do the image height
    bool setHeight = false;
    if(which_widget == 3 || doAll)
    {
        temp = heightLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int h;
            okay = (sscanf(temp.latin1(), "%d", &h) == 1);
            if(okay)
            {
                setHeight = (saveWindowAtts->GetHeight() != h);
                saveWindowAtts->SetHeight(h);
            }
        }

        if(!okay)
        {
            msg.sprintf("The height was invalid. "
                "Resetting to the last good value %d.",
                saveWindowAtts->GetHeight());
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
    if(doAll && saveWindowAtts->GetMaintainAspect() &&
       saveWindowAtts->GetWidth() != saveWindowAtts->GetHeight())
    {
        if(setWidth)
            saveWindowAtts->SetHeight(saveWindowAtts->GetWidth());
        else if(setHeight)
            saveWindowAtts->SetWidth(saveWindowAtts->GetHeight());
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
    GetCurrentValues(0);
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
    GetCurrentValues(1);
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
// Method: QvisSaveWindow::maintainAspectToggled
//
// Purpose: 
//   This is a Qt slot function that sets the flag indicating whether
//   or not the image resolution should be a square.
//
// Arguments:
//   val : The state of the toggle button.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 17:27:07 PST 2001
//
// Modifications:
//   
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
// ****************************************************************************

void
QvisSaveWindow::maintainAspectToggled(bool val)
{
    saveWindowAtts->SetMaintainAspect(val);

    // Make sure the height and width are equal if we're turning on maintain
    // 1:1 aspect ratio.
    if(val)
    {
        saveWindowAtts->SetWidth(saveWindowAtts->GetWidth());
        saveWindowAtts->SetHeight(saveWindowAtts->GetWidth());
    }

    Apply();
}

// ****************************************************************************
// Method: QvisSaveWindow::processwidthLineEdit
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
// ****************************************************************************

void
QvisSaveWindow::processWidthText()
{
    GetCurrentValues(2);
    // If we're maintaining the 1:1 aspect ratio, udpate
    // the height too.
    if(saveWindowAtts->GetMaintainAspect())
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
// ****************************************************************************

void
QvisSaveWindow::processHeightText()
{
    GetCurrentValues(3);
    // If we're maintaining the 1:1 aspect ratio, udpate
    // the width too.
    if(saveWindowAtts->GetMaintainAspect())
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
// ****************************************************************************

void
QvisSaveWindow::selectOutputDirectory()
{
    //
    // Try and get a directory using a file dialog.
    //
    QString initialDir(saveWindowAtts->GetOutputDirectory().c_str());
    QString dirName = QFileDialog::getExistingDirectory(initialDir, this,
        "getDirectoryDialog", "Select output directory");

    //
    // If a directory was chosen, use it as the output directory.
    //
    if(!dirName.isEmpty())
    {
        outputDirectoryLineEdit->setText(dirName);
        GetCurrentValues(0);
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
