/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

#include <QvisCinemaWizard.h>
#include <CinemaAttributes.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimer>
#include <QWidget>

#include <ViewerProxy.h>

#include <visit-config.h>
#include <InstallationFunctions.h>
#include <DebugStream.h>

#include <StringHelpers.h>
#include <snprintf.h>

#define cinemaAtts ((CinemaAttributes *)localCopy)

// Add other file formats as needed.
const char *fileFormats[] = {
"bmp", 
#ifdef HAVE_LIBOPENEXR
"exr",
#endif
"jpeg",
"png",
"tiff"
};

static int
FormatToMenuIndex(const std::string &fmt)
{
    for(int i = 0; i < sizeof(fileFormats)/sizeof(const char *); ++i)
    { 
        if(fmt == fileFormats[i])
            return i;
    }
    return 3; // png
}

const char *cameraModes[] = {"static", "phi-theta"};

static int
CameraToMenuIndex(const std::string &fmt)
{
    for(int i = 0; i < sizeof(cameraModes)/sizeof(const char *); ++i)
    { 
        if(fmt == cameraModes[i])
            return i;
    }
    return 0;
}

// ****************************************************************************
// Method: QvisCinemaWizard::QvisCinemaWizard
//
// Purpose: 
//   This is the constructor for the QvisCinemaWizard class.
//
// Arguments:
//   atts   : A pointer to the attribute subject that the wizard will set.
//   parent : The wizard's parent.
//   name   : The name of the wizard.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:52:46 PDT 2017
//
// Modifications:
//   Kathleen Biagas, Thursday December 13, 2018
//   Disable default button feature of this wizard.
//
// ****************************************************************************

QvisCinemaWizard::QvisCinemaWizard(AttributeSubject *atts, QWidget *parent) : 
    QvisWizard(atts, parent)
{
    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::HaveHelpButton, false);
    setOption(QWizard::HaveNextButtonOnLastPage, false);

    default_image_size[0] = 600;
    default_image_size[1] = 600;
    default_num_frames = 1;

    // Set the wizard's title.
    setWindowTitle(tr("Save Cinema wizard"));

    // Make a side widget.
    setWizardStyle(QWizard::ModernStyle);

    // Disable the default button feature.
    // This wizard has line edits, and if there is a 'default' button, when
    // the user presses 'enter' in the line edit, the wizard will advance to
    // the next page, which isn't always desirable.
    setOption(QWizard::NoDefaultButton, true);

    QWidget *sideW = new QWidget(0);
    QVBoxLayout *sLayout = new QVBoxLayout(sideW);
    sLayout->setMargin(0);
    sLayout->setSpacing(0);
    QPixmap logo;
    std::string logoFilename = GetVisItResourcesFile(VISIT_RESOURCES_IMAGES, "cinema-watermark.png");
    if(logo.load(logoFilename.c_str()))
    {
        debug1 << "loaded file " << logoFilename << endl;
        QLabel *logoLabel = new QLabel(sideW);
        logoLabel->setPixmap(logo);
        sLayout->addWidget(logoLabel);
    }
    else
    {
        debug1 << "Failed to load " << logoFilename << endl;
    }

    QLabel *link = new QLabel(tr("<a href=\"http://www.cinemascience.org\">www.cinemascience.org</a>"), sideW);
    link->setTextInteractionFlags(Qt::TextBrowserInteraction);
    link->setOpenExternalLinks(true);
    link->setAlignment(Qt::AlignCenter);
    sLayout->addWidget(link);

    setSideWidget(sideW);


    // Create the wizard pages.
    CreateMainPage();
    CreateNumFramesPage();

    setStartId(Page_Main);
}

// ****************************************************************************
// Method: QvisCinemaWizard::~QvisCinemaWizard
//
// Purpose: 
//   Destructor for the QvisCinemaWizard class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:52:46 PDT 2017
//
// Modifications:
//
// ****************************************************************************

QvisCinemaWizard::~QvisCinemaWizard()
{
}

