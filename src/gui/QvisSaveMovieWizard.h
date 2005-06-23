#ifndef QVIS_SAVE_MOVIE_WIZARD_H
#define QVIS_SAVE_MOVIE_WIZARD_H
#include <QvisWizard.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListView;
class QPushButton;
class QRadioButton;
class QSpinBox;

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
//   
// ****************************************************************************

class QvisSaveMovieWizard : public QvisWizard
{
    Q_OBJECT
public:
    QvisSaveMovieWizard(AttributeSubject *, QWidget *parent, 
                        const char *name = 0);
    virtual ~QvisSaveMovieWizard();

    int Exec();
protected:
    virtual void keyPressEvent(QKeyEvent *e);
private slots:
    void pageChanged(const QString &);

    void page0_movieTypeChanged(int);

    void page2_customizeChanged(int);

    void page6_saveAsTemplateChanged(int);

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
    void page10_stereoChanged(int);

    void page11_processOutputDirectoryText(const QString &);
    void page11_selectOutputDirectory();
    void page11_processFilebaseText(const QString &);

    void page12_generationMethodChanged(int);

private:
    QWidget *CreateSimplePage(int i);
    void CreateYesNoPage(int pageIndex, QWidget **page,
                         QButtonGroup **bg, const char *slot);
    void UpdatePage();
    void UpdatePageLinking(int);

    void page8_UpdateMovieSettings();
    void page9_UpdateOutputs();
    void page9_UpdateResolution(bool, double, int w, int h);
    bool page9_UpdateFormat(const QString &format);
    void page11_UpdateButtons();

    void CreateMovieTypePage();        // page0
    void CreateCustomizePage();        // page2
    void CreateSaveTemplatePage();     // page6
    void CreateSettingsOkayPage();     // page8
    void CreateFormatPage();           // page9
    void CreateStereoMoviePage();      // page10
    void CreateFilenamePage();         // page11
    void CreateGenerationMethodPage(); // page12

    QString SplitPrompt(const QString &s) const;

    bool             page0_usePreviousSettingsAllowed;
    bool             page1_createNewTemplate;
    bool             page2_customizeTemplate;
    bool             page6_saveTemplate;
    bool             page8_settingsOkay;
    float            page9_aspect;

    QWidget         *page0;
    QButtonGroup    *page0_buttongroup;
    QRadioButton    *page0_r1;

    QWidget         *page1;

    QWidget         *page2;
    QButtonGroup    *page2_buttongroup;

    QWidget         *page3;

    QWidget         *page4;
    QWidget         *page5;

    QWidget         *page6;
    QButtonGroup    *page6_buttongroup;

    QWidget         *page7;

    QWidget         *page8;
    QButtonGroup    *page8_buttongroup;
    QListView       *page8_settingsListView;

    QWidget          *page9;
    QComboBox        *page9_formatComboBox;
    QButtonGroup     *page9_sizeTypeButtonGroup;
    QSpinBox         *page9_scaleSpinBox;
    QLabel           *page9_scaleLabel;
    QSpinBox         *page9_widthSpinBox;
    QLabel           *page9_widthLabel;
    QSpinBox         *page9_heightSpinBox;
    QLabel           *page9_heightLabel;
    QCheckBox        *page9_aspectLock;
    QPushButton      *page9_addOutputButton;
    QPushButton      *page9_removeOutputButton;
    QListView        *page9_outputFormats;

    QWidget         *page10;
    QButtonGroup    *page10_buttongroup;

    QWidget         *page11;
    QLineEdit       *page11_outputDirectoryLineEdit;
    QLineEdit       *page11_filebaseLineEdit;

    QWidget         *page12;
    QButtonGroup    *page12_buttongroup;
};

#endif
