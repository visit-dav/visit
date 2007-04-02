#include <QvisSaveMovieWizard.h>
#include <MovieAttributes.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qframe.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtimer.h>

#include <visit-config.h>
#include <Utility.h>

#define movieAtts ((MovieAttributes *)localCopy)


//
// Wizard page information.
//
struct wizard_page_info
{
    const char *title;
    const char *description;
};

wizard_page_info pageInfo[] = {
    {"Movie type", "Would you like to create a new simple movie or use your previous settings?"},
    {"Pick template", "Pick a movie template."},
    {"Customize template", "Would you like to customize the movie template that you selected?"},
    {"Viewports", "Change the movie template's viewports. Viewports are regions of the final movie image to which VisIt's visualization windows can be mapped."},
    {"Mapping", "Map visualization windows to movie template viewports."},
    {"Special effects", "Set up special effects."},
    {"Save new template", "Save this movie as a new template?"},
    {"Save new template as", "Choose a name for your new template."},
    {"Settings check", "Do these movie settings look okay?"},
    {"Choose format", "Choose movie formats and resolutions."},
    {"Stereo", "Would you like to make a stereo movie?"},
    {"Choose filename", "Choose the output directory and base filename for your movie(s)."},
    {"Choose method", "Choose when and how you would like VisIt to create your movies."}
};

#define N_WIZARD_PAGES (sizeof(pageInfo) / sizeof(wizard_page_info))

//
// Movie format information.
//
#if !defined(_WIN32)
#define MPEG_FORMAT "MPEG movie"
#define TIFF_FORMAT "TIFF images"
#else
// Make TIFF images be the default on Windows for now.
#define MPEG_FORMAT "TIFF images"
#define TIFF_FORMAT "TIFF images"
#endif

struct movie_format_info
{
    const char *menu_name;
    const char *format;
};

movie_format_info movieFormatInfo[] = {
    {"BMP  images",     "bmp"},
    {"JPEG images",     "jpeg"},
    {"PNG  images",     "png"},
    {"PPM  images",     "ppm"},
    {"RGB  images",     "rgb"},
    {TIFF_FORMAT,       "tiff"}
#if !defined(_WIN32)
   ,{MPEG_FORMAT,       "mpeg"},
    {"Quicktime movie", "qt"},
    {"Streaming movie", "sm"}
#endif
};

#define N_MOVIE_FORMATS  (sizeof(movieFormatInfo) / sizeof(movie_format_info))

// ****************************************************************************
// Function: FormatToMenuName
//
// Purpose: 
//   Converts a movie format name to its menu name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:15:35 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

const char *
FormatToMenuName(const char *format)
{
    for(int i = 0; i < N_MOVIE_FORMATS; ++i)
    {
        if(strcmp(format, movieFormatInfo[i].format) == 0)
            return movieFormatInfo[i].menu_name;
    }

    return "";
}

// ****************************************************************************
// Function: MenuNameToFormat
//
// Purpose: 
//   Converts a menu name to its movie format name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:15:35 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

const char *
MenuNameToFormat(const char *menu_name)
{
    for(int i = 0; i < N_MOVIE_FORMATS; ++i)
    {
        if(strcmp(menu_name, movieFormatInfo[i].menu_name) == 0)
            return movieFormatInfo[i].format;      
    }

    return "";
}

// ****************************************************************************
// Class: QvisSaveMovieWizardListViewItem
//
// Purpose:
//   This class lets us store items into QListView without letting the QListView
//   mess up the order in which the items are displayed.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:16:27 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisSaveMovieWizardListViewItem : public QListViewItem
{
public:
    QvisSaveMovieWizardListViewItem(QListView *lv) : QListViewItem(lv)
    {
        keyIndex = staticKeyIndex++;
    }

    QvisSaveMovieWizardListViewItem(QListViewItem *item) : QListViewItem(item)
    {
        keyIndex = staticKeyIndex++;
    }

    virtual ~QvisSaveMovieWizardListViewItem()
    {
    }

    virtual QString key(int column, bool ascending) const
    {
        QString k; k.sprintf("%08d", keyIndex);
        return k;
    }
private:
    static int staticKeyIndex;
    int keyIndex;
};

int QvisSaveMovieWizardListViewItem::staticKeyIndex = 0;

// ****************************************************************************
// Method: QvisSaveMovieWizard::QvisSaveMovieWizard
//
// Purpose: 
//   This is the constructor for the QvisSaveMovieWizard class.
//
// Arguments:
//   atts   : A pointer to the attribute subject that the wizard will set.
//   parent : The wizard's parent.
//   name   : The name of the wizard.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:17:10 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Feb 2 18:49:33 PST 2006
//   Initialized default_movie_size.
//
// ****************************************************************************

