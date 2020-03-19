// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_DATABASECORRELATION_WINDOW_H
#define QVIS_DATABASECORRELATION_WINDOW_H
#include <QvisWindowBase.h>

class DatabaseCorrelation;
class QComboBox;
class QLineEdit;
class QListWidget;
class QPushButton;

// ****************************************************************************
// Class: QvisDatabaseCorrelationWindow
//
// Purpose:
//   This class provides controls for designing a database correlation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Sat Jan 31 02:45:19 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:48:45 PDT 2008
//   QString for caption.
//
// ****************************************************************************

class GUI_API QvisDatabaseCorrelationWindow : public QvisWindowBase
{
    Q_OBJECT
public:
    QvisDatabaseCorrelationWindow(const QString &correlationName,
        const QString &caption);
    QvisDatabaseCorrelationWindow(const DatabaseCorrelation &correlation,
        const QString &caption);
    virtual ~QvisDatabaseCorrelationWindow();

signals:
    void deleteMe(QvisWindowBase *);
protected slots:
    void setAddButtonEnabled(int);
    void setRemoveButtonEnabled(int);
    void addSources();
    void removeSources();
    void actionClicked();
    void cancelClicked();
protected:
    void CreateWidgets(const DatabaseCorrelation &correlation);
    void UpdateAddRemoveButtonsEnabledState();
    int  SelectedCount(const QListWidget *) const;
    void TransferItems(QListWidget *srcLB, QListWidget *destLB);

    static int   instanceCount;
    bool         createMode;

    // Widgets and layout
    QLineEdit    *correlationNameLineEdit;
    QListWidget     *sourcesListBox;
    QListWidget     *correlatedSourcesListBox;
    QPushButton  *addButton;
    QPushButton  *removeButton;
    QComboBox    *correlationMethodComboBox;
//    QCheckBox    *automaticNStatesCheckBox;
//    QSpinBox     *nStatesSpinBox;
};

#endif
