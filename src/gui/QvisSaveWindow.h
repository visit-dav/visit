// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SAVE_WINDOW_H
#define QVIS_SAVE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

// Forward declarations.
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSixCharLineEdit;
class QPushButton;
class QSlider;
class SaveWindowAttributes;
class QRadioButton;
class QButtonGroup;
class QvisOpacitySlider;
class DBPluginInfoAttributes;

// ****************************************************************************
// Class: QvisSaveWindow
//
// Purpose:
//   This class contains the code necessary to create a window that
//   observes the save image attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 16:46:07 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Jan 23 15:13:23 PST 2002
//   Added quality and progressive toggles.
//
//   Hank Childs, Fri May 24 07:45:52 PDT 2002
//   Renamed SaveImageAtts to SaveWindowAtts.  Added support for STL.
//
//   Hank Childs, Sun May 26 17:31:18 PDT 2002
//   Added a toggle for binary vs ASCII.
//
//   Kathleen Bonnell, Thu Nov 13 12:14:30 PST 2003 
//   Added combo box for compression type. 
//
//   Brad Whitlock, Fri Jul 30 15:14:44 PST 2004
//   Added save path and a slot to save the image.
//
//   Kathleen Bonnell, Wed Dec 15 08:20:11 PST 2004 
//   Added slot 'saveButtonClicked'. 
//
//   Jeremy Meredith, Thu Apr  5 17:23:37 EDT 2007
//   Added button to force a merge of parallel geometry.
//
//   Brad Whitlock, Wed Apr  9 10:56:15 PDT 2008
//   QString for caption, shortName.
//
//   Hank Childs, Thu Jul 22 09:55:03 PDT 2010
//   Added support for multi-window saves.  Also re-orged window some to
//   put check boxes close to the functionality they go with.
//
//   Brad Whitlock, Wed Aug 25 13:32:46 PDT 2010
//   I moved some code into helper methods.
//
//   Kathleen Biagas, Wed Jan  7 12:39:12 PST 2015
//   Added dismissOnSave flag, dismissAndSaveButtonClicked slot.
//
//   Eric Brugger, Mon Aug 31 10:29:30 PDT 2015
//   I overhauled the window.
//
//   Brad Whitlock, Wed Sep 20 18:07:45 PDT 2017
//   Added pixel options.
//
//   Kathleen Biagas, Fri Aug 31 13:56:10 PDT 2018
//   Add dbPluginInfoAtts so DBOptionsAttributes can be passed to the writer.
//
// ****************************************************************************

class GUI_API QvisSaveWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSaveWindow(SaveWindowAttributes *subj, 
                   const QString &caption = QString(),
                   const QString &shortName = QString(),
                   QvisNotepadArea *notepad = 0);
    virtual ~QvisSaveWindow();

    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    virtual void ConnectSubjects(DBPluginInfoAttributes *dbp);

public slots:
    virtual void apply();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);

protected slots:
    void processFilenameText();
    void familyToggled(bool);
    void outputToCurrentDirectoryToggled(bool);
    void processOutputDirectoryText();
    void selectOutputDirectory();
    void fileFormatChanged(int);
    void qualityChanged(int);
    void progressiveToggled(bool);
    void compressionTypeChanged(int);
    void binaryToggled(bool);
    void stereoToggled(bool);
    void rgbToggled(bool);
    void aToggled(bool);
    void depthToggled(bool);
    void luminanceToggled(bool);
    void valueToggled(bool);
    void forceMergeToggled(bool);
    void aspectRatioChanged(int);
    void processWidthText();
    void processHeightText();
    void screenCaptureToggled(bool);
    void multiWindowSaveToggled(bool);
    void multiWindowSaveTypeToggled(bool);
    void mwsWindowComboBoxChanged(int);
    void omitWindowCheckBoxToggled(bool);
    void processmwsIndWidthText();
    void processmwsIndHeightText();
    void processmwsPosXText();
    void processmwsPosYText();
    void mwsLayerComboBoxChanged(int);
    void imageTransparencyChanged(int);
    void saveWindow();
    void saveButtonClicked();
    void saveAndDismissButtonClicked();

private:
    int currentWindow;
    bool dismissOnSave;

    enum MultiWindowSaveMode
    {
        Tiled,
        Advanced
    };
    MultiWindowSaveMode  multiWindowSaveMode;
    bool                 multiWindowSaveEnabled;
    bool                 ignoreNextMultiWindowSaveChange;

    QLineEdit           *filenameLineEdit;
    QCheckBox           *familyCheckBox;
    QCheckBox           *outputToCurrentDirectoryCheckBox;
    QLabel              *outputDirectoryLabel;
    QLineEdit           *outputDirectoryLineEdit;
    QPushButton         *outputDirectorySelectButton;
    QComboBox           *fileFormatComboBox;
    QLabel              *qualityLabel;
    QSlider             *qualitySlider;
    QCheckBox           *progressiveCheckBox;
    QLabel              *compressionTypeLabel;
    QComboBox           *compressionTypeComboBox;
    QCheckBox           *binaryCheckBox;
    QCheckBox           *stereoCheckBox;
    QCheckBox           *forceMergeCheckBox;
    QGroupBox           *pdGroup;
    QCheckBox           *pdRGB;
    QCheckBox           *pdA;
    QCheckBox           *pdLuminance;
    QCheckBox           *pdValue;
    QCheckBox           *pdDepth;
    QGroupBox           *aspectAndResolutionBox;
    QLabel              *aspectRatioLabel;
    QComboBox           *aspectRatioComboBox;
    QLabel              *widthLabel;
    QSixCharLineEdit    *widthLineEdit;
    QLabel              *heightLabel;
    QSixCharLineEdit    *heightLineEdit;
    QCheckBox           *screenCaptureCheckBox;
    QGroupBox           *multiWindowSaveBox;
    QButtonGroup        *multiWindowSaveTypeButtonGroup;
    QRadioButton        *tiledButton;
    QRadioButton        *advancedButton;
    QLabel              *mwsWindowLabel;
    QComboBox           *mwsWindowComboBox;
    QCheckBox           *omitWindowCheckBox;
    QLabel              *mwsIndWidthLabel;
    QSixCharLineEdit    *mwsIndWidthLineEdit;
    QLabel              *mwsIndHeightLabel;
    QSixCharLineEdit    *mwsIndHeightLineEdit;
    QLabel              *mwsPosXLabel;
    QSixCharLineEdit    *mwsPosXLineEdit;
    QLabel              *mwsPosYLabel;
    QSixCharLineEdit    *mwsPosYLineEdit;
    QLabel              *mwsLayerLabel;
    QComboBox           *mwsLayerComboBox;
    QLabel              *imageTransparencyLabel;
    QvisOpacitySlider   *imageTransparency;

    SaveWindowAttributes   *saveWindowAtts;
    DBPluginInfoAttributes *dbPluginInfoAtts;
};

#endif
