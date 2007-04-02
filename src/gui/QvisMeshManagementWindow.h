#ifndef QVISMESHMANAGEMENTWINDOW_H
#define QVISMESHMANAGEMENTWINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations.
class MeshManagementAttributes;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QTabWidget;
class QVBox;

// ****************************************************************************
// Class: QvisMeshManagementWindow
//
// Purpose: Creates window for mesh management controls 
//
// Programmer: Mark C. Miller 
// Creation:   November 5, 2005
//
// ****************************************************************************

class GUI_API QvisMeshManagementWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisMeshManagementWindow(MeshManagementAttributes *subj,
                        const char *caption = 0,
                        const char *shortName = 0,
                        QvisNotepadArea *notepad = 0);
    virtual ~QvisMeshManagementWindow();
    virtual void CreateWindowContents();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
    void GetCurrentValues(const QWidget *widget = 0);
private slots:
    void apply();
    void processDiscretizationToleranceText();
    void processDiscretizationToleranceText(const QString &);
    void tabSelected(const QString &tabLabel);
    void renderCSGDirectChanged(bool);
    void discretizeBoundaryOnlyChanged(bool);
    void discretizationModeChanged(int);
private:
    MeshManagementAttributes *mmAtts;

    QVBox            *pageCSG;
    QGroupBox        *pageCSGGroup;
    QCheckBox        *renderCSGDirect;
    QCheckBox        *discretizeBoundaryOnly;
    QLabel           *discretizeModeLabel;
    QButtonGroup     *discretizationMode;
    QLabel           *discretizationToleranceLabel;
    QLineEdit        *discretizationToleranceLineEdit;


    QTabWidget       *tabs;

};

#endif