// ****************************************************************************
// Method: QvisCinemaWizard::Exec
//
// Purpose: 
//   This method executes the wizard.
//
// Returns:    A return value for the wizard, which will be used to tell the
//             user what to do.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:52:46 PDT 2017
//
// Modifications:
//
// ****************************************************************************

int
QvisCinemaWizard::Exec()
{
    QTimer::singleShot(10, this, SLOT(raise()));
    activateWindow();
    restart();

    // Go through the wizard.
    int retval = exec();

    return retval;
}

// ****************************************************************************
// Method: QvisCinemaWizard::SetDefaultImageSize
//
// Purpose: 
//   Sets the default image size.
//
// Arguments:
//   w : The image width.
//   h : The image height.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:52:46 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void
QvisCinemaWizard::SetDefaultImageSize(int w, int h)
{
    default_image_size[0] = w;
    default_image_size[1] = h;
}

// ****************************************************************************
// Method: QvisCinemaWizard::SetDefaultNumFrames
//
// Purpose: 
//   Sets the default number of movie frames
//
// Programmer: Dave Bremer
// Creation:   Fri Oct  5 15:41:01 PDT 2007
// ****************************************************************************

void
QvisCinemaWizard::SetDefaultNumFrames(int nFrames)
{
    default_num_frames = nFrames;
}

// ****************************************************************************
// Method: QvisCinemaWizard::GetDefaultNumFrames
//
// Purpose: 
//   Gets the default number of movie frames
//
// Programmer: Dave Bremer
// Creation:   Fri Oct  5 15:41:01 PDT 2007
// ****************************************************************************

int  
QvisCinemaWizard::GetDefaultNumFrames()
{
    return default_num_frames;
}

// ****************************************************************************
// Method: QvisCinemaWizard::CreateFilenameControl
//
// Purpose: 
//   Creates the filename control on page0.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:52:46 PDT 2017
//
// Modifications:
//   Kathleen Biagas, Thursday December 13, 2018
//   Changed page0_fileNameLineEdit signal connections from 'textChanged'
//   which is called with every character entered, to 'editingFinished'
//   and 'returnPressed' which are called when the user is finished with the
//   widget.
//
// ****************************************************************************
QWidget *
QvisCinemaWizard::CreateFilenameControl(QWidget *parent)
{
    QWidget *filenameParent = new QWidget(parent);
    QHBoxLayout *oLayout = new QHBoxLayout(filenameParent);
    oLayout->setSpacing(0);
    oLayout->setMargin(0);

    page0_fileNameLineEdit = new QLineEdit(filenameParent);
    page0_fileNameLineEdit->setMinimumWidth(300);
    oLayout->addWidget(page0_fileNameLineEdit);
    oLayout->setStretchFactor(page0_fileNameLineEdit, 100);
    connect(page0_fileNameLineEdit, SIGNAL(returnPressed()),
            this, SLOT(page0_processFileName()));
    connect(page0_fileNameLineEdit, SIGNAL(editingFinished()),
            this, SLOT(page0_processFileName()));

    QPushButton *outputSelectButton = new QPushButton("...", filenameParent);
    oLayout->addWidget(outputSelectButton);
#if !defined(Q_OS_MAC)
    outputSelectButton->setMaximumWidth(
         fontMetrics().boundingRect("...").width() + 6);
#endif
    outputSelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
         QSizePolicy::Minimum));
    connect(outputSelectButton, SIGNAL(clicked()),
            this, SLOT(page0_selectFileName()));

    return filenameParent;
}

