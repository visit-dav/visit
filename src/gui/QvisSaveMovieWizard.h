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

#ifndef QVIS_SAVE_MOVIE_WIZARD_H
#define QVIS_SAVE_MOVIE_WIZARD_H
#include <QvisWizard.h>
#include <MovieUtility.h>
#include <map>
#include <vectortypes.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QIconView;
class QLabel;
class QLineEdit;
class QListBox;
class QListView;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTextEdit;

class QvisColorButton;
class QvisDialogLineEdit;
class QvisOpacitySlider;
class QvisScreenPositionEdit;
class QvisSequenceView;
class QvisSessionSourceChanger;
class QvisViewportWidget;

class MovieTemplateConfig;

// ****************************************************************************
// Class: QvisSaveMovieWizard
//
// Purpose: 
//   This class contains the "Save movie" wizard that leads the user through
//   all of the questions needed to design a movie.
//
// Notes:      All of the pages kind of make this like 10 windows in 1.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 21 16:44:22 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu Feb 2 18:48:28 PST 2006
//   Added default size for movie.
//
//   Brad Whitlock, Fri Sep 22 16:48:58 PST 2006
//   Added support for movie templates.
//
//   Kathleen Bonnell, Fri Jul 20 10:59:28 PDT 2007 
//   Added GetMovieAttsOutputDir(). 
//
//   Dave Bremer, Tue Oct  9 14:13:12 PDT 2007
//   Added a new page to set fps and start/end index, moved pages 
//   10-12 to 11-13, and added a methods to set/get the number of frames
// ****************************************************************************

class QvisSaveMovieWizard : public QvisWizard
{
    Q_OBJECT
public:
    QvisSaveMovieWizard(AttributeSubject *, QWidget *parent, 
                        const char *name = 0);
    virtual ~QvisSaveMovieWizard();

    int Exec();
    void SetDefaultMovieSize(int,int);
    void SetDefaultNumFrames(int);
    int  GetDefaultNumFrames();

    virtual void showPage(QWidget *page);
protected:
    virtual void keyPressEvent(QKeyEvent *e);
private slots:
    void pageChanged(const QString &);

    void page0_movieTypeChanged(int);

    void page1_newTemplateChanged(int);

    void page2_selectedTemplateChanged();

    void page4_viewportSelected();
    void page4_addViewport();
    void page4_deleteViewport();
    void page4_viewportAdded(const QString &id,
                             float llx, float lly, float urx, float ury);
    void page4_viewportChanged(const QString &id,
                               float llx, float lly, float urx, float ury);
    void page4_viewportRemoved(const QString &id);
    void page4_viewportActivated(const QString &d);
    void page4_lowerLeftChanged(double x, double y);
    void page4_upperRightChanged(double x, double y);
    void page4_compositingModeChanged(int);
    void page4_viewportOpacityChanged(int);
    void page4_viewportColorChanged(const QColor &);
    void page4_dropShadowChanged(bool);
    void page4_usePredefinedViewports(int);
    
    void page5_selectedSequenceChanged();
    void page5_newSequenceClicked(int);
    void page5_deleteSequenceClicked();
    void page5_destinationViewportChanged(int);
    void page5_typedNewSequenceName(const QString &);
    void page5_updatedMapping(const QString &vp1,
                        const QStringList &seqList1);
    void page5_updatedMapping(const QString &vp1,
                        const QStringList &seqList1,
                        const QString &vp2,
                        const QStringList &seqList2);
    void page5_sequenceUIFileChanged();

    void page6_saveAsTemplateChanged(int);

    void page7_templateFileChanged(const QString &);
    void page7_templateNameChanged(const QString &);
    void page7_templateDescriptionChanged();
    void page7_previewImageFileChanged(const QString &);

    void page8_settingsOkayChanged(int);
    void page8_delayedUpdate();

    void page9_formatChanged(int);
    void page9_widthChanged(int);
    void page9_heightChanged(int);
    void page9_aspectLockChanged(bool);
    void page9_addOutput();
    void page9_removeOutput();
    void page9_sizeTypeChanged(int);
    void page9_scaleChanged(int);
    void page9_stereoChanged(bool);
    void page9_stereoTypeChanged(int);

    void page10_fpsChanged(const QString &s);
    void page10_startIndexChanged(const QString &s);
    void page10_endIndexChanged(const QString &s);

    void page11_processOutputDirectoryText(const QString &);
    void page11_selectOutputDirectory();
    void page11_processFilebaseText(const QString &);

    void page12_emailNotificationChanged(int);
    void page12_emailAddressChanged(const QString &);

    void page13_generationMethodChanged(int);

private:
    struct MovieTemplateData
    {
        std::string              filename;
        bool                     userDefined;
        MovieTemplateInformation info;
    };
  
    typedef std::map<std::string, MovieTemplateData> StringMovieTemplateDataMap;

    struct SequenceUI
    {
        std::string name;
        QWidget    *ui;
    };

    typedef std::vector<SequenceUI> SequenceUIVector;

    QWidget *CreateSimplePage(int i);
    void CreateYesNoPage(int pageIndex, QWidget **page,
                         QButtonGroup **bg, const char *slot);
    void UpdatePage();
    void UpdatePageLinking(int);
    int  CurrentPageToStaticPageIndex() const;

    void page2_PopulateTemplates();
    bool LoadTemplateSpecification(const std::string &);
    void UpdateCustomPagesWithDefaultValues();
    void UpdateDefaultValuesFromCustomPages();

    void RemoveSequencePages();
    bool AddSequencePages();
    void WriteTemplateSpecification();

    void page3_PopulateSources();

    void page4_UpdateViews(int flags);