QvisSaveMovieWizard::QvisSaveMovieWizard(AttributeSubject *atts, QWidget *parent, 
    const char *name) : QvisWizard(atts, parent, name)
{
    default_movie_size[0] = 400;
    default_movie_size[1] = 400;

    //
    // Decision variables that influence which path is taken through the wizard.
    //
    page0_usePreviousSettingsAllowed = (movieAtts->GetFileFormats().size() > 0);
    page1_createNewTemplate = false;
    page2_customizeTemplate = false;
    page6_saveTemplate = false;
    page8_settingsOkay = true;
    page9_aspect = 1.;

    connect(this, SIGNAL(selected(const QString &)),
            this, SLOT(pageChanged(const QString &)));

    // Set the wizard's title.
    topLevelWidget()->setCaption("Save movie wizard");

    CreateMovieTypePage();
    page1 = CreateSimplePage(1);
    CreateCustomizePage();
    page3 = CreateSimplePage(3);
    page4 = CreateSimplePage(4);
    page5 = CreateSimplePage(5);
    CreateSaveTemplatePage();
    page7 = CreateSimplePage(7);
    CreateSettingsOkayPage();
    CreateFormatPage();
    CreateStereoMoviePage();
    CreateFilenamePage();
    CreateGenerationMethodPage();

    // Hide the help button since we don't want it.
    if(helpButton() != 0)
        helpButton()->hide();
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::~QvisSaveMovieWizard
//
// Purpose: 
//   Destructor for the QvisSaveMovieWizard class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:18:08 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisSaveMovieWizard::~QvisSaveMovieWizard()
{
    delete page0_buttongroup;
    delete page9_sizeTypeButtonGroup;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::Exec
//
// Purpose: 
//   This method executes the wizard.
//
// Returns:    A return value for the wizard, which will be used to tell the
//             user what to do.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:18:32 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

int
QvisSaveMovieWizard::Exec()
{
    // Set the enabled state on the "use previous settings" radio button.
    page0_r1->setEnabled(page0_usePreviousSettingsAllowed);

    UpdatePageLinking(0);
    showPage(page(0));

    // Go through the wizard.
    int retval = exec();

    // If we've just successfully gone through the wizard for the first time,
    // enable the "use previous settings" radio button so we can use it
    // next time.
    if(!page0_usePreviousSettingsAllowed)
    {
        page0_usePreviousSettingsAllowed = (retval == QDialog::Accepted);
        page0_r1->setEnabled(page0_usePreviousSettingsAllowed);
    }

    // If we accepted some settings then make sure we think they're good
    // by default the next time we get to page 8.
    if(retval == QDialog::Accepted)
        page8_settingsOkay = true;

    return retval;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::keyPressEvent
//
// Purpose: 
//   This event handling method intercepts Enter and Return key presses for
//   the wizard and prevents QDialog::keyPressEvent from getting them in
//   certain cases because QDialog would make the active pushbutton fire, which
//   could be the "Next", "Finish", etc buttons and we don't want that.
//
// Arguments:
//   e : The keypress event to handle.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:19:28 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::keyPressEvent(QKeyEvent *e)
{
     if(e->state() == 0 || (e->state() & Keypad && e->key() == Key_Enter))
     {
         if(e->key() == Key_Enter || e->key() == Key_Return)
         {
             if((backButton() != 0 && backButton()->hasFocus()) ||
                (nextButton() != 0 && nextButton()->hasFocus()) ||
                (finishButton() != 0 && finishButton()->hasFocus()) ||
                (cancelButton() != 0 && cancelButton()->hasFocus()))
             {
                 QvisWizard::keyPressEvent(e);
             }
             else
             {
                 e->ignore();
             }
         }
         else
             QvisWizard::keyPressEvent(e);
     }
     else
         QvisWizard::keyPressEvent(e);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::SetDefaultMovieSize
//
// Purpose: 
//   Sets the default movie size.
//
// Arguments:
//   w : The movie width.
//   h : The movie height.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 2 18:52:54 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::SetDefaultMovieSize(int w, int h)
{
    default_movie_size[0] = w;
    default_movie_size[1] = h;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateMovieTypePage
//
// Purpose: 
//   Creates page0.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:13 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::CreateMovieTypePage()
{
    QFrame *frame = new QFrame(this, pageInfo[0].title);
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[0].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(pageLayout);
    hCenterLayout->addStretch(5);
    QVBoxLayout *buttonLayout = new QVBoxLayout(hCenterLayout);
    hCenterLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page0_buttongroup = new QButtonGroup(0, "page0_buttongroup");
    page0_r1 = new QRadioButton("Use my previous movie settings",
        frame, "r1");
    page0_r1->setEnabled(false);
    page0_buttongroup->insert(page0_r1, 0);
    buttonLayout->addWidget(page0_r1);

    QRadioButton *r2 = new QRadioButton("New simple movie", frame, "r2");
    page0_buttongroup->insert(r2, 1);
    buttonLayout->addWidget(r2);
#ifdef ALLOW_MOVIE_TEMPLATES
    QRadioButton *r3 = new QRadioButton("New movie using movie template",
        frame, "r3");
    page0_buttongroup->insert(r3, 2);
    buttonLayout->addWidget(r3);
#endif
    pageLayout->addSpacing(10);
    pageLayout->addStretch(10);
    page0_buttongroup->setButton(1);
    connect(page0_buttongroup, SIGNAL(clicked(int)),
            this, SLOT(page0_movieTypeChanged(int)));

    // Add the page.
    page0 = frame;
    addPage(page0, pageInfo[0].title);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateCustomizePage
//
// Purpose: 
//   Creates page 2, which is currently unused.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::CreateCustomizePage()
{
    CreateYesNoPage(2, &page2, &page2_buttongroup, SLOT(page2_customizeChanged(int)));
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateSaveTemplatePage
//
// Purpose: 
//   Creates page 6, which is currently unused.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::CreateSaveTemplatePage()
{
    CreateYesNoPage(6, &page6, &page6_buttongroup, SLOT(page6_saveAsTemplateChanged(int)));
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateSettingsOkayPage
//
// Purpose: 
//   Creates page 8.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::CreateSettingsOkayPage()
{
    QFrame *frame = new QFrame(this, pageInfo[8].title);
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[8].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *buttonLayout = new QHBoxLayout(pageLayout);
    buttonLayout->setSpacing(5);
    buttonLayout->addStretch(5);

    page8_buttongroup = new QButtonGroup(0, "page8_buttongroup");
    connect(page8_buttongroup, SIGNAL(clicked(int)),
            this, SLOT(page8_settingsOkayChanged(int)));

    QRadioButton *r1 = new QRadioButton("Yes",
        frame, "r1");
    page8_buttongroup->insert(r1);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton("No",
        frame, "r2");
    page8_buttongroup->insert(r2);
    buttonLayout->addWidget(r2);

    buttonLayout->addStretch(5);
    pageLayout->addSpacing(10);

    //
    // Create the output formats list.
    //
    QGroupBox *settingsGroup = new QGroupBox(frame, "settingsGroup");
    settingsGroup->setTitle("Movie settings");
    pageLayout->addWidget(settingsGroup, 10);
    QVBoxLayout *f3innerLayout = new QVBoxLayout(settingsGroup);
    f3innerLayout->setMargin(10);
    f3innerLayout->addSpacing(10);
    QVBoxLayout *f3layout = new QVBoxLayout(f3innerLayout);
    f3layout->setSpacing(5);

    page8_settingsListView = new QListView(settingsGroup,
        "page8_settingsListView");
    page8_settingsListView->addColumn("Settings");
    page8_settingsListView->addColumn("Values");
    page8_settingsListView->setAllColumnsShowFocus(true);
    page8_settingsListView->setColumnAlignment(0, Qt::AlignLeft);
    page8_settingsListView->setColumnAlignment(1, Qt::AlignLeft);
    f3layout->addWidget(page8_settingsListView);

    // Add the page.
    page8 = frame;
    addPage(page8, pageInfo[8].title);
    setFinishEnabled(page8, true);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateFormatPage
//
// Purpose: 
//   Creates page 9.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::CreateFormatPage()
{
    QFrame *frame = new QFrame(this, pageInfo[9].title);
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[9].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(pageLayout);

    //
    // Create the left frame, which contains the format and resolution options.
    //
    QGroupBox *formatAndResolution = new QGroupBox(frame, "formatAndResolutionFrame");
    formatAndResolution->setTitle("Format and resolution");
    hCenterLayout->addWidget(formatAndResolution, 10);
    QVBoxLayout *f2innerLayout = new QVBoxLayout(formatAndResolution);
    f2innerLayout->setMargin(10);
    f2innerLayout->addSpacing(10);
    QGridLayout *f2layout = new QGridLayout(f2innerLayout, 6, 3);
    f2layout->setSpacing(5);

    page9_formatComboBox = new QComboBox(formatAndResolution, "page9_formatComboBox");
    // Add all of the movie formats from the table.
    for(int i = 0; i < N_MOVIE_FORMATS; ++i)
        page9_formatComboBox->insertItem(movieFormatInfo[i].menu_name, i);
    page9_formatComboBox->setCurrentItem(6);
    QLabel *formatLabel = new QLabel(page9_formatComboBox, "Format",
        formatAndResolution, "formatLabel");
    f2layout->addMultiCellWidget(page9_formatComboBox, 0, 0, 1, 2);
    f2layout->addWidget(formatLabel, 0, 0);

    page9_sizeTypeButtonGroup = new QButtonGroup(0, "page9_sizeTypeButtonGroup");
    connect(page9_sizeTypeButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(page9_sizeTypeChanged(int)));
    QRadioButton *rb = new QRadioButton("Use current window size",
        formatAndResolution, "winSize");
    page9_sizeTypeButtonGroup->insert(rb, 0);
    f2layout->addMultiCellWidget(rb, 1, 1, 0, 2);

    page9_scaleSpinBox = new QSpinBox(1, 10, 1, formatAndResolution,
        "page9_scaleSpinBox");
    connect(page9_scaleSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(page9_scaleChanged(int)));
    page9_scaleLabel = new QLabel(page9_scaleSpinBox, "Scale",
        formatAndResolution, "scaleLabel");
    f2layout->addWidget(page9_scaleLabel, 2, 0);
    f2layout->addWidget(page9_scaleSpinBox, 2, 1);
    
    rb = new QRadioButton("Specify movie size", formatAndResolution, "winSize2");
    page9_sizeTypeButtonGroup->insert(rb, 1);
    f2layout->addMultiCellWidget(rb, 3, 3, 0, 2);

    page9_widthSpinBox = new QSpinBox(32, 4096, 1, formatAndResolution,
        "page9_widthSpinBox");
    page9_widthSpinBox->setValue(default_movie_size[0]);
    page9_widthSpinBox->setEnabled(false);
    connect(page9_widthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(page9_widthChanged(int)));
    page9_widthLabel = new QLabel(page9_widthSpinBox, "Width",
        formatAndResolution, "widthLabel");
    page9_widthLabel->setEnabled(false);
    f2layout->addWidget(page9_widthLabel, 4, 0);
    f2layout->addWidget(page9_widthSpinBox, 4, 1);
    
    page9_heightSpinBox = new QSpinBox(32, 4096, 1, formatAndResolution,
        "page9_heightSpinBox");
    page9_heightSpinBox->setValue(default_movie_size[1]);
    page9_heightSpinBox->setEnabled(false);
    connect(page9_heightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(page9_heightChanged(int)));
    page9_heightLabel = new QLabel(page9_widthSpinBox, "Height",
        formatAndResolution, "heightLabel");
    page9_heightLabel->setEnabled(false);
    f2layout->addWidget(page9_heightLabel, 5, 0);
    f2layout->addWidget(page9_heightSpinBox, 5, 1);

    page9_aspectLock = new QCheckBox("lock aspect",
        formatAndResolution, "page9_aspectLock");
    page9_aspectLock->setChecked(true);
    page9_aspectLock->setEnabled(false);
    connect(page9_aspectLock, SIGNAL(toggled(bool)),
            this, SLOT(page9_aspectLockChanged(bool)));
    f2layout->addMultiCellWidget(page9_aspectLock, 4,5,2,2);

    //
    // Create the ->, <- buttons
    //
    QVBoxLayout *centerButtonLayout = new QVBoxLayout(hCenterLayout);
    centerButtonLayout->setSpacing(5);
    centerButtonLayout->addStretch(5);
    page9_addOutputButton = new QPushButton("->", frame, "page9_addOutputButton");
    connect(page9_addOutputButton, SIGNAL(clicked()),
            this, SLOT(page9_addOutput()));
    centerButtonLayout->addWidget(page9_addOutputButton);
    page9_removeOutputButton = new QPushButton("<-", frame, "page9_removeOutputButton");
    connect(page9_removeOutputButton, SIGNAL(clicked()),
            this, SLOT(page9_removeOutput()));
    centerButtonLayout->addWidget(page9_removeOutputButton);
    centerButtonLayout->addStretch(5);


    //
    // Create the output formats list.
    //
    QGroupBox *outputGroup = new QGroupBox(frame, "outputListGroup");
    outputGroup->setTitle("Output");
    hCenterLayout->addWidget(outputGroup, 10);
    QVBoxLayout *f3innerLayout = new QVBoxLayout(outputGroup);
    f3innerLayout->setMargin(10);
    f3innerLayout->addSpacing(10);
    QVBoxLayout *f3layout = new QVBoxLayout(f3innerLayout);
    f3layout->setSpacing(5);

    page9_outputFormats = new QListView(outputGroup, "page9_outputFormats");
    page9_outputFormats->setMinimumWidth(fontMetrics().
        boundingRect("Quicktime movie 2000x2000").width());
    page9_outputFormats->addColumn("Format");
    page9_outputFormats->addColumn("Resolution");
    page9_outputFormats->setAllColumnsShowFocus(true);
    page9_outputFormats->setColumnAlignment(0, Qt::AlignHCenter);
    page9_outputFormats->setColumnAlignment(1, Qt::AlignHCenter);
    f3layout->addWidget(page9_outputFormats);

    // Add the page.
    page9 = frame;
    addPage(page9, pageInfo[9].title);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateStereoMoviePage
//
// Purpose: 
//   Creates page 10.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::CreateStereoMoviePage()
{
    CreateYesNoPage(10, &page10, &page10_buttongroup, SLOT(page10_stereoChanged(int)));
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateFilenamePage
//
// Purpose: 
//   Creates page 11.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Jul 14 13:51:20 PST 2005
//   Prevented the "..." button width from being set on MacOS X.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateFilenamePage()
{
    QFrame *frame = new QFrame(this, pageInfo[11].title);
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[11].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QGridLayout *gLayout = new QGridLayout(pageLayout, 2, 2);
    gLayout->setSpacing(5);
    pageLayout->addStretch(20);

    //
    // Create the output directory selection controls.
    //
    QLabel *outputDirectoryLabel = new QLabel("Output directory",
        frame, "outputDirectoryLabel");
    gLayout->addWidget(outputDirectoryLabel, 0, 0);

    QHBox *outputDirectoryParent = new QHBox(frame, "outputDirectoryParent");    
    page11_outputDirectoryLineEdit = new QLineEdit(outputDirectoryParent,
        "outputDirectoryLineEdit");
    connect(page11_outputDirectoryLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page11_processOutputDirectoryText(const QString &)));
    QPushButton *outputSelectButton = new QPushButton("...", outputDirectoryParent,
        "outputSelectButton");
#ifndef Q_WS_MACX
    outputSelectButton->setMaximumWidth(
         fontMetrics().boundingRect("...").width() + 6);
#endif
    outputSelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
         QSizePolicy::Minimum));
    connect(outputSelectButton, SIGNAL(clicked()),
            this, SLOT(page11_selectOutputDirectory()));
    outputDirectoryParent->setSpacing(0);
    outputDirectoryParent->setStretchFactor(page11_outputDirectoryLineEdit, 100);
    outputDirectoryLabel->setBuddy(outputDirectoryParent);
    gLayout->addWidget(outputDirectoryParent, 0, 1);

    //
    // Create the controls for selecting the file base.
    //
    QLabel *filebaseLabel = new QLabel("Base filename",
        frame, "filebaseLabel");
    gLayout->addWidget(filebaseLabel, 1, 0);
    page11_filebaseLineEdit = new QLineEdit(frame, "page11_filebaseLineEdit");
    connect(page11_filebaseLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page11_processFilebaseText(const QString &)));    
    gLayout->addWidget(page11_filebaseLineEdit, 1, 1);

    // Add the page.
    page11 = frame;
    addPage(page11, pageInfo[11].title);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateGenerationMethodPage
//
// Purpose: 
//   Creates page 12.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::CreateGenerationMethodPage()
{
    QFrame *frame = new QFrame(this, pageInfo[12].title);
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[12].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(pageLayout);
    hCenterLayout->addStretch(5);
    QVBoxLayout *buttonLayout = new QVBoxLayout(hCenterLayout);
    hCenterLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page12_buttongroup = new QButtonGroup(0, "page4_buttongroup");
    connect(page12_buttongroup, SIGNAL(clicked(int)),
            this, SLOT(page12_generationMethodChanged(int)));
    QRadioButton *r1 = new QRadioButton("Now, use currently allocated processors",
        frame, "r1");
    page12_buttongroup->insert(r1);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton("Now, use a new instance of VisIt",
        frame, "r2");
    page12_buttongroup->insert(r2);
    buttonLayout->addWidget(r2);

    QRadioButton *r3 = new QRadioButton("Later, tell me the command to run",
        frame, "r3");
    page12_buttongroup->insert(r3);
    buttonLayout->addWidget(r3);
    pageLayout->addSpacing(10);
    pageLayout->addStretch(10);

    // Add the page.
    page12 = frame;
    addPage(page12, pageInfo[12].title);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateYesNoPage
//
// Purpose: 
//   Creates a page with a prompt and Yes/No radio buttons.
//
// Arguments:
//   pageIndex : The index of the page to create.
//   page      : The return pointer for the created page.
//   bg        : The return pointer for the created button group.
//   slot      : The Qt slot to connect.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::CreateYesNoPage(int pageIndex, QWidget **page,
    QButtonGroup **bg, const char *slot)
{
    QFrame *frame = new QFrame(this, pageInfo[pageIndex].title);
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[pageIndex].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *buttonLayout = new QHBoxLayout(pageLayout);
    buttonLayout->setSpacing(5);
    buttonLayout->addStretch(5);

    QString n; n.sprintf("page%d_buttongroup", pageIndex);
    QButtonGroup *buttonGroup = new QButtonGroup(0, n.latin1());
    connect(buttonGroup, SIGNAL(clicked(int)),
            this, slot);

    QRadioButton *r1 = new QRadioButton("Yes",
        frame, "r1");
    buttonGroup->insert(r1);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton("No",
        frame, "r2");
    buttonGroup->insert(r2);
    buttonLayout->addWidget(r2);

    buttonLayout->addStretch(5);
    pageLayout->addSpacing(10);
    pageLayout->addStretch(10);

    // Add the page.
    *page = frame;
    *bg = buttonGroup;
    addPage(*page, pageInfo[pageIndex].title);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateSimplePage
//
// Purpose: 
//   Creates a simple page with just a prompt.
//
// Arguments:
//   i : The index of the page to create.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisSaveMovieWizard::CreateSimplePage(int i)
{
    QLabel *w = new QLabel(SplitPrompt(pageInfo[i].description), this, pageInfo[i].title);
    addPage(w, pageInfo[i].title);
    return w;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::SplitPrompt
//
// Purpose: 
//   Splits a prompt into words and reassembles it into a paragraph of text 
//   where each line has a max length.
//
// Arguments:
//   s : The string to split.
//
// Returns:    The processed string.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:26:59 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QString
QvisSaveMovieWizard::SplitPrompt(const QString &s) const
{
    static const int MAX_PROMPT_LENGTH = 60;
    if(s.length() < MAX_PROMPT_LENGTH)
        return s;
    else
    {
        stringVector words(SplitValues(std::string(s.latin1()), ' '));
        QString r;
        int len = 0;
        for(int i = 0; i < words.size(); ++i)
        {
            if(len > 0)
                r += " ";
            r += QString(words[i].c_str());
            len += words[i].size();
            if(len >= MAX_PROMPT_LENGTH)
            {
                r += "\n";
                len = 0;
            }
        }
        return r;
    }
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::UpdatePage
//
// Purpose: 
//   This method is called when a page is shown and we call it so the page
//   gets updated with the right values from the movieAtts.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:29:19 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul 11 08:59:19 PDT 2005
//   Fixed the output directory on Windows.
//
//   Brad Whitlock, Thu Jul 14 14:24:22 PST 2005
//   Disabled the Next button on Windows and MacOS X.
//
//   Brad Whitlock, Thu Feb 2 18:50:54 PST 2006
//   I made it use the default_movie_size members instead of hard-coding
//   values.
//
// ****************************************************************************

void
QvisSaveMovieWizard::UpdatePage()
{
    int pageIndex = -1;
    for(int i = 0; i < pageCount(); ++i)
    {
        if(currentPage() == page(i))
        {
            pageIndex = i;
            break;
        }
    }

    switch(pageIndex)
    {
    case 0:
        //qDebug("Update the movie type page");
        // Initialize page0's button group with the current movie type.
        page0_buttongroup->blockSignals(true);
        {
        int btn = int(movieAtts->GetMovieType());
        // Prevent the "use previous settings button if we have not gone
        // through the wizard before.
        if(!page0_usePreviousSettingsAllowed && btn == 0)
            btn = 1;
#ifdef ALLOW_MOVIE_TEMPLATES
        if(btn > 1) btn = 1;
#endif
        page0_buttongroup->setButton(btn);
        }
        page0_buttongroup->blockSignals(false);
        break;
    case 2:
        //qDebug("Update the customize template page");
        // Initialize page6's button group with whether or not we want to
        // customize the template.
        page2_buttongroup->blockSignals(true);
        page2_buttongroup->setButton(page2_customizeTemplate?0:1);
        page2_buttongroup->blockSignals(false);
        break;
    case 6:
        //qDebug("Update the save template page");
        // Initialize page6's button group with whether or not we want to
        // save the modified template as a new template.
        page6_buttongroup->blockSignals(true);
        page6_buttongroup->setButton(page6_saveTemplate?0:1);
        page6_buttongroup->blockSignals(false);
        break;
    case 8:
        //qDebug("Update the settings okay page");
        // Initialize page8's button group with whether or not we think the
        // current movie settings look okay.
        page8_buttongroup->blockSignals(true);
        page8_buttongroup->setButton(page8_settingsOkay?0:1);
        page8_buttongroup->blockSignals(false);
        page8_UpdateMovieSettings();
        QTimer::singleShot(400, this, SLOT(page8_delayedUpdate()));
        break;
    case 9:
        //qDebug("Update the movie formats page.");
        page9_UpdateOutputs();
        if(movieAtts->GetFileFormats().size() > 0)
        {
            // Try for the first format but default to TIFF otherwise.
            const stringVector &formats = movieAtts->GetFileFormats();
            const intVector &w = movieAtts->GetWidths();
            const intVector &h = movieAtts->GetHeights();
            const unsignedCharVector &useCurrent = movieAtts->GetUseCurrentSize();
            const doubleVector &scales = movieAtts->GetScales();

            if(!page9_UpdateFormat(FormatToMenuName(formats[0].c_str())))
                page9_UpdateFormat(TIFF_FORMAT);

            page9_UpdateResolution(useCurrent[0]>0, scales[0], w[0], h[0]);
        }
        else
        {
            // Try for MPEG but default to TIFF otherwise.
            if(!page9_UpdateFormat(MPEG_FORMAT))
                page9_UpdateFormat(TIFF_FORMAT);

            page9_UpdateResolution(true, 1., default_movie_size[0],
                                   default_movie_size[1]);
        }
    case 10: // stereo
        //qDebug("Update the stereo page.");
        page10_buttongroup->blockSignals(true);
        page10_buttongroup->setButton(movieAtts->GetStereo()?0:1);
        page10_buttongroup->blockSignals(false);
        break;
    case 11: // filename, output dir.
        // If the path is set to the current directory then use Qt to expand
        // the path so we have the whole path.
        if(movieAtts->GetOutputDirectory() == ".")
        {
            std::string outDir(QDir::currentDirPath().latin1());
#if defined(_WIN32)
            for(int c = 0; c < outDir.length(); ++c)
                outDir[c] = (outDir[c] == '/') ? SLASH_CHAR : outDir[c];
#endif
            if(outDir.size() > 0 && outDir[outDir.size() - 1] != SLASH_CHAR)
                outDir += SLASH_STRING;
            movieAtts->SetOutputDirectory(outDir);
        }
        //qDebug("Update the filename, output dir page.");
        page11_outputDirectoryLineEdit->blockSignals(true);
        page11_outputDirectoryLineEdit->setText(movieAtts->
            GetOutputDirectory().c_str());
        page11_outputDirectoryLineEdit->blockSignals(false);
        page11_filebaseLineEdit->blockSignals(true);
        page11_filebaseLineEdit->setText(movieAtts->
            GetOutputName().c_str());
        page11_filebaseLineEdit->blockSignals(false);
        page11_UpdateButtons();
        break;
    case 12:
        //qDebug("Update the generation method page.");
        // Initialize page4's button group with the current movie 
        // generation strategy.
        page12_buttongroup->blockSignals(true);
        if(movieAtts->GetGenerationMethod() == MovieAttributes::NowCurrentInstance)
            page12_buttongroup->setButton(0);
        else if(movieAtts->GetGenerationMethod() == MovieAttributes::NowNewInstance)
            page12_buttongroup->setButton(1);
        else
            page12_buttongroup->setButton(2);
        page12_buttongroup->blockSignals(false);
#if defined(Q_WS_WIN) || defined(Q_WS_MACX)
        nextButton()->setEnabled(false);
#endif
        break;
    default:
        ;//qDebug("Update page: pageIndex=%d", pageIndex);
    }

    // Enable the finish button on page 12.
    finishButton()->setEnabled(pageIndex == 12);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page8_UpdateMovieSettings
//
// Purpose: 
//   This method updates the movie settings shown on page 8.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:30:48 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page8_UpdateMovieSettings()
{
    page8_settingsListView->clear();

    QListViewItem *item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, "Generation method");
    if(movieAtts->GetGenerationMethod() == MovieAttributes::NowCurrentInstance)
        item->setText(1, "Now, use currently allocated processors");
    else if(movieAtts->GetGenerationMethod() == MovieAttributes::NowNewInstance)
        item->setText(1, "Now, use a new instance of VisIt");
    else
        item->setText(1, "Later, tell me the command to run");

#if 0
    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, "Movie type");
    if(movieAtts->GetMovieType() == MovieAttributes::UseLastSettings)
        item->setText(1, "-");
    else if(movieAtts->GetMovieType() == MovieAttributes::NewSimple)
        item->setText(1, "New simple movie");
    else
        item->setText(1, "New movie using movie template");

    if(movieAtts->GetMovieType() == MovieAttributes::NewUsingTemplate)
    {
        item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
        item->setText(0, "Movie template");
        item->setText(1, movieAtts->GetTemplateFile().c_str());
    }
#endif

    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, "Output directory");
    item->setText(1, movieAtts->GetOutputDirectory().c_str());

    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, "Base filename");
    item->setText(1, movieAtts->GetOutputName().c_str());

    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, "Formats");
    const stringVector &formats = movieAtts->GetFileFormats();
    const intVector &w = movieAtts->GetWidths();
    const intVector &h = movieAtts->GetHeights();
    const unsignedCharVector &useCurrent = movieAtts->GetUseCurrentSize();
    const doubleVector &scales = movieAtts->GetScales();
    QString s;
    for(int i = 0; i < formats.size(); ++i)
    {
        QString tmp;
        if(useCurrent[i] > 0)
        {
            tmp.sprintf("%s Current %dx", 
                FormatToMenuName(formats[i].c_str()), int(scales[i]));
        }
        else
        {
            tmp.sprintf("%s %dx%d", 
                FormatToMenuName(formats[i].c_str()), w[i], h[i]);
        }
        s += tmp;
        if(i < formats.size() - 1)
            s += ", ";
    }
    item->setText(1, s);

    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, "Stereo");
    item->setText(1, movieAtts->GetStereo()?"yes":"no");
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page9_UpdateOutputs
//
// Purpose: 
//   This method populates the list of movie output formats on page 9.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:31:14 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page9_UpdateOutputs()
{
    const stringVector &formats = movieAtts->GetFileFormats();

    page9_outputFormats->blockSignals(true);
    page9_outputFormats->clear();
    if(formats.size() > 0)
    {
        // Try for the first format but default to TIFF otherwise.
        const intVector    &w = movieAtts->GetWidths();
        const intVector    &h = movieAtts->GetHeights();
        const doubleVector &scales = movieAtts->GetScales();
        const unsignedCharVector  &useCurrent = movieAtts->GetUseCurrentSize();

        for(int i = 0; i < formats.size(); ++i)
        {
            QString res;
            if(useCurrent[i] > 0)
                res.sprintf("Current %dx", int(scales[i]));
            else
                res.sprintf("%dx%d", w[i], h[i]);
            QListViewItem *item = new QvisSaveMovieWizardListViewItem(page9_outputFormats);
            item->setText(0, FormatToMenuName(formats[i].c_str()));
            item->setText(1, res);
            bool isLast = (i == formats.size()-1);
            item->setSelected(isLast);
            if(isLast)
                page9_outputFormats->setCurrentItem(item);
        }

        nextButton()->setEnabled(true);
    }
    else
        nextButton()->setEnabled(false);
    page9_outputFormats->blockSignals(false);
    page9_removeOutputButton->setEnabled(formats.size() > 0);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page9_UpdateFormat
//
// Purpose: 
//   This method sets the movie format combobox on page 9.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:32:03 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSaveMovieWizard::page9_UpdateFormat(const QString &format)
{
    bool setVal = false;
    for(int i = 0; i < page9_formatComboBox->count(); ++i)
    {
        if(page9_formatComboBox->text(i) == format)
        {  
            page9_formatComboBox->blockSignals(true);
            page9_formatComboBox->setCurrentItem(i);
            page9_formatComboBox->blockSignals(false);
            setVal = true;
            break;
        }
    }

    return setVal;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page9_UpdateResolution
//
// Purpose: 
//   This method sets the resolution widgets on page 9.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:32:29 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page9_UpdateResolution(bool useCurrent, double scale, int w, int h)
{
    page9_sizeTypeButtonGroup->blockSignals(true);
    int btn = useCurrent?0:1;
    page9_sizeTypeButtonGroup->setButton(btn);
    page9_sizeTypeChanged(btn);
    page9_sizeTypeButtonGroup->blockSignals(false);

    page9_scaleSpinBox->blockSignals(true);
    page9_scaleSpinBox->setValue(int(scale));
    page9_scaleSpinBox->blockSignals(false);

    page9_widthSpinBox->blockSignals(true);
    page9_widthSpinBox->setValue(w);
    page9_widthSpinBox->blockSignals(false);

    page9_heightSpinBox->blockSignals(true);
    page9_heightSpinBox->setValue(h);
    page9_heightSpinBox->blockSignals(false);

    if(page9_aspectLock->isChecked())
        page9_aspect = float(w) / float(h);
    else
        page9_aspect = 1.;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page11_UpdateButtons
//
// Purpose: 
//   This method sets the enabled state for the next button when we're on
//   page 11.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:32:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page11_UpdateButtons()
{
    bool e = (movieAtts->GetOutputName().size() > 0) &&
             (movieAtts->GetOutputDirectory().size() > 0);
    nextButton()->setEnabled(e);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::UpdatePageLinking
//
// Purpose: 
//   This method updates the list of appropriate pages, based on the movieAtts,
//   so the next page of the wizard will always be the right page.
//
// Arguments:
//   pageIndex : The page that we're going to .
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:33:15 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::UpdatePageLinking(int pageIndex)
{
    if(pageIndex == 0)
    {
        if(movieAtts->GetMovieType() == MovieAttributes::UseLastSettings)
        {
            setAppropriate(page1, false);
            setAppropriate(page2, false);
            setAppropriate(page3, false);
            setAppropriate(page4, false);
            setAppropriate(page5, false);
            setAppropriate(page6, false);
            setAppropriate(page7, false);
            setAppropriate(page8, true);
        }
        else if(movieAtts->GetMovieType() == MovieAttributes::NewSimple)
        {
            setAppropriate(page1, false);
            setAppropriate(page2, false);
            setAppropriate(page3, false);
            setAppropriate(page4, false);
            setAppropriate(page5, false);
            setAppropriate(page6, false);
            setAppropriate(page7, false);
            setAppropriate(page8, false);
            setAppropriate(page9, true);
        }
        else
            setAppropriate(page1, true);
    }
    else if(pageIndex == 1)
    {
        if(page1_createNewTemplate)
        {
            setAppropriate(page2, false);
            setAppropriate(page3, true);
        }
        else
            setAppropriate(page2, true);
    }
    else if(pageIndex == 2)
    {
        if(page2_customizeTemplate)
            setAppropriate(page3, true);
        else
        {
            setAppropriate(page3, false);
            setAppropriate(page4, false);
            setAppropriate(page5, false);
            setAppropriate(page6, false);
            setAppropriate(page7, false);
            setAppropriate(page8, false);
            setAppropriate(page9, true);
        }
    }
    else if(pageIndex == 3)
    {
        setAppropriate(page4, true);
    }
    else if(pageIndex == 4)
    {
        setAppropriate(page5, true);
    }
    else if(pageIndex == 5)
    {
        setAppropriate(page6, true);
    }
    else if(pageIndex == 6)
    {
        if(page6_saveTemplate)
        {
            setAppropriate(page7, false);
            setAppropriate(page8, false);
            setAppropriate(page9, true);
        }
        else
            setAppropriate(page7, true);
    }
    else if(pageIndex == 7)
    {
        setAppropriate(page8, false);
        setAppropriate(page9, true);
    }
    else if(pageIndex == 8)
    {
        if(!page8_settingsOkay)
            setAppropriate(page9, true);
        else
        {
            // This should have a finish button.
            finishButton()->setEnabled(page8_settingsOkay);
        }
    }
    else if(pageIndex == 9)
    {
        setAppropriate(page10, true);
    }
    else if(pageIndex == 11)
    {
        setAppropriate(page12, true);
    }
    else if(pageIndex == 12)
    {
        // This should have a finish button.
    }
}

//
// Qt slot functions.
//

void
QvisSaveMovieWizard::pageChanged(const QString &title)
{
    // Update the page linking so the wizard knows how to go to the next page.
    for(int i = 0; i < pageCount(); ++i)
    {
        if(QString(page(i)->name()) == QString(pageInfo[i].title))
        {
            UpdatePageLinking(i);
            break;
        }
    }

    UpdatePage();
}

void
QvisSaveMovieWizard::page0_movieTypeChanged(int val)
{
    movieAtts->SetMovieType((MovieAttributes::MovieTypeEnum)val);
    UpdatePageLinking(0);
}

void
QvisSaveMovieWizard::page2_customizeChanged(int val)
{
    page2_customizeTemplate = (val == 0);
    UpdatePageLinking(2);
}

void
QvisSaveMovieWizard::page6_saveAsTemplateChanged(int val)
{
    page6_saveTemplate = (val == 0);
    UpdatePageLinking(6);
}

void
QvisSaveMovieWizard::page8_settingsOkayChanged(int val)
{
    page8_settingsOkay = (val == 0);
    UpdatePageLinking(8);

    nextButton()->setEnabled(!page8_settingsOkay);
    finishButton()->setEnabled(page8_settingsOkay);
}

void
QvisSaveMovieWizard::page8_delayedUpdate()
{
    page8_settingsOkayChanged(page8_settingsOkay?0:1);
}

void
QvisSaveMovieWizard::page9_formatChanged(int)
{
}

void
QvisSaveMovieWizard::page9_sizeTypeChanged(int val)
{
    page9_scaleSpinBox->setEnabled(val==0);
    page9_scaleLabel->setEnabled(val==0);

    page9_widthSpinBox->setEnabled(val==1);
    page9_widthLabel->setEnabled(val==1);
    page9_heightSpinBox->setEnabled(val==1);
    page9_heightLabel->setEnabled(val==1);
    page9_aspectLock->setEnabled(val==1);

    if(val == 0)
        page9_scaleSpinBox->setFocus();
    else
        page9_widthSpinBox->setFocus();
}

void
QvisSaveMovieWizard::page9_scaleChanged(int val)
{
    //qDebug("page9_scaleChanged: scale=%g", val);
}

void
QvisSaveMovieWizard::page9_widthChanged(int val)
{
    if(page9_aspectLock->isChecked())
    {
        int newHeight = int((1. / page9_aspect) * val);
        page9_heightSpinBox->blockSignals(true);
        page9_heightSpinBox->setValue(newHeight);
        page9_heightSpinBox->blockSignals(false);
    }
}

void
QvisSaveMovieWizard::page9_heightChanged(int val)
{
    if(page9_aspectLock->isChecked())
    {
        int newWidth = int(page9_aspect * val);
        page9_widthSpinBox->blockSignals(true);
        page9_widthSpinBox->setValue(newWidth);
        page9_widthSpinBox->blockSignals(false);
    }
}

void
QvisSaveMovieWizard::page9_aspectLockChanged(bool val)
{
    if(val)
    {
        page9_aspect = float(page9_widthSpinBox->value()) /
                       float(page9_heightSpinBox->value());
    }
    else
        page9_aspect = 1.;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page9_addOutput
//
// Purpose: 
//   This is a Qt slot function that is called when the "->" button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:34:47 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page9_addOutput()
{
    // Determine the movie format from the menu name.
    QString menuName(page9_formatComboBox->currentText());
    std::string format(MenuNameToFormat(menuName.latin1()));
    int w = page9_widthSpinBox->value();
    int h = page9_heightSpinBox->value();
    double scale = double(page9_scaleSpinBox->value());
    int id = page9_sizeTypeButtonGroup->id(page9_sizeTypeButtonGroup->selected());
    bool useCurrent = id == 0;
    
    //
    // Add the format and resolution to the movie attributes.
    //
    movieAtts->GetFileFormats().push_back(format);
    movieAtts->SelectFileFormats();

    movieAtts->GetWidths().push_back(w);
    movieAtts->SelectWidths();

    movieAtts->GetHeights().push_back(h);
    movieAtts->SelectHeights();

    movieAtts->GetScales().push_back(scale);
    movieAtts->SelectScales();

    movieAtts->GetUseCurrentSize().push_back(useCurrent?1:0);
    movieAtts->SelectUseCurrentSize();

    //qDebug("Adding movie: format = %s, useCurrent=%s, scale=%g, w=%d, h=%d",
    //       format.c_str(), (useCurrent?"true":"false"), scale, w, h);

    page9_UpdateOutputs();
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page9_removeOutput
//
// Purpose: 
//   This is a Qt slot function that is called when the "<-" button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:34:47 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page9_removeOutput()
{
    QListViewItem *item = page9_outputFormats->selectedItem();
    if(item != 0)
    {
        // Remove the selected value from the movie atts.
        stringVector &formats = movieAtts->GetFileFormats();
        unsignedCharVector &useCurrents = movieAtts->GetUseCurrentSize();
        doubleVector &scales = movieAtts->GetScales();
        intVector &widths = movieAtts->GetWidths();
        intVector &heights = movieAtts->GetHeights();

        int i, w, h;
        bool  useCurrent;
        float scale;
        bool  deleted = false;
        for(i = 0; i < formats.size(); ++i)
        {
            QString fmt(FormatToMenuName(formats[i].c_str()));
            QString res;  res.sprintf("%dx%d", widths[i], heights[i]);
            QString res2; res2.sprintf("Current %dx", int(scales[i]));
            if(fmt == item->text(0) && 
               (res == item->text(1) || res2 == item->text(1)))
            {
                w = widths[i];
                h = heights[i];
                scale = (float)scales[i];
                useCurrent = useCurrents[i]>0;
                stringVector::iterator si = formats.begin();
                intVector::iterator    ii1 = widths.begin();
                intVector::iterator    ii2 = heights.begin();
                doubleVector::iterator ii3 = scales.begin();
                unsignedCharVector::iterator  ii4 = useCurrents.begin();
                for(int j = 0; j < i; ++j)
                { ++si; ++ii1; ++ii2; ++ii3; ++ii4; }

                formats.erase(si);
                widths.erase(ii1);
                heights.erase(ii2);
                scales.erase(ii3);
                useCurrents.erase(ii4);

                movieAtts->SelectFileFormats();
                movieAtts->SelectWidths();
                movieAtts->SelectHeights();
                movieAtts->SelectScales();
                movieAtts->SelectUseCurrentSize();

                deleted = true;
                break;
            }
        }

        if(deleted)
        {
            // Populate the format and resolution widgets.
            page9_UpdateFormat(item->text(0));
            page9_UpdateResolution(useCurrent, scale, w, h);
            page9_UpdateOutputs();
        }
    }
}

void
QvisSaveMovieWizard::page10_stereoChanged(int val)
{
    movieAtts->SetStereo(val == 0);
}


void
QvisSaveMovieWizard::page11_processOutputDirectoryText(const QString &s)
{
    std::string outDir(s.latin1());
    if(outDir.size() > 0 && outDir[outDir.size() - 1] != SLASH_CHAR)
        outDir += SLASH_STRING;
    movieAtts->SetOutputDirectory(outDir);
    page11_UpdateButtons();
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page11_selectOutputDirectory
//
// Purpose: 
//   This is a Qt slot function that is called when we want to select a new
//   destination directory for our movie.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:36:04 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page11_selectOutputDirectory()
{
    //
    // Try and get a directory using a file dialog.
    //
    QString initialDir(movieAtts->GetOutputDirectory().c_str());
    QString dirName = QFileDialog::getExistingDirectory(initialDir, this,
        "getDirectoryDialog", "Select output directory");

    //
    // If a directory was chosen, use it as the output directory.
    //
    if(!dirName.isEmpty())
    {
        std::string outDir(dirName.latin1());
        if(outDir.size() > 0 && outDir[outDir.size() - 1] != SLASH_CHAR)
            outDir += SLASH_STRING;

        movieAtts->SetOutputDirectory(outDir);
        page11_outputDirectoryLineEdit->blockSignals(true);
        page11_outputDirectoryLineEdit->setText(outDir.c_str());
        page11_outputDirectoryLineEdit->blockSignals(false);
    }
}

void
QvisSaveMovieWizard::page11_processFilebaseText(const QString &s)
{
    movieAtts->SetOutputName(s.latin1());
    page11_UpdateButtons();
}

void
QvisSaveMovieWizard::page12_generationMethodChanged(int val)
{
    movieAtts->SetGenerationMethod((MovieAttributes::GenerationMethodEnum)val);
}