// ****************************************************************************
// Method: QvisCinemaWizard::CreateMainPage
//
// Purpose: 
//   Creates page 0.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:52:46 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void
QvisCinemaWizard::CreateMainPage()
{
    page0 = new QWizardPage(this);
    page0->setTitle(tr("Cinema settings"));
    page0->setSubTitle(tr("Set Cinema export options."));
    page0->setMinimumWidth(400);

    QVBoxLayout *pageLayout = new QVBoxLayout(page0);
    pageLayout->setSpacing(10);
    pageLayout->setMargin(5);

    //
    // Database settings.
    //
    QGroupBox *gbDB = new QGroupBox(tr("Database settings"), page0);
    QGridLayout *dbLayout = new QGridLayout(gbDB);
    dbLayout->setMargin(5);
    pageLayout->addWidget(gbDB);

    QLabel *filenameLabel = new QLabel(tr("File name"), gbDB);
    dbLayout->addWidget(filenameLabel, 0, 0);
    QWidget *fn = CreateFilenameControl(gbDB);
    filenameLabel->setBuddy(fn);
    dbLayout->addWidget(fn, 0, 1, 1, 3);

    QLabel *dbTypeLabel = new QLabel(tr("Specification"), gbDB);
    dbTypeLabel->setToolTip(tr("The specification determines the type of Cinema database that will be created."));
    dbLayout->addWidget(dbTypeLabel, 1, 0);
    page0_specification = new QButtonGroup(gbDB);
    connect(page0_specification, SIGNAL(buttonClicked(int)),
            this, SLOT(page0_specificationChanged(int)));
    QRadioButton *rb0 = new QRadioButton(tr("A"), gbDB);
    page0_specification->addButton(rb0, 0);
    dbLayout->addWidget(rb0, 1, 1);
    QRadioButton *rb1 = new QRadioButton(tr("C"), gbDB);
    page0_specification->addButton(rb1, 1);
    dbLayout->addWidget(rb1, 1, 2);
    // disable rb1 until C-spec is fixed.
    rb1->setEnabled(false);
    QRadioButton *rb2 = new QRadioButton(tr("D"), gbDB);
    page0_specification->addButton(rb2, 2);
    dbLayout->addWidget(rb2, 1, 3);

    //
    // Image settings.
    //
    QGroupBox *gbImage = new QGroupBox(tr("Image settings"), page0);
    QGridLayout *iLayout = new QGridLayout(gbImage);
    iLayout->setMargin(5);
    pageLayout->addWidget(gbImage);

    iLayout->addWidget(new QLabel(tr("File format"), gbImage));
    page0_format = new QComboBox(gbImage);
    for(int i = 0; i < sizeof(fileFormats)/sizeof(const char *); ++i)
        page0_format->addItem(fileFormats[i]);
    iLayout->addWidget(page0_format, 0, 1);
    connect(page0_format, SIGNAL(activated(int)),
            this, SLOT(page0_formatActivated(int)));

    page0_composite = new QCheckBox(tr("Create composite images"), gbImage);
    page0_composite->setToolTip(tr("A composite image consists of data files containing luminance, depth, and value data."));
    iLayout->addWidget(page0_composite, 1, 0, 1, 4);
    connect(page0_composite, SIGNAL(stateChanged(int)),
            this, SLOT(page0_compositeStateChanged(int)));

    page0_screenCapture = new QCheckBox(tr("Use screen capture"), gbImage);
    iLayout->addWidget(page0_screenCapture, 2, 0, 1, 4);
    connect(page0_screenCapture, SIGNAL(stateChanged(int)),
            this, SLOT(page0_screenCaptureStateChanged(int)));

    page0_widthLabel = new QLabel(tr("Width"), gbImage);
    page0_width = new QSpinBox(gbImage);
    page0_width->setMinimum(200);
    page0_width->setMaximum(4096);
    connect(page0_width, SIGNAL(valueChanged(int)),
            this, SLOT(page0_widthValueChanged(int)));
    iLayout->addWidget(page0_widthLabel, 3, 0);
    iLayout->addWidget(page0_width, 3, 1);

    page0_heightLabel = new QLabel(tr("Height"), gbImage);
    page0_height = new QSpinBox(gbImage);
    page0_height->setMinimum(200);
    page0_height->setMaximum(4096);
    connect(page0_height, SIGNAL(valueChanged(int)),
            this, SLOT(page0_heightValueChanged(int)));
    iLayout->addWidget(page0_heightLabel, 3, 2);
    iLayout->addWidget(page0_height, 3, 3);

    //
    // Camera settings.
    //
    QGroupBox *gbCamera = new QGroupBox(tr("Camera"), page0);
    QGridLayout *cLayout = new QGridLayout(gbCamera);
    cLayout->setMargin(5);
    pageLayout->addWidget(gbCamera);

    QLabel *ctLabel = new QLabel(tr("Camera type"), gbCamera);
    cLayout->addWidget(ctLabel, 0, 0);
    page0_cameraMode = new QComboBox(gbCamera);
    for(int i = 0; i < sizeof(cameraModes)/sizeof(const char *); ++i)
        page0_cameraMode->addItem(cameraModes[i]);
    cLayout->addWidget(page0_cameraMode, 0, 1, 1, 3);
    connect(page0_cameraMode, SIGNAL(activated(int)),
            this, SLOT(page0_cameraModeActivated(int)));

    page0_phiLabel = new QLabel(tr("Phi"), gbCamera);
    page0_phi = new QSpinBox(gbCamera);
    page0_phi->setMinimum(1);
    page0_phi->setMaximum(360);
    connect(page0_phi, SIGNAL(valueChanged(int)),
            this, SLOT(page0_phiValueChanged(int)));
    cLayout->addWidget(page0_phiLabel, 1, 0);
    cLayout->addWidget(page0_phi, 1, 1);

    page0_thetaLabel = new QLabel(tr("Theta"), gbCamera);
    page0_theta = new QSpinBox(gbCamera);
    page0_theta->setMinimum(1);
    page0_theta->setMaximum(360);
    connect(page0_theta, SIGNAL(valueChanged(int)),
            this, SLOT(page0_thetaValueChanged(int)));
    cLayout->addWidget(page0_thetaLabel, 1, 2);
    cLayout->addWidget(page0_theta, 1, 3);

    // Add the page.
    setPage(Page_Main, page0);
}