    void page5_Update(int flags);
    bool page5_updatedMappingEx(const QString &, const QStringList &);

    void page7_Update();

    void page8_UpdateMovieSettings();

    void page9_UpdateOutputs();
    void page9_UpdateResolution(bool, double, int w, int h, int s);
    bool page9_UpdateFormat(const QString &format);

    void page10_UpdateStartEndIndex();

    void page11_UpdateButtons();

    void page12_UpdateButtons();

    void CreateMovieTypePage();         // page0
    void CreateNewTemplatePromptPage(); // page1
    void CreateChooseTemplatePage();    // page2
    void CreateChooseNewSourcesPage();  // page3
    void CreateViewportPage();          // page4
    void CreateSequencesPage();         // page5
    void CreateSaveTemplatePage();      // page6
    void CreateSaveTemplateAsPage();    // page7
    void CreateSettingsOkayPage();      // page8
    void CreateFormatPage();            // page9
    void CreateNumFramesPage();         // page10
    void CreateFilenamePage();          // page11
    void CreateEmailPage();             // page12
    void CreateGenerationMethodPage();  // page13

    QString SplitPrompt(const QString &s) const;

    std::string GetMovieAttsOutputDir();

    // Some movie template-related members.
    StringMovieTemplateDataMap  templateTitleToInfo;
    MovieTemplateConfig        *templateSpec;
    std::string                 currentMovieTemplate;

    int                     decision_movieType;
    int                     decision_templateUsage;
    bool                    decision_saveTemplate;
    bool                    decision_settingsOkay;

    bool                    removingPages;
    bool                    page0_usePreviousSettingsAllowed;
    bool                    page2_templatesPopulated;
    float                   page9_aspect;

    float                   default_movie_size[2];
    int                     default_num_frames;

    // Custom sequence pages.
    SequenceUIVector        sequencePages;
    bool                    sequencePagesAdded;

    // Use last settings, simple, template?
    QWidget                *page0;
    QButtonGroup           *page0_buttongroup;
    QRadioButton           *page0_r1;

    // Use, Modify, Create template?
    QWidget                *page1;
    QButtonGroup           *page1_buttongroup;

    // Choose template
    QWidget                *page2;
    QListBox               *page2_templates;
    QLabel                 *page2_template_image;
    QTextEdit              *page2_template_description;

    // Pick new sources
    QWidget                *page3;
    QvisSessionSourceChanger *page3_sessionSources;

    // Viewports
    QWidget                *page4;
    QvisViewportWidget     *page4_viewportDisplay;
    QListBox               *page4_viewportList;
    QPushButton            *page4_deleteViewportButton;
    QvisScreenPositionEdit *page4_lowerLeft;
    QvisScreenPositionEdit *page4_upperRight;
    QButtonGroup           *page4_compositingMode;
    QvisOpacitySlider      *page4_viewportOpacity;
    QvisColorButton        *page4_viewportColor;
    QCheckBox              *page4_dropShadow;

    // Sequences
    QWidget                *page5;
    QGroupBox              *page5_sequenceProperties;
    QListBox               *page5_sequenceList;
    QPushButton            *page5_deleteSequence;
    QLineEdit              *page5_sequenceName;
    QLabel                 *page5_sequenceUILabel;
    QvisDialogLineEdit     *page5_sequenceUIFile;
    QPushButton            *page5_sequenceChooseUIFile;
    QLabel                 *page5_sequenceNameLabel;
    QComboBox              *page5_sequenceDestinationViewport;
    QvisSequenceView       *page5_sequenceView;

    // Save template?
    QWidget                *page6;
    QButtonGroup           *page6_buttongroup;

    // Save template as
    QWidget                *page7;
    QLineEdit              *page7_templateName;
    QTextEdit              *page7_templateDescription;
    QvisDialogLineEdit     *page7_templateFile;
    QvisDialogLineEdit     *page7_previewImageFile;

    // Last settings look okay?
    QWidget                *page8;
    QButtonGroup           *page8_buttongroup;
    QListView              *page8_settingsListView;

    // Choose formats
    QWidget                *page9;
    QComboBox              *page9_formatComboBox;
    QButtonGroup           *page9_sizeTypeButtonGroup;
    QSpinBox               *page9_scaleSpinBox;
    QLabel                 *page9_scaleLabel;
    QSpinBox               *page9_widthSpinBox;
    QLabel                 *page9_widthLabel;
    QSpinBox               *page9_heightSpinBox;
    QLabel                 *page9_heightLabel;
    QCheckBox              *page9_aspectLock;
    QCheckBox              *page9_stereoCheckBox;
    QLabel                 *page9_stereoLabel;
    QComboBox              *page9_stereoType;
    QPushButton            *page9_addOutputButton;
    QPushButton            *page9_removeOutputButton;
    QListView              *page9_outputFormats;

    // Choose movie length
    QWidget                *page10;
    QLabel                 *page10_fpsLabel;
    QLineEdit              *page10_fpsLineEdit;
    QLabel                 *page10_startIndexLabel;
    QLineEdit              *page10_startIndexLineEdit;
    QLabel                 *page10_endIndexLabel;
    QLineEdit              *page10_endIndexLineEdit;

    // Choose filenames
    QWidget                *page11;
    QLineEdit              *page11_outputDirectoryLineEdit;
    QLineEdit              *page11_filebaseLineEdit;

    // Email
    QWidget                *page12;
    QButtonGroup           *page12_buttongroup;
    QLabel                 *page12_emailLabel;
    QLineEdit              *page12_emailLineEdit;

    // How to generate?
    QWidget                *page13;
    QButtonGroup           *page13_buttongroup;
};

#endif
