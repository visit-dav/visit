// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SCATTER_PLOT_WIZARD_H
#define QVIS_SCATTER_PLOT_WIZARD_H
#include <QvisWizard.h>

class QvisScatterPlotWizardPage;
// ****************************************************************************
// Class: QvisScatterPlotWizard
//
// Purpose:
//   This class is a wizard that helps the user choose initial parameters for
//   the Scatter plot.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:53:27 PDT 2004
//
// Modifications:
//   Brad Whitlock, Fri Aug  8 11:25:55 PDT 2008
//   Qt 4.
//
//   Cyrus Harrison, Thu Aug 19 13:16:58 PDT 2010
//   New single page wizard, majority of the functionality moved into
//   QvisScatterPlotWizardPage.
//
// ****************************************************************************

class QvisScatterPlotWizard : public QvisWizard
{
    Q_OBJECT
public:
    QvisScatterPlotWizard(AttributeSubject *s,
                          QWidget *parent,
                          const std::string &var_name);
    virtual ~QvisScatterPlotWizard();

protected:
    bool validateCurrentPage();
    void SetScatterAttributes();
    QvisScatterPlotWizardPage *mainPage;

};

#endif