// ****************************************************************************
// Method: QvisCinemaWizard::CreateNumFramesPage
//
// Purpose: 
//   Creates page 1.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:52:46 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void
QvisCinemaWizard::CreateNumFramesPage()
{
    page1 = new QWizardPage(this);
    page1->setTitle(tr("Choose length"));
    page1->setSubTitle(
        tr("Choose start/end frame and stride."));
    page1->setMinimumWidth(400);

    QVBoxLayout *pageLayout = new QVBoxLayout(page1);
    pageLayout->setMargin(5);
    pageLayout->setSpacing(10);

    QGridLayout *gLayout = new QGridLayout(0);
    pageLayout->addLayout(gLayout);
    gLayout->setMargin(0);
    gLayout->setSpacing(5);
    gLayout->setColumnStretch(1, 100);
    pageLayout->addStretch(20);

    gLayout->addWidget(new QLabel(tr("Frame start"), page1), 0, 0);
    page1_frameStart = new QSpinBox(page1);
    page1_frameStart->setMinimum(0);
    gLayout->addWidget(page1_frameStart, 0, 1);
    connect(page1_frameStart, SIGNAL(valueChanged(int)),
            this, SLOT(page1_frameStartValueChanged(int)));    

    gLayout->addWidget(new QLabel(tr("Frame end"), page1), 1, 0);
    page1_frameEnd = new QSpinBox(page1);
    page1_frameEnd->setMinimum(1);
    gLayout->addWidget(page1_frameEnd, 1, 1);
    connect(page1_frameEnd, SIGNAL(valueChanged(int)),
            this, SLOT(page1_frameEndValueChanged(int)));    

    gLayout->addWidget(new QLabel(tr("Frame stride"), page1), 2, 0);
    page1_frameStride = new QSpinBox(page1);
    page1_frameStride->setMinimum(1);
    page1_frameStride->setMaximum(100);
    gLayout->addWidget(page1_frameStride, 2, 1);
    connect(page1_frameStride, SIGNAL(valueChanged(int)),
            this, SLOT(page1_frameStrideValueChanged(int)));

    // Add the page.
    setPage(Page_NumFrames, page1);
}

// ****************************************************************************
// Method: QvisCinemaWizard::nextId
//
// Purpose: 
//   This method is called when we want to know the id of the next wizard page.
//
// Arguments:
//
// Returns:    The id of the next wizard page.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 15:02:43 PDT 2017
//
// Modifications:
//
// ****************************************************************************

