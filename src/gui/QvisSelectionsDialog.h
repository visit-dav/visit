// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SELECTIONS_DIALOG_H
#define QVIS_SELECTIONS_DIALOG_H
#include <QDialog>

class QLabel;
class QLineEdit;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisSelectionsDialog
//
// Purpose:
//   This class lets us select a selection's name and its source.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun  9 14:46:22 PDT 2011
//
// Modifications:
//   Brad Whitlock, Sat Nov  5 02:14:58 PDT 2011
//   Let the user pick different id variable types.
//
//   Dave Pugmire, Thu Mar 15 11:19:33 EDT 2012
//   Add location named selections.
//
// ****************************************************************************

class QvisSelectionsDialog : public QDialog
{
    Q_OBJECT
public:
    typedef enum 
    {
        SOURCE_USE_DB,
        SOURCE_USE_DB_OR_PLOT
    } SourceSelectionMode;

    typedef enum
    {
        UseZoneIDForID,
        UseGlobalZoneIDForID,
        UseLocationsForID,
        UseVariableForID
    } IDVariableType;

    QvisSelectionsDialog(SourceSelectionMode m, QWidget *parent = 0);
    virtual ~QvisSelectionsDialog();

    void setSelectionName(const QString &val);
    void setPlotName(const QString &val);
    void setDBName(const QString &val);

    int exec(QString &selName, QString &selSource, bool &plotSource,
             IDVariableType &idtype, QString &idvar);

private slots:
    void selectionChoiceChanged(int);
    void idVariableChanged(const QString &);
    void idVariableTypeChanged(int);
private:
    bool           choosePlot;
    QString        selectionName;
    QString        plot;
    QString        db;
    IDVariableType idType;
    QString        idVariable;

    QLineEdit     *selectionNameLineEdit;
    QLabel        *plotLabel;
    QLabel        *dbLabel;
    QvisVariableButton *idVariableButton;
};

#endif
