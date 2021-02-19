// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_MACRO_WINDOW_H
#define QVIS_MACRO_WINDOW_H
#include <QvisPostableWindow.h>

class QButtonGroup;
class QGroupBox;
class QGridLayout;

// ****************************************************************************
// Class: QvisMacroWindow
//
// Purpose:
//   This class implements a window that shows buttons that let you execute
//   one-click macros.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 14 16:51:28 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:56:54 PDT 2008
//   QString for captionString, shortName.
//
// ****************************************************************************

class QvisMacroWindow : public QvisPostableWindow
{
    Q_OBJECT
public:
    QvisMacroWindow(const QString &captionString = QString(),
                    const QString &shortName = QString(),
                    QvisNotepadArea *n = 0);
    virtual ~QvisMacroWindow();
    virtual void CreateWindowContents();

signals:
    void runCommand(const QString &);
public slots:
    void addMacro(const QString &);
    void clearMacros();
private slots:
    void invokeMacro(int);
private:
    QGroupBox    *macroGroup;
    QGridLayout  *buttonLayout;
    QButtonGroup *macroButtons;
};

#endif
