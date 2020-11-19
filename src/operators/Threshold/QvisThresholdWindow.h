// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISTHRESHOLDWINDOW_H
#define QVISTHRESHOLDWINDOW_H

#include <QvisOperatorWindow.h>
#include <ThresholdAttributes.h>

class QTableWidget;
class QButtonGroup;

// ****************************************************************************
// Class: QvisThresholdWindow
//
// Purpose: 
//   Defines QvisThresholdWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//
//   Brad Whitlock, Fri Dec 10 09:39:57 PDT 2004
//   Added a variable button.
//
//   Hank Childs, Thu Sep 15 15:31:34 PDT 2005
//   Added windowing for creating point meshes.
//
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Upgraded to support multiple threshold variables.
//
//   Mark Blair, Tue Aug  8 17:47:00 PDT 2006
//   Now accommodates an empty list of threshold variables.
//
//   Mark Blair, Wed Sep  6 19:33:00 PDT 2006
//   Removed problematic mechanism for accommodating ExtentsAttributes from
//   extents tool.
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Added support for input from Extents tool.  Certain Threshold changes are
//   preserved when viewer sends attributes that do not know about the changes.
//   (See full explanation in header of RestoreAppropriateUnappliedAttributes.)
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Previous change undone.  Too much conflict when multiple vis windows in
//   use.  All unapplied Threshold GUI changes are now lost if user dismisses
//   then reopens GUI, or if user moves an arrowhead in Extents tool of a second
//   vis window tool-locked to the Threshold operator's vis window.  Too bad.
//
//   Mark Blair, Tue Apr 17 16:24:42 PDT 2007
//   Rewritten to support new Threshold GUI.
//
//   Cyrus Harrison, Thu Aug 21 08:45:29 PDT 2008
//   Qt4 Port.
//
//   Kevin Griffin, Mon Feb  6 13:48:42 PST 2017
//   Feature #2646 categorical option for threshold
//
// ****************************************************************************

class QvisThresholdWindow : public QvisOperatorWindow
{
    Q_OBJECT

public:
    QvisThresholdWindow(const int type,
                        ThresholdAttributes *subj,
                        const QString &caption = QString(),
                        const QString &shortName = QString(),
                        QvisNotepadArea *notepad = 0);
    virtual            ~QvisThresholdWindow();
    virtual void        CreateWindowContents();

protected:
    void                UpdateWindow(bool doAll);
    virtual void        GetCurrentValues(int which_widget);

private slots:
    void                variableAddedToList(const QString &variableToAdd);
    void                selectedVariableDeleted();
    void                boundsInputTypeChanged(int buttonID);
    void                outputMeshTypeChanged(int buttonID);
    void                apply();

private:
    void                PopulateThresholdVariablesList();
    void                AddNewRowToVariablesList(const QString &variableName);
    QString             PrepareVariableNameText(const QString &variableName,
                                                int maxDisplayChars);
    void                SetZoneIncludeSelectEnabled(bool val);
    void                SetTableWidth();
    void                SetLowerUpper(int idx, double lower, double upper);
    bool                IsSimpleBounds(const QString &);

    ThresholdAttributes *atts;

    QTableWidget        *threshVars;
    QButtonGroup        *outputMeshType;
    QButtonGroup        *boundsInputType;
    stringVector        guiFullVarNames;
};

#endif
