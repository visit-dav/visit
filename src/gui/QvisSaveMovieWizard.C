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

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qframe.h>
#include <qhbox.h>
#include <qiconview.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollview.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qwidgetfactory.h>

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
// Wizard page information.
//
struct wizard_page_info
{
    QString title;
    QString description;
};

#define N_WIZARD_PAGES 14
wizard_page_info pageInfo[N_WIZARD_PAGES];

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
    {MPEG_FORMAT,       "mpeg"}
#if !defined(_WIN32)
   ,{"Quicktime movie", "qt"},
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
    debug1 << "EnsureDirectoryExists: dir=" << dirName.latin1()
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
// ****************************************************************************

QvisSaveMovieWizard::QvisSaveMovieWizard(AttributeSubject *atts, QWidget *parent, 
    const char *name) : QvisWizard(atts, parent, name), templateTitleToInfo(),
    sequencePages()
{
    // Populate the pageInfo with translations.
    pageInfo[0].title = tr("Movie type");
    pageInfo[0].description = tr("Would you like to create a new simple movie or use your previous settings?");
    pageInfo[1].title = tr("Template uage");
    pageInfo[1].description = tr("Movie templates allow you to create complex movies. What would you like to do?");
    pageInfo[2].title = tr("Choose template");
    pageInfo[2].description = tr("Choose a movie template.");
    pageInfo[3].title = tr("Update sources");
    pageInfo[3].description = tr("Make sure that the sources used in this template are up to date. You can change the sources here to make a movie using a template that was created with other data.");
    pageInfo[4].title = tr("Viewports");
    pageInfo[4].description = tr("Change the movie template's viewports. Viewports are regions of the final movie image to which VisIt's visualization windows can be mapped.");
    pageInfo[5].title = tr("Sequences");
    pageInfo[5].description = tr("Create new sequences of frames and map them to movie template viewports.");
    pageInfo[6].title = tr("Save new template");
    pageInfo[6].description = tr("Save this movie as a new template?");
    pageInfo[7].title = tr("Save new template as");
    pageInfo[7].description = tr("Enter the information that will be saved with your template.");
    pageInfo[8].title = tr("Settings check");
    pageInfo[8].description = tr("Do these movie settings look okay?");
    pageInfo[9].title = tr("Choose format");
    pageInfo[9].description = tr("Choose movie formats and resolutions.");
    pageInfo[10].title = tr("Choose length");
    pageInfo[10].description = tr("Choose movie start/end time and frames per second.");
    pageInfo[11].title = tr("Choose filename");
    pageInfo[11].description = tr("Choose the output directory and base filename for your movie(s).");
    pageInfo[12].title = tr("E-mail notification");
    pageInfo[12].description = tr("Do you want to be notified by E-mail when your movie completes?");
    pageInfo[13].title = tr("Choose method");
    pageInfo[13].description = tr("Choose when and how you would like VisIt to create your movies.");

    default_movie_size[0] = 400;
    default_movie_size[1] = 400;
    default_num_frames = 1;
    removingPages = false;

    //
    // Decision variables that influence which path is taken through the wizard.
    //
    decision_movieType = MOVIE_TYPE_SIMPLE;
    decision_templateUsage = 0;
    decision_settingsOkay = false;
    decision_saveTemplate = false;

    //
    // Internal state for certain pages.
    //
    page0_usePreviousSettingsAllowed = (movieAtts->GetFileFormats().size() > 0);
    page2_templatesPopulated = false;
    page9_aspect = 1.;

    sequencePagesAdded = false;
    templateSpec = 0;
    currentMovieTemplate = "";

    connect(this, SIGNAL(selected(const QString &)),
            this, SLOT(pageChanged(const QString &)));

    // Set the wizard's title.
    topLevelWidget()->setCaption(tr("Save movie wizard"));

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
// ****************************************************************************

QvisSaveMovieWizard::~QvisSaveMovieWizard()
{
    // Delete parentless button groups.
    delete page0_buttongroup;
    delete page1_buttongroup;
    delete page4_compositingMode;
    delete page6_buttongroup;
    delete page8_buttongroup;
    delete page9_sizeTypeButtonGroup;
    delete page12_buttongroup;
    delete page13_buttongroup;

    // Delete any custom pages that may be loaded.
    for(int i = 0; i < sequencePages.size(); ++i)
        delete sequencePages[i].ui;
    sequencePages.clear();

    if(templateSpec != 0)
        delete templateSpec;
}

// ****************************************************************************
// Method: QvisSaveMovieWizard::showPage
//
// Purpose: 
//   This method is called before a page is shown and we use it to introduce
//   custom pages into the wizard when they need to appear.
//
// Arguments:
//   page : The page that is about to be shown.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:13:26 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::showPage(QWidget *page)
{
    if(!removingPages)
    {
        if(page == page5)
        {
            // If we are going backwards from a custom page to page 5 where the
            // sequence pages could be removed by changing sequence->viewport
            // mapping then we need to read the values from the custom pages
            // and commit their data into the template spec.
            UpdateDefaultValuesFromCustomPages();
        }
        else if(page == page6)
        {
            // If we're calling for page6 to be visible then we need to install any
            // sequence related input pages before we call it so we can change the
            // list of pages so the sequence pages show up before page 6.

            // Add the sequence pages if they have not been added.
            bool inserted = AddSequencePages();

            // Enable any sequence pages that have been added so they come before page 6.
            for(int i = 0; i < sequencePages.size(); ++i) 
                setAppropriate(sequencePages[i].ui, true);

            if(inserted && sequencePages.size() > 0)
                page = sequencePages[0].ui;
        }
        else if(page == page9 && decision_movieType == MOVIE_TYPE_TEMPLATE) //decision_templateUsage == 0)
        {
            // Add the sequence pages if they have not been added.
            bool inserted = AddSequencePages();

            // Enable any sequence pages that have been added so they come before page 5.
            for(int i = 0; i < sequencePages.size(); ++i) 
                setAppropriate(sequencePages[i].ui, true);

            if(inserted && sequencePages.size() > 0)
                page = sequencePages[0].ui;
        }
    }

    // Call the base class's show page.
    QvisWizard::showPage(page);
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
// ****************************************************************************

int
QvisSaveMovieWizard::Exec()
{
    // Set the enabled state on the "use previous settings" radio button.
    page0_r1->setEnabled(page0_usePreviousSettingsAllowed);

    UpdatePageLinking(0);
    showPage(page(0));
    raise();
    setActiveWindow();

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
//   Brad Whitlock, Mon Oct 2 11:08:48 PDT 2006
//   Added code to set the aspect in page4's viewport object.
//
// ****************************************************************************

void
QvisSaveMovieWizard::SetDefaultMovieSize(int w, int h)
{
    default_movie_size[0] = w;
    default_movie_size[1] = h;

    // Update page4's viewport object.
    double aspect = 1.;
    if(default_movie_size[0] != 0)
        aspect = double(default_movie_size[1]) / double(default_movie_size[0]);
    page4_viewportDisplay->setAspect(aspect);
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateMovieTypePage()
{
    QFrame *frame = new QFrame(this, pageInfo[0].title.ascii());
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
    page0_r1 = new QRadioButton(tr("Use my previous movie settings"),
        frame, "r1");
    page0_r1->setEnabled(false);
    page0_buttongroup->insert(page0_r1, 0);
    buttonLayout->addWidget(page0_r1);

    QRadioButton *r2 = new QRadioButton(tr("New simple movie"), frame, "r2");
    page0_buttongroup->insert(r2, 1);
    buttonLayout->addWidget(r2);
    QRadioButton *r3 = new QRadioButton(tr("New template movie"),
        frame, "r3");
    page0_buttongroup->insert(r3, 2);
    buttonLayout->addWidget(r3);

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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateNewTemplatePromptPage()
{
    QFrame *frame = new QFrame(this, pageInfo[1].title.ascii());
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[1].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(pageLayout);
    hCenterLayout->addStretch(5);
    QVBoxLayout *buttonLayout = new QVBoxLayout(hCenterLayout);
    hCenterLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page1_buttongroup = new QButtonGroup(0, "page1_buttongroup");
    QRadioButton *r1 = new QRadioButton(tr("Use existing template"),
        frame, "r1");
    page1_buttongroup->insert(r1, 0);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("Edit existing template"), frame, "r2");
    page1_buttongroup->insert(r2, 1);
    buttonLayout->addWidget(r2);
    QRadioButton *r3 = new QRadioButton(tr("Create new template"),
        frame, "r3");
    page1_buttongroup->insert(r3, 2);
    buttonLayout->addWidget(r3);

    pageLayout->addSpacing(10);
    pageLayout->addStretch(10);
    page1_buttongroup->setButton(0);
    connect(page1_buttongroup, SIGNAL(clicked(int)),
            this, SLOT(page1_newTemplateChanged(int)));

    // Add the page.
    page1 = frame;
    addPage(page1, pageInfo[1].title);
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateChooseTemplatePage()
{
    QFrame *frame = new QFrame(this, pageInfo[2].title.ascii());
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[2].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(pageLayout);
    pageLayout->setStretchFactor(hCenterLayout, 10);

    // The list of templates.
    page2_templates = new QListBox(frame, "templates");
    connect(page2_templates, SIGNAL(selectionChanged()),
            this, SLOT(page2_selectedTemplateChanged()));
    hCenterLayout->addWidget(page2_templates);
    hCenterLayout->setStretchFactor(page2_templates, 10);

    QVBox *page2_description_vbox = new QVBox(frame, "page2_description_vbox");
    page2_description_vbox->setSpacing(10);
    hCenterLayout->addWidget(page2_description_vbox);
    hCenterLayout->setStretchFactor(page2_description_vbox, 10);

    // Add a picture of the template
    page2_template_image = new QLabel(tr("No preview"), page2_description_vbox, 
        "page2_template_image");
    page2_template_image->setMinimumSize(200,200);
    page2_template_image->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    page2_description_vbox->setStretchFactor(page2_template_image, 10);

    // Add the description.
    page2_template_description = new QTextEdit(page2_description_vbox,
        "page2_template_description");
    page2_template_description->setReadOnly(true);

    // Add the page.
    page2 = frame;
    addPage(page2, pageInfo[2].title);
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
//   
// ****************************************************************************

void
QvisSaveMovieWizard::CreateChooseNewSourcesPage()
{
    QFrame *frame = new QFrame(this, pageInfo[3].title.ascii());
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(10);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[3].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    page3_sessionSources = new QvisSessionSourceChanger(frame, "page3_sessionSources");
    pageLayout->addWidget(page3_sessionSources);

    // Add the page.
    page3 = frame;
    addPage(page3, pageInfo[3].title);
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateViewportPage()
{
    QFrame *frame = new QFrame(this, pageInfo[4].title.ascii());
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(10);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[4].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(pageLayout);
    pageLayout->setStretchFactor(hCenterLayout, 10);

    // Do the left side where we have all of the viewport controls.
    QGridLayout *leftLayout = new QGridLayout(hCenterLayout, 3, 3);
    leftLayout->setSpacing(10);
 
    // The viewport list.
    page4_viewportList = new QListBox(frame, "page4_viewportList");
    connect(page4_viewportList, SIGNAL(selectionChanged()),
            this, SLOT(page4_viewportSelected()));
    leftLayout->addMultiCellWidget(page4_viewportList, 0, 0, 0, 2);

    // The buttons to add, remove viewports (also change layouts)
    QPushButton *page4_addViewportButton = new QPushButton(tr("New"), 
        frame, "page4_addViewportButton");
    connect(page4_addViewportButton, SIGNAL(clicked()),
            this, SLOT(page4_addViewport()));
    leftLayout->addWidget(page4_addViewportButton, 1, 0);

    page4_deleteViewportButton = new QPushButton(tr("Delete"), 
        frame, "page4_deleteViewportButton");
    connect(page4_deleteViewportButton, SIGNAL(clicked()),
            this, SLOT(page4_deleteViewport()));
    leftLayout->addWidget(page4_deleteViewportButton, 1, 1);

    QvisPredefinedViewports *page4_predef = new QvisPredefinedViewports(
        frame, "page4_predef");
    connect(page4_predef, SIGNAL(activated(int)),
            this, SLOT(page4_usePredefinedViewports(int)));
    leftLayout->addWidget(page4_predef, 1, 2);

    // Add the frame stuff at the bottom to show the viewport properties.
    QGroupBox *viewportProps = new QGroupBox(frame, "viewportProps");
    viewportProps->setTitle(tr("Viewport properties"));
    leftLayout->addMultiCellWidget(viewportProps, 2, 2, 0, 2);
    QVBoxLayout *viewportPropsinnerLayout = new QVBoxLayout(viewportProps);
    viewportPropsinnerLayout->setMargin(10);
    viewportPropsinnerLayout->addSpacing(10);
    QGridLayout *viewportLayout = new QGridLayout(viewportPropsinnerLayout, 8, 2);
    viewportLayout->setSpacing(5);

    page4_lowerLeft = new QvisScreenPositionEdit(viewportProps, "page4_lowerLeft");
    connect(page4_lowerLeft, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(page4_lowerLeftChanged(double,double)));
    QLabel *page4_lowerLeftLabel = new QLabel(page4_lowerLeft, tr("Lower left"),
        viewportProps, "page4_lowerLeftLabel");
    viewportLayout->addWidget(page4_lowerLeftLabel, 0, 0);
    viewportLayout->addWidget(page4_lowerLeft, 0, 1);

    page4_upperRight = new QvisScreenPositionEdit(viewportProps, "page4_upperRight");
    connect(page4_upperRight, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(page4_upperRightChanged(double,double)));
    QLabel *page4_upperRightLabel = new QLabel(page4_upperRight, tr("Upper right"),
        viewportProps, "page4_upperRightLabel");
    viewportLayout->addWidget(page4_upperRightLabel, 1, 0);
    viewportLayout->addWidget(page4_upperRight, 1, 1);

    viewportLayout->addWidget(new QLabel(tr("Compositing"), viewportProps), 2, 0);

    page4_compositingMode = new QButtonGroup(0, "page4_compositingMode");
    connect(page4_compositingMode, SIGNAL(clicked(int)),
            this, SLOT(page4_compositingModeChanged(int)));
    QRadioButton *rb0 = new QRadioButton(tr("Overlay"), viewportProps, "Overlay");
    page4_compositingMode->insert(rb0, 0);
    viewportLayout->addWidget(rb0, 3, 0);
    QRadioButton *rb1 = new QRadioButton(tr("Blend"), viewportProps, "Blend");
    page4_compositingMode->insert(rb1, 1);
    viewportLayout->addWidget(rb1, 4, 0);
    QRadioButton *rb2 = new QRadioButton(tr("Replace color"), viewportProps, "ReplaceColor");
    page4_compositingMode->insert(rb2, 2);
    viewportLayout->addWidget(rb2, 5, 0);
    QRadioButton *rb3 = new QRadioButton(tr("Replace background color"), viewportProps, "ReplaeBGColor");
    page4_compositingMode->insert(rb3, 3);
    viewportLayout->addMultiCellWidget(rb3, 6, 6, 0, 1);

    page4_viewportOpacity = new QvisOpacitySlider(viewportProps, "page4_viewportOpacity");
    connect(page4_viewportOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(page4_viewportOpacityChanged(int)));
    viewportLayout->addWidget(page4_viewportOpacity, 4, 1);

    page4_viewportColor = new QvisColorButton(viewportProps, "page4_viewportColor");
    connect(page4_viewportColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(page4_viewportColorChanged(const QColor &)));
    viewportLayout->addWidget(page4_viewportColor, 5, 1);

    page4_dropShadow = new QCheckBox(tr("Add drop shadow"), viewportProps, "page4_dropShadow");
    connect(page4_dropShadow, SIGNAL(toggled(bool)),
            this, SLOT(page4_dropShadowChanged(bool)));
    viewportLayout->addMultiCellWidget(page4_dropShadow, 7, 7, 0, 1);
    

    // Add the viewport widget on the right.
    page4_viewportDisplay = new QvisViewportWidget(1., 320, 320, frame,
        "page4_viewportDisplay");
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
    page4 = frame;
    addPage(page4, pageInfo[4].title);    
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateSequencesPage()
{
    QFrame *frame = new QFrame(this, pageInfo[5].title.ascii());
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(10);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[5].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QGridLayout *gridLayout = new QGridLayout(pageLayout, 4, 4);
    gridLayout->setSpacing(5);

    
    page5_sequenceList = new QListBox(frame, "page5_sequenceList");
    connect(page5_sequenceList, SIGNAL(selectionChanged()),
            this, SLOT(page5_selectedSequenceChanged()));
    gridLayout->addMultiCellWidget(page5_sequenceList, 0, 0, 0, 1);

    QvisSequenceButton *newSequence = new QvisSequenceButton(frame, "newSequence");
    connect(newSequence, SIGNAL(activated(int)),
            this, SLOT(page5_newSequenceClicked(int)));
    gridLayout->addWidget(newSequence, 1, 0);

    page5_deleteSequence = new QPushButton(tr("Delete"), frame, "page5_deleteSequence");
    connect(page5_deleteSequence, SIGNAL(clicked()),
            this, SLOT(page5_deleteSequenceClicked()));
    gridLayout->addWidget(page5_deleteSequence, 1, 1);

    page5_sequenceProperties = new QGroupBox(tr("Sequence properties"), frame,
        "page5_sequenceProperties");
    gridLayout->addMultiCellWidget(page5_sequenceProperties, 0, 1, 2, 3);
    gridLayout->setRowStretch(0, 5);
    QVBoxLayout *seqPropinnerLayout = new QVBoxLayout(page5_sequenceProperties);
    seqPropinnerLayout->setMargin(10);
    seqPropinnerLayout->addSpacing(10);
    QGridLayout *seqPropLayout = new QGridLayout(seqPropinnerLayout, 3, 2);
    seqPropLayout->setSpacing(5);

    // Create the sequence name controls.
    page5_sequenceName = new QLineEdit(page5_sequenceProperties, "page5_sequenceName");
    connect(page5_sequenceName, SIGNAL(textChanged(const QString &)),
            this, SLOT(page5_typedNewSequenceName(const QString &)));
    page5_sequenceNameLabel = new QLabel(page5_sequenceName, tr("Name"),
        page5_sequenceProperties);
    seqPropLayout->addWidget(page5_sequenceNameLabel, 0, 0);
    seqPropLayout->addWidget(page5_sequenceName, 0, 1);

    // Create the combo box that lets us map the sequence to a viewport.
    page5_sequenceDestinationViewport = new QComboBox(page5_sequenceProperties,
        "page5_sequenceName");
    connect(page5_sequenceDestinationViewport, SIGNAL(activated(int)),
            this, SLOT(page5_destinationViewportChanged(int)));
    seqPropLayout->addWidget(page5_sequenceDestinationViewport, 1, 1);
    seqPropLayout->addWidget(new QLabel(page5_sequenceDestinationViewport, 
        tr("Map to viewport"), page5_sequenceProperties), 1, 0);

    // Create the uiFile name controls
    page5_sequenceUIFile = new QvisDialogLineEdit(page5_sequenceProperties,
        "page5_sequenceUIFile");
    page5_sequenceUIFile->setDialogMode(QvisDialogLineEdit::ChooseLocalFile);
    connect(page5_sequenceUIFile, SIGNAL(returnPressed()),
            this, SLOT(page5_sequenceUIFileChanged()));
    page5_sequenceUIFile->setDialogFilter(tr("User interface (*.ui)"));
    page5_sequenceUIFile->setDialogCaption(tr("Get User Interface filename"));

    page5_sequenceUILabel = new QLabel(page5_sequenceUIFile,
        "User interface", page5_sequenceProperties, "page5_sequenceUILabel");
    QToolTip::add(page5_sequenceUILabel, tr("User interface definition file."));

    seqPropLayout->addWidget(page5_sequenceUILabel, 2, 0);
    seqPropLayout->addWidget(page5_sequenceUIFile, 2, 1);

    // Create the sequence view that lets us order sequences in a viewport.
    page5_sequenceView = new QvisSequenceView(frame, "page5_sequenceView");
    connect(page5_sequenceView,
            SIGNAL(updatedMapping(const QString &,const QStringList &)),
            this,
            SLOT(page5_updatedMapping(const QString &,const QStringList &)));
    connect(page5_sequenceView,
            SIGNAL(updatedMapping(const QString &,const QStringList &,const QString &,const QStringList &)),
            this,
            SLOT(page5_updatedMapping(const QString &,const QStringList &,const QString &,const QStringList &)));

    QLabel *seqOrderLabel = new QLabel(page5_sequenceView, tr("Sequence to viewport mapping"),
        frame, "seqOrderLabel");
    gridLayout->addMultiCellWidget(seqOrderLabel, 2, 2, 0, 3);
    gridLayout->addMultiCellWidget(page5_sequenceView, 3,3,0,3);
    gridLayout->setRowStretch(3, 15);

    // Add the page.
    page5 = frame;
    addPage(page5, pageInfo[5].title);    
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateSaveTemplateAsPage()
{
    QFrame *frame = new QFrame(this, pageInfo[7].title.ascii());
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(10);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[7].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QGridLayout *gLayout = new QGridLayout(pageLayout, 4, 2);
    gLayout->setSpacing(5);
    pageLayout->addStretch(20);

    //
    // Create the template title
    //
    QLabel *titleLabel = new QLabel(tr("Title"),
        frame, "titleLabel");
    gLayout->addWidget(titleLabel, 0, 0);
    page7_templateName = new QLineEdit(frame, "page7_templateName");
    connect(page7_templateName, SIGNAL(textChanged(const QString &)),
            this, SLOT(page7_templateNameChanged(const QString &)));    
    gLayout->addWidget(page7_templateName, 0, 1);

    //
    // Create the template description
    //
    QLabel *descriptionLabel = new QLabel(tr("Description"),
        frame, "descriptionLabel");
    gLayout->addWidget(descriptionLabel, 1, 0);
    page7_templateDescription = new QTextEdit(frame, "page7_templateDescription");
    connect(page7_templateDescription, SIGNAL(textChanged()),
            this, SLOT(page7_templateDescriptionChanged()));    
    gLayout->addWidget(page7_templateDescription, 1, 1, Qt::AlignTop);

    //
    // Create the template file controls.
    //
    QLabel *filenameLabel = new QLabel(tr("Template filename"),
        frame, "filenameLabel");
    gLayout->addWidget(filenameLabel, 2, 0);
    page7_templateFile = new QvisDialogLineEdit(frame, "page7_templateDescription");
    page7_templateFile->setDialogMode(QvisDialogLineEdit::ChooseLocalFile);
    page7_templateFile->setDialogCaption(tr("Choose template filename"));
    page7_templateFile->setDialogFilter("*");
    connect(page7_templateFile, SIGNAL(textChanged(const QString &)),
            this, SLOT(page7_templateFileChanged(const QString &)));    
    gLayout->addWidget(page7_templateFile, 2, 1);

    //
    // Create the template file controls.
    //
    QLabel *previewImageLabel = new QLabel("Preview image filename",
        frame, "previewImageLabel");
    gLayout->addWidget(previewImageLabel, 3, 0);
    page7_previewImageFile = new QvisDialogLineEdit(frame, "page7_previewImageFile");
    page7_previewImageFile->setDialogMode(QvisDialogLineEdit::ChooseLocalFile);
    page7_previewImageFile->setDialogCaption(tr("Choose preview image filename"));
    page7_previewImageFile->setDialogFilter("XPM image (*.xpm)");
    connect(page7_previewImageFile, SIGNAL(textChanged(const QString &)),
            this, SLOT(page7_previewImageFileChanged(const QString &)));    
    gLayout->addWidget(page7_previewImageFile, 3, 1);

    // Add the page.
    page7 = frame;
    addPage(page7, pageInfo[7].title);
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateSettingsOkayPage()
{
    QFrame *frame = new QFrame(this, pageInfo[8].title.ascii());
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

    QRadioButton *r1 = new QRadioButton(tr("Yes"),
        frame, "r1");
    page8_buttongroup->insert(r1);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("No"),
        frame, "r2");
    page8_buttongroup->insert(r2);
    buttonLayout->addWidget(r2);

    buttonLayout->addStretch(5);
    pageLayout->addSpacing(10);

    //
    // Create the output formats list.
    //
    QGroupBox *settingsGroup = new QGroupBox(frame, "settingsGroup");
    settingsGroup->setTitle(tr("Movie settings"));
    pageLayout->addWidget(settingsGroup, 10);
    QVBoxLayout *f3innerLayout = new QVBoxLayout(settingsGroup);
    f3innerLayout->setMargin(10);
    f3innerLayout->addSpacing(10);
    QVBoxLayout *f3layout = new QVBoxLayout(f3innerLayout);
    f3layout->setSpacing(5);

    page8_settingsListView = new QListView(settingsGroup,
        "page8_settingsListView");
    page8_settingsListView->addColumn(tr("Settings"));
    page8_settingsListView->addColumn(tr("Values"));
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
//   Hank Childs, Thu Jun  8 13:48:28 PDT 2006
//   Fix compiler warning for casting.
//
//   Brad Whitlock, Tue Oct 10 10:52:03 PDT 2006
//   Made it be page9 and I merged the stereo controls into this page.
//
//   Brad Whitlock, Tue Apr  8 16:08:04 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisSaveMovieWizard::CreateFormatPage()
{
    QFrame *frame = new QFrame(this, pageInfo[9].title.ascii());
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
    formatAndResolution->setTitle(tr("Format and resolution"));
    hCenterLayout->addWidget(formatAndResolution, 10);
    QVBoxLayout *f2innerLayout = new QVBoxLayout(formatAndResolution);
    f2innerLayout->setMargin(10);
    f2innerLayout->addSpacing(15);
    QGridLayout *f2layout = new QGridLayout(f2innerLayout, 10, 3);
    f2layout->setSpacing(5);
    f2innerLayout->addStretch(10);

    page9_formatComboBox = new QComboBox(formatAndResolution, "page9_formatComboBox");
    // Add all of the movie formats from the table.
    for(int i = 0; i < N_MOVIE_FORMATS; ++i)
        page9_formatComboBox->insertItem(movieFormatInfo[i].menu_name, i);
    page9_formatComboBox->setCurrentItem(6);
    QLabel *formatLabel = new QLabel(page9_formatComboBox, tr("Format"),
        formatAndResolution, "formatLabel");
    f2layout->addMultiCellWidget(page9_formatComboBox, 0, 0, 1, 2);
    f2layout->addWidget(formatLabel, 0, 0);
    QFrame *hline1 = new QFrame(formatAndResolution, "hline1");
    hline1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    f2layout->addMultiCellWidget(hline1, 1, 1, 0, 2);
    f2layout->addRowSpacing(1, 10);

    page9_sizeTypeButtonGroup = new QButtonGroup(0, "page9_sizeTypeButtonGroup");
    connect(page9_sizeTypeButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(page9_sizeTypeChanged(int)));
    QRadioButton *rb = new QRadioButton(tr("Use current window size"),
        formatAndResolution, "winSize");
    page9_sizeTypeButtonGroup->insert(rb, 0);
    f2layout->addMultiCellWidget(rb, 2, 2, 0, 2);

    page9_scaleSpinBox = new QSpinBox(1, 10, 1, formatAndResolution,
        "page9_scaleSpinBox");
    connect(page9_scaleSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(page9_scaleChanged(int)));
    page9_scaleLabel = new QLabel(page9_scaleSpinBox, tr("Scale"),
        formatAndResolution, "scaleLabel");
    f2layout->addWidget(page9_scaleLabel, 3, 0);
    f2layout->addWidget(page9_scaleSpinBox, 3, 1);
    
    rb = new QRadioButton(tr("Specify movie size"), formatAndResolution, "winSize2");
    page9_sizeTypeButtonGroup->insert(rb, 1);
    f2layout->addMultiCellWidget(rb, 4, 4, 0, 2);

    page9_widthSpinBox = new QSpinBox(32, 4086, 1, formatAndResolution,
        "page9_widthSpinBox");
    page9_widthSpinBox->setValue((int)(default_movie_size[0]));
    page9_widthSpinBox->setEnabled(false);
    connect(page9_widthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(page9_widthChanged(int)));
    page9_widthLabel = new QLabel(page9_widthSpinBox, tr("Width"),
        formatAndResolution, "widthLabel");
    page9_widthLabel->setEnabled(false);
    f2layout->addWidget(page9_widthLabel, 5, 0);
    f2layout->addWidget(page9_widthSpinBox, 5, 1);
    
    page9_heightSpinBox = new QSpinBox(32, 4086, 1, formatAndResolution,
        "page9_heightSpinBox");
    page9_heightSpinBox->setValue((int)(default_movie_size[1]));
    page9_heightSpinBox->setEnabled(false);
    connect(page9_heightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(page9_heightChanged(int)));
    page9_heightLabel = new QLabel(page9_widthSpinBox, tr("Height"),
        formatAndResolution, "heightLabel");
    page9_heightLabel->setEnabled(false);
    f2layout->addWidget(page9_heightLabel, 6, 0);
    f2layout->addWidget(page9_heightSpinBox, 6, 1);

    page9_aspectLock = new QCheckBox(tr("lock aspect"),
        formatAndResolution, "page9_aspectLock");
    page9_aspectLock->setChecked(true);
    page9_aspectLock->setEnabled(false);
    connect(page9_aspectLock, SIGNAL(toggled(bool)),
            this, SLOT(page9_aspectLockChanged(bool)));
    f2layout->addMultiCellWidget(page9_aspectLock, 5,6,2,2);

    QFrame *hline2 = new QFrame(formatAndResolution, "hline2");
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    f2layout->addMultiCellWidget(hline2, 7, 7, 0, 2);
    f2layout->addRowSpacing(7, 10);

    page9_stereoCheckBox = new QCheckBox(tr("Stereo movie"), formatAndResolution,
        "page9_stereoCheckBox");
    connect(page9_stereoCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(page9_stereoChanged(bool)));
    f2layout->addMultiCellWidget(page9_stereoCheckBox,8,8,0,2);
    page9_stereoType = new QComboBox(formatAndResolution, "page9_stereoType");
    connect(page9_stereoType, SIGNAL(activated(int)),
            this, SLOT(page9_stereoTypeChanged(int)));
    page9_stereoType->insertItem(tr("Left/Right"));
    page9_stereoType->insertItem(tr("Red/Blue"));
    page9_stereoType->insertItem(tr("Red/Green"));
    page9_stereoLabel = new QLabel(page9_stereoType, tr("Stereo type"),
        formatAndResolution, "page9_stereoLabel");
    f2layout->addWidget(page9_stereoLabel,9,0);
    f2layout->addMultiCellWidget(page9_stereoType,9,9,1,2);

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
    outputGroup->setTitle(tr("Output"));
    hCenterLayout->addWidget(outputGroup, 10);
    QVBoxLayout *f3innerLayout = new QVBoxLayout(outputGroup);
    f3innerLayout->setMargin(10);
    f3innerLayout->addSpacing(10);
    QVBoxLayout *f3layout = new QVBoxLayout(f3innerLayout);
    f3layout->setSpacing(5);

    page9_outputFormats = new QListView(outputGroup, "page9_outputFormats");
    page9_outputFormats->setMinimumWidth(fontMetrics().
        boundingRect("Quicktime movie 2000x2000").width());
    page9_outputFormats->addColumn(tr("Format"));
    page9_outputFormats->addColumn(tr("Resolution"));
    page9_outputFormats->addColumn(tr("Stereo"));
    page9_outputFormats->setAllColumnsShowFocus(true);
    page9_outputFormats->setColumnAlignment(0, Qt::AlignHCenter);
    page9_outputFormats->setColumnAlignment(1, Qt::AlignHCenter);
    page9_outputFormats->setColumnAlignment(2, Qt::AlignHCenter);
    f3layout->addWidget(page9_outputFormats);

    // Add the page.
    page9 = frame;
    addPage(page9, pageInfo[9].title);
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateNumFramesPage()
{
    QFrame *frame = new QFrame(this, pageInfo[10].title.ascii());
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[10].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QGridLayout *gLayout = new QGridLayout(pageLayout, 3, 2);
    gLayout->setSpacing(5);
    pageLayout->addStretch(20);

    page10_fpsLabel = new QLabel(tr("Frames per second"), frame, "fpsLabel");
    gLayout->addWidget(page10_fpsLabel, 0, 0);

    page10_fpsLineEdit = new QLineEdit(frame, "fpsLineEdit");
    gLayout->addWidget(page10_fpsLineEdit, 0, 1);
    connect(page10_fpsLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page10_fpsChanged(const QString &)));    

    page10_startIndexLabel = new QLabel(tr("First frame"), frame, "startIndexLabel");
    gLayout->addWidget(page10_startIndexLabel, 1, 0);

    page10_startIndexLineEdit = new QLineEdit(frame, "startIndexLineEdit");
    gLayout->addWidget(page10_startIndexLineEdit, 1, 1);
    connect(page10_startIndexLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page10_startIndexChanged(const QString &)));    

    page10_endIndexLabel = new QLabel(tr("Last frame"), frame, "endIndexLabel");
    gLayout->addWidget(page10_endIndexLabel, 2, 0);

    page10_endIndexLineEdit = new QLineEdit(frame, "endIndexLineEdit");
    gLayout->addWidget(page10_endIndexLineEdit, 2, 1);
    connect(page10_endIndexLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page10_endIndexChanged(const QString &)));    

    // Add the page.
    page10 = frame;
    addPage(page10, pageInfo[10].title);
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateFilenamePage()
{
    QFrame *frame = new QFrame(this, pageInfo[11].title.ascii());
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
    QLabel *outputDirectoryLabel = new QLabel(tr("Output directory"),
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
    QLabel *filebaseLabel = new QLabel(tr("Base filename"),
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateEmailPage()
{
    QFrame *frame = new QFrame(this, pageInfo[12].title.ascii());
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[12].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *buttonLayout = new QHBoxLayout(pageLayout);
    buttonLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page12_buttongroup = new QButtonGroup(0, "page12_buttongroup");
    connect(page12_buttongroup, SIGNAL(clicked(int)),
            this, SLOT(page12_emailNotificationChanged(int)));
    QRadioButton *r1 = new QRadioButton(tr("Yes"), frame, "r1");
    page12_buttongroup->insert(r1);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("No"), frame, "r2");
    page12_buttongroup->insert(r2);
    buttonLayout->addWidget(r2);
    buttonLayout->addStretch(5);

    pageLayout->addSpacing(20);

    QHBoxLayout *emailLayout = new QHBoxLayout(pageLayout);
    page12_emailLineEdit = new QLineEdit(frame, "page12_emailLineEdit");
    connect(page12_emailLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page12_emailAddressChanged(const QString &)));
    page12_emailLabel = new QLabel(page12_emailLineEdit, tr("E-mail address"),
        frame, "page12_emailLabel");
    emailLayout->addStretch(5);
    emailLayout->addWidget(page12_emailLabel);
    emailLayout->addWidget(page12_emailLineEdit, 10);
    emailLayout->addStretch(5);

    pageLayout->addStretch(10);

    // Add the page.
    page12 = frame;
    addPage(page12, pageInfo[12].title);
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateGenerationMethodPage()
{
    QFrame *frame = new QFrame(this, pageInfo[13].title.ascii());
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(pageInfo[13].description), frame, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(pageLayout);
    hCenterLayout->addStretch(5);
    QVBoxLayout *buttonLayout = new QVBoxLayout(hCenterLayout);
    hCenterLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page13_buttongroup = new QButtonGroup(0, "page13_buttongroup");
    connect(page13_buttongroup, SIGNAL(clicked(int)),
            this, SLOT(page13_generationMethodChanged(int)));
    QRadioButton *r1 = new QRadioButton(tr("Now, use currently allocated processors"),
        frame, "r1");
    page13_buttongroup->insert(r1);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("Now, use a new instance of VisIt"),
        frame, "r2");
    page13_buttongroup->insert(r2);
    buttonLayout->addWidget(r2);

    QRadioButton *r3 = new QRadioButton(tr("Later, tell me the command to run"),
        frame, "r3");
    page13_buttongroup->insert(r3);
    buttonLayout->addWidget(r3);
    pageLayout->addSpacing(10);
    pageLayout->addStretch(10);

    // Add the page.
    page13 = frame;
    addPage(page13, pageInfo[13].title);
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
// ****************************************************************************

void
QvisSaveMovieWizard::CreateYesNoPage(int pageIndex, QWidget **page,
    QButtonGroup **bg, const char *slot)
{
    QFrame *frame = new QFrame(this, pageInfo[pageIndex].title.ascii());
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

    QRadioButton *r1 = new QRadioButton(tr("Yes"),
        frame, "r1");
    buttonGroup->insert(r1);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("No"),
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
    QLabel *w = new QLabel(SplitPrompt(pageInfo[i].description), this, pageInfo[i].title.ascii());
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
// Method: QvisSaveMovieWizard::CurrentPageToStaticPageIndex
//
// Purpose: 
//   Turns the current page number into an index that identifies one of the
//   static wizard pages, or -1 if we're on a dynamic page.
//
// Returns:    The page index or -1.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 13:54:10 PST 2006
//
// Modifications:
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Moved pages 10-12 to 11-13.
//   
// ****************************************************************************

int
QvisSaveMovieWizard::CurrentPageToStaticPageIndex() const
{
    int pageIndex = -1;

    // Only count the page if it is one of the static pages.
    if(currentPage() == page0)
        pageIndex = 0;
    else if(currentPage() == page1)
        pageIndex = 1;
    else if(currentPage() == page2)
        pageIndex = 2;
    else if(currentPage() == page3)
        pageIndex = 3;
    else if(currentPage() == page4)
        pageIndex = 4;
    else if(currentPage() == page5)
        pageIndex = 5;
    else if(currentPage() == page6)
        pageIndex = 6;
    else if(currentPage() == page7)
        pageIndex = 7;
    else if(currentPage() == page8)
        pageIndex = 8;
    else if(currentPage() == page9)
        pageIndex = 9;
    else if(currentPage() == page10)
        pageIndex = 10;
    else if(currentPage() == page11)
        pageIndex = 11;
    else if(currentPage() == page12)
        pageIndex = 12;
    else if(currentPage() == page13)
        pageIndex = 13;

    return pageIndex;
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
// ****************************************************************************

void
QvisSaveMovieWizard::UpdatePage()
{
    int pageIndex = CurrentPageToStaticPageIndex();
    char tmp[100];

    // The page must be a dynamic template page. We don't need to 
    // update those.
    if(pageIndex == -1)
        return;

    // Update the page.
    switch(pageIndex)
    {
    case 0:
        //qDebug("Update the movie type page");
        // Initialize page0's button group with the current movie type.
        page0_buttongroup->blockSignals(true);
        page0_buttongroup->setButton(decision_movieType);
        page0_buttongroup->blockSignals(false);

        // Make sure that the movie attributes have the right movie type
        // in them.
        if(decision_movieType == MOVIE_TYPE_SIMPLE)
            movieAtts->SetMovieType(MovieAttributes::Simple);
        else if(decision_movieType == MOVIE_TYPE_TEMPLATE)
            movieAtts->SetMovieType(MovieAttributes::UsingTemplate);
        break;
    case 1:
        // Template usage
        page1_buttongroup->blockSignals(true);
        page1_buttongroup->setButton(decision_templateUsage);
        page1_buttongroup->blockSignals(false);
        break;
    case 2:
        // Select template
        page2_PopulateTemplates();
        break;
    case 3:
        // Select template
        page3_PopulateSources();
        break;
    case 4:
        // Viewports
        page4_UpdateViews(-1);
        break;
    case 5:
        // Sequences
        page5_Update(-1);
        break;
    case 6:
        // Save template
        page6_buttongroup->blockSignals(true);
        page6_buttongroup->setButton(decision_saveTemplate?0:1);
        page6_buttongroup->blockSignals(false);
        break;
    case 7:
        // Save template as
        page7_Update();
        break;
    case 8:
        //qDebug("Update the settings okay page");
        // Initialize page8's button group with whether or not we think the
        // current movie settings look okay.
        page8_buttongroup->blockSignals(true);
        page8_buttongroup->setButton(decision_settingsOkay?0:1);
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
    case 10: // fps, start/end index
        page10_fpsLineEdit->blockSignals(true);
        SNPRINTF(tmp, 100, "%d", movieAtts->GetFps());
        page10_fpsLineEdit->setText(tmp);
        page10_fpsLineEdit->blockSignals(false);

        page10_UpdateStartEndIndex();
        break;
    case 11: // filename, output dir.
        //qDebug("Update the filename, output dir page.");
        page11_outputDirectoryLineEdit->blockSignals(true);
        page11_outputDirectoryLineEdit->setText(GetMovieAttsOutputDir().c_str());
        page11_outputDirectoryLineEdit->blockSignals(false);
        page11_filebaseLineEdit->blockSignals(true);
        page11_filebaseLineEdit->setText(movieAtts->
            GetOutputName().c_str());
        page11_filebaseLineEdit->blockSignals(false);
        page11_UpdateButtons();
        break;
    case 12:
        //qDebug("Update the email page.");
        page12_buttongroup->blockSignals(true);
        page12_buttongroup->setButton(movieAtts->GetSendEmailNotification()?0:1);
        page12_buttongroup->blockSignals(false);
        page12_emailLineEdit->blockSignals(true);
        page12_emailLineEdit->setText(movieAtts->GetEmailAddress().c_str());
        page12_emailLineEdit->blockSignals(false);
        page12_UpdateButtons();
        break;
    case 13:
        //qDebug("Update the generation method page.");
        page13_buttongroup->blockSignals(true);
        if(movieAtts->GetGenerationMethod() == MovieAttributes::NowCurrentInstance)
            page13_buttongroup->setButton(0);
        else if(movieAtts->GetGenerationMethod() == MovieAttributes::NowNewInstance)
            page13_buttongroup->setButton(1);
        else
            page13_buttongroup->setButton(2);
        page13_buttongroup->blockSignals(false);
#if defined(Q_WS_WIN) || defined(Q_WS_MACX)
        nextButton()->setEnabled(false);
#endif
        break;
    default:
        ;//qDebug("Update page: pageIndex=%d", pageIndex);
    }

    // Enable the finish button on page 13.
    finishButton()->setEnabled(pageIndex == 13);
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
                    title = std::string(tr("[User defined] ").latin1());
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
                page2_templates->insertItem(QString(title.c_str()));
            }
        }

        page2_templatesPopulated = true;

        // Select the first item. This will make the other widgets update so they
        // show the information for the first movie template.
        if(page2_templates->count() > 0)
        {
            page2_templates->setSelected(0, true);
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
                page4_viewportList->insertItem(QString(vpName.c_str()));
            }
        }

        if(templateSpec->ViewportGetActiveName(vpName))
        {
            int index = -1;
            if(templateSpec->ViewportGetIndexForName(vpName, index))
                page4_viewportList->setSelected(index, true);
        }

        page4_viewportList->blockSignals(false);

        // Set the enabled state for the next button.
        nextButton()->setEnabled(nvp > 0);
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
        page4_compositingMode->setButton(compositing);
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
        page4_viewportDisplay->sendBackgroundToBack();
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
        page5_sequenceDestinationViewport->insertItem(tr("(none)"), 0);
        int nvp = templateSpec->GetNumberOfViewports();
        int index = 1;
        for(int i = 0; i < nvp; ++i)
        {
            std::string vpName;
            if(templateSpec->ViewportGetNameForIndex(i, vpName))
            {
                QString name(vpName.c_str());
                page5_sequenceDestinationViewport->insertItem(name, index);
                ++index;
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
             page5_sequenceList->insertItem(pos->second);

            if(s != 0 && QString(s->GetName().c_str()) == pos->second)
                activeIndex = index;
        }

        // Set the active item.
        if(activeIndex != -1)
            page5_sequenceList->setSelected(activeIndex, true);
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
        page5_sequenceDestinationViewport->setCurrentItem(index);
        page5_sequenceDestinationViewport->blockSignals(false);

        // Set the name of the UI file.
        page5_sequenceUIFile->blockSignals(true);
        page5_sequenceUIFile->setText(uiName);
        page5_sequenceUIFile->blockSignals(false);
        page5_sequenceUILabel->setEnabled(allowUI);
        page5_sequenceUIFile->setEnabled(allowUI);
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

    nextButton()->setEnabled(templateSpec->GetNumberOfMappedSequences() > 0);
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
// ****************************************************************************

void
QvisSaveMovieWizard::page8_UpdateMovieSettings()
{
    page8_settingsListView->clear();

    QListViewItem *item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, tr("Generation method"));
    if(movieAtts->GetGenerationMethod() == MovieAttributes::NowCurrentInstance)
        item->setText(1, tr("Now, use currently allocated processors"));
    else if(movieAtts->GetGenerationMethod() == MovieAttributes::NowNewInstance)
        item->setText(1, tr("Now, use a new instance of VisIt"));
    else
        item->setText(1, tr("Later, tell me the command to run"));

    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, tr("Movie type"));
    if(movieAtts->GetMovieType() == MovieAttributes::Simple)
        item->setText(1, tr("New simple movie"));
    else
        item->setText(1, tr("Use movie template"));

    if(movieAtts->GetMovieType() == MovieAttributes::UsingTemplate)
    {
        item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
        item->setText(0, tr("Movie template"));
        item->setText(1, movieAtts->GetTemplateFile().c_str());
    }

    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, tr("Output directory"));
    item->setText(1, GetMovieAttsOutputDir().c_str());

    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
    item->setText(0, tr("Base filename"));
    item->setText(1, movieAtts->GetOutputName().c_str());

    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
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

    item = new QvisSaveMovieWizardListViewItem(page8_settingsListView);
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
            QListViewItem *item = new QvisSaveMovieWizardListViewItem(page9_outputFormats);
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
//   This method sets the movie format combobox on page 8.
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
//   This method sets the resolution widgets on page 8.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:32:29 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Oct 20 12:09:29 PDT 2006
//   Added stereo.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page9_UpdateResolution(bool useCurrent, double scale, int w, int h, int s)
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

    if(s == 0)
    {
        page9_stereoType->blockSignals(true);
        page9_stereoType->setCurrentItem(0);
        page9_stereoType->blockSignals(false);
    }
    else
    {
        page9_stereoType->blockSignals(true);
        page9_stereoType->setCurrentItem(s - 1);
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
    nextButton()->setEnabled(e);
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
// ****************************************************************************

void
QvisSaveMovieWizard::page12_UpdateButtons()
{
    if(!movieAtts->GetSendEmailNotification())
        nextButton()->setEnabled(true);
    else
    {
        bool e = (movieAtts->GetSendEmailNotification() &&
                  movieAtts->GetEmailAddress().size() > 3); // some min length...
        nextButton()->setEnabled(e);
    }

    page12_emailLabel->setEnabled(movieAtts->GetSendEmailNotification());
    page12_emailLineEdit->setEnabled(movieAtts->GetSendEmailNotification());
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
//   Brad Whitlock, Thu Sep 28 12:10:31 PDT 2006
//   Added support for movie templates, etc.
//
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Moved pages 10-12 to 11-13.
// ****************************************************************************

void
QvisSaveMovieWizard::UpdatePageLinking(int pageIndex)
{
    if(pageIndex == 0)
    {
        if(decision_movieType == MOVIE_TYPE_LAST_SETTINGS)
        {
            // We're going to use last settings so make page 8 the next page.
            setAppropriate(page1, false);
            setAppropriate(page2, false);
            setAppropriate(page3, false);
            setAppropriate(page4, false);
            setAppropriate(page5, false);
            setAppropriate(page6, false);
            setAppropriate(page7, false);
            setAppropriate(page8, true);

            // Disable all of the custom pages without removing them.
            for(int i = 0; i < sequencePages.size(); ++i) 
                setAppropriate(sequencePages[i].ui, false);
        }
        else if(decision_movieType == MOVIE_TYPE_SIMPLE)
        {
            // We're going to make a simple movie so make page 9 the next page.
            setAppropriate(page1, false);
            setAppropriate(page2, false);
            setAppropriate(page3, false);
            setAppropriate(page4, false);
            setAppropriate(page5, false);
            setAppropriate(page6, false);
            setAppropriate(page7, false);
            setAppropriate(page8, false);
            setAppropriate(page9, true);

            // Disable all of the custom pages without removing them.
            for(int i = 0; i < sequencePages.size(); ++i) 
                setAppropriate(sequencePages[i].ui, false);
        }
        else
        {
            // We'll use templates somehow so make page 1 the next page.
            setAppropriate(page1, true);
        }
    }
    else if(pageIndex == 1)
    {
        if(decision_templateUsage == 2)
        {
            // Create a new template so skip to page 4
            setAppropriate(page2, false);
            setAppropriate(page3, false);
            setAppropriate(page4, true);

            // Remove the sequence pages and create a default config.
            currentMovieTemplate="";
            RemoveSequencePages();
            debug2 << "Creating new movie template spec" << endl;
            delete templateSpec;
            templateSpec = new MovieTemplateConfig;
        }
        else
        {
            // We'll be picking a template so go to page 2.
            setAppropriate(page2, true);
        }
    }
    else if(pageIndex == 2)
    {
        StringMovieTemplateDataMap::const_iterator it = 
            templateTitleToInfo.find(currentMovieTemplate);
        if(it == templateTitleToInfo.end())
        {
            // If there is no current movie template or if we can't get 
            // information about it then allow page 3.
            setAppropriate(page3, true);
        }
        else
        {
            // There could be pre-existing wizard pages in the wizard
            // if the sequencePagesAdded flag is false. This could happen
            // if we backed up from a later page and switched movie templates.
            // In that event, we want to disable any existing custom pages
            // that have been added because they will be removed, and the
            // pages inserted, when we try to go to page 9.
            if(!sequencePagesAdded)
            {
                for(int i = 0; i < sequencePages.size(); ++i)
                    setAppropriate(sequencePages[i].ui, false);
            }

            if(it->second.info.usesSessionFile)
            {
                // The movie template uses a session file so go to page 3,
                // the page that lets us pick different sources.
                setAppropriate(page3, true);
            }
            // The movie template does not use a session file or the 
            // session file was not provided in the template specification.
            else if(decision_templateUsage == 0)
            {
                // We'll just use a template so tell the wizard to go to page 9,
                // the formats page. If there are custom pages for the template
                // then they will be inserted before page 9.
                setAppropriate(page3, false);
                setAppropriate(page4, false);
                setAppropriate(page5, false);
                setAppropriate(page6, false);
                setAppropriate(page7, false);
                setAppropriate(page8, false);
                setAppropriate(page9, true);
            }
            else if(decision_templateUsage == 1)
            {
                setAppropriate(page3, false);

                // We're going to edit the template so allow pages 4,5.
                setAppropriate(page4, true);
                setAppropriate(page5, true);
            }
        }            
    }
    else if(pageIndex == 3)
    {
        if(decision_templateUsage == 0)
        {
            // We'll just use a template so tell the wizard to go to page 9,
            // the formats page. If there are custom pages for the template
            // then they will be inserted before page 9.
            setAppropriate(page4, false);
            setAppropriate(page5, false);
            setAppropriate(page6, false);
            setAppropriate(page7, false);
            setAppropriate(page8, false);
            setAppropriate(page9, true);
        }
        else if(decision_templateUsage == 1)
        {
            // We're going to edit the template so allow pages 4,5.
            setAppropriate(page4, true);
            setAppropriate(page5, true);
        }
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
        if(decision_saveTemplate)
            setAppropriate(page7, true);
        else
        {
            setAppropriate(page7, false);
            setAppropriate(page8, false);
            setAppropriate(page9, true);
        }
    }
    else if(pageIndex == 7)
    {
        // We're arriving at page 6 so we should set the template filename
        // to something involving the user's movie template directory.
        std::string newSpecFile;
        if(decision_templateUsage == 2)
        {
            // We're creating a new template so use a new filename.
            newSpecFile = GetNewTemplateSpecificationName();
        }
        else
        {
            // Try and use the existing name because we're using an 
            // existing template.
            StringMovieTemplateDataMap::const_iterator it = 
                templateTitleToInfo.find(currentMovieTemplate);
            if(it != templateTitleToInfo.end())
                newSpecFile = it->second.info.specificationFile;
            else
                newSpecFile = GetNewTemplateSpecificationName();
        }
        debug1 << "We will save the template. The default name is: "
               << newSpecFile << endl;
        templateSpec->SetTemplateFile(newSpecFile);

        setAppropriate(page8, false);
        setAppropriate(page9, true);
    }
    else if(pageIndex == 8)
    {
        if(!decision_settingsOkay)
        {
            // Disable all of the custom pages.
            for(int i = 0; i < sequencePages.size(); ++i) 
                setAppropriate(sequencePages[i].ui, false);

            setAppropriate(page9, true);
        }
        else
        {
            // This should have a finish button.
            finishButton()->setEnabled(decision_settingsOkay);
        }
    }
    else if(pageIndex == 9)
    {
        setAppropriate(page10, true);
    }
    else if(pageIndex == 10)
    {
        setAppropriate(page11, true);
    }
    else if(pageIndex == 11)
    {
        setAppropriate(page12, true);
    }
    else if(pageIndex == 12)
    {
        setAppropriate(page13, true);
    }
    else if(pageIndex == 13)
    {
        // This should have a finish button.
    }
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
// Method: QvisSaveMovieWizard::RemoveSequencePages
//
// Purpose: 
//   This method removes custom UI sequence pages from the wizard.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:20:09 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::RemoveSequencePages()
{
    // Make a copy of the sequence page vector before we start removing pages
    // because removePage can make showPage(page6) happen and that needs to have
    // an empty sequences vector if we're removing pages here.
    SequenceUIVector tmp(sequencePages);
    sequencePages.clear();

    this->removingPages = true;
    for(int i = 0; i < tmp.size(); ++i)
    {
        debug1 << "Removing page from wizard" << endl;
        removePage(tmp[i].ui);
        delete tmp[i].ui;
    }
    this->removingPages = false;
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
//   
// ****************************************************************************

bool
QvisSaveMovieWizard::AddSequencePages()
{
    const char *mName = "QvisSaveMovieWizard::AddSequencePages: ";

    if(sequencePagesAdded)
    {
        debug3 << mName << "Sequence pages already added." << endl;
        return false;
    }

    // Remove any custom pages that might be installed.
    debug3 << mName << "Removing sequence pages" << endl;
    RemoveSequencePages();

    // Return if we don't have a template spec.
    if(templateSpec == 0)
    {
        debug3 << "There is no template spec" << endl;
        return false;
    }

    // Iterate over all of the viewports in order and then on all
    // of the sequences within each viewport. We do it like this
    // rather than iterating over the sequences directly so we can
    // keep the UI's for sequences within a viewport grouped.
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
                    SequenceUI uiInfo;
                    uiInfo.name = s->GetName();
                    uiInfo.ui = ui;
                    sequencePages.push_back(uiInfo);
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
        // Insert pages
        for(int i = 0; i < sequencePages.size(); ++i)
        {
            QString num; num.sprintf("%d", i+1);
            QString pageTitle(tr("%1 page %2 for sequence %3"));
            pageTitle.replace("%1", currentMovieTemplate.c_str());
            pageTitle.replace("%2", num);
            pageTitle.replace("%3", sequencePages[i].name.c_str());
            debug1 << "Inserting new wizard page at " << (i+6) << endl;
            insertPage(sequencePages[i].ui, pageTitle, i+6);
        }

        // Update the default values in the new pages.
        UpdateCustomPagesWithDefaultValues();

        // Tell
        sequencePagesAdded = true;
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
        std::string outDir(QDir::currentDirPath().latin1());
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
// Method: QvisSaveMovieWizard::pageChanged
//
// Purpose: 
//   This Qt slot is called to update the controls on the wizard pages.
//
// Arguments:
//   title : The title of the page being opened.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 17:21:20 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSaveMovieWizard::pageChanged(const QString &title)
{
    // Determine the page index of the current page.
    int pageIndex = CurrentPageToStaticPageIndex();

    // Update the page linking
    UpdatePageLinking(pageIndex);

    // Update the page.
    UpdatePage();
}

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

    UpdatePageLinking(0);
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
    UpdatePageLinking(1);
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
// ****************************************************************************

void
QvisSaveMovieWizard::page2_selectedTemplateChanged()
{
    QString title(page2_templates->currentText());

    StringMovieTemplateDataMap::const_iterator it;
    it = templateTitleToInfo.find(std::string(title.latin1()));
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
        currentMovieTemplate = title.latin1();

        // Set this flag to false so the next time we call AddSequencePages,
        // the right pages get added.
        sequencePagesAdded = false;

        // Set up the next page.
        UpdatePageLinking(2);
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
    std::string name(page4_viewportList->currentText().latin1());
    if(templateSpec->ViewportActivate(name))
        page4_UpdateViews(PAGE4_PROPERTIES | PAGE4_DISPLAY_SELECTION);
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
    // Delete the active viewport.
    page4_viewportDisplay->removeViewport(page4_viewportList->currentText());
}

void
QvisSaveMovieWizard::page4_viewportAdded(const QString &name, float llx, float lly, float urx, float ury)
{
    // Add the viewport directly to the movie template configs.
    templateSpec->ViewportAdd(std::string(name.latin1()), llx, lly, urx, ury);
}

void
QvisSaveMovieWizard::page4_viewportChanged(const QString &name, float llx, float lly, float urx, float ury)
{
    if(templateSpec->ViewportSetCoordinates(std::string(name.latin1()),
        llx, lly, urx, ury))
    {
        page4_UpdateViews(PAGE4_PROPERTIES);
    }
}

void
QvisSaveMovieWizard::page4_viewportRemoved(const QString &name)
{
    // Remove the viewport.
    if(templateSpec->ViewportRemove(std::string(name.latin1())))
        page4_UpdateViews(PAGE4_LIST | PAGE4_PROPERTIES);
}

void
QvisSaveMovieWizard::page4_viewportActivated(const QString &name)
{
    if(templateSpec->ViewportActivate(std::string(name.latin1())))
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
// ****************************************************************************

void
QvisSaveMovieWizard::page4_usePredefinedViewports(int index)
{
    // Remove all of the existing viewports.
    templateSpec->ViewportRemoveAll();

    page4_viewportDisplay->blockSignals(true);
    page4_viewportDisplay->clear();
    page4_viewportDisplay->blockSignals(false);

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
    std::string seqName(page5_sequenceList->currentText().latin1());
    if(templateSpec->SequenceActivate(seqName))
        page5_Update(PAGE5_UPDATE_SEQUENCE_PROPS);
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
            if(templateSpec->SequenceRename(s->GetName(), newName.latin1()))
            {
                flags = PAGE5_UPDATE_SEQUENCE_LIST | 
                        PAGE5_UPDATE_SEQVIEW_MAPPING;

                // Invalidate the sequence pages since one will have a new name
                // and it should be recreated to reflect that.
                sequencePagesAdded = false;
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
//   
// ****************************************************************************

void
QvisSaveMovieWizard::page5_destinationViewportChanged(int vpt)
{
    MovieSequence *s = templateSpec->GetActiveSequence();
    if(s != 0)
    {
        std::string seqName(s->GetName()), vpName;

        // Unmap the sequence from any viewport.
        bool b1 = templateSpec->SequenceUnmap(s->GetName());

        // Try and map the sequence to the viewport.
        bool b2 = false;
        if(vpt > 0)
        {
            if(templateSpec->ViewportGetNameForIndex(vpt-1, vpName))
            {
                b2 = templateSpec->SequenceMapToViewport(
                    s->GetName(), vpName);
            }
        }

        // If we modified the mapping then update the display
        if(b1 || b2)
        {
            // Invalidate the sequence pages the destination viewport changed.
            sequencePagesAdded = false;

            page5_Update(PAGE5_UPDATE_SEQVIEW_MAPPING);
        }
    }
}

bool
QvisSaveMovieWizard::page5_updatedMappingEx(const QString &vp,
    const QStringList &sList)
{
    std::string vpName(vp.latin1());
    stringVector seqList;
    for(int i = 0; i < sList.size(); ++i)
        seqList.push_back(std::string(sList[i].latin1()));

    return templateSpec->ViewportSetSequenceList(vpName, seqList);
}

void
QvisSaveMovieWizard::page5_updatedMapping(const QString &vp1,
    const QStringList &seqList1)
{
    page5_updatedMappingEx(vp1, seqList1);

    // Invalidate the sequence pages the destination viewport changed.
    sequencePagesAdded = false;
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

    // Invalidate the sequence pages the destination viewport changed.
    sequencePagesAdded = false;
}

// Called when return is pressed in the uifile line edit.
void
QvisSaveMovieWizard::page5_sequenceUIFileChanged()
{
    QString s(page5_sequenceUIFile->displayText().simplifyWhiteSpace());
    if(!s.isEmpty())
    {
        MovieSequence *m = templateSpec->GetActiveSequence();
        if(m != 0 && m->SupportsCustomUI())
            m->SetUIFile(s.latin1());
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
    UpdatePageLinking(6);
}

//
// Page 7 slots
//

void
QvisSaveMovieWizard::page7_templateNameChanged(const QString &s)
{
    templateSpec->SetTitle(std::string(s.latin1()));
}

void
QvisSaveMovieWizard::page7_templateDescriptionChanged()
{
    std::string s(page7_templateDescription->text().latin1());
    templateSpec->SetDescription(s);
}

void
QvisSaveMovieWizard::page7_templateFileChanged(const QString &s)
{
    bool ret = templateSpec->SetTemplateFile(std::string(s.latin1()));
}

void
QvisSaveMovieWizard::page7_previewImageFileChanged(const QString &s)
{
    templateSpec->SetPreviewImageFile(std::string(s.latin1()));
}

//
// Page 8 slots
//

void
QvisSaveMovieWizard::page8_settingsOkayChanged(int val)
{
    decision_settingsOkay = (val == 0);
    UpdatePageLinking(8);

    nextButton()->setEnabled(!decision_settingsOkay);
    finishButton()->setEnabled(decision_settingsOkay);
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
    bool stereo = page9_stereoCheckBox->isChecked();
    int stereoType = page9_stereoType->currentItem();

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
//   Kathleen Bonnell, Fri Jul 20 11:07:11 PDT 2007
//   Use new method to get the outputdirectory from movie atts.
//   
//   Dave Bremer, Wed Oct 10 17:11:51 PDT 2007
//   Moved pages 10-12 to 11-13.
//
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisSaveMovieWizard::page11_selectOutputDirectory()
{
    //
    // Try and get a directory using a file dialog.
    //
    QString initialDir(GetMovieAttsOutputDir().c_str());
    QString dirName = QFileDialog::getExistingDirectory(initialDir, this,
        "getDirectoryDialog", tr("Select output directory"));

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
    movieAtts->SetEmailAddress(std::string(val.latin1()));
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

