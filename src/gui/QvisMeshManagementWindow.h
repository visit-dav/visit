// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISMESHMANAGEMENTWINDOW_H
#define QVISMESHMANAGEMENTWINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations.
class MeshManagementAttributes;
class QButtonGroup;
class QRadioButton;
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
// Modifications:
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added makeDefault and reset slots
//
//    Mark C. Miller, Wed Dec 19 11:32:58 PST 2007
//    Made Qt objects and visual controls input to mmatts a little more
//    user-friendly. However, mmatts themselves were not changed.
//
//    Brad Whitlock, Wed Apr  9 11:34:22 PDT 2008
//    QString for caption, shortName.
//
//   Cyrus Harrison, Wed Jul  2 11:16:25 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Thu Dec 18 09:29:37 PST 2008
//   Removed tabSelected slot b/c it was an empty method.
//
//   Jeremy Meredith, Fri Feb 26 14:13:08 EST 2010
//   Added a new "multi-pass" discretization algorithm
//
// ****************************************************************************

class GUI_API QvisMeshManagementWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisMeshManagementWindow(MeshManagementAttributes *subj,
                        const QString &caption = QString(),
                        const QString &shortName = QString(),
                        QvisNotepadArea *notepad = 0);
    virtual ~QvisMeshManagementWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
    void GetCurrentValues(const QWidget *widget = 0);
private slots:
    void processSmallestZoneText();
    void processSmallestZoneText(const QString &);
    void processFlatEnoughText();
    void processFlatEnoughText(const QString &);
    void renderCSGDirectChanged(bool);
    void discretizeBoundaryOnlyChanged(bool);
    void discretizationModeChanged(int);
private:
    MeshManagementAttributes *mmAtts;

    QWidget          *pageCSGGroup;
    QCheckBox        *renderCSGDirect;
    QCheckBox        *discretizeBoundaryOnly;
    QLabel           *discretizeModeLabel;
    QButtonGroup     *discretizationMode;
    QRadioButton     *discretizeMultiPass;
    QRadioButton     *discretizeUniform;
    QRadioButton     *discretizeAdaptive;
    QLabel           *smallestZoneLabel;
    QLineEdit        *smallestZoneLineEdit;
    QLabel           *flatEnoughLabel;
    QLineEdit        *flatEnoughLineEdit;

    QTabWidget       *tabs;

};
#endif