int
QvisCinemaWizard::nextId() const
{
    int id;

    switch(currentId())
    {
    case Page_Main:
        id = Page_NumFrames;
        break;
    case Page_NumFrames:
        // there is no next page
        id = -1;
    default:
        id = page(currentId())->nextId();
        break;
    };

    return id;
}

// ****************************************************************************
// Method: QvisCinemaWizard::validateCurrentPage
//
// Purpose: 
//   This method is called before we go to the next page. If this method
//   returns false then the wizard won't go to the next page.
//
// Arguments:
//
// Returns:    True if the wizard can go to the next page.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 15:02:43 PDT 2017
//
// Modifications:
//   Kathleen Biagas, Thursday December 13, 2018
//   Check if file can be written.
// 
// ****************************************************************************

bool
QvisCinemaWizard::validateCurrentPage()
{
    bool valid = true;

    switch(currentId())
    {
    case Page_Main:
        valid = !cinemaAtts->GetFileName().empty() &&
                 cinemaAtts->GetWidth() > 1 &&
                 cinemaAtts->GetHeight() > 1;
        if(valid) // now check if filename is writeable
        {
            if(!QFileInfo(QFileInfo(cinemaAtts->GetFileName().c_str()).absolutePath()).isWritable())
            {
                Error(tr("File location is not writable."));
                valid = false;
            }
        }
        break;
    default:
        valid = page(currentId())->validatePage();
    }

    return valid;
}

// ****************************************************************************
// Method: QvisCinemaWizard::initializePage
//
// Purpose: 
//   This method is called when a page is shown and we call it so the page
//   gets updated with the right values from the cinemaAtts.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 15:02:43 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void
QvisCinemaWizard::initializePage(int pageId)
{
    bool tpEnabled;
    int cw,ch;

    switch(pageId)
    {
    case Page_Main:     
        page0_specification->blockSignals(true);
        if(cinemaAtts->GetSpecification() == "C")
            page0_specification->button(1)->setChecked(true);
        else if(cinemaAtts->GetSpecification() == "D")
            page0_specification->button(2)->setChecked(true);
        else //if(cinemaAtts->GetSpecification() == "A")
            page0_specification->button(0)->setChecked(true);
        page0_specification->blockSignals(false);

        page0_fileNameLineEdit->setText(cinemaAtts->GetFileName().c_str());

        page0_format->blockSignals(true);
        page0_format->setCurrentIndex(FormatToMenuIndex(cinemaAtts->GetFormat()));
        page0_format->blockSignals(false);

        page0_screenCapture->blockSignals(true);
        page0_screenCapture->setChecked(cinemaAtts->GetUseScreenCapture());
        page0_screenCapture->blockSignals(false);

        page0_composite->blockSignals(true);
        page0_composite->setChecked(cinemaAtts->GetComposite());
        page0_composite->blockSignals(false);

        cw = cinemaAtts->GetUseScreenCapture() ? default_image_size[0] : cinemaAtts->GetWidth();
        ch = cinemaAtts->GetUseScreenCapture() ? default_image_size[1] : cinemaAtts->GetHeight();
        page0_width->blockSignals(true);
        page0_width->setValue(cw);
        page0_width->blockSignals(false);
        page0_height->blockSignals(true);
        page0_height->setValue(ch);
        page0_height->blockSignals(false);

        page0_cameraMode->blockSignals(true);
        page0_cameraMode->setCurrentIndex(CameraToMenuIndex(cinemaAtts->GetCameraMode()));
        page0_cameraMode->blockSignals(false);

        page0_theta->blockSignals(true);
        page0_theta->setValue(cinemaAtts->GetTheta());
        page0_theta->blockSignals(false);

        page0_phi->blockSignals(true);
        page0_phi->setValue(cinemaAtts->GetPhi());
        page0_phi->blockSignals(false);

        // Set enabled state.
        page0_composite->setEnabled(cinemaAtts->GetSpecification() == "C");
        page0_screenCapture->setEnabled(cinemaAtts->GetSpecification()!="C" && !cinemaAtts->GetComposite());

        page0_heightLabel->setEnabled(!cinemaAtts->GetUseScreenCapture());
        page0_height->setEnabled(!cinemaAtts->GetUseScreenCapture());
        page0_widthLabel->setEnabled(!cinemaAtts->GetUseScreenCapture());
        page0_width->setEnabled(!cinemaAtts->GetUseScreenCapture());

        tpEnabled = cinemaAtts->GetCameraMode() != "static";
        page0_theta->setEnabled(tpEnabled);
        page0_phi->setEnabled(tpEnabled);
        page0_thetaLabel->setEnabled(tpEnabled);
        page0_phiLabel->setEnabled(tpEnabled);

        break;
    case Page_NumFrames:
        page1_frameStart->blockSignals(true);
        page1_frameStart->setValue(cinemaAtts->GetFrameStart());
        page1_frameStart->blockSignals(false);

        page1_frameEnd->blockSignals(true);
        page1_frameEnd->setValue(cinemaAtts->GetFrameEnd());
        page1_frameEnd->blockSignals(false);

        page1_frameStride->blockSignals(true);
        page1_frameStride->setValue(cinemaAtts->GetFrameStride());
        page1_frameStride->blockSignals(false);
        break;
    default:
        page(pageId)->initializePage();
        //qDebug("Update page: pageId=%d", pageId);
        break;
    }
}

