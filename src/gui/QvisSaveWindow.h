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
class QPushButton;
class QSlider;
class SaveWindowAttributes;

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
// ****************************************************************************

class GUI_API QvisSaveWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSaveWindow(SaveWindowAttributes *subj, const char *caption = 0,
                   const char *shortName = 0,
                   QvisNotepadArea *notepad = 0);
    virtual ~QvisSaveWindow();
public slots:
    virtual void apply();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
protected slots:
    void outputToCurrentDirectoryToggled(bool);
    void processOutputDirectoryText();
    void processFilenameText();
    void familyToggled(bool);
    void stereoToggled(bool);
    void fileFormatChanged(int);
    void maintainAspectToggled(bool);
    void processWidthText();
    void processHeightText();
    void progressiveToggled(bool);
    void binaryToggled(bool);
    void qualityChanged(int);
    void screenCaptureToggled(bool);
    void saveTiledToggled(bool);
    void compressionTypeChanged(int);
    void saveWindow();
    void selectOutputDirectory();
    void saveButtonClicked();
private:
    QCheckBox           *outputToCurrentDirectoryCheckBox;
    QLabel              *outputDirectoryLabel;
    QLineEdit           *outputDirectoryLineEdit;
    QPushButton         *outputDirectorySelectButton;
    QLineEdit           *filenameLineEdit;
    QCheckBox           *familyCheckBox;
    QCheckBox           *stereoCheckBox;
    QComboBox           *fileFormatComboBox;
    QLabel              *qualityLabel;
    QSlider             *qualitySlider;
    QLabel              *compressionTypeLabel;
    QComboBox           *compressionTypeComboBox;
    QCheckBox           *progressiveCheckBox;
    QCheckBox           *binaryCheckBox;
    QGroupBox           *resolutionBox;
    QCheckBox           *maintainAspectCheckBox;
    QLineEdit           *widthLineEdit;
    QLineEdit           *heightLineEdit;
    QCheckBox           *screenCaptureCheckBox;
    QCheckBox           *saveTiledCheckBox;
    SaveWindowAttributes  *saveWindowAtts;
};

#endif
