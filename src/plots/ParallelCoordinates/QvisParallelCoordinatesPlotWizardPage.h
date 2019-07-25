// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PARALLEL_COORDINATES_PLOT_WIZARD_PAGE_H
#define QVIS_PARALLEL_COORDINATES_PLOT_WIZARD_PAGE_H
#include <QvisWizard.h>

#include <vectortypes.h>
#include <QMap>

class QListWidget;
class QPushButton;
class QButtonGroup;
class QLabel;
class QvisParallelCoordinatesWidget;
class avtDatabaseMetaData;
class ExpressionList;


// ****************************************************************************
// Class: QvisParallelCoordinatesPlotWizardPage
//
// Purpose:
//   Wizard page that provides the interface for selecting axis variables.
//
// Notes: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul  8 09:02:28 PDT 2010
//
// Modifications:
//
// ****************************************************************************

class QvisParallelCoordinatesPlotWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    QvisParallelCoordinatesPlotWizardPage(AttributeSubject *s,
                                QWidget *parent,
                                const std::string &varName,
                                const avtDatabaseMetaData *md,
                                const ExpressionList *exprList);

    virtual ~QvisParallelCoordinatesPlotWizardPage();
    void GetAxisNames(stringVector &res);


protected slots:
    void OnScalarVarSelectionChanged();
    void OnAxisVarSelectionChanged();
    void OnAddButtonPressed();
    void OnUpButtonPressed();
    void OnDownButtonPressed();
    void OnRemoveButtonPressed();

protected:

    virtual bool isComplete() const;

    void  InitScalarVarNames(const avtDatabaseMetaData *md,
                             const ExpressionList *expList);

    void SetupAxisVariableSelectionPage();

    void UpdateVarsList();
    void UpdateAxisVarsList();
    void UpdatePreview(QvisParallelCoordinatesWidget *preview);


    bool                                  ready;
    std::string                           varName;

    QLabel                               *infoLbl;
    QListWidget                          *varsList;
    QListWidget                          *axisVarsList;
    QPushButton                          *addButton;
    QPushButton                          *upButton;
    QPushButton                          *downButton;
    QPushButton                          *removeButton;

    QvisParallelCoordinatesWidget        *selectionPreview;

    QStringList                           scalarVarNames;
    QStringList                           scalarExprNames;
    QStringList                           scalarDBExprNames;
    QMap<QString,bool>                    usedVars;

};

#endif
