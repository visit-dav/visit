#ifndef QVIS_EXPORT_DB_WINDOW_H
#define QVIS_EXPORT_DB_WINDOW_H
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
class QvisVariableButton;
class ExportDBAttributes;

// ****************************************************************************
// Class: QvisExportDBWindow
//
// Purpose:
//   This class contains the code necessary to create a window for exporting
//   databases.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   Brad Whitlock, Mon Jun 27 13:51:01 PST 2005
//   Added a directory selection button.
//
// ****************************************************************************

class GUI_API QvisExportDBWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisExportDBWindow(ExportDBAttributes *subj, const char *caption = 0,
                   const char *shortName = 0,
                   QvisNotepadArea *notepad = 0);
    virtual ~QvisExportDBWindow();
public slots:
    virtual void apply();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
protected slots:
    void variableProcessText();
    void processFilenameText();
    void processDirectoryNameText();
    void fileFormatChanged(int);
    void exportDB();
    void exportButtonClicked();
    void addVariable(const QString &);
    void selectOutputDirectory();
private:
    QLineEdit           *filenameLineEdit;
    QLineEdit           *directoryNameLineEdit;
    QComboBox           *fileFormatComboBox;
    ExportDBAttributes  *exportDBAtts;
    QvisVariableButton  *varsButton;
    QLineEdit           *varsLineEdit;
};

#endif
