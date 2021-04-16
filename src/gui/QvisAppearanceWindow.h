// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISAPPEARANCEWINDOW_H
#define QVISAPPEARANCEWINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations.
class AppearanceAttributes;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QvisColorButton;
class QvisDialogLineEdit;

// ****************************************************************************
// Class: QvisAppearancePlotWindow
//
// Purpose: 
//   This class creates a window that has controls for changing GUI color,
//   style, and font.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:23:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 29 13:15:07 PST 2002
//   Added orientation combo box.
//
//   Brad Whitlock, Fri Oct 3 10:06:22 PDT 2003
//   Added an internal method.
//
//   Brad Whitlock, Thu Mar 15 15:24:58 PST 2007
//   Added font controls.
//
//   Brad Whitlock, Wed Apr  9 11:10:05 PDT 2008
//   QString for caption, shortName.
//
//   Cyrus Harrison, Mon Nov 24 11:57:42 PST 2008
//   Support for default system appearance.
//
//   Kathleen Biagas, Fri Jan 22 14:11:18 PST 2016
//   Add styleNames.
//
// ****************************************************************************

class GUI_API QvisAppearanceWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisAppearanceWindow(AppearanceAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisAppearanceWindow();
    virtual void CreateWindowContents();
signals:
    void changeAppearance(bool);
public slots:
    virtual void apply();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
    bool ColorsNotTooClose(const QColor &c0, const char *c1str);
    void GetCurrentValues(int);
    void UpdateWindowSensitivity();

private slots:
    void useSysDefaultChanged(bool val);
    void backgroundChanged(const QColor &bg);
    void foregroundChanged(const QColor &fg);
    void styleChanged(int index);
    void fontNameChanged(const QString &);
    void orientationChanged(int index);
private:
    QCheckBox          *useSysDefaultCheckBox;
    QvisColorButton    *backgroundColorButton;
    QLabel             *backgroundColorLabel;
    QvisColorButton    *foregroundColorButton;
    QLabel             *foregroundColorLabel;
    QComboBox          *styleComboBox;
    QLabel             *styleLabel;
    QComboBox          *orientationComboBox;
    QLabel             *orientationLabel;
    QvisDialogLineEdit *fontName;
    QLabel             *fontLabel;
    QStringList         styleNames;
};

#endif
