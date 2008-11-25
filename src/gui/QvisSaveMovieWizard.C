/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <QvisSaveMovieWizard.h>
#include <MovieAttributes.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTextEdit>
#include <QTimer>
#include <QToolTip>
#include <QTreeWidget>
#include <QWidget>

#include <QvisColorButton.h>
#include <QvisDialogLineEdit.h>
#include <QvisOpacitySlider.h>
#include <QvisPredefinedViewports.h>
#include <QvisScreenPositionEdit.h>
#include <QvisSequenceButton.h>
#include <QvisSequenceView.h>
#include <QvisSessionSourceChanger.h>
#include <QvisViewportWidget.h>

#include <ViewerProxy.h>

#include <visit-config.h>
#include <Utility.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <MovieSequence.h>
#include <MovieTemplateConfig.h>
#include <AccessViewerSession.h>
#include <snprintf.h>

#define movieAtts ((MovieAttributes *)localCopy)

#define MOVIE_TYPE_LAST_SETTINGS 0
#define MOVIE_TYPE_SIMPLE        1
#define MOVIE_TYPE_TEMPLATE      2

//
// Movie format information.
//
#define MPEG_FORMAT "MPEG movie"
#define TIFF_FORMAT "TIFF images"

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
    {TIFF_FORMAT,       "tiff"},
    {MPEG_FORMAT,       "mpeg"},
    {"Streaming movie", "sm"}
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
// Method: EnsureDirectoryExists
//
// Purpose: 
//   Makes sure that a directory exists.
//
// Arguments:
//   name       : The directory to make sure exists (Can contain a filename
//                on the end).
//   nameIsFile : True if the name is a file; False otherwise.
//
// Returns:    True if the directory exists or was created.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 14:05:35 PST 2006
//
// Modifications:
//   
// ****************************************************************************

