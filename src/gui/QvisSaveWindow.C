#include <stdio.h> // for sscanf

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
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
// ****************************************************************************

void
QvisSaveWindow::CreateWindowContents()
{
    // Create a group box for the file information.
    QGroupBox *infoBox = new QGroupBox(central, "infoBox");
    infoBox->setTitle("File information");
    topLayout->addWidget(infoBox);

    QGridLayout *infoLayout = new QGridLayout(infoBox, 7, 2);
    infoLayout->setMargin(10);
    infoLayout->setSpacing(5);
    infoLayout->addRowSpacing(0, 10);
    
    hostLineEdit = new QLineEdit(infoBox, "hostLineEdit");
    connect(hostLineEdit, SIGNAL(returnPressed()), this, SLOT(processHostText()));
    QLabel *hostLabel = new QLabel(hostLineEdit, "Host", infoBox, "hostLabel");
    infoLayout->addWidget(hostLabel, 1, 0);
    infoLayout->addWidget(hostLineEdit, 1, 1);
    
    filenameLineEdit = new QLineEdit(infoBox, "filenameLineEdit");
    connect(filenameLineEdit, SIGNAL(returnPressed()), this, SLOT(processFilenameText()));
    QLabel *filenameLabel = new QLabel(filenameLineEdit, "Filename", infoBox, "filenameLabel");
    infoLayout->addWidget(filenameLabel, 2, 0);
    infoLayout->addWidget(filenameLineEdit, 2, 1);

    fileFormatComboBox = new QComboBox(false, infoBox, "fileFormatComboBox");
    fileFormatComboBox->insertItem("bmp");
    fileFormatComboBox->insertItem("curve");
    fileFormatComboBox->insertItem("jpeg");
    fileFormatComboBox->insertItem("obj");
    fileFormatComboBox->insertItem("png");
    fileFormatComboBox->insertItem("postscript");
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
    infoLayout->addWidget(formatLabel, 3, 0);
    infoLayout->addWidget(fileFormatComboBox, 3, 1);

    // The quality slider.
    qualitySlider = new QSlider(Qt::Horizontal, infoBox, "qualitySlider");
    qualitySlider->setMinValue(0);
    qualitySlider->setMaxValue(100);
    connect(qualitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(qualityChanged(int)));
    infoLayout->addWidget(qualitySlider, 4, 1);
    qualityLabel = new QLabel(qualitySlider, "Quality",
                              infoBox, "qualityLabel");
    infoLayout->addWidget(qualityLabel, 4, 0);

    // The progressive toggle.
    progressiveCheckBox = new QCheckBox("Progressive", infoBox, "progressiveCheckBox");
    connect(progressiveCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(progressiveToggled(bool)));
    infoLayout->addWidget(progressiveCheckBox, 5, 1, Qt::AlignRight);

    compressionTypeComboBox = new QComboBox(false, infoBox, "compressionTypeComboBox");
    compressionTypeComboBox->insertItem("None");
    compressionTypeComboBox->insertItem("PackBits");
    compressionTypeComboBox->insertItem("JPEG");
    compressionTypeComboBox->insertItem("Deflate");
    //compressionTypeComboBox->insertItem("LZW");
    connect(compressionTypeComboBox, SIGNAL(activated(int)),
           this, SLOT(compressionTypeChanged(int)));
    compressionTypeLabel = new QLabel(compressionTypeComboBox, "Compression type",
                                     infoBox, "compressionLabel");
    infoLayout->addWidget(compressionTypeLabel, 6, 0);
    infoLayout->addWidget(compressionTypeComboBox, 6, 1);

    // Create a group box for the image resolution.
    resolutionBox = new QGroupBox(central, "resolutionBox");
    resolutionBox->setTitle("Resolution");
    topLayout->addWidget(resolutionBox);

    QGridLayout *resolutionLayout = new QGridLayout(resolutionBox, 3, 4);
    resolutionLayout->setMargin(10);
    resolutionLayout->setSpacing(5);
    resolutionLayout->addRowSpacing(0, 10);

    maintainAspectCheckBox = new QCheckBox("Maintain 1:1 apsect", resolutionBox,
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

    QHBoxLayout *toggleLayout = new QHBoxLayout(topLayout);
    toggleLayout->setSpacing(5);
    toggleLayout->addStretch(10);
    familyCheckBox = new QCheckBox("Family", central, "familyCheckBox");
    connect(familyCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(familyToggled(bool)));
    toggleLayout->addWidget(familyCheckBox);

    screenCaptureCheckBox = new QCheckBox("Screen capture", central, "screenCaptureCheckBox");
    connect(screenCaptureCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(screenCaptureToggled(bool)));
    toggleLayout->addWidget(screenCaptureCheckBox);

    saveTiledCheckBox = new QCheckBox("Save tiled", central, "saveTiledCheckBox");
    connect(saveTiledCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(saveTiledToggled(bool)));
    toggleLayout->addWidget(saveTiledCheckBox);
    toggleLayout->addStretch(10);

    // The binary toggle.
    QHBoxLayout *toggleLayout2 = new QHBoxLayout(topLayout);
    toggleLayout2->setSpacing(5);
    toggleLayout2->addStretch(10);
    binaryCheckBox = new QCheckBox("Binary", central, "binaryCheckBox");
    connect(binaryCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(binaryToggled(bool)));
    toggleLayout2->addWidget(binaryCheckBox);
    toggleLayout2->addStretch(10);

    // The stereo toggle.
    stereoCheckBox = new QCheckBox("Stereo", central, "stereoCheckBox");
    connect(stereoCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(stereoToggled(bool)));
    toggleLayout2->addWidget(stereoCheckBox);
    toggleLayout2->addStretch(10);

#if 1
    // Get rid of this code when saving images is fully implemented in the
    // viewer.
    hostLabel->setEnabled(false);
    hostLineEdit->setEnabled(false);
    saveTiledCheckBox->setEnabled(false);
#endif
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
        case 0: // host name
            hostLineEdit->setText(saveWindowAtts->GetHostName().c_str());
            break;
        case 1: // file name
            filenameLineEdit->setText(saveWindowAtts->GetFileName().c_str());
            break;
        case 2: // family
            familyCheckBox->blockSignals(true);
            familyCheckBox->setChecked(saveWindowAtts->GetFamily());
            familyCheckBox->blockSignals(false);
            break;
        case 3: // format
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
            }
            else
            {
                stereoCheckBox->setEnabled(false);
            }
            break;
        case 4: // maintain aspect
            maintainAspectCheckBox->blockSignals(true);
            maintainAspectCheckBox->setChecked(saveWindowAtts->GetMaintainAspect());
            maintainAspectCheckBox->blockSignals(false);
            break;
        case 5: // width
            temp.sprintf("%d", saveWindowAtts->GetWidth());
            widthLineEdit->setText(temp);
            break;
        case 6: // height
            temp.sprintf("%d", saveWindowAtts->GetHeight());
            heightLineEdit->setText(temp);
            break;
        case 7: // screen capture
            screenCaptureCheckBox->blockSignals(true);
            screenCaptureCheckBox->setChecked(saveWindowAtts->GetScreenCapture());
            screenCaptureCheckBox->blockSignals(false);

            // Set the enabled state of the resolution widgets.
            resolutionBox->setEnabled(!saveWindowAtts->GetScreenCapture());
            break;
        case 8: // save tiled
            saveTiledCheckBox->blockSignals(true);
            saveTiledCheckBox->setChecked(saveWindowAtts->GetSaveTiled());
            saveTiledCheckBox->blockSignals(false);
            break;
        case 9: // quality
            qualitySlider->blockSignals(true);
            qualitySlider->setValue(saveWindowAtts->GetQuality());
            qualitySlider->blockSignals(false);
            break;
        case 10: // progressive
            progressiveCheckBox->blockSignals(true);
            progressiveCheckBox->setChecked(saveWindowAtts->GetProgressive());
            progressiveCheckBox->blockSignals(false);
            break;
        case 11: // binary
            binaryCheckBox->blockSignals(true);
            binaryCheckBox->setChecked(saveWindowAtts->GetBinary());
            binaryCheckBox->blockSignals(false);
            break;
        case 13: // stereo
            stereoCheckBox->blockSignals(true);
            stereoCheckBox->setChecked(saveWindowAtts->GetStereo());
            stereoCheckBox->blockSignals(false);
            break;
        case 14: // tiffCompression
            compressionTypeComboBox->blockSignals(true);
            compressionTypeComboBox->setCurrentItem(saveWindowAtts->GetCompression());
            compressionTypeComboBox->blockSignals(false);
            break;
        }
    } // end for
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
//   
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
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
        temp = hostLineEdit->displayText().simplifyWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            saveWindowAtts->SetHostName(temp.latin1());
        }
        else
        {
            msg.sprintf("The hostname was invalid. "
                "Resetting to the last good value \"%s\".",
                 saveWindowAtts->GetHostName().c_str());
            Message(msg);
            saveWindowAtts->SetHostName(saveWindowAtts->GetHostName());
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
                saveWindowAtts->SetWidth(w);

                // If we're maintaining the 1:1 aspect ratio, udpate
                // the height too.
                if(saveWindowAtts->GetMaintainAspect())
                    saveWindowAtts->SetHeight(w);
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
                saveWindowAtts->SetHeight(h);

                // If we're maintaining the 1:1 aspect ratio, udpate
                // the width too.
                if(saveWindowAtts->GetMaintainAspect())
                    saveWindowAtts->SetWidth(h);
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
// Method: QvisSaveWindow::processHostText
//
// Purpose: 
//   This is a Qt slot function that sets the host where we'll save the image.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 16:57:34 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveWindow::processHostText()
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
//   
// ****************************************************************************

void
QvisSaveWindow::processWidthText()
{
    GetCurrentValues(2);
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
//   
// ****************************************************************************

void
QvisSaveWindow::processHeightText()
{
    GetCurrentValues(3);
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
//   
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
// ****************************************************************************

void
QvisSaveWindow::screenCaptureToggled(bool val)
{
    saveWindowAtts->SetScreenCapture(val);

    // If we're doing screen capture, prevent saving tiled images.
    if(val)
        saveWindowAtts->SetSaveTiled(false);

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
//   
//   Hank Childs, Fri May 24 13:36:05 PDT 2002
//   Renamed saveImageAtts to saveWindowAtts.
//
// ****************************************************************************

void
QvisSaveWindow::saveTiledToggled(bool val)
{
    saveWindowAtts->SetSaveTiled(val);

    // If we're doing tiled, prevent screen capture.
    if(val)
        saveWindowAtts->SetScreenCapture(false);

    Apply();
}