// ****************************************************************************
// Method: QvisCinemaWizard::AddPath
//
// Purpose:
//   Prepends the name of the current directory to the Cinema attributes.
//
// Arguments:
//   filename : The filename, which might have a path.
//
// Returns:    The absolute path to the file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 15:52:48 PDT 2017
//
// Modifications:
//   Kathleen Biagas, Thursday December 13, 2018
//   Allow both style file-separators on Windows. Convert to linux-style
//   in final string, as it eases the burden of ensuring the correct escapes
//   when passing the strings around.
//
// ****************************************************************************

QString
QvisCinemaWizard::FullyQualified(const QString &filename) const
{
    QString path;

    // If the filename does not contain a path, then prepend one.
#if defined Q_OS_WIN
    if(filename.contains("/") || filename.contains("\\"))
#else
    if(filename.contains(VISIT_SLASH_STRING))
#endif
        path = filename;
    else
    {
#if defined(Q_OS_WIN)
        path = QString(GetUserVisItDirectory().c_str());
#else
        QDir d(QDir::currentPath());
        d.makeAbsolute();
        path = d.path();
#endif
        if(path.endsWith(VISIT_SLASH_STRING))
            path += filename;
        else
            path = path + QString(VISIT_SLASH_STRING) + filename;
    }

#if defined(Q_OS_WIN)
    // using linux style makes it easier
    path.replace("\\", "/");
#endif
    return path;
}

//
// Qt slot functions.
//

//
// Page 0 slots
//

void
QvisCinemaWizard::page0_specificationChanged(int val)
{
    std::string spec("A");
    if(val == 1)
        spec = "C";
    else if(val == 2)
        spec = "D";
    cinemaAtts->SetSpecification(spec);

    page0_composite->setEnabled(val == 1);
    // Turn off composite
    if(val != 1)
    {
        page0_compositeStateChanged(0);
        page0_composite->blockSignals(true);
        page0_composite->setChecked(cinemaAtts->GetComposite());
        page0_composite->blockSignals(false);
    }
}

// ****************************************************************************
// Modifications:
//   Kathleen Biagas, Thursday December 13, 2018
//   Check for 'cdb' extension. Get fully qualified name.
// ****************************************************************************
void
QvisCinemaWizard::page0_processFileName()
{
    QString fname = page0_fileNameLineEdit->text();
    if(!fname.endsWith(".cdb"))
        fname += ".cdb";
    QString full_name(FullyQualified(fname));
    page0_fileNameLineEdit->blockSignals(true);
    page0_fileNameLineEdit->setText(full_name);
    page0_fileNameLineEdit->blockSignals(false);
    cinemaAtts->SetFileName(full_name.toStdString());
}

// ****************************************************************************
// Method: QvisCinemaWizard::page0_selectFileName
//
// Purpose: 
//   This is a Qt slot function that is called when we want to select a new
//   destination directory for our output.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:36:04 PDT 2005
//
// Modifications:
//
// ****************************************************************************