static bool
EnsureDirectoryExists(std::string &name, bool nameIsFile)
{
    QString dirName;
    if(nameIsFile)
    {
        std::string::size_type pos = name.rfind(SLASH_STRING);
        if(pos != std::string::npos)
        {
            dirName = QString(name.substr(0, pos).c_str());
        }
        else
            dirName = QString(name.c_str());
    }
    else
        dirName = QString(name.c_str());

    // Use a QDir object to see if the directory exists.
    bool exists = false;
    QDir dir(dirName);
    debug1 << "EnsureDirectoryExists: dir=" << dirName.toStdString()
           << endl;
    if(!dir.exists(dirName))
    {
        exists = dir.mkdir(dirName);
        if(exists)
            debug1 << "EnsureDirectoryExists: directory was created." << endl;
        else
        {
            debug1 << "EnsureDirectoryExists: directory could not "
                      "be created." << endl;
        }
    }
    else
    {
        exists = true;
        debug1 << "EnsureDirectoryExists: directory exists." << endl;
    }

    return exists;
}

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
//   Brad Whitlock, Fri Sep 22 16:38:06 PST 2006
//   Added support for movie templates.
//
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Added num frames page.
//
//   Brad Whitlock, Tue Apr  8 16:06:21 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Oct  7 13:13:33 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisSaveMovieWizard::QvisSaveMovieWizard(AttributeSubject *atts, QWidget *parent) : 
    QvisWizard(atts, parent), templateTitleToInfo(), sequencePages()
{
    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::HaveHelpButton, false);
    setOption(QWizard::HaveNextButtonOnLastPage, false);

    default_movie_size[0] = 400;
    default_movie_size[1] = 400;
    default_num_frames = 1;
    removingPages = false;

    //
    // Decision variables that influence which path is taken through the wizard.
    //
    decision_movieType = MOVIE_TYPE_SIMPLE;
    decision_templateUsage = Template_Use;
    decision_settingsOkay = false;
    decision_saveTemplate = false;

    //
    // Internal state for certain pages.
    //
    page0_usePreviousSettingsAllowed = (movieAtts->GetFileFormats().size() > 0);
    page2_templatesPopulated = false;
    page9_aspect = 1.;

    templateSpec = 0;
    currentMovieTemplate = "";

    // Set the wizard's title.
    setWindowTitle(tr("Save movie wizard"));

    // Create the wizard pages.
    CreateMovieTypePage();
    CreateNewTemplatePromptPage();
    CreateChooseTemplatePage();
    CreateChooseNewSourcesPage();
    CreateViewportPage();
    CreateSequencesPage();
    CreateSaveTemplatePage();
    CreateSaveTemplateAsPage();
    CreateSettingsOkayPage();
    CreateFormatPage();
    CreateNumFramesPage();
    CreateFilenamePage();
    CreateEmailPage();
    CreateGenerationMethodPage();

    setStartId(Page_MovieType);
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
//   Brad Whitlock, Thu Oct  9 10:26:37 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisSaveMovieWizard::~QvisSaveMovieWizard()
{
    // Delete any custom pages that may be loaded.
    for(int i = 0; i < sequencePages.size(); ++i)
        delete sequencePages[i].ui;
    sequencePages.clear();

    if(templateSpec != 0)
        delete templateSpec;
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
//   Brad Whitlock, Wed Sep 27 09:46:42 PDT 2006
//   Added code to write out the template options file.
//
//   Brad Whitlock, Fri Jan 18 16:06:27 PST 2008
//   Force the dialog to be raised and to get focus.
//
//   Brad Whitlock, Tue Oct  7 13:18:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

int
QvisSaveMovieWizard::Exec()
{
    // Set the enabled state on the "use previous settings" radio button.
    page0_r1->setEnabled(page0_usePreviousSettingsAllowed);

    // Reset the settings okay flag and make page9 a final page.
    decision_settingsOkay = true;
    page8->setFinalPage(true);

    raise();
    activateWindow();
    restart();

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
    {
        decision_settingsOkay = true;
        WriteTemplateSpecification();
    }

    return retval;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::WriteTemplateSpecification
//
// Purpose: 
//   Writes out a movie template specification if one is required.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 16:36:40 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Nov 21 14:38:09 PST 2006
//   Added code to make sure the destination directories exist.
//
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisSaveMovieWizard::WriteTemplateSpecification()
{
    const char *mName = "QvisSaveMovieWizard::WriteTemplateSpecification: ";

    // If we have template options to store and we're not using our
    // previous settings then write out the template options file.
    bool addSources = false;
    if(templateSpec != 0 && 
       decision_movieType == MOVIE_TYPE_TEMPLATE)
    {
        std::string templateFile, specificationFile;

        StringMovieTemplateDataMap::const_iterator it;
        it = templateTitleToInfo.find(currentMovieTemplate);
        if(it != templateTitleToInfo.end())
        {
            //
            // We're using an existing template. We'll want to create a new
            // specification file for this run in the current directory so
            // determine the name of the specification file.
            //
            const std::string &tfile = it->second.info.specificationFile;
            std::string::size_type slashpos = tfile.rfind(SLASH_STRING);
            if(slashpos != std::string::npos)
                specificationFile = tfile.substr(slashpos+1, tfile.size()-slashpos-1);
            else
                specificationFile = tfile;

            debug1 << mName << "We're going to create a new specification "
                "file called " << specificationFile.c_str()
                   << " based on " << tfile.c_str() << endl;

            templateFile = it->second.info.templateFile;
            addSources = true;

            //
            // If we're saving the "pre-existing" template as a different template
            // then we should handle that here. We'll need to do a lot of the same
            // things as in the case below but the new PY file will have to
            // derive from the class name in the pre-existing template file in 
            // order to preserve the HandleScriptingSequence method. We need to
            // override the SetupVisualization method like below also.
            //
        }
        else if(decision_saveTemplate)
        {
            //
            // We created a new template file and we're saving it as a template
            // that we want to reuse in the future. We have to do a few more
            // things in this case such as tell the viewer to save a session
            // file that can be used to restore the state before creating
            // movie frames.
            //
            debug1 << mName << "Saving new template." << endl;
            templateSpec->GetTemplateFile(specificationFile);
            debug1 << mName << "Specification file: " << specificationFile.c_str() << endl;

            // Create a new .py template filename from the specification file.
            // Also create the name of the session file that we'll be saving.
            std::string specExt(specificationFile.substr(
                specificationFile.size()-3, specificationFile.size()-1));
            std::string sessionFile;
            if(specExt == ".mt")
            {
                std::string base(specificationFile.substr(0, 
                    specificationFile.size() - 3));

                templateFile = base + ".py";
                sessionFile = base + ".session";
            }
            else
            {
                templateFile = specificationFile + ".py";
                sessionFile = specificationFile + ".session";
            }
            debug1 << mName << "templateFile = " << templateFile.c_str() << endl;
            debug1 << mName << "sessionFile = " << sessionFile.c_str() << endl;

            // Make sure that the template's destination directory exists.
            EnsureDirectoryExists(templateFile, true);

            //
            // If we are saving this template for the first time, rather than 
            // editing, the session file will be empty. If the session file is
            // empty then tell the viewer to save a session file.
            //
            std::string curSessionFile;
            templateSpec->GetSessionFile(curSessionFile);
            if(curSessionFile.size() == 0)
            {
                // Tell the viewer to save its session so we'll have the setup
                // for creating the right plots the next time that we use the
                // new template
                debug1 << mName << "Telling viewer to export session "
                       << sessionFile.c_str() << endl;
                GetViewerMethods()->ExportEntireState(sessionFile);
                templateSpec->SetSessionFile(sessionFile);
            }

            // Save the movie template Python file.
            FILE *pyFile = fopen(templateFile.c_str(), "wt");
            if(pyFile != 0)
            {
                fprintf(pyFile, "Source(\"%s\")\n\n", GetVisItMovieTemplateBaseClass().c_str());
                fprintf(pyFile, "class CustomMovieTemplate(VisItMovieTemplate):\n");
                fprintf(pyFile, "    def __init__(self, mm, tr):\n");
                fprintf(pyFile, "        VisItMovieTemplate.__init__(self, mm, tr)\n");
                fprintf(pyFile, "    def __del__(self):\n");
                fprintf(pyFile, "        VisItMovieTemplate.__del__(self)\n");
                fprintf(pyFile, "    # Override SetupVisualization so it uses a session file.\n");
                fprintf(pyFile, "    def SetupVisualization(self):\n");
                fprintf(pyFile, "        self.SetupVisualizationFromSession()\n\n");
                fprintf(pyFile, "def InstantiateMovieTemplate(moviemaker, templateReader):\n");
                fprintf(pyFile, "    return CustomMovieTemplate(moviemaker, templateReader)\n");
                fclose(pyFile);

                debug1 << mName << "Finished writing template Python file." << endl;
            }
            else
            {
                Error(tr("VisIt could not save the new movie template's Python script."));
                templateFile = GetVisItMovieTemplateBaseClass();
                debug1 << mName << "Could not write the template Python file. "
                    "Make the template specification file use the generictemplate.py "
                    " Python file for movie generation." << endl;
            }

            // Make the template spec use the name of the template file that
            // we just created..
            templateSpec->SetTemplateFile(templateFile);
        }
        else
        {
            //
            // We created a new template but we're not going to keep it beyond
            // this VisIt session so just call it "current".
            //
            debug1 << "Use new template settings that were not saved." << endl;
            specificationFile = "current";
            if(!templateSpec->GetTemplateFile(templateFile))
                templateFile = GetVisItMovieTemplateBaseClass();
            addSources = true;

            debug1 << mName << "templateFile = " << templateFile.c_str() << endl;
        }

        // If we're not saving a template file then we are creating a modified copy
        // of it so let's save it off in the user's directory or in the current dir
        // on UNIX.
        if(!decision_saveTemplate)
        {
#if defined(_WIN32)
            specificationFile = GetUserVisItDirectory() + specificationFile + ".opt";
#else
            specificationFile = specificationFile + ".opt";
#endif
            debug1 << mName << "specificationFile = "
                   << specificationFile.c_str() << endl;
        }

        // Look through the custom pages and update any template file options
        // that have been changed.
        UpdateDefaultValuesFromCustomPages();

        // If we need to update the list of sources in the template spec then
        // do that now.
        if(addSources)
        {
            if(page3_sessionSources->getSources().size() > 0)
            {
                templateSpec->SetSources(page3_sessionSources->getSources());
                debug1 << mName << "Setting template sources to:\n";
                for(int i = 0; i < page3_sessionSources->getSources().size();++i)
                    debug1 << "\t" << page3_sessionSources->getSources()[i] << endl;
            }
        }

        // Make sure that the template's destination directory exists.
        EnsureDirectoryExists(specificationFile, true);

        // Write the template values to the new file.
        debug1 << mName << "Using template file: " << templateFile.c_str() << endl;
        debug1 << mName << "Writing template specification file: "
               << specificationFile.c_str() << endl;
        templateSpec->WriteConfigFile(specificationFile.c_str());

        // Save the name of the template specification file in the movie 
        // so we can tell the interpreter in the CLI the name of the template
        // specification file that it should use for this movie.
        movieAtts->SetTemplateFile(specificationFile);
    }
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
//   Brad Whitlock, Mon Oct 2 11:08:48 PDT 2006
//   Added code to set the aspect in page4's viewport object.
//
// ****************************************************************************

void
QvisSaveMovieWizard::SetDefaultMovieSize(int w, int h)
{
    default_movie_size[0] = w;
    default_movie_size[1] = h;

/*
    // Update page4's viewport object.
    double aspect = 1.;
    if(default_movie_size[0] != 0)
        aspect = double(default_movie_size[1]) / double(default_movie_size[0]);
    page4_viewportDisplay->setAspect(aspect);
*/
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::SetDefaultNumFrames
//
// Purpose: 
//   Sets the default number of movie frames
//
// Programmer: Dave Bremer
// Creation:   Fri Oct  5 15:41:01 PDT 2007
// ****************************************************************************

void
QvisSaveMovieWizard::SetDefaultNumFrames(int nFrames)
{
    default_num_frames = nFrames;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::GetDefaultNumFrames
//
// Purpose: 
//   Gets the default number of movie frames
//
// Programmer: Dave Bremer
// Creation:   Fri Oct  5 15:41:01 PDT 2007
// ****************************************************************************

int  
QvisSaveMovieWizard::GetDefaultNumFrames()
{
    return default_num_frames;
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
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Oct  7 13:21:55 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateMovieTypePage()
{
    page0 = new QWizardPage(this);
    page0->setTitle(tr("Movie type"));
    page0->setSubTitle(tr("Would you like to create a new simple movie or "
                           "use your previous settings?"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page0);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    hCenterLayout->addStretch(5);
    QVBoxLayout *buttonLayout = new QVBoxLayout(0);
    hCenterLayout->addLayout(buttonLayout);
    hCenterLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page0_buttongroup = new QButtonGroup(this);
    page0_r1 = new QRadioButton(tr("Use my previous movie settings"), page0);
    page0_r1->setEnabled(false);
    page0_buttongroup->addButton(page0_r1, 0);
    buttonLayout->addWidget(page0_r1);
    QRadioButton *r2 = new QRadioButton(tr("New simple movie"), page0);
    page0_buttongroup->addButton(r2, 1);
    buttonLayout->addWidget(r2);
    QRadioButton *r3 = new QRadioButton(tr("New template movie"), page0);
    page0_buttongroup->addButton(r3, 2);
    buttonLayout->addWidget(r3);

    pageLayout->addStretch(10);
    r2->setChecked(true);
    connect(page0_buttongroup, SIGNAL(buttonClicked(int)),
            this, SLOT(page0_movieTypeChanged(int)));

    // Add the page.
    setPage(Page_MovieType, page0);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateNewTemplatePromptPage
//
// Purpose: 
//   Creates page1.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:13 PDT 2005
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Thu Oct  9 10:28:27 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateNewTemplatePromptPage()
{
    page1 = new QWizardPage(this);
    page1->setTitle(tr("Template usage"));
    page1->setSubTitle(tr("Movie templates allow you to create complex movies. "
                          "What would you like to do?"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page1);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    hCenterLayout->addStretch(5);
    QVBoxLayout *buttonLayout = new QVBoxLayout(0);
    hCenterLayout->addLayout(buttonLayout);
    hCenterLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page1_buttongroup = new QButtonGroup(page1);
    QRadioButton *r1 = new QRadioButton(tr("Use existing template"), page1);
    r1->setChecked(true);
    page1_buttongroup->addButton(r1, 0);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("Edit existing template"), page1);
    page1_buttongroup->addButton(r2, 1);
    buttonLayout->addWidget(r2);
    QRadioButton *r3 = new QRadioButton(tr("Create new template"), page1);
    page1_buttongroup->addButton(r3, 2);
    buttonLayout->addWidget(r3);

    pageLayout->addStretch(10);
    connect(page1_buttongroup, SIGNAL(buttonClicked(int)),
            this, SLOT(page1_newTemplateChanged(int)));

    // Add the page.
    setPage(Page_TemplateAction, page1);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateChooseTemplatePage
//
// Purpose: 
//   Lets the user choose from the available movie templates.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 09:34:25 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Oct 13 14:37:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateChooseTemplatePage()
{
    page2 = new QWizardPage(this);
    page2->setTitle(tr("Choose template"));
    page2->setSubTitle(tr("Choose a movie template."));

    QVBoxLayout *pageLayout = new QVBoxLayout(page2);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    pageLayout->setStretchFactor(hCenterLayout, 10);

    // The list of templates.
    page2_templates = new QListWidget(page2);
    connect(page2_templates, SIGNAL(itemSelectionChanged()),
            this, SLOT(page2_selectedTemplateChanged()));
    hCenterLayout->addWidget(page2_templates);
    hCenterLayout->setStretchFactor(page2_templates, 10);

    QWidget *page2_description_vbox = new QWidget(page2);
    QVBoxLayout *vbox = new QVBoxLayout(page2_description_vbox);
    vbox->setMargin(0);
    vbox->setSpacing(10);
    hCenterLayout->addWidget(page2_description_vbox);
    hCenterLayout->setStretchFactor(page2_description_vbox, 10);

    // Add a picture of the template
    page2_template_image = new QLabel(tr("No preview"), page2_description_vbox);
    page2_template_image->setMinimumSize(200,200);
    page2_template_image->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    vbox->addWidget(page2_template_image);
    vbox->setStretchFactor(page2_template_image, 10);

    // Add the description.
    page2_template_description = new QTextEdit(page2_description_vbox);
    page2_template_description->setReadOnly(true);
    vbox->addWidget(page2_template_description);

    // Add the page.
    setPage(Page_PickTemplate, page2);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateChooseNewSourcesPage
//
// Purpose: 
//   Create the wizard page that lets the user choose new sources for
//   a template that was based on a session file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 14:03:29 PST 2006
//
// Modifications:
//   Brad Whitlock, Mon Oct 13 15:52:00 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateChooseNewSourcesPage()
{
    page3 = new QWizardPage(this);
    page3->setTitle(tr("Update sources"));
    page3->setSubTitle(tr("Make sure that the sources used in this template "
        "are up to date. You can change the sources here to make a movie "
        "using a template that was created with other data.")); 

    QVBoxLayout *pageLayout = new QVBoxLayout(page3);
    pageLayout->setSpacing(10);

    page3_sessionSources = new QvisSessionSourceChanger(page3);
    pageLayout->addWidget(page3_sessionSources);

    // Add the page.
    setPage(Page_TemplateSources, page3);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateViewportPage
//
// Purpose: 
//   Creates the viewport page.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 14:19:55 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Oct 13 16:06:14 PDT 2008
//   Qt 4.
//
//   Cyrus Harrison, Fri Nov  7 16:14:21 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateViewportPage()
{
    page4 = new QWizardPage(this);
    page4->setTitle(tr("Viewports"));
    page4->setSubTitle(tr("Change the movie template's viewports. Viewports "
        "are regions of the final movie image to which VisIt's visualization "
        "windows can be mapped."));

    QVBoxLayout *pageLayout = new QVBoxLayout(page4);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    pageLayout->setStretchFactor(hCenterLayout, 10);

    // Do the left side where we have all of the viewport controls.
    QGridLayout *leftLayout = new QGridLayout(0);
    hCenterLayout->addLayout(leftLayout);
    leftLayout->setSpacing(10);
 
    // The viewport list.
    page4_viewportList = new QListWidget(page4);
    page4_viewportList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(page4_viewportList, SIGNAL(itemSelectionChanged()),
            this, SLOT(page4_viewportSelected()));
    leftLayout->addWidget(page4_viewportList, 0, 0, 1, 3);

    // The buttons to add, remove viewports (also change layouts)
    QPushButton *page4_addViewportButton = new QPushButton(tr("New"), page4);
    connect(page4_addViewportButton, SIGNAL(clicked()),
            this, SLOT(page4_addViewport()));
    leftLayout->addWidget(page4_addViewportButton, 1, 0);

    page4_deleteViewportButton = new QPushButton(tr("Delete"), page4);
    connect(page4_deleteViewportButton, SIGNAL(clicked()),
            this, SLOT(page4_deleteViewport()));
    leftLayout->addWidget(page4_deleteViewportButton, 1, 1);

    QvisPredefinedViewports *page4_predef = new QvisPredefinedViewports(page4);
    connect(page4_predef, SIGNAL(activated(int)),
            this, SLOT(page4_usePredefinedViewports(int)));
    leftLayout->addWidget(page4_predef, 1, 2);

    // Add the page4 stuff at the bottom to show the viewport properties.
    QGroupBox *viewportProps = new QGroupBox(tr("Viewport properties"), page4);
    leftLayout->addWidget(viewportProps, 2, 0, 1, 3);
    QGridLayout *viewportLayout = new QGridLayout(viewportProps);
//    viewportLayout->setSpacing(5);

    page4_lowerLeft = new QvisScreenPositionEdit(viewportProps);
    connect(page4_lowerLeft, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(page4_lowerLeftChanged(double,double)));
    QLabel *page4_lowerLeftLabel = new QLabel(tr("Lower left"), viewportProps);
    page4_lowerLeftLabel->setBuddy(page4_lowerLeft);
    viewportLayout->addWidget(page4_lowerLeftLabel, 0, 0);
    viewportLayout->addWidget(page4_lowerLeft, 0, 1);

    page4_upperRight = new QvisScreenPositionEdit(viewportProps);
    connect(page4_upperRight, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(page4_upperRightChanged(double,double)));
    QLabel *page4_upperRightLabel = new QLabel(tr("Upper right"), viewportProps);
    page4_upperRightLabel->setBuddy(page4_upperRight);
    viewportLayout->addWidget(page4_upperRightLabel, 1, 0);
    viewportLayout->addWidget(page4_upperRight, 1, 1);

    viewportLayout->addWidget(new QLabel(tr("Compositing"), viewportProps), 2, 0);

    page4_compositingMode = new QButtonGroup(this);
    connect(page4_compositingMode, SIGNAL(buttonClicked(int)),
            this, SLOT(page4_compositingModeChanged(int)));
    QRadioButton *rb0 = new QRadioButton(tr("Overlay"), viewportProps);
    page4_compositingMode->addButton(rb0, 0);
    viewportLayout->addWidget(rb0, 3, 0);
    QRadioButton *rb1 = new QRadioButton(tr("Blend"), viewportProps);
    page4_compositingMode->addButton(rb1, 1);
    viewportLayout->addWidget(rb1, 4, 0);
    QRadioButton *rb2 = new QRadioButton(tr("Replace color"), viewportProps);
    page4_compositingMode->addButton(rb2, 2);
    viewportLayout->addWidget(rb2, 5, 0);
    QRadioButton *rb3 = new QRadioButton(tr("Replace background color"), viewportProps);
    page4_compositingMode->addButton(rb3, 3);
    viewportLayout->addWidget(rb3, 6, 0, 1, 2);

    page4_viewportOpacity = new QvisOpacitySlider(viewportProps);
    connect(page4_viewportOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(page4_viewportOpacityChanged(int)));
    viewportLayout->addWidget(page4_viewportOpacity, 4, 1);

    page4_viewportColor = new QvisColorButton(viewportProps);
    connect(page4_viewportColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(page4_viewportColorChanged(const QColor &)));
    viewportLayout->addWidget(page4_viewportColor, 5, 1);

    page4_dropShadow = new QCheckBox(tr("Add drop shadow"));
    connect(page4_dropShadow, SIGNAL(toggled(bool)),
            this, SLOT(page4_dropShadowChanged(bool)));
    viewportLayout->addWidget(page4_dropShadow, 7, 0, 1, 2);
    

    // Add the viewport widget on the right.
    page4_viewportDisplay = new QvisViewportWidget(1., 320, 320, page4);
    connect(page4_viewportDisplay, SIGNAL(viewportAdded(const QString &, float, float, float, float)),
            this, SLOT(page4_viewportAdded(const QString &, float, float, float, float)));
    connect(page4_viewportDisplay, SIGNAL(viewportChanged(const QString &, float, float, float, float)),
           this, SLOT(page4_viewportChanged(const QString &, float, float, float, float)));
    connect(page4_viewportDisplay, SIGNAL(viewportRemoved(const QString &)),
           this, SLOT(page4_viewportRemoved(const QString &)));
    connect(page4_viewportDisplay, SIGNAL(viewportActivated(const QString &)),
            this, SLOT(page4_viewportActivated(const QString &)));
    hCenterLayout->addWidget(page4_viewportDisplay);

    // Add the page.
    setPage(Page_Viewports, page4);    
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateSequencesPage
//
// Purpose: 
//   Creates the sequences page.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 14:20:55 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
// 
///  Brad Whitlock, Mon Oct 13 17:11:03 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateSequencesPage()
{
    page5 = new QWizardPage(this);
    page5->setTitle(tr("Sequences"));
    page5->setSubTitle(tr("Create new sequences of frames and map them to "
                          "movie template viewports."));

    QVBoxLayout *pageLayout = new QVBoxLayout(page5);
    pageLayout->setSpacing(10);

    QGridLayout *gridLayout = new QGridLayout(0);
    pageLayout->addLayout(gridLayout);
    gridLayout->setSpacing(5);

    page5_sequenceList = new QListWidget(page5);
    connect(page5_sequenceList, SIGNAL(itemSelectionChanged()),
            this, SLOT(page5_selectedSequenceChanged()));
    gridLayout->addWidget(page5_sequenceList, 0, 0, 1, 2);

    QvisSequenceButton *newSequence = new QvisSequenceButton(page5);
    connect(newSequence, SIGNAL(activated(int)),
            this, SLOT(page5_newSequenceClicked(int)));
    gridLayout->addWidget(newSequence, 1, 0);

    page5_deleteSequence = new QPushButton(tr("Delete"));
    connect(page5_deleteSequence, SIGNAL(clicked()),
            this, SLOT(page5_deleteSequenceClicked()));
    gridLayout->addWidget(page5_deleteSequence, 1, 1);

    page5_sequenceProperties = new QGroupBox(tr("Sequence properties"), page5);
    gridLayout->addWidget(page5_sequenceProperties, 0, 2, 2, 2);
    gridLayout->setRowStretch(0, 5);
    QGridLayout *seqPropLayout = new QGridLayout(page5_sequenceProperties);    
    seqPropLayout->setSpacing(5);

    // Create the sequence name controls.
    page5_sequenceName = new QLineEdit(page5_sequenceProperties);
    connect(page5_sequenceName, SIGNAL(textChanged(const QString &)),
            this, SLOT(page5_typedNewSequenceName(const QString &)));
    page5_sequenceNameLabel = new QLabel(tr("Name"), page5_sequenceProperties);
    page5_sequenceNameLabel->setBuddy(page5_sequenceName);
    seqPropLayout->addWidget(page5_sequenceNameLabel, 0, 0);
    seqPropLayout->addWidget(page5_sequenceName, 0, 1);

    // Create the combo box that lets us map the sequence to a viewport.
    page5_sequenceDestinationViewport = new QComboBox(page5_sequenceProperties);
    connect(page5_sequenceDestinationViewport, SIGNAL(activated(int)),
            this, SLOT(page5_destinationViewportChanged(int)));
    seqPropLayout->addWidget(page5_sequenceDestinationViewport, 1, 1);
    seqPropLayout->addWidget(new QLabel(tr("Map to viewport"), page5_sequenceProperties),
                             1, 0);

    // Create the uiFile name controls
    page5_sequenceUIFile = new QvisDialogLineEdit(page5_sequenceProperties);
    page5_sequenceUIFile->setDialogMode(QvisDialogLineEdit::ChooseLocalFile);
    connect(page5_sequenceUIFile, SIGNAL(returnPressed()),
            this, SLOT(page5_sequenceUIFileChanged()));
    page5_sequenceUIFile->setDialogFilter(tr("User interface (*.ui)"));
    page5_sequenceUIFile->setDialogCaption(tr("Get User Interface filename"));

    page5_sequenceUILabel = new QLabel(tr("User interface"), page5_sequenceProperties);
    page5_sequenceUILabel->setToolTip(tr("User interface definition file."));

    seqPropLayout->addWidget(page5_sequenceUILabel, 2, 0);
    seqPropLayout->addWidget(page5_sequenceUIFile, 2, 1);

    // Create the sequence view that lets us order sequences in a viewport.
    page5_sequenceView = new QvisSequenceView(page5);
    connect(page5_sequenceView,
            SIGNAL(updatedMapping(const QString &,const QStringList &)),
            this,
            SLOT(page5_updatedMapping(const QString &,const QStringList &)));
    connect(page5_sequenceView,
            SIGNAL(updatedMapping(const QString &,const QStringList &,const QString &,const QStringList &)),
            this,
            SLOT(page5_updatedMapping(const QString &,const QStringList &,const QString &,const QStringList &)));
    QLabel *seqOrderLabel = new QLabel(tr("Sequence to viewport mapping"), page5);
    gridLayout->addWidget(seqOrderLabel, 2, 0, 1, 4);
    gridLayout->addWidget(page5_sequenceView, 3, 0, 1, 4);
    gridLayout->setRowStretch(3, 15);

    // Add the page.
    setPage(Page_Sequences, page5);
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
//   Brad Whitlock, Mon Oct 13 16:41:27 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateSaveTemplatePage()
{
    CreateYesNoPage(&page6, &page6_buttongroup, SLOT(page6_saveAsTemplateChanged(int)));
    page6->setTitle(tr("Save new template"));
    page6->setSubTitle(tr("Save this movie as a new template?"));
    setPage(Page_SaveTemplate, page6);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateSaveTemplateAsPage
//
// Purpose: 
//   Creates the "save template as" wizard page.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 10 15:39:24 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Oct 13 16:31:10 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateSaveTemplateAsPage()
{
    page7 = new QWizardPage(this);
    page7->setTitle(tr("Save new template as"));
    page7->setSubTitle(tr("Enter the information that will be saved with your template."));

    QVBoxLayout *pageLayout = new QVBoxLayout(page7);
    pageLayout->setSpacing(10);
    QGridLayout *gLayout = new QGridLayout(0);
    pageLayout->addLayout(gLayout);
    gLayout->setSpacing(5);
    pageLayout->addStretch(20);

    //
    // Create the template title
    //
    QLabel *titleLabel = new QLabel(tr("Title"), page7);
    gLayout->addWidget(titleLabel, 0, 0);
    page7_templateName = new QLineEdit(page7);
    connect(page7_templateName, SIGNAL(textChanged(const QString &)),
            this, SLOT(page7_templateNameChanged(const QString &)));    
    gLayout->addWidget(page7_templateName, 0, 1);

    //
    // Create the template description
    //
    QLabel *descriptionLabel = new QLabel(tr("Description"), page7);
    gLayout->addWidget(descriptionLabel, 1, 0);
    page7_templateDescription = new QTextEdit(page7);
    connect(page7_templateDescription, SIGNAL(textChanged()),
            this, SLOT(page7_templateDescriptionChanged()));    
    gLayout->addWidget(page7_templateDescription, 1, 1, Qt::AlignTop);

    //
    // Create the template file controls.
    //
    QLabel *filenameLabel = new QLabel(tr("Template filename"), page7);
    gLayout->addWidget(filenameLabel, 2, 0);
    page7_templateFile = new QvisDialogLineEdit(page7);
    page7_templateFile->setDialogMode(QvisDialogLineEdit::ChooseLocalFile);
    page7_templateFile->setDialogCaption(tr("Choose template filename"));
    page7_templateFile->setDialogFilter("*");
    connect(page7_templateFile, SIGNAL(textChanged(const QString &)),
            this, SLOT(page7_templateFileChanged(const QString &)));    
    gLayout->addWidget(page7_templateFile, 2, 1);

    //
    // Create the template file controls.
    //
    QLabel *previewImageLabel = new QLabel("Preview image filename", page7);
    gLayout->addWidget(previewImageLabel, 3, 0);
    page7_previewImageFile = new QvisDialogLineEdit(page7);
    page7_previewImageFile->setDialogMode(QvisDialogLineEdit::ChooseLocalFile);
    page7_previewImageFile->setDialogCaption(tr("Choose preview image filename"));
    page7_previewImageFile->setDialogFilter("XPM image (*.xpm)");
    connect(page7_previewImageFile, SIGNAL(textChanged(const QString &)),
            this, SLOT(page7_previewImageFileChanged(const QString &)));    
    gLayout->addWidget(page7_previewImageFile, 3, 1);

    // Add the page.
    setPage(Page_SaveTemplateAs, page7);
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
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Oct 10 16:26:54 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateSettingsOkayPage()
{
    page8 = new QWizardPage(this);
    page8->setTitle(tr("Settings check"));
    page8->setSubTitle(tr("Do these movie settings look okay?"));
    page8->setFinalPage(true);

    QVBoxLayout *pageLayout = new QVBoxLayout(page8);
    pageLayout->setSpacing(10);

    QHBoxLayout *buttonLayout = new QHBoxLayout(0);
    pageLayout->addLayout(buttonLayout);
    buttonLayout->setSpacing(5);
    buttonLayout->addStretch(5);

    page8_buttongroup = new QButtonGroup(this);
    connect(page8_buttongroup, SIGNAL(buttonClicked(int)),
            this, SLOT(page8_settingsOkayChanged(int)));

    QRadioButton *r1 = new QRadioButton(tr("Yes"), page8);
    page8_buttongroup->addButton(r1, 0);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("No"), page8);
    page8_buttongroup->addButton(r2, 1);
    buttonLayout->addWidget(r2);

    buttonLayout->addStretch(5);
    pageLayout->addSpacing(10);

    //
    // Create the output formats list.
    //
    page8_settings = new QTreeWidget(page8);
    page8_settings->setColumnCount(2);
    QStringList headers;
    headers << tr("Settings") << tr("Values");
    page8_settings->setHeaderLabels(headers);
    page8_settings->setAllColumnsShowFocus(true);
    pageLayout->addWidget(page8_settings);

    // Add the page.
    setPage(Page_ReviewSettings, page8);
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
//   Hank Childs, Thu Jun  8 13:48:28 PDT 2006
//   Fix compiler warning for casting.
//
//   Brad Whitlock, Tue Oct 10 10:52:03 PDT 2006
//   Made it be page9 and I merged the stereo controls into this page.
//
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Oct  8 10:05:48 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Thu Oct  9 15:57:00 PDT 2008
//   Conditionally add img2sm to the menu.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateFormatPage()
{
    page9 = new QWizardPage(this);
    page9->setTitle(tr("Choose format"));
    page9->setSubTitle(tr("Choose movie formats and resolutions."));

    QHBoxLayout *pageLayout = new QHBoxLayout(page9);

    //
    // Create the left page9, which contains the format and resolution options.
    //
    QGroupBox *formatAndResolution = new QGroupBox(page9);
    formatAndResolution->setTitle(tr("Format and resolution"));
    pageLayout->addWidget(formatAndResolution);
    QVBoxLayout *f2innerLayout = new QVBoxLayout(formatAndResolution);
    f2innerLayout->setMargin(10);
    QGridLayout *f2layout = new QGridLayout(0);
    f2innerLayout->addLayout(f2layout);
    f2layout->setSpacing(5);
    f2innerLayout->addStretch(10);

    page9_formatComboBox = new QComboBox(formatAndResolution);
#ifdef _WIN32
    int nFormats = N_MOVIE_FORMATS-1;
#else
    // See if the img2sm encoder is present in the user's path.
    QProcess img2sm;
    img2sm.start("img2sm", QStringList());
    bool started = img2sm.waitForStarted();
    int nFormats = started ? N_MOVIE_FORMATS : (N_MOVIE_FORMATS-1);
#endif
    // Add all of the movie formats from the table.
    for(int i = 0; i < nFormats; ++i)
        page9_formatComboBox->addItem(movieFormatInfo[i].menu_name);
    page9_formatComboBox->setCurrentIndex(6);
    QLabel *formatLabel = new QLabel(tr("Format"), formatAndResolution);
    formatLabel->setBuddy(page9_formatComboBox);
    f2layout->addWidget(page9_formatComboBox, 0, 1, 1, 2);
    f2layout->addWidget(formatLabel, 0, 0);
    QFrame *hline1 = new QFrame(formatAndResolution);
    hline1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    f2layout->addWidget(hline1, 1, 0, 1, 3);
    f2layout->setRowMinimumHeight(1, 15);

    page9_sizeTypeButtonGroup = new QButtonGroup(page9);
    connect(page9_sizeTypeButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(page9_sizeTypeChanged(int)));
    QRadioButton *rb = new QRadioButton(tr("Use current window size"),
        formatAndResolution);
    page9_sizeTypeButtonGroup->addButton(rb, 0);
    f2layout->addWidget(rb, 2, 0, 1, 3);

    page9_scaleSpinBox = new QSpinBox(formatAndResolution);
    page9_scaleSpinBox->setMinimum(1);
    page9_scaleSpinBox->setMaximum(10);
    connect(page9_scaleSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(page9_scaleChanged(int)));
    page9_scaleLabel = new QLabel(tr("Scale"), formatAndResolution);
    page9_scaleLabel->setBuddy(page9_scaleSpinBox);
    f2layout->addWidget(page9_scaleLabel, 3, 0);
    f2layout->addWidget(page9_scaleSpinBox, 3, 1);
    
    rb = new QRadioButton(tr("Specify movie size"), formatAndResolution);
    page9_sizeTypeButtonGroup->addButton(rb, 1);
    f2layout->addWidget(rb, 4, 0, 1, 3);

    page9_widthSpinBox = new QSpinBox(formatAndResolution);
    page9_widthSpinBox->setMinimum(32);
    page9_widthSpinBox->setMaximum(OSMESA_SIZE_LIMIT);
    page9_widthSpinBox->setValue((int)(default_movie_size[0]));
    page9_widthSpinBox->setEnabled(false);
    connect(page9_widthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(page9_widthChanged(int)));
    page9_widthLabel = new QLabel(tr("Width"), formatAndResolution);
    page9_widthLabel->setBuddy(page9_widthSpinBox);
    page9_widthLabel->setEnabled(false);
    f2layout->addWidget(page9_widthLabel, 5, 0);
    f2layout->addWidget(page9_widthSpinBox, 5, 1);
    
    page9_heightSpinBox = new QSpinBox(formatAndResolution);
    page9_heightSpinBox->setMinimum(32);
    page9_heightSpinBox->setMaximum(OSMESA_SIZE_LIMIT);
    page9_heightSpinBox->setValue((int)(default_movie_size[1]));
    page9_heightSpinBox->setEnabled(false);
    connect(page9_heightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(page9_heightChanged(int)));
    page9_heightLabel = new QLabel(tr("Height"), formatAndResolution);
    page9_heightLabel->setBuddy(page9_widthSpinBox);
    page9_heightLabel->setEnabled(false);
    f2layout->addWidget(page9_heightLabel, 6, 0);
    f2layout->addWidget(page9_heightSpinBox, 6, 1);

    page9_aspectLock = new QCheckBox(tr("lock aspect"), formatAndResolution);
    page9_aspectLock->setChecked(true);
    page9_aspectLock->setEnabled(false);
    connect(page9_aspectLock, SIGNAL(toggled(bool)),
            this, SLOT(page9_aspectLockChanged(bool)));
    f2layout->addWidget(page9_aspectLock, 5, 2, 2, 1);

    QFrame *hline2 = new QFrame(formatAndResolution);
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    f2layout->addWidget(hline2, 7, 0, 1, 3);
    f2layout->setRowMinimumHeight(7, 15);

    page9_stereoCheckBox = new QCheckBox(tr("Stereo movie"), formatAndResolution);
    connect(page9_stereoCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(page9_stereoChanged(bool)));
    f2layout->addWidget(page9_stereoCheckBox, 8, 0, 1, 3);
    page9_stereoType = new QComboBox(formatAndResolution);
    page9_stereoType->addItem(tr("Left/Right"));
    page9_stereoType->addItem(tr("Red/Blue"));
    page9_stereoType->addItem(tr("Red/Green"));
    page9_stereoType->setCurrentIndex(0);
    connect(page9_stereoType, SIGNAL(activated(int)),
            this, SLOT(page9_stereoTypeChanged(int)));
    page9_stereoLabel = new QLabel(tr("Stereo type"), formatAndResolution);
    page9_stereoLabel->setBuddy(page9_stereoType);
    f2layout->addWidget(page9_stereoLabel,9,0);
    f2layout->addWidget(page9_stereoType, 9, 1, 1, 2);

    //
    // Create the ->, <- buttons
    //
    QVBoxLayout *centerButtonLayout = new QVBoxLayout(0);
    pageLayout->addLayout(centerButtonLayout);
    centerButtonLayout->setSpacing(5);
    centerButtonLayout->addStretch(5);
    page9_addOutputButton = new QPushButton("->", page9);
    connect(page9_addOutputButton, SIGNAL(clicked()),
            this, SLOT(page9_addOutput()));
    centerButtonLayout->addWidget(page9_addOutputButton);
    page9_removeOutputButton = new QPushButton("<-", page9);
    connect(page9_removeOutputButton, SIGNAL(clicked()),
            this, SLOT(page9_removeOutput()));
    centerButtonLayout->addWidget(page9_removeOutputButton);
    centerButtonLayout->addStretch(5);

    //
    // Create the output formats list.
    //
    QGroupBox *outputGroup = new QGroupBox(tr("Output"), page9);
    pageLayout->addWidget(outputGroup, 10);
    QVBoxLayout *f3layout = new QVBoxLayout(outputGroup);
    f3layout->setSpacing(5);
    page9_outputFormats = new QTreeWidget(outputGroup);
    page9_outputFormats->setMinimumWidth(fontMetrics().
        boundingRect("Quicktime movie 2000x2000").width());
    page9_outputFormats->setColumnCount(3);
    QStringList headers;
    headers << tr("Format") << tr("Resolution") << tr("Stereo");
    page9_outputFormats->setHeaderLabels(headers);
    page9_outputFormats->setAllColumnsShowFocus(true);
    f3layout->addWidget(page9_outputFormats);

    // Add the page.
    setPage(Page_Formats, page9);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateNumFramesPage
//
// Purpose: 
//   Creates page 10.
//
// Programmer: Dave Bremer
// Creation:   Mon Oct  8 12:15:33 PDT 2007
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Oct  8 16:38:07 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateNumFramesPage()
{
    page10 = new QWizardPage(this);
    page10->setTitle(tr("Choose length"));
    page10->setSubTitle(
        tr("Choose movie start/end time and frames per second."));

    QVBoxLayout *pageLayout = new QVBoxLayout(page10);
    pageLayout->setSpacing(10);

    QGridLayout *gLayout = new QGridLayout(0);
    pageLayout->addLayout(gLayout);
    gLayout->setSpacing(5);
    pageLayout->addStretch(20);

    page10_fpsLabel = new QLabel(tr("Frames per second"), page10);
    gLayout->addWidget(page10_fpsLabel, 0, 0);

    page10_fpsLineEdit = new QLineEdit(page10);
    gLayout->addWidget(page10_fpsLineEdit, 0, 1);
    connect(page10_fpsLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page10_fpsChanged(const QString &)));    

    page10_startIndexLabel = new QLabel(tr("First frame"), page10);
    gLayout->addWidget(page10_startIndexLabel, 1, 0);

    page10_startIndexLineEdit = new QLineEdit(page10);
    gLayout->addWidget(page10_startIndexLineEdit, 1, 1);
    connect(page10_startIndexLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page10_startIndexChanged(const QString &)));    

    page10_endIndexLabel = new QLabel(tr("Last frame"), page10);
    gLayout->addWidget(page10_endIndexLabel, 2, 0);

    page10_endIndexLineEdit = new QLineEdit(page10);
    gLayout->addWidget(page10_endIndexLineEdit, 2, 1);
    connect(page10_endIndexLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page10_endIndexChanged(const QString &)));    

    // Add the page.
    setPage(Page_NumFrames, page10);
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
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Moved pages 10-12 to 11-13.
//
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Oct  8 16:51:17 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateFilenamePage()
{
    page11 = new QWizardPage(this);
    page11->setTitle(tr("Choose filename"));
    page11->setSubTitle(
        tr("Choose the output directory and base filename for your movie(s)."));

    QVBoxLayout *pageLayout = new QVBoxLayout(page11);
    pageLayout->setSpacing(10);
    QGridLayout *gLayout = new QGridLayout(0);
    pageLayout->addLayout(gLayout);
    gLayout->setSpacing(5);
    pageLayout->addStretch(20);

    //
    // Create the output directory selection controls.
    //
    QLabel *outputDirectoryLabel = new QLabel(tr("Output directory"), page11);
    gLayout->addWidget(outputDirectoryLabel, 0, 0);

    QWidget *outputDirectoryParent = new QWidget(page11);
    QHBoxLayout *oLayout = new QHBoxLayout(outputDirectoryParent);
    page11_outputDirectoryLineEdit = new QLineEdit(outputDirectoryParent);
    oLayout->addWidget(page11_outputDirectoryLineEdit);
    connect(page11_outputDirectoryLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page11_processOutputDirectoryText(const QString &)));
    QPushButton *outputSelectButton = new QPushButton("...", outputDirectoryParent);
    oLayout->addWidget(outputSelectButton);
#ifndef Q_WS_MACX
    outputSelectButton->setMaximumWidth(
         fontMetrics().boundingRect("...").width() + 6);
#endif
    outputSelectButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
         QSizePolicy::Minimum));
    connect(outputSelectButton, SIGNAL(clicked()),
            this, SLOT(page11_selectOutputDirectory()));
    oLayout->setSpacing(0);
    oLayout->setMargin(0);
    oLayout->setStretchFactor(page11_outputDirectoryLineEdit, 100);
    outputDirectoryLabel->setBuddy(outputDirectoryParent);
    gLayout->addWidget(outputDirectoryParent, 0, 1);

    //
    // Create the controls for selecting the file base.
    //
    QLabel *filebaseLabel = new QLabel(tr("Base filename"), page11);
    gLayout->addWidget(filebaseLabel, 1, 0);
    page11_filebaseLineEdit = new QLineEdit(page11);
    connect(page11_filebaseLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page11_processFilebaseText(const QString &)));    
    gLayout->addWidget(page11_filebaseLineEdit, 1, 1);

    // Add the page.
    setPage(Page_Filenames, page11);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateEmailPage
//
// Purpose: 
//   Creates a wizard page that prompts for e-mail address, etc.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 10 10:59:06 PDT 2006
//
// Modifications:
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Moved pages 10-12 to 11-13.
//
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Thu Oct  9 09:38:06 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateEmailPage()
{
    page12 = new QWizardPage(this);
    page12->setTitle(tr("E-mail notification"));
    page12->setSubTitle(tr("Do you want to be notified by E-mail when your "
                           "movie completes?"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page12);
    pageLayout->setSpacing(10);
    QHBoxLayout *buttonLayout = new QHBoxLayout(0);
    pageLayout->addLayout(buttonLayout);
    buttonLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page12_buttongroup = new QButtonGroup(this);
    connect(page12_buttongroup, SIGNAL(buttonClicked(int)),
            this, SLOT(page12_emailNotificationChanged(int)));
    QRadioButton *r1 = new QRadioButton(tr("Yes"), page12);
    page12_buttongroup->addButton(r1, 0);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("No"), page12);
    page12_buttongroup->addButton(r2, 1);
    buttonLayout->addWidget(r2);
    buttonLayout->addStretch(5);

    pageLayout->addSpacing(20);

    QHBoxLayout *emailLayout = new QHBoxLayout(0);
    pageLayout->addLayout(emailLayout);
    page12_emailLineEdit = new QLineEdit(page12);
    connect(page12_emailLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page12_emailAddressChanged(const QString &)));
    page12_emailLabel = new QLabel(tr("E-mail address"), page12);
    page12_emailLabel->setBuddy(page12_emailLineEdit);
    emailLayout->addStretch(5);
    emailLayout->addWidget(page12_emailLabel);
    emailLayout->addWidget(page12_emailLineEdit, 10);
    emailLayout->addStretch(5);

    pageLayout->addStretch(10);

    // Add the page.
    setPage(Page_Email, page12);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::CreateGenerationMethodPage
//
// Purpose: 
//   Creates page 11.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:21:37 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Sep 28 10:59:48 PDT 2006
//   Made it be page 12 instead.
//
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Moved pages 10-12 to 11-13.
//
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Thu Oct  9 10:20:25 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateGenerationMethodPage()
{
    page13 = new QWizardPage(this);
    page13->setTitle(tr("Choose method"));
    page13->setSubTitle(tr("Choose when and how you would like VisIt to "
                           "create your movies."));

    QVBoxLayout *pageLayout = new QVBoxLayout(page13);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    hCenterLayout->addStretch(5);
    QVBoxLayout *buttonLayout = new QVBoxLayout(0);
    hCenterLayout->addLayout(buttonLayout);
    hCenterLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page13_buttongroup = new QButtonGroup(this);
    connect(page13_buttongroup, SIGNAL(buttonClicked(int)),
            this, SLOT(page13_generationMethodChanged(int)));
    QRadioButton *r1 = new QRadioButton(tr("Now, use currently allocated processors"),
        page13);
    page13_buttongroup->addButton(r1, 0);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("Now, use a new instance of VisIt"),
        page13);
    page13_buttongroup->addButton(r2, 1);
    buttonLayout->addWidget(r2);

    QRadioButton *r3 = new QRadioButton(tr("Later, tell me the command to run"),
        page13);
    page13_buttongroup->addButton(r3, 2);
    buttonLayout->addWidget(r3);
    pageLayout->addStretch(10);

    // Add the page.
    setPage(Page_Generation, page13);
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
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Oct 13 16:37:43 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateYesNoPage(QWizardPage **page, QButtonGroup **bg, 
    const char *slot)
{
    *page = new QWizardPage(this);
    QVBoxLayout *pageLayout = new QVBoxLayout(*page);
    pageLayout->setSpacing(10);

    QHBoxLayout *buttonLayout = new QHBoxLayout(0);
    pageLayout->addLayout(buttonLayout);
    buttonLayout->setSpacing(5);
    buttonLayout->addStretch(5);

    *bg = new QButtonGroup(this);
    connect(*bg, SIGNAL(buttonClicked(int)),
            this, slot);

    QRadioButton *r1 = new QRadioButton(tr("Yes"), *page);
    (*bg)->addButton(r1, 0);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("No"), *page);
    (*bg)->addButton(r2, 1);
    buttonLayout->addWidget(r2);

    buttonLayout->addStretch(5);
    pageLayout->addSpacing(10);
    pageLayout->addStretch(10);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::NumSequencePages
//
// Purpose: 
//   Counts the number of real sequence pages that we have.
//
// Returns:    The number of sequence pages that we have.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 14 17:03:51 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisSaveMovieWizard::NumSequencePages() const
{
    int n = 0;
    for(int i = 0; i < sequencePages.size(); ++i)
        n += (sequencePages[i].ui != 0) ? 1 : 0;
    return n;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::nextId
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
// Creation:   Tue Oct 14 09:22:38 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisSaveMovieWizard::nextId() const
{
    int id;

    switch(currentId())
    {
    case Page_MovieType:
        if(decision_movieType == MOVIE_TYPE_LAST_SETTINGS)
            id = Page_ReviewSettings;
        else if(decision_movieType == MOVIE_TYPE_SIMPLE)
            id = Page_Formats;
        else
            id = Page_TemplateAction;
        break;
    case Page_TemplateAction:
        if(decision_templateUsage == Template_Create)
            id = Page_Viewports;
        else
            id = Page_PickTemplate;
        break;
    case Page_PickTemplate:
        { // New scope
        StringMovieTemplateDataMap::const_iterator it = 
            templateTitleToInfo.find(currentMovieTemplate);
        if(it == templateTitleToInfo.end())
            id = Page_TemplateSources;
        else
        {
            if(it->second.info.usesSessionFile)
            {
                // The movie template uses a session file so go to page 3,
                // the page that lets us pick different sources.
                id = Page_TemplateSources;
            }
            // The movie template does not use a session file or the 
            // session file was not provided in the template specification.
            else if(decision_templateUsage == Template_Use)
            {
                // If there are custom pages then tell the wizard to go there.
                // The last page in that set will go to page 9. If there are
                // no custom pages then go directly to page 9.
                if(NumSequencePages() > 0)
                    id = Page_Custom0;
                else
                    id = Page_Formats;
            }
            else
                id = Page_Viewports;
        }
        } // new scope
        break;
    case Page_TemplateSources:
        if(decision_templateUsage == Template_Use)
        {
            // We'll just use a template so tell the wizard to go to page 9,
            // the formats page. If there are custom pages for the template
            // then they will be inserted before page 9.
            id = Page_Formats;
        }
        else
        {
            // We're going to edit the template so allow pages 4,5.
            id = Page_Viewports;
        }
        break;
    case Page_Viewports:
        id = Page_Sequences;
        break;
    case Page_Sequences:
        id = Page_SaveTemplate;
        break;
    case Page_SaveTemplate:
        if(decision_saveTemplate)
            id = Page_SaveTemplateAs;
        else
            id = Page_Formats;
        break;
    case Page_SaveTemplateAs:
        id = Page_Formats;
        break;
    case Page_ReviewSettings:
        if(!decision_settingsOkay)
            id = Page_Formats;
        else
            id = -1;
        break;
    case Page_Formats:
        id = Page_NumFrames;
        break;
    case Page_NumFrames:
        id = Page_Filenames;
        break;
    case Page_Filenames:
        id = Page_Email;
        break;
    case Page_Email:
        id = Page_Generation;
        break;
    case Page_Generation:
        id = -1; // There is no next page.
        break;
    default:
        id = page(currentId())->nextId();
        break;
    };

    return id;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::validateCurrentPage
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
// Creation:   Wed Oct 15 08:49:58 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSaveMovieWizard::validateCurrentPage()
{
    bool valid = true;

    switch(currentId())
    {
    case Page_TemplateAction:
        currentMovieTemplate="";
        delete templateSpec;
        templateSpec = new MovieTemplateConfig;
        break;
    case Page_PickTemplate:
        // Remove the sequence pages and create a default config.
        if(decision_templateUsage == Template_Use)
            AddSequencePages();
        break;
    case Page_Viewports:
        valid = templateSpec->GetNumberOfViewports() > 0;
        break;
    case Page_Sequences:
        valid = templateSpec->GetNumberOfMappedSequences() > 0;
        if(valid)
            AddSequencePages();
        break;
    case Page_Formats:
        valid = movieAtts->GetFileFormats().size() > 0;
        break;
    case Page_Filenames:
        valid = (movieAtts->GetOutputName().size() > 0) &&
                (GetMovieAttsOutputDir().size() > 0);
        break;
    case Page_Email:
        if(!movieAtts->GetSendEmailNotification())
            valid = true;
        else
        {
            valid = movieAtts->GetSendEmailNotification() &&
                    (movieAtts->GetEmailAddress().size() > 3); // some min length...
        }
        break;
    default:
        valid = page(currentId())->validatePage();
    }

    return valid;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::initializePage
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
//   Brad Whitlock, Wed Sep 27 11:00:51 PDT 2006
//   I added support for movie templates.
//
//   Kathleen Bonnell, Fri Jul 20 11:07:11 PDT 2007
//   Move expansion of '.' outputDir to new method 'GetMovieAttsOutputDir'.
//   Use new method to get the outputdirectory from movie atts.
//   
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Added an update for the new num frames page, and 
//   moved pages 10-12 to 11-13.
//
//   Brad Whitlock, Tue Oct 14 09:17:59 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::initializePage(int pageId)
{
    switch(pageId)
    {
    case Page_MovieType:
        // Initialize page0's button group with the current movie type.
        page0_buttongroup->blockSignals(true);
        page0_buttongroup->button(decision_movieType)->setChecked(true);
        page0_buttongroup->blockSignals(false);

        // Make sure that the movie attributes have the right movie type
        // in them.
        if(decision_movieType == MOVIE_TYPE_SIMPLE)
            movieAtts->SetMovieType(MovieAttributes::Simple);
        else if(decision_movieType == MOVIE_TYPE_TEMPLATE)
            movieAtts->SetMovieType(MovieAttributes::UsingTemplate);
        break;
    case Page_TemplateAction:
        page1_buttongroup->blockSignals(true);
        page1_buttongroup->button(decision_templateUsage)->setChecked(true);
        page1_buttongroup->blockSignals(false);
        break;
    case Page_PickTemplate:
        page2_PopulateTemplates();
        break;
    case Page_TemplateSources:
        page3_PopulateSources();
        break;
    case Page_Viewports:
        page4_UpdateViews(-1);
        break;
    case Page_Sequences:
        page5_Update(-1);
        break;
    case Page_SaveTemplate:
        page6_buttongroup->blockSignals(true);
        page6_buttongroup->button(decision_saveTemplate?0:1)->setChecked(true);
        page6_buttongroup->blockSignals(false);
        break;
    case Page_SaveTemplateAs:
        page7_Update();
        break;
    case Page_ReviewSettings:
        // Initialize page8's button group with whether or not we think the
        // current movie settings look okay.
        page8_buttongroup->blockSignals(true);
        page8_buttongroup->button(decision_settingsOkay?0:1)->setChecked(true);
        page8_buttongroup->blockSignals(false);
        page8_UpdateMovieSettings();
        QTimer::singleShot(400, this, SLOT(page8_delayedUpdate()));
        break;
    case Page_Formats:
        page9_UpdateOutputs();
        if(movieAtts->GetFileFormats().size() > 0)
        {
            // Try for the first format but default to TIFF otherwise.
            const stringVector &formats = movieAtts->GetFileFormats();
            const intVector &w = movieAtts->GetWidths();
            const intVector &h = movieAtts->GetHeights();
            const intVector &s = movieAtts->GetStereoFlags();
            const unsignedCharVector &useCurrent = movieAtts->GetUseCurrentSize();
            const doubleVector &scales = movieAtts->GetScales();

            if(!page9_UpdateFormat(FormatToMenuName(formats[0].c_str())))
                page9_UpdateFormat(TIFF_FORMAT);

            page9_UpdateResolution(useCurrent[0]>0, scales[0], w[0], h[0], s[0]);
        }
        else
        {
            // Try for MPEG but default to TIFF otherwise.
            if(!page9_UpdateFormat(MPEG_FORMAT))
                page9_UpdateFormat(TIFF_FORMAT);

            page9_UpdateResolution(true, 1., (int)(default_movie_size[0]),
                                   (int)(default_movie_size[1]), 0);
        }
        break;
    case Page_NumFrames:
        page10_fpsLineEdit->blockSignals(true);
        page10_fpsLineEdit->setText(QString("%1").arg(movieAtts->GetFps()));
        page10_fpsLineEdit->blockSignals(false);

        page10_UpdateStartEndIndex();
        break;
    case Page_Filenames:
        page11_outputDirectoryLineEdit->blockSignals(true);
        page11_outputDirectoryLineEdit->setText(GetMovieAttsOutputDir().c_str());
        page11_outputDirectoryLineEdit->blockSignals(false);
        page11_filebaseLineEdit->blockSignals(true);
        page11_filebaseLineEdit->setText(movieAtts->
            GetOutputName().c_str());
        page11_filebaseLineEdit->blockSignals(false);
        page11_UpdateButtons();
        break;
    case Page_Email:
        page12_buttongroup->blockSignals(true);
        page12_buttongroup->button(movieAtts->GetSendEmailNotification()?0:1)->setChecked(true);
        page12_buttongroup->blockSignals(false);
        page12_emailLineEdit->blockSignals(true);
        page12_emailLineEdit->setText(movieAtts->GetEmailAddress().c_str());
        page12_emailLineEdit->blockSignals(false);
        page12_UpdateButtons();
        break;
    case Page_Generation:
        page13_buttongroup->blockSignals(true);
        if(movieAtts->GetGenerationMethod() == MovieAttributes::NowCurrentInstance)
            page13_buttongroup->button(0)->setChecked(true);
        else if(movieAtts->GetGenerationMethod() == MovieAttributes::NowNewInstance)
            page13_buttongroup->button(1)->setChecked(true);
        else
            page13_buttongroup->button(2)->setChecked(true);
        page13_buttongroup->blockSignals(false);
        break;
    default:
        page(pageId)->initializePage();
        //qDebug("Update page: pageId=%d", pageId);
        break;
    }
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page2_PopulateTemplates
//
// Purpose: 
//   Populates the list of movie templates.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:11:38 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Oct 14 12:09:23 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page2_PopulateTemplates()
{
    if(!page2_templatesPopulated)
    {
        // Read the movie template directory and get the list of available movie 
        // templates.
        debug1 << "Getting movie template files" << endl;
        MovieTemplateFileList templateFiles(GetVisItMovieTemplates());

        // Read information about each of the movie templates and store it in the
        // title to info map.
        templateTitleToInfo.clear();
        for(int i = 0; i < templateFiles.filenames.size(); ++i)
        {
            MovieTemplateData mtdata;
            mtdata.filename = templateFiles.filenames[i];
            mtdata.userDefined = templateFiles.userDefined[i];

            // Get the template description.
            debug1 << "Getting information for movie template: "
                   << templateFiles.filenames[i].c_str() << endl;
            if(GetMovieTemplateInformation(templateFiles.filenames[i], mtdata.info))
            {
                debug1 << "\tTITLE=" << mtdata.info.title.c_str() << endl;
                debug1 << "\tDESCRIPTION=" << mtdata.info.description.c_str() << endl;
                debug1 << "\tSPECIFICATIONFILE=" << mtdata.info.specificationFile.c_str() << endl;
                debug1 << "\tTEMPLATEFILE=" << mtdata.info.templateFile.c_str() << endl;
                debug1 << "\tPREVIEWIMAGE=" << mtdata.info.previewImageFile.c_str() << endl;
                debug1 << "\tSESSIONFILE=" << mtdata.info.sessionFile.c_str() << endl;
                debug1 << "\tusesSessionFile=" << (mtdata.info.usesSessionFile ? "true": "false") << endl;

                // Come up with a unique title for the template.
                std::string title;
                if(mtdata.userDefined)
                    title = std::string(tr("[User defined] ").toStdString());
                title += mtdata.info.title;

                StringMovieTemplateDataMap::const_iterator it;
                std::string title2(title);
                int ver = 2;
                while((it = templateTitleToInfo.find(title)) != templateTitleToInfo.end())
                {
                    char buf[10];
                    SNPRINTF(buf, 10, " (%d)", ver);
                    ++ver;
                    title = title2 + buf;
                }

                // Remember the template information.
                templateTitleToInfo[title] = mtdata;

                // Add an entry in the list of templates.
                page2_templates->addItem(QString(title.c_str()));
            }
        }

        page2_templatesPopulated = true;

        // Select the first item. This will make the other widgets update so they
        // show the information for the first movie template.
        if(page2_templates->count() > 0)
        {
            page2_templates->item(0)->setSelected(true);
            page2_templates->setCurrentRow(0);
            page2_selectedTemplateChanged();
        }
    }
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page3_PopulateSources
//
// Purpose: 
//   Reads the session file for the movie template and populates the list
//   of movie sources so the user can replace them with new sources.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:15:02 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page3_PopulateSources()
{
    const char *mName = "QvisSaveMovieWizard::page3_PopulateSources: ";
    stringVector keys, values;
    std::map<std::string, stringVector> uses;
    std::string sessionFile;

    // Get the name of the template's session file from the template
    // specification
    // 
    if(!templateSpec->GetSessionFile(sessionFile))
    {
        debug1 << mName << "Could not get session file from template "
            "specification." << endl;
        page3_sessionSources->setSources(keys, values, uses);
        return;
    }

    // Try and read the session file that goes with the template
    // so we can get the list of source ids and sources as well as
    // how they are used so we can populate the source control.
    AccessViewerSession sessionAccess;
    if(!sessionAccess.ReadConfigFile(sessionFile.c_str()))
    {
        QString err(tr("VisIt could not read the session file: %1."));
        err.replace("%1", sessionFile.c_str());
        Error(err);
        debug1 << mName << "Could not read the session file: "
               << sessionFile.c_str() << endl;
        page3_sessionSources->setSources(keys, values, uses);
        return;
    }

    if(!sessionAccess.GetSourceMap(keys, values, uses))
    {
        Error(tr("VisIt was able to read the session file that is used by "
              "the current movie template but the session file might be "
              "from before VisIt 1.5.5"));
    }
    else
    {
        debug1 << mName << "Retrieved the source map." << endl;
    }
    page3_sessionSources->setSources(keys, values, uses);
}

#define PAGE4_LIST                 1
#define PAGE4_PROPERTIES           2
#define PAGE4_DISPLAY              4
#define PAGE4_DISPLAY_SELECTION    8

// ****************************************************************************
// Method: QvisSaveMovieWizard::page4_UpdateViews
//
// Purpose: 
//   This method updates the controls on page 4.
//
// Arguments:
//   flags : The union of the flags that are used to update different types
//           of controls.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:15:37 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Oct 14 11:46:27 PDT 2008
//   Qt 4.
//
//   Cyrus Harrison, Fri Nov  7 16:14:21 PST 2008
//   Enabled Viewport Widget for Qt4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page4_UpdateViews(int flags)
{
    // Update the list.
    if((flags & PAGE4_LIST) > 0)
    {
        std::string vpName;
        page4_viewportList->blockSignals(true);
        page4_viewportList->clear();

        int nvp = templateSpec->GetNumberOfViewports();
        for(int i = 0; i < nvp; ++i)
        {
            if(templateSpec->ViewportGetNameForIndex(i, vpName))
            {
                page4_viewportList->addItem(QString(vpName.c_str()));
            }
        }

        if(templateSpec->ViewportGetActiveName(vpName))
        {
            int index = -1;
            if(templateSpec->ViewportGetIndexForName(vpName, index))
            {
                page4_viewportList->item(index)->setSelected(true);
                page4_viewportList->setCurrentItem(page4_viewportList->item(index));
            }
        }

        page4_viewportList->blockSignals(false);

        // Set the enabled state for the next button.
        page4_deleteViewportButton->setEnabled(nvp > 0);
    }

    // Update the viewport properties.
    if((flags & PAGE4_PROPERTIES) > 0)
    {
        float llx, lly, urx, ury;
        llx = lly = 0.f;
        urx = ury = 1.f;
        page4_lowerLeft->blockSignals(true);
        page4_upperRight->blockSignals(true);
        if(!templateSpec->ViewportGetCoordinates(llx, lly, urx, ury))
        {
            debug5 << "Could not read coordinate values for active viewport." << endl;
        }
        page4_lowerLeft->setPosition(llx, lly);
        page4_upperRight->setPosition(urx, ury);
        page4_lowerLeft->blockSignals(false);
        page4_upperRight->blockSignals(false); 

        int compositing = 0;
        if(!templateSpec->ViewportGetCompositing(compositing))
        {
            debug5 << "Could not read compositing value for active viewport." << endl;
        }
        page4_compositingMode->blockSignals(true);
        page4_compositingMode->button(compositing)->setChecked(true);
        page4_compositingMode->blockSignals(false);
        page4_viewportOpacity->setEnabled(compositing == 1);
        page4_viewportColor->setEnabled(compositing == 2);

        float opacity = 1.;
        if(!templateSpec->ViewportGetOpacity(opacity))
        {
            debug5 << "Could not read opacity value for active viewport." << endl;
        }
        page4_viewportOpacity->blockSignals(true);
        page4_viewportOpacity->setValue(int(opacity * 100.));
        page4_viewportOpacity->blockSignals(false);

        int replaceColor[3] = {255,255,255};
        if(!templateSpec->ViewportGetReplaceColor(replaceColor))
        {
            debug5 << "Could not read replaceColor values for active viewport." << endl;
        }
        page4_viewportColor->blockSignals(true);
        page4_viewportColor->setButtonColor(
            QColor(replaceColor[0],
                   replaceColor[1],
                   replaceColor[2]));
        page4_viewportColor->blockSignals(false);

        bool dropShadow = false;
        if(!templateSpec->ViewportGetDropShadow(dropShadow))
        {
            debug5 << "Could not read dropShadow values for active viewport." << endl;
        }
        page4_dropShadow->blockSignals(true);
        page4_dropShadow->setChecked(dropShadow);
        page4_dropShadow->blockSignals(false);
    }
    if((flags & PAGE4_DISPLAY) > 0)
    {
        page4_viewportDisplay->blockSignals(true);
        page4_viewportDisplay->clear();

        for(int i = 0; i < templateSpec->GetNumberOfViewports(); ++i)
        {
            std::string vpName;
            if(templateSpec->ViewportGetNameForIndex(i, vpName))
            {
                float llx, lly, urx, ury;
                if(templateSpec->ViewportGetCoordinates(
                    vpName, llx, lly, urx, ury))
                {
                    page4_viewportDisplay->addViewport(
                        QString(vpName.c_str()), llx, lly, urx, ury);
                }
            }
        }

        page4_viewportDisplay->blockSignals(false);
    }

    if((flags & PAGE4_DISPLAY_SELECTION) > 0)
    {
        page4_viewportDisplay->blockSignals(true);
        std::string vpName;
        if(templateSpec->ViewportGetActiveName(vpName))
        {
            page4_viewportDisplay->setActiveViewport(QString(vpName.c_str()));
        }
        page4_viewportDisplay->blockSignals(false);
    }
}

#define PAGE5_UPDATE_VIEWPORT_LIST   1
#define PAGE5_UPDATE_SEQUENCE_LIST   2
#define PAGE5_UPDATE_SEQUENCE_PROPS  4
#define PAGE5_UPDATE_SEQVIEW_MAPPING 8
// ****************************************************************************
// Method: QvisSaveMovieWizard::page5_Update
//
// Purpose: 
//   This method updates the widgets in the sequences page.
//
// Arguments:
//   flags : The union of flags indicating which groups of controls to update.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:16:21 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Oct 14 11:47:18 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page5_Update(int flags)
{
    const char *mName = "QvisSaveMovieWizard::page5_Update: ";

    if((flags & PAGE5_UPDATE_VIEWPORT_LIST) > 0)
    {
        page5_sequenceDestinationViewport->blockSignals(true);
        page5_sequenceDestinationViewport->clear();
        page5_sequenceDestinationViewport->addItem(tr("(none)"));
        int nvp = templateSpec->GetNumberOfViewports();
        for(int i = 0; i < nvp; ++i)
        {
            std::string vpName;
            if(templateSpec->ViewportGetNameForIndex(i, vpName))
            {
                QString name(vpName.c_str());
                page5_sequenceDestinationViewport->addItem(name);
            }
        }
        page5_sequenceDestinationViewport->blockSignals(false);
    }

    if((flags & PAGE5_UPDATE_SEQUENCE_LIST) > 0)
    {
        page5_sequenceList->blockSignals(true);
        page5_sequenceList->clear();

        // Iterate through the sequences and sort them alphabetically
        // (and numerically using their trailing numbers) and store the
        // results in a map.
        int nseq = templateSpec->GetNumberOfSequences();
        std::map<QString, QString> sortedNames;
        for(int i = 0; i < nseq; ++i)
        {
            std::string name;
            if(templateSpec->SequenceGetNameForIndex(i, name))
            {
                std::string digits;
                for(int j = name.size() - 1; j > 0; --j)
                {
                    char tmp[2] = {'\0', '\0'};
                    if(name[j] >= '0' && name[j] <= '9')
                    {
                        tmp[0] = name[j];
                        digits = std::string(tmp) + digits;
                    }
                    else
                        break;
                }

                QString itemName(name.c_str());
                QString itemKey(name.c_str());
                if(digits.size() > 0)
                {
                    int number;
                    if(sscanf(digits.c_str(), "%d", &number) == 1)
                    {
                        QString pre(name.substr(0, name.size()-digits.size()).c_str());
                        QString idx;
                        idx.sprintf("%05d", number);
                        itemKey = pre + idx;
                    }
                }

                sortedNames[itemKey] = itemName;
            }
        }

        // Populate the listbox and determine the active index.
        int index = 0, activeIndex = -1;
        MovieSequence *s = templateSpec->GetActiveSequence();
        for(std::map<QString, QString>::const_iterator pos = sortedNames.begin();
            pos != sortedNames.end(); ++pos, ++index)
        {
             page5_sequenceList->addItem(pos->second);

            if(s != 0 && QString(s->GetName().c_str()) == pos->second)
                activeIndex = index;
        }

        // Set the active item.
        if(activeIndex != -1)
        {
            page5_sequenceList->item(activeIndex)->setSelected(true);
            page5_sequenceView->selectSequence(page5_sequenceList->item(activeIndex)->text());
        }
        page5_sequenceList->blockSignals(false);

        page5_deleteSequence->setEnabled(nseq>0);
    }

    if((flags & PAGE5_UPDATE_SEQUENCE_PROPS) > 0)
    {
        // Get the sequence name and the index of the viewport to which it is mapped.
        MovieSequence *s = templateSpec->GetActiveSequence();
        QString name, uiName;
        int index = 0;
        bool te = true;
        bool allowUI = false;
        if(s != 0)
        {
            name = QString(s->GetName().c_str());
            std::string vpName;
            if(templateSpec->SequenceGetViewport(s->GetName(), vpName))
            {
                if(templateSpec->ViewportGetIndexForName(vpName, index))
                    ++index;
            }

            te = !s->ReadOnly();
            allowUI = s->SupportsCustomUI();
            if(allowUI)
                uiName = QString(s->GetUIFile().c_str());
        }

        // Set the name of the viewport.
        page5_sequenceName->blockSignals(true);
        page5_sequenceName->setText(name);
        page5_sequenceName->setEnabled(te);
        page5_sequenceNameLabel->setEnabled(te);
        page5_sequenceName->blockSignals(false);

        // Set whether the delete button is enabled.
        page5_deleteSequence->setEnabled(te);

        // Set the viewport to which the sequence is mapped.
        page5_sequenceDestinationViewport->blockSignals(true);
        page5_sequenceDestinationViewport->setCurrentIndex(index);
        page5_sequenceDestinationViewport->blockSignals(false);

        // Set the name of the UI file.
        page5_sequenceUIFile->blockSignals(true);
        page5_sequenceUIFile->setText(uiName);
        page5_sequenceUIFile->blockSignals(false);
        page5_sequenceUILabel->setEnabled(allowUI);
        page5_sequenceUIFile->setEnabled(allowUI);

        // Select the sequence in the sequence view
        page5_sequenceView->selectSequence(name);
    }

    if((flags & PAGE5_UPDATE_SEQVIEW_MAPPING) > 0)
    {
        page5_sequenceView->blockSignals(true);
        page5_sequenceView->clear();

        int nvp = templateSpec->GetNumberOfViewports();
        for(int i = 0; i < nvp; ++i)
        {
            std::string vpName;
            if(templateSpec->ViewportGetNameForIndex(i, vpName))
            {
                // Add the viewport.
                QString name(vpName.c_str());
                page5_sequenceView->addViewport(name);

                // Add the viewport's sequence list.
                stringVector seqList;
                if(templateSpec->ViewportGetSequenceList(vpName, seqList))
                {
                    for(int s = 0; s < seqList.size(); ++s)
                    {
                        MovieSequence *seq = templateSpec->
                            GetSequence(seqList[s]);
                        if(seq != 0)
                        {
                            // Look up the sequence type somewhere...
                            int seqType = seq->SequenceId();

                            // Create a pixmap based on the sequence type...
                            QPixmap pix = seq->Pixmap();

                            //  Add the sequence.
                            QString seqName(seqList[s].c_str());
                            if(pix.isNull())
                            {
                                page5_sequenceView->addSequenceToViewport(name,
                                    seqName, seqType);
                            }
                            else
                            {
                                page5_sequenceView->addSequenceToViewport(name,
                                    seqName, pix, seqType);
                            }
                        }
                        else
                        {
                            debug3 << mName << "Could not find a sequence called "
                                   << seqList[s].c_str() << " in the list of sequences."
                                   << endl;
                        }
                    }
                }
            }
        }
        page5_sequenceView->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page7_Update
//
// Purpose: 
//   Updates the controls on the save template page.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:16:58 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page7_Update()
{
    std::string templateName;
    templateSpec->GetTitle(templateName);
    page7_templateName->blockSignals(true);
    page7_templateName->setText(templateName.c_str());
    page7_templateName->blockSignals(false);

    std::string templateDescription;
    templateSpec->GetDescription(templateDescription);
    page7_templateDescription->blockSignals(true);
    page7_templateDescription->setText(templateDescription.c_str());
    page7_templateDescription->blockSignals(false);

    std::string templateFile;
    templateSpec->GetTemplateFile(templateFile);
    page7_templateFile->blockSignals(true);
    page7_templateFile->setText(templateFile.c_str());
    page7_templateFile->blockSignals(false);

    std::string previewImage;
    templateSpec->GetPreviewImageFile(previewImage);
    page7_previewImageFile->blockSignals(true);
    page7_previewImageFile->setText(previewImage.c_str());
    page7_previewImageFile->blockSignals(false);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page8_UpdateMovieSettings
//
// Purpose: 
//   This method updates the movie settings shown on page 7.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:30:48 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Sep 28 12:00:24 PDT 2006
//   Added information about templates and email.
//
//   Kathleen Bonnell, Fri Jul 20 11:07:11 PDT 2007
//   Use new method to get the outputdirectory from movie atts.
//
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Oct 10 16:29:34 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page8_UpdateMovieSettings()
{
    page8_settings->clear();

    QTreeWidgetItem *item = new QTreeWidgetItem(page8_settings);
    item->setText(0, tr("Generation method"));
    if(movieAtts->GetGenerationMethod() == MovieAttributes::NowCurrentInstance)
        item->setText(1, tr("Now, use currently allocated processors"));
    else if(movieAtts->GetGenerationMethod() == MovieAttributes::NowNewInstance)
        item->setText(1, tr("Now, use a new instance of VisIt"));
    else
        item->setText(1, tr("Later, tell me the command to run"));

    item = new QTreeWidgetItem(page8_settings);
    item->setText(0, tr("Movie type"));
    if(movieAtts->GetMovieType() == MovieAttributes::Simple)
        item->setText(1, tr("New simple movie"));
    else
        item->setText(1, tr("Use movie template"));

    if(movieAtts->GetMovieType() == MovieAttributes::UsingTemplate)
    {
        item = new QTreeWidgetItem(page8_settings);
        item->setText(0, tr("Movie template"));
        item->setText(1, movieAtts->GetTemplateFile().c_str());
    }

    item = new QTreeWidgetItem(page8_settings);
    item->setText(0, tr("Output directory"));
    item->setText(1, GetMovieAttsOutputDir().c_str());

    item = new QTreeWidgetItem(page8_settings);
    item->setText(0, tr("Base filename"));
    item->setText(1, movieAtts->GetOutputName().c_str());

    item = new QTreeWidgetItem(page8_settings);
    item->setText(0, tr("Formats"));
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
            QString scale; scale.sprintf("%dx",  int(scales[i]));
            tmp = QString(FormatToMenuName(formats[i].c_str())) + 
                  QString(" ") + tr("Current") + QString(" ") +
                  scale;
        }
        else
        {
            tmp.sprintf("%s %dx%d", 
                FormatToMenuName(formats[i].c_str()), w[i], h[i]);
        }
        s += tmp;
        int stereoType = movieAtts->GetStereoFlags()[i];
        if(stereoType == 1)
            s += tr(" Left/Right stereo");
        else if(stereoType == 2)
            s += tr(" Red/Blue stereo");
        else if(stereoType == 3)
            s += tr(" Red/Green stereo");

        if(i < formats.size() - 1)
            s += ", ";
    }
    item->setText(1, s);

    item = new QTreeWidgetItem(page8_settings);
    item->setText(0, tr("E-mail notification"));
    if(movieAtts->GetSendEmailNotification())
        item->setText(1, movieAtts->GetEmailAddress().c_str());
    else
        item->setText(1, tr("none"));
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page9_UpdateOutputs
//
// Purpose: 
//   This method populates the list of movie output formats on page 8.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:31:14 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Oct 20 13:39:45 PST 2006
//   Added stereo.
//
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Oct  8 11:23:46 PDT 2008
//   Qt 4.
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
        const intVector    &s = movieAtts->GetStereoFlags();
        const doubleVector &scales = movieAtts->GetScales();
        const unsignedCharVector  &useCurrent = movieAtts->GetUseCurrentSize();

        for(int i = 0; i < formats.size(); ++i)
        {
            QString res;
            if(useCurrent[i] > 0)
            {
                res.sprintf(" %dx", int(scales[i]));
                res = tr("Current") + res;
            }
            else
                res.sprintf("%dx%d", w[i], h[i]);
            QTreeWidgetItem *item = new QTreeWidgetItem(page9_outputFormats);
            item->setText(0, FormatToMenuName(formats[i].c_str()));
            item->setText(1, res);
            QString stereo(tr("off"));
            if(s[i] == 1)
                stereo = tr("Left/Right");
            else if(s[i] == 2)
                stereo = tr("Red/Blue");
            else if(s[i] == 3)
                stereo = tr("Red/Green");
            item->setText(2, stereo);
            bool isLast = (i == formats.size()-1);
            item->setSelected(isLast);
            if(isLast)
                page9_outputFormats->setCurrentItem(item);
        }
    }
    page9_outputFormats->blockSignals(false);
    page9_removeOutputButton->setEnabled(formats.size() > 0);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page9_UpdateFormat
//
// Purpose: 
//   This method sets the movie format combobox on page 8.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:32:03 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Oct  8 11:22:26 PDT 2008
//   Qt 4.
//
// ****************************************************************************

bool
QvisSaveMovieWizard::page9_UpdateFormat(const QString &format)
{
    bool setVal = false;
    for(int i = 0; i < page9_formatComboBox->count(); ++i)
    {
        if(page9_formatComboBox->itemText(i) == format)
        {  
            page9_formatComboBox->blockSignals(true);
            page9_formatComboBox->setCurrentIndex(i);
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
//   This method sets the resolution widgets on page 8.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:32:29 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Oct 20 12:09:29 PDT 2006
//   Added stereo.
//
//   Brad Whitlock, Wed Oct  8 11:22:57 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page9_UpdateResolution(bool useCurrent, double scale, int w, int h, int s)
{
    page9_sizeTypeButtonGroup->blockSignals(true);
    int btn = useCurrent?0:1;
    page9_sizeTypeButtonGroup->button(btn)->setChecked(true);
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

    if(s == 0)
    {
        page9_stereoType->blockSignals(true);
        page9_stereoType->setCurrentIndex(0);
        page9_stereoType->blockSignals(false);
    }
    else
    {
        page9_stereoType->blockSignals(true);
        page9_stereoType->setCurrentIndex(s - 1);
        page9_stereoType->blockSignals(false);
    }
    page9_stereoCheckBox->blockSignals(true);
    page9_stereoCheckBox->setChecked(s != 0);
    page9_stereoCheckBox->blockSignals(false);
    page9_stereoLabel->setEnabled(s != 0);
    page9_stereoType->setEnabled(s != 0);

    if(page9_aspectLock->isChecked())
        page9_aspect = float(w) / float(h);
    else
        page9_aspect = 1.;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page10_UpdateStartEndIndex
//
// Purpose: 
//   This method sets the text in the start/end frame widgets.
//   For templated movies, the number of frames is set by the template.
//   So, if a template is used, I want to disable the start/end index,
//   and clear the values, because they could be misleading.  I could
//   hide the widgets instead, but perhaps that would be more confusing.
//
// Programmer: Dave Bremer
// Creation:   Wed Oct 10 16:31:09 PDT 2007
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page10_UpdateStartEndIndex()
{
    if (movieAtts->GetMovieType() == MovieAttributes::UsingTemplate)
    {
        page10_startIndexLineEdit->blockSignals(true);
        page10_startIndexLineEdit->setText(tr("set in template"));
        page10_startIndexLineEdit->blockSignals(false);

        page10_endIndexLineEdit->blockSignals(true);
        page10_endIndexLineEdit->setText(tr("set in template"));
        page10_endIndexLineEdit->blockSignals(false);

        page10_startIndexLineEdit->setEnabled(false);
        page10_endIndexLineEdit->setEnabled(false);
    }
    else
    {
        page10_startIndexLineEdit->setEnabled(true);
        page10_endIndexLineEdit->setEnabled(true);

        char tmp[100];
        page10_startIndexLineEdit->blockSignals(true);
        SNPRINTF(tmp, 100, "%d", movieAtts->GetStartIndex());
        page10_startIndexLineEdit->setText(tmp);
        page10_startIndexLineEdit->blockSignals(false);

        page10_endIndexLineEdit->blockSignals(true);
        SNPRINTF(tmp, 100, "%d", movieAtts->GetEndIndex());
        page10_endIndexLineEdit->setText(tmp);
        page10_endIndexLineEdit->blockSignals(false);
    }
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
//   Kathleen Bonnell, Fri Jul 20 11:07:11 PDT 2007
//   Use new method to get the outputdirectory from movie atts.
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page11_UpdateButtons()
{
    bool e = (movieAtts->GetOutputName().size() > 0) &&
             (GetMovieAttsOutputDir().size() > 0);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page12_UpdateButtons
//
// Purpose: 
//   This method sets the enabled state for the next button when we're on
//   page 12.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 11:08:57 PDT 2006
//
// Modifications:
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Moved pages 10-12 to 11-13.
//
//   Brad Whitlock, Thu Oct  9 09:52:20 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page12_UpdateButtons()
{
    page12_emailLabel->setEnabled(movieAtts->GetSendEmailNotification());
    page12_emailLineEdit->setEnabled(movieAtts->GetSendEmailNotification());
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::LoadTemplateSpecification
//
// Purpose: 
//   This method loads a movie template specification file so the wizard 
//   can use it.
//
// Arguments:
//   spec : The name of the file to load.
//
// Returns:    True on success; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:17:57 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
QvisSaveMovieWizard::LoadTemplateSpecification(const std::string &spec)
{
    const char *mName = "QvisSaveMovieWizard::LoadTemplateSpecification: ";

    // Delete the current template config.
    delete templateSpec;
    templateSpec = new MovieTemplateConfig;

    // If there is no defaults file then return.
    if(spec == "")
    {
        debug4 << mName << "There is no defaults file for this movie template." << endl;
        return false;
    }

    // Read the defaults file.
    TRY
    {
        templateSpec->ReadConfigFile(spec.c_str());
        if(templateSpec->GetRootNode() == 0)
        {
            debug1 << mName << "The defaults file \"" << spec.c_str()
                   << "\" could not be read" << endl;
        }
    }
    CATCHALL(...)
    {
        debug1 << mName << "The defaults file \"" << spec.c_str()
               << "\" could not be read and VisIt caught an unknown exception "
               << "that was thrown when trying to read the file."
               << endl;
    }
    ENDTRY

    return templateSpec->GetRootNode() != 0;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::UpdateCustomPagesWithDefaultValues
//
// Purpose: 
//   This method updates the custom pages with default values from the
//   template specification.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:19:07 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::UpdateCustomPagesWithDefaultValues()
{
    const char *mName = "QvisSaveMovieWizard::UpdateCustomPagesWithDefaultValues: ";

    DataNode *root = templateSpec->GetRootNode();
    if(root != 0)
    {
        DataNode *seqValues = root->GetNode("SEQUENCEDATA");
        if(seqValues == 0)
        {
            debug4 << mName << "No SEQUENCES data node found in defaults." << endl;
            return;
        }

        for(int i = 0; i < sequencePages.size(); ++i)
        {
            // Get the data node.
            const std::string &seqName = sequencePages[i].name;
            DataNode *pageValues = seqValues->GetNode(seqName);

            // Get the movie sequence.
            MovieSequence *s = templateSpec->GetSequence(seqName);

            // Let the movie sequence read its UI values from the data node.
            if(pageValues != 0 && s != 0)
                s->ReadUIValues(sequencePages[i].ui, pageValues);
        }
    }
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::UpdateDefaultValuesFromCustomPages
//
// Purpose: 
//   This method updates the template specification from the custom UI pages.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:19:31 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::UpdateDefaultValuesFromCustomPages()
{
    const char *mName = "QvisSaveMovieWizard::UpdateDefaultValuesFromCustomPages: ";

    // If there are no defaults then return.
    if(templateSpec == 0)
    {
        debug4 << mName << "No defaults are currently loaded." << endl;
        return;
    }

    // If there are no template pages then return.
    if(sequencePages.size() < 1)
    {
        debug4 << mName << "There are no custom pages for which to retrieve values." << endl;
        return;
    }

    DataNode *root = templateSpec->GetRootNode();
    if(root == 0)
    {
        debug4 << mName << "Defaults are not set." << endl;
        return;
    }

    DataNode *seqValues = root->GetNode("SEQUENCEDATA");
    if(seqValues == 0)
    {
        debug4 << mName << "No SEQUENCEDATA data node found." << endl;
        return;
    }

    // For each page, find widgets that have counterparts in the defaults
    // and for those that do have counterparts, read the widget values
    // and overwrite the values in the defaults.
    for(int i = 0; i < sequencePages.size(); ++i)
    {
        // Get the data node.
        const std::string &seqName = sequencePages[i].name;
        DataNode *pageValues = seqValues->GetNode(seqName);

        // Get the movie sequence.
        MovieSequence *s = templateSpec->GetSequence(seqName);
        if(s != 0)
        {
            // This is a sequence that the user created so we need to
            // create a node for the page values.
            if(pageValues == 0)
            {
                pageValues = new DataNode(seqName);
                pageValues->AddNode(new DataNode("sequenceType", s->SequenceName()));
                seqValues->AddNode(pageValues);

            }

            // Let the movie sequence write its custom values to the
            // data node.
            s->WriteUIValues(sequencePages[i].ui, pageValues);
        }
    }        
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::AddSequencePages
//
// Purpose: 
//   This method adds the template specification's custom pages to the wizard.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:20:27 PST 2006
//
// Modifications:
//   Brad Whitlock, Wed Oct 15 09:22:33 PDT 2008
//   Qt 4.
//
// ****************************************************************************

bool
QvisSaveMovieWizard::AddSequencePages()
{
    const char *mName = "QvisSaveMovieWizard::AddSequencePages: ";

    // Return if we don't have a template spec.
    if(templateSpec == 0)
    {
        debug3 << "There is no template spec" << endl;
        return false;
    }

    // We can't delete pages from the wizard but we can delete the
    // ui's that we had installed in them.
    for(size_t i = 0; i < sequencePages.size(); ++i)
    {
        if(sequencePages[i].ui != 0)
        {
            sequencePages[i].page->removeWidget(sequencePages[i].ui);
            delete sequencePages[i].ui;
            sequencePages[i].ui = 0;
            sequencePages[i].page->setNextId(-1);
        }
    }

    // Iterate over all of the viewports in order and then on all
    // of the sequences within each viewport. We do it like this
    // rather than iterating over the sequences directly so we can
    // keep the UI's for sequences within a viewport grouped.
    int uiCount = 0;
    int noLoadCount = 0;
    QString badUIFiles;
    int nvpt = templateSpec->GetNumberOfViewports();
    debug1 << mName << "Number of viewports: " << nvpt << endl;
    for(int vpt = 0; vpt < nvpt; ++vpt)
    {
        std::string vpName;
        stringVector seqList;
        if(templateSpec->ViewportGetNameForIndex(vpt, vpName) &&
           templateSpec->ViewportGetSequenceList(vpName, seqList))
        {
            for(int i = 0; i < seqList.size(); ++i)
            {
                // If we can't get the sequence then continue.
                MovieSequence *s = templateSpec->
                    GetSequence(seqList[i]);
                if(s == 0)
                    continue;

                // Get the movie sequence's user interface.
                QWidget *ui = s->CreateUI();
                if(ui != 0)
                {
                    // If there are not enough pages in the wizard, add a new page.
                    if(uiCount >= sequencePages.size())
                    {
                        SequenceUI uiInfo;
                        uiInfo.page = new QvisCustomWizardPage(this);
                        setPage(Page_Custom0 + sequencePages.size(), uiInfo.page);
                        sequencePages.push_back(uiInfo);
                    }

                    // Install our new UI in the most recent custom page.
                    sequencePages[uiCount].name = s->GetName();
                    sequencePages[uiCount].ui = ui;
                    sequencePages[uiCount].page->addWidget(ui);
                    sequencePages[uiCount].page->setTitle(vpName.c_str());
                    uiCount++;
                }

                // Log what happened with creating the UI.
                if(s->SupportsCustomUI())
                {
                    if(ui != 0)
                        debug3 << "Created";
                    else
                    {
                        debug3 << "FAILED to create";
                        if(badUIFiles.length() > 0)
                            badUIFiles += ", ";
                        badUIFiles += QString(s->GetUIFile().c_str());
                        ++noLoadCount;
                    }
                    debug3 << " user interface for sequence "
                           << s->GetName().c_str()
                           << " from user interface file "
                           << s->GetUIFile().c_str()
                           << endl;
                }
                else if(ui != 0)
                {
                    debug3 << "Created user interface for sequence "
                           << s->GetName().c_str() << endl;
                }
                else
                {
                    debug3 << "Sequence " << s->GetName().c_str()
                           << " does not provide a user interface."
                           << endl;
                }                        
            }
        }
    }

    // Tell the user about any loading errors that happened.
    if(noLoadCount > 0)
    {
        for(int i = 0; i < sequencePages.size(); ++i)
            delete sequencePages[i].ui;
        sequencePages.clear();

        QString msg2;
        QString msg2_sing(tr("The user interface file %1 could not be loaded."));
        QString msg2_plur(tr("The user interface files %1 could not be loaded."));
        msg2 = msg2_sing;
        if(noLoadCount > 1)
            msg2 = msg2_plur;
        msg2.replace("%1", badUIFiles);
        Warning(msg2);
    }
    else
    {
        // Update some page info.
        int nsp = NumSequencePages();
        for(int i = 0; i < nsp; ++i)
        {
            int nid;
            // Set up the page ordering for the custom pages.
            if(i < nsp-1)
                nid = Page_Custom0 + i + 1;
            else
                nid = Page_Formats;
            debug3 << "Setting custom page " << i << "'s nextId to " << nid << endl;
            sequencePages[i].page->setNextId(nid);
            sequencePages[i].page->setSubTitle(tr("%1 sequence").arg(sequencePages[i].name.c_str()));
        }

        // Update the default values in the new pages.
        UpdateCustomPagesWithDefaultValues();
    }

    return true;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::GetMovieAttsOutputDir
//
// Purpose: 
//   This retrieves the OutputDirectory from movieAtts, expanding '.' to
//   full path if necessary. 
//
// Programmer: Kathleen Bonnell 
// Creation:   July 20, 2007 
//
// Modifications:
//   Brad Whitlock, Tue Oct  7 13:30:37 PDT 2008
//   
// ****************************************************************************

std::string
QvisSaveMovieWizard::GetMovieAttsOutputDir()
{
    // If the path is set to the current directory then use Qt to expand
    // the path so we have the whole path.
    if(movieAtts->GetOutputDirectory() == ".")
    {
#if defined(_WIN32)
        std::string outDir = GetUserVisItDirectory();
#else
        QDir d(QDir::currentPath());
        d.makeAbsolute();
        std::string outDir(d.path().toStdString());
#endif
        if(outDir.size() > 0 && outDir[outDir.size() - 1] != SLASH_CHAR)
            outDir += SLASH_STRING;
        movieAtts->SetOutputDirectory(outDir);
    }
    return movieAtts->GetOutputDirectory();
}

//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisSaveMovieWizard::page0_movieTypeChanged
//
// Purpose: 
//   This Qt slot is called when the user changes the type of movie.
//
// Arguments:
//   val : The new movie type.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:21:58 PST 2006
//
// Modifications:
//   Dave Bremer, Wed Oct 10 15:50:03 PDT 2007
//   Added code to disable the start/end index fields if a template is 
//   used to make the movie.
// ****************************************************************************

void
QvisSaveMovieWizard::page0_movieTypeChanged(int val)
{
    const char *mName = "QvisSaveMovieWizard::page0_movieTypeChanged";
    decision_movieType = val;

    if(val == MOVIE_TYPE_SIMPLE)
    {
        debug1 << mName << "Chose simple movie." << endl;
        movieAtts->SetMovieType(MovieAttributes::Simple);
    }
    else if(val == MOVIE_TYPE_TEMPLATE)
    {
        debug1 << mName << "Chose template movie." << endl;
        movieAtts->SetMovieType(MovieAttributes::UsingTemplate);
    }
    else
    {
        debug1 << mName << "Chose last movie settings." << endl;
    }
    page10_UpdateStartEndIndex();
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page1_newTemplateChanged
//
// Purpose: 
//   This Qt slot function is called when the user picks a template 
//   generation method.
//
// Arguments:
//   val : The new template method.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:22:39 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page1_newTemplateChanged(int val)
{
    decision_templateUsage = val;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page2_selectedTemplateChanged
//
// Purpose: 
//   This Qt slot is called when the user picks a predefined movie template.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:23:21 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:28:16 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Oct 13 14:58:05 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page2_selectedTemplateChanged()
{
    std::string title;
    if(page2_templates->currentItem() != 0)
        title = page2_templates->currentItem()->text().toStdString();

    StringMovieTemplateDataMap::const_iterator it;
    it = templateTitleToInfo.find(title);
    if(it != templateTitleToInfo.end())
    {
        // Try and load the preview image...
        QPixmap pixmap;
        bool haveImage = false;
        if(it->second.info.previewImageFile != "")
        {
            QString imageFile(it->second.info.previewImageFile.c_str());
            haveImage = pixmap.load(imageFile);
        }

        // Display the description.
        if(it->second.info.description == "")
        {
            if(haveImage)
            {
                page2_template_image->setPixmap(pixmap);
                page2_template_description->hide();
                page2_template_image->show();
            }
            else
            {
                page2_template_description->setText(tr("No description available"));
                page2_template_description->show();
                page2_template_image->hide();
            }
        }
        else
        {
            if(haveImage)
            {
                page2_template_image->setPixmap(pixmap);
                page2_template_image->show(); 
            }
            else
                page2_template_image->hide(); 

            page2_template_description->setText(it->second.info.description.c_str());
            page2_template_description->show();
        }

        // Load the defaults file for the template since it will have
        // the names of the sequences and their custom UI files.
        LoadTemplateSpecification(it->second.info.specificationFile);

        // Save off the title of the currently selected movie template.
        currentMovieTemplate = title;
    }
}


//
// Page 3 slots
//

//
// Page 4 slots
//

void
QvisSaveMovieWizard::page4_viewportSelected()
{
    QList<QListWidgetItem *> sel = page4_viewportList->selectedItems();
    if(sel.count() > 0)
    {
        std::string name(sel[0]->text().toStdString());
        if(templateSpec->ViewportActivate(name))
            page4_UpdateViews(PAGE4_PROPERTIES | PAGE4_DISPLAY_SELECTION);
    }
}

void 
QvisSaveMovieWizard::page4_addViewport()
{
    // Use the display to add a viewport. This will cause it to emit a
    // signal that causes the page4_viewportAdded slot to be called.
    page4_viewportDisplay->addViewport(0.25, 0.25, 0.75, 0.75);
}


void 
QvisSaveMovieWizard::page4_deleteViewport()
{
    QList<QListWidgetItem *> sel = page4_viewportList->selectedItems();
    if(sel.count() > 0)
        page4_viewportDisplay->removeViewport(sel[0]->text());
}

void
QvisSaveMovieWizard::page4_viewportAdded(const QString &name, 
    float llx, float lly, float urx, float ury)
{
    // Add the viewport directly to the movie template configs.
    templateSpec->ViewportAdd(name.toStdString(), llx, lly, urx, ury);
}

void
QvisSaveMovieWizard::page4_viewportChanged(const QString &name,
    float llx, float lly, float urx, float ury)
{
    if(templateSpec->ViewportSetCoordinates(name.toStdString(),
        llx, lly, urx, ury))
    {
        page4_UpdateViews(PAGE4_PROPERTIES);
    }
}

void
QvisSaveMovieWizard::page4_viewportRemoved(const QString &name)
{
    // Remove the viewport.
    if(templateSpec->ViewportRemove(name.toStdString()))
        page4_UpdateViews(PAGE4_LIST | PAGE4_PROPERTIES);
}

void
QvisSaveMovieWizard::page4_viewportActivated(const QString &name)
{
    // set current item
    QList<QListWidgetItem *> items= page4_viewportList->findItems(name,Qt::MatchExactly);
    
    if(items.count() > 0)
        page4_viewportList->setCurrentItem(items[0]);
    
    if(templateSpec->ViewportActivate(name.toStdString()))
        page4_UpdateViews(PAGE4_LIST | PAGE4_PROPERTIES);
}

#define MIN_VAL(A,B) (((A) < (B)) ? (A) : (B))
#define MAX_VAL(A,B) (((A) > (B)) ? (A) : (B))

// ****************************************************************************
// Method: QvisSaveMovieWizard::page4_lowerLeftChanged
//
// Purpose: 
//   Called when the active viewport's lower left coordinate changes.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:27:22 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page4_lowerLeftChanged(double x, double y)
{
    float lower_left[2], upper_right[2];

    if(templateSpec->ViewportGetCoordinates(lower_left[0], lower_left[1],
        upper_right[0], upper_right[1]))
    {
        lower_left[0] = x;
        float tmp = MIN_VAL(lower_left[0], upper_right[0]);
        lower_left[0] = MIN_VAL(tmp, lower_left[0]);
        upper_right[0] = MAX_VAL(tmp, upper_right[0]);

        lower_left[1] = y;
        tmp = MIN_VAL(lower_left[1], upper_right[1]);
        lower_left[1] = MIN_VAL(tmp, lower_left[1]);
        upper_right[1] = MAX_VAL(tmp, upper_right[1]);

        // Store the modified coordinates back into the viewport.
        if(templateSpec->ViewportSetCoordinates(
            lower_left[0], lower_left[1],
            upper_right[0], upper_right[1]))
        {
            page4_UpdateViews(PAGE4_PROPERTIES | PAGE4_DISPLAY);
        }
    }
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page4_upperRightChanged
//
// Purpose: 
//   Called when the active viewport's upper right coordinate changes.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:26:56 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page4_upperRightChanged(double x, double y)
{
    float lower_left[2], upper_right[2];

    if(templateSpec->ViewportGetCoordinates(lower_left[0], lower_left[1],
        upper_right[0], upper_right[1]))
    {
        upper_right[0] = x;
        float tmp = MAX_VAL(lower_left[0], upper_right[0]);
        lower_left[0] = MIN_VAL(tmp, lower_left[0]);
        upper_right[0] = MAX_VAL(tmp, upper_right[0]);

        upper_right[1] = y;
        tmp = MIN_VAL(lower_left[1], upper_right[1]);
        lower_left[1] = MIN_VAL(tmp, lower_left[1]);
        upper_right[1] = MAX_VAL(tmp, upper_right[1]);

        // Store the modified coordinates back into the viewport.
        if(templateSpec->ViewportSetCoordinates(
            lower_left[0], lower_left[1],
            upper_right[0], upper_right[1]))
        {
            page4_UpdateViews(PAGE4_PROPERTIES | PAGE4_DISPLAY);
        }
    }
}

#undef MIN_VAL
#undef MAX_VAL

void
QvisSaveMovieWizard::page4_compositingModeChanged(int value)
{
    if(templateSpec->ViewportSetCompositing(value))
        page4_UpdateViews(PAGE4_PROPERTIES);
}

void
QvisSaveMovieWizard::page4_viewportOpacityChanged(int value)
{
    templateSpec->ViewportSetOpacity(float(value) / 100.);
}

void
QvisSaveMovieWizard::page4_viewportColorChanged(const QColor &c)
{
    int replaceColor[3];
    replaceColor[0] = c.red();
    replaceColor[1] = c.green();
    replaceColor[2] = c.blue();
    templateSpec->ViewportSetReplaceColor(replaceColor);
}

void
QvisSaveMovieWizard::page4_dropShadowChanged(bool val)
{
    templateSpec->ViewportSetDropShadow(val);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page4_usePredefinedViewports
//
// Purpose: 
//   Qt slot called when we pick a predefined viewport layout.
//
// Arguments:
//   index : The viewport layout.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:26:22 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Kathleen Bonnell, Wed Apr 30 11:05:45 PDT 2008 
//   Use char* for 'n' instead of QString, to prevent conversion between
//   QString and std::string, which cannot be done with Windows compiler.  
//
//   Cyrus Harrison, Fri Nov  7 16:14:21 PST 2008
//   Qt4 Refactor.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page4_usePredefinedViewports(int index)
{
    // Remove all of the existing viewports.
    templateSpec->ViewportRemoveAll();

    const char *n[4] = {"Viewport 1", 
                        "Viewport 2", 
                        "Viewport 3", 
                        "Viewport 4"};
    if(index == 0)
    {
        page4_viewportAdded(n[0], 0., 0., 1., 1.);
    }
    else if(index == 1)
    {
        page4_viewportAdded(n[0], 0., 0., 0.5, 1.);
        page4_viewportAdded(n[1], 0.5, 0., 1., 1.);
    }
    else if(index == 2)
    {
        page4_viewportAdded(n[0], 0., 0.5, 0.5, 1.);
        page4_viewportAdded(n[1], 0.5, 0.5, 1., 1.);
        page4_viewportAdded(n[2], 0., 0., 0.5, 0.5);
        page4_viewportAdded(n[3], 0.5, 0., 1., 0.5);
    }
    else if(index == 3)
    {
        page4_viewportAdded(n[0], 0., 0., 1., 1.);
        page4_viewportAdded(n[1], 0.6, 0.6, 0.95, 0.95);

        // Add drop shadow to viewport 2.
        templateSpec->ViewportSetDropShadow(n[1], true);
    }
    else if(index == 4)
    {
        page4_viewportAdded(n[0], 0., 0., 1., 1.);
        page4_viewportAdded(n[1], 0.6, 0.6, 0.95, 0.95);
        page4_viewportAdded(n[2], 0.6, 0.05, 0.95, 0.4);

        // Add drop shadow to viewport 2 and 3
        templateSpec->ViewportSetDropShadow(n[1], true);
        templateSpec->ViewportSetDropShadow(n[2], true);
    }
    else if(index == 5)
    {
        page4_viewportAdded(n[0], 0., 0., 0.5, 1.);
        page4_viewportAdded(n[1], 0.2, 0.7, 0.45, 0.95);

        page4_viewportAdded(n[2], 0.5, 0., 1., 1.);
        page4_viewportAdded(n[3], 0.7, 0.7, 0.95, 0.95);

        // Add drop shadow to viewport 2 and 4
        templateSpec->ViewportSetDropShadow(n[1], true);
        templateSpec->ViewportSetDropShadow(n[3], true);
    }

    page4_UpdateViews(-1);
}

//
// Page 5 slots
//

void
QvisSaveMovieWizard::page5_selectedSequenceChanged()
{
    if(page5_sequenceList->currentItem() != 0)
    {
        std::string seqName(page5_sequenceList->currentItem()->text().toStdString());
        if(templateSpec->SequenceActivate(seqName))
            page5_Update(PAGE5_UPDATE_SEQUENCE_PROPS);
    }
}

void
QvisSaveMovieWizard::page5_newSequenceClicked(int id)
{
    std::string newName;
    if(templateSpec->SequenceAdd(id, newName))
        page5_Update(PAGE5_UPDATE_SEQUENCE_LIST | PAGE5_UPDATE_SEQUENCE_PROPS);
}

void
QvisSaveMovieWizard::page5_deleteSequenceClicked()
{
    MovieSequence *s = templateSpec->GetActiveSequence();
    if(s != 0 && 
       !s->ReadOnly() &&
       templateSpec->SequenceRemove(s->GetName()))
    {
        page5_Update(PAGE5_UPDATE_SEQUENCE_LIST | 
                     PAGE5_UPDATE_SEQUENCE_PROPS |
                     PAGE5_UPDATE_SEQVIEW_MAPPING);
    }
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page5_typedNewSequenceName
//
// Purpose: 
//   Qt slot called when we rename a sequence.
//
// Arguments:
//   newName : The new name of the sequence.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:25:36 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page5_typedNewSequenceName(const QString &newName)
{
    int flags = 0;

    if(newName.isNull())
    {
        // Don't allow NULL names
        flags = PAGE5_UPDATE_SEQUENCE_LIST | 
                PAGE5_UPDATE_SEQUENCE_PROPS |
                PAGE5_UPDATE_SEQVIEW_MAPPING;
    }
    else
    {
        MovieSequence *s = templateSpec->GetActiveSequence();
        if(s != 0)
        {
            if(templateSpec->SequenceRename(s->GetName(), newName.toStdString()))
            {
                flags = PAGE5_UPDATE_SEQUENCE_LIST | 
                        PAGE5_UPDATE_SEQVIEW_MAPPING;
            }            
        }
        else
            flags = PAGE5_UPDATE_SEQUENCE_PROPS;
    }

    page5_Update(flags);
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::page5_destinationViewportChanged
//
// Purpose: 
//   Qt slot called when we map a sequence to a viewport.
//
// Arguments:
//   vpt : The viewport to which we're mapping the active sequence.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:24:57 PST 2006
//
// Modifications:
//   Brad Whitlock, Fri Oct 17 16:25:10 PDT 2008
//   Adjust for the index change.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page5_destinationViewportChanged(int index)
{
    int vpt = index - 1;
    MovieSequence *s = templateSpec->GetActiveSequence();
    if(s != 0)
    {
        std::string seqName(s->GetName()), vpName;

        // Unmap the sequence from any viewport.
        bool b1 = templateSpec->SequenceUnmap(s->GetName());

        // Try and map the sequence to the viewport.
        bool b2 = false;
        if(vpt >= 0)
        {
            if(templateSpec->ViewportGetNameForIndex(vpt, vpName))
            {
                b2 = templateSpec->SequenceMapToViewport(
                    s->GetName(), vpName);
            }
        }

        // If we modified the mapping then update the display
        if(b1 || b2)
        {
            page5_Update(PAGE5_UPDATE_SEQVIEW_MAPPING);
        }
    }
}

bool
QvisSaveMovieWizard::page5_updatedMappingEx(const QString &vp,
    const QStringList &sList)
{
    std::string vpName(vp.toStdString());
    stringVector seqList;
    for(int i = 0; i < sList.size(); ++i)
        seqList.push_back(std::string(sList[i].toStdString()));

    return templateSpec->ViewportSetSequenceList(vpName, seqList);
}

void
QvisSaveMovieWizard::page5_updatedMapping(const QString &vp1,
    const QStringList &seqList1)
{
    page5_updatedMappingEx(vp1, seqList1);
}

void
QvisSaveMovieWizard::page5_updatedMapping(const QString &vp1,
    const QStringList &seqList1, const QString &vp2,
    const QStringList &seqList2)
{
    bool b1 = page5_updatedMappingEx(vp1, seqList1);
    bool b2 = page5_updatedMappingEx(vp2, seqList2);
    if(b1 || b2)
        page5_Update(PAGE5_UPDATE_SEQUENCE_PROPS);
}

// Called when return is pressed in the uifile line edit.
void
QvisSaveMovieWizard::page5_sequenceUIFileChanged()
{
    QString s(page5_sequenceUIFile->displayText().simplified());
    if(!s.isEmpty())
    {
        MovieSequence *m = templateSpec->GetActiveSequence();
        if(m != 0 && m->SupportsCustomUI())
            m->SetUIFile(s.toStdString());
    }
    page5_Update(PAGE5_UPDATE_SEQUENCE_PROPS);
}

//
// Page 6 slots
//

void
QvisSaveMovieWizard::page6_saveAsTemplateChanged(int val)
{
    decision_saveTemplate = (val == 0);
//    UpdatePageLinking(6);
}

//
// Page 7 slots
//

void
QvisSaveMovieWizard::page7_templateNameChanged(const QString &s)
{
    templateSpec->SetTitle(s.toStdString());
}

void
QvisSaveMovieWizard::page7_templateDescriptionChanged()
{
    std::string s(page7_templateDescription->toPlainText().toStdString());
    templateSpec->SetDescription(s);
}

void
QvisSaveMovieWizard::page7_templateFileChanged(const QString &s)
{
    bool ret = templateSpec->SetTemplateFile(std::string(s.toStdString()));
}

void
QvisSaveMovieWizard::page7_previewImageFileChanged(const QString &s)
{
    templateSpec->SetPreviewImageFile(s.toStdString());
}

//
// Page 8 slots
//

void
QvisSaveMovieWizard::page8_settingsOkayChanged(int val)
{
    decision_settingsOkay = (val == 0);
    page8->setFinalPage(decision_settingsOkay);
    if(!decision_settingsOkay)
        next();
}

void
QvisSaveMovieWizard::page8_delayedUpdate()
{
    page8_settingsOkayChanged(decision_settingsOkay?0:1);
}

//
// Page 9 slots
//

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
//   Brad Whitlock, Fri Oct 20 11:59:53 PDT 2006
//   Added stereo.
//
//   Brad Whitlock, Wed Oct  8 10:47:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page9_addOutput()
{
    // Determine the movie format from the menu name.
    QString menuName(page9_formatComboBox->currentText());
    std::string format(MenuNameToFormat(menuName.toStdString().c_str()));
    int w = page9_widthSpinBox->value();
    int h = page9_heightSpinBox->value();
    double scale = double(page9_scaleSpinBox->value());
    int id = page9_sizeTypeButtonGroup->checkedId();
    bool stereo = page9_stereoCheckBox->isChecked();
    int stereoType = page9_stereoType->currentIndex();

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

    int mStereo = stereo ? (stereoType+1) : 0;
    movieAtts->GetStereoFlags().push_back(mStereo);
    movieAtts->SelectStereoFlags();

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
//   Brad Whitlock, Fri Oct 20 12:00:27 PDT 2006
//   Added stereo.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Oct  8 10:50:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page9_removeOutput()
{
    QTreeWidgetItem *item = page9_outputFormats->currentItem();
    if(item != 0)
    {
        // Remove the selected value from the movie atts.
        stringVector &formats = movieAtts->GetFileFormats();
        unsignedCharVector &useCurrents = movieAtts->GetUseCurrentSize();
        doubleVector &scales = movieAtts->GetScales();
        intVector &widths = movieAtts->GetWidths();
        intVector &heights = movieAtts->GetHeights();
        intVector &stereoFlags = movieAtts->GetStereoFlags();

        int i, w, h, mStereo;
        bool  useCurrent;
        float scale;
        bool  deleted = false;
        for(i = 0; i < formats.size(); ++i)
        {
            QString fmt(FormatToMenuName(formats[i].c_str()));
            QString res;  res.sprintf("%dx%d", widths[i], heights[i]);
            QString res2; 
            res2.sprintf(" %dx", int(scales[i]));
            res2 = tr("Current") + res2;
            QString stereo(tr("off"));
            if(stereoFlags[i] == 1)
                stereo = tr("Left/Right");
            else if(stereoFlags[i] == 2)
                stereo = tr("Red/Blue");
            else if(stereoFlags[i] == 3)
                stereo = tr("Red/Green");
            if(fmt == item->text(0) && 
               (res == item->text(1) || res2 == item->text(1)) &&
               stereo == item->text(2))
            {
                w = widths[i];
                h = heights[i];
                scale = (float)scales[i];
                mStereo = stereoFlags[i];

                useCurrent = useCurrents[i]>0;
                stringVector::iterator si = formats.begin();
                intVector::iterator    ii1 = widths.begin();
                intVector::iterator    ii2 = heights.begin();
                doubleVector::iterator ii3 = scales.begin();
                unsignedCharVector::iterator  ii4 = useCurrents.begin();
                intVector::iterator    ii5 = stereoFlags.begin();
                for(int j = 0; j < i; ++j)
                { ++si; ++ii1; ++ii2; ++ii3; ++ii4; ++ii5;}

                formats.erase(si);
                widths.erase(ii1);
                heights.erase(ii2);
                scales.erase(ii3);
                useCurrents.erase(ii4);
                stereoFlags.erase(ii5);

                movieAtts->SelectFileFormats();
                movieAtts->SelectWidths();
                movieAtts->SelectHeights();
                movieAtts->SelectScales();
                movieAtts->SelectUseCurrentSize();
                movieAtts->SelectStereoFlags();

                deleted = true;
                break;
            }
        }

        if(deleted)
        {
            // Populate the format and resolution widgets.
            page9_UpdateFormat(item->text(0));
            page9_UpdateResolution(useCurrent, scale, w, h, mStereo);
            page9_UpdateOutputs();
        }
    }
}

void
QvisSaveMovieWizard::page9_stereoChanged(bool val)
{
    page9_stereoLabel->setEnabled(val);
    page9_stereoType->setEnabled(val);
}

void
QvisSaveMovieWizard::page9_stereoTypeChanged(int val)
{

}

//
// Page 10 slots
//

// ****************************************************************************
// Method: QvisSaveMovieWizard::page10_fpsChanged
//
// Purpose: 
//   This is a Qt slot function invoked when any change occurs in the FPS field
//
// Programmer: Dave Bremer
// Creation:   Tue Oct  9 18:40:06 PDT 2007
//
// ****************************************************************************

void
QvisSaveMovieWizard::page10_fpsChanged(const QString &s)
{
    bool okay = true;
    int newFPS = s.toInt(&okay);
    if (okay)
        movieAtts->SetFps(newFPS);
    else
        movieAtts->SetFps(10);
}


// ****************************************************************************
// Method: QvisSaveMovieWizard::page10_startIndexChanged
//
// Purpose: 
//   This is a Qt slot function invoked when any change occurs in the 
//   start index field.
//
// Programmer: Dave Bremer
// Creation:   Tue Oct  9 18:40:06 PDT 2007
//
// ****************************************************************************

void
QvisSaveMovieWizard::page10_startIndexChanged(const QString &s)
{
    bool okay = true;
    int newStartIndex = s.toInt(&okay);

    if (okay)
        movieAtts->SetStartIndex(newStartIndex);
    else
        movieAtts->SetStartIndex(0);
}


// ****************************************************************************
// Method: QvisSaveMovieWizard::page10_endIndexChanged
//
// Purpose: 
//   This is a Qt slot function invoked when any change occurs in the 
//   end index field.
//
// Programmer: Dave Bremer
// Creation:   Tue Oct  9 18:40:06 PDT 2007
//
// ****************************************************************************

void
QvisSaveMovieWizard::page10_endIndexChanged(const QString &s)
{
    bool okay = true;
    int newEndIndex = s.toInt(&okay);

    if (okay)
        movieAtts->SetEndIndex(newEndIndex);
    else
        movieAtts->SetEndIndex(default_num_frames - 1);
}


//
// Page 11 slots
//

void
QvisSaveMovieWizard::page11_processOutputDirectoryText(const QString &s)
{
    std::string outDir(s.toStdString());
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
//   Kathleen Bonnell, Fri Jul 20 11:07:11 PDT 2007
//   Use new method to get the outputdirectory from movie atts.
//   
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Moved pages 10-12 to 11-13.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Wed Oct  8 17:00:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page11_selectOutputDirectory()
{
    //
    // Try and get a directory using a file dialog.
    //
    QString initialDir(GetMovieAttsOutputDir().c_str());
    QString dirName = QFileDialog::getExistingDirectory(this, 
        tr("Select output directory"), initialDir);

    //
    // If a directory was chosen, use it as the output directory.
    //
    if(!dirName.isEmpty())
    {
        std::string outDir(dirName.toStdString());
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
    movieAtts->SetOutputName(s.toStdString());
    page11_UpdateButtons();
}

//
// Page 12 slots
//

void
QvisSaveMovieWizard::page12_emailNotificationChanged(int val)
{
    movieAtts->SetSendEmailNotification(val==0);
    page12_UpdateButtons();
}

void
QvisSaveMovieWizard::page12_emailAddressChanged(const QString &val)
{
    movieAtts->SetEmailAddress(std::string(val.toStdString()));
    page12_UpdateButtons();
}

//
// Page 13 slots
//

void
QvisSaveMovieWizard::page13_generationMethodChanged(int val)
{
    movieAtts->SetGenerationMethod((MovieAttributes::GenerationMethodEnum)val);
}

//
// Other classes
//

QvisSaveMovieWizard::QvisCustomWizardPage::QvisCustomWizardPage(QWidget *parent) : 
    QWizardPage(parent)
{
    nid = -1;
    vlayout = new QVBoxLayout(this);
    scroll = new QScrollArea(this);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    vlayout->addWidget(scroll);
}

QvisSaveMovieWizard::QvisCustomWizardPage::~QvisCustomWizardPage()
{
}

void
QvisSaveMovieWizard::QvisCustomWizardPage::addWidget(QWidget *ui)
{
    ui->setParent(this);
    scroll->setWidget(ui);
}

void
QvisSaveMovieWizard::QvisCustomWizardPage::removeWidget(QWidget *ui)
{
    scroll->takeWidget();
    ui->setParent(0);
}

void
QvisSaveMovieWizard::QvisCustomWizardPage::setNextId(int n)
{
    nid = n;
}

int
QvisSaveMovieWizard::QvisCustomWizardPage::nextId() const
{
    return nid;
}
