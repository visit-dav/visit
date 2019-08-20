// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SCATTER_PLOT_WIZARD_PAGE_H
#define QVIS_SCATTER_PLOT_WIZARD_PAGE_H
#include <QvisWizard.h>

#include <vectortypes.h>
#include <QMap>


class QvisScatterWidget;
class QCheckBox;
class QvisVariableButton;


// ****************************************************************************
// Class: QvisScaterPlotWizardPage
//
// Purpose:
//   Wizard page that provides the interface setting up a scatter plot.
//
// Notes: Refactored from QvisScatterPlotWizard.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul  8 09:02:28 PDT 2010
//
// Modifications:
//
// ****************************************************************************



class QvisScatterPlotWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    QvisScatterPlotWizardPage(AttributeSubject *s,
                             QWidget *parent,
                             const std::string &xvar_name);

    virtual ~QvisScatterPlotWizardPage();

    void GetSelectedVars(stringVector &res) const;
    bool ZEnabled() const;
    bool ColorEnabled() const;

protected slots:
    void choseXVariable(const QString &);
    void choseYVariable(const QString &);
    void choseZVariable(const QString &);
    void choseColorVariable(const QString &);

    void decideZ(int);
    void decideColor(int);
protected:
    virtual bool        isComplete() const;
    void                checkComplete();

    bool                ready;
    QvisScatterWidget  *preview;
    QvisVariableButton *xVarButton;
    QvisVariableButton *yVarButton;
    QvisVariableButton *zVarButton;
    QCheckBox          *zVarCheck;
    QvisVariableButton *colorVarButton;
    QCheckBox          *colorVarCheck;

    std::string         xVarName;
    std::string         yVarName;
    std::string         zVarName;
    std::string         colorVarName;

};

#endif