void
QvisCinemaWizard::page0_selectFileName()
{
    //
    // Try and get a new filename using a file dialog.
    //
    QString initialFile(FullyQualified(cinemaAtts->GetFileName().c_str()));
    QString initialFileDir = QFileInfo(initialFile).dir().absolutePath();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                           initialFileDir,
                           tr("Cinema database (*.cdb)"));

    //
    // If a directory was chosen, use it as the output directory.
    //
    if(!fileName.isEmpty())
    {
        if(!fileName.endsWith(".cdb"))
            fileName += ".cdb";
        cinemaAtts->SetFileName(fileName.toStdString());
        page0_fileNameLineEdit->blockSignals(true);
        page0_fileNameLineEdit->setText(fileName);
        page0_fileNameLineEdit->blockSignals(false);
    }
}
void
QvisCinemaWizard::page0_formatActivated(int val)
{
    cinemaAtts->SetFormat(fileFormats[val]);
}

void
QvisCinemaWizard::page0_widthValueChanged(int val)
{
    cinemaAtts->SetWidth(val);
}

void
QvisCinemaWizard::page0_heightValueChanged(int val)
{
    cinemaAtts->SetHeight(val);
}

void
QvisCinemaWizard::page0_compositeStateChanged(int val)
{
    cinemaAtts->SetComposite(val > 0);

    page0_screenCapture->setEnabled(val == 0);
    // turn off screen capture.
    if(val > 0)
    {
        page0_screenCaptureStateChanged(0);
        page0_screenCapture->blockSignals(true);
        page0_screenCapture->setChecked(cinemaAtts->GetUseScreenCapture());
        page0_screenCapture->blockSignals(false);
    }
}

void
QvisCinemaWizard::page0_screenCaptureStateChanged(int val)
{
    cinemaAtts->SetUseScreenCapture(val > 0);
//cout << "val = " << val << endl;
    // If we're turning screen capture on then show the default/actual image size.
    int cw = cinemaAtts->GetUseScreenCapture() ? default_image_size[0] : cinemaAtts->GetWidth();
    int ch = cinemaAtts->GetUseScreenCapture() ? default_image_size[1] : cinemaAtts->GetHeight();
//cout << "Update wdith,height = " << cw << ", " << ch << endl;
    page0_width->blockSignals(true);
    page0_width->setValue(cw);
    page0_width->blockSignals(false);
    page0_height->blockSignals(true);
    page0_height->setValue(ch);
    page0_height->blockSignals(false);

    // Set enabled state.
    page0_heightLabel->setEnabled(!cinemaAtts->GetUseScreenCapture());
    page0_height->setEnabled(!cinemaAtts->GetUseScreenCapture());
    page0_widthLabel->setEnabled(!cinemaAtts->GetUseScreenCapture());
    page0_width->setEnabled(!cinemaAtts->GetUseScreenCapture());
}

void
QvisCinemaWizard::page0_cameraModeActivated(int val)
{
    cinemaAtts->SetCameraMode(cameraModes[val]);

    bool tpEnabled = cinemaAtts->GetCameraMode() != "static";
    page0_theta->setEnabled(tpEnabled);
    page0_phi->setEnabled(tpEnabled);
    page0_thetaLabel->setEnabled(tpEnabled);
    page0_phiLabel->setEnabled(tpEnabled);
}

void
QvisCinemaWizard::page0_thetaValueChanged(int val)
{
    cinemaAtts->SetTheta(val);
}

void
QvisCinemaWizard::page0_phiValueChanged(int val)
{
    cinemaAtts->SetPhi(val);
}

//
// Page 1 slots
//

void
QvisCinemaWizard::page1_frameStartValueChanged(int val)
{
    cinemaAtts->SetFrameStart(val);
}

void
QvisCinemaWizard::page1_frameEndValueChanged(int val)
{
    cinemaAtts->SetFrameEnd(val);
}

void
QvisCinemaWizard::page1_frameStrideValueChanged(int val)
{
    cinemaAtts->SetFrameStride(val);
}

