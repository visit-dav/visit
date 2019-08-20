// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PARALLEL_COORDINATES_PLOT_WIZARD_H
#define QVIS_PARALLEL_COORDINATES_PLOT_WIZARD_H
#include <QvisWizard.h>

#include <vectortypes.h>
#include <QMap>

class avtDatabaseMetaData;
class ExpressionList;
class QvisParallelCoordinatesPlotWizardPage;

// ****************************************************************************
// Class: QvisParallelCoordinatesPlotWizard
//
// Purpose:
//   This class is a wizard that helps the user choose variables for initial
//   axes of a ParallelCoordinates plot.
//
// Notes: initial implementation taken from Mark Blair's ParallelAxis plot.
//
// Programmer: Jeremy Meredith
// Creation:   January 31, 2008
//
// Modifications:
//    Jeremy Meredith, Thu Feb  7 12:58:15 EST 2008
//    A wizard is needed because you can't reset the default plot attributes
//    without a wizard's accept action having been called.  If you don't, then
//    you'll have the wrong number of axes defined in the plot attributes.
//    As such, I extended the wizard to support a "no-op" mode.
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port.
//
//    Cyrus Harrison, Wed May 13 08:28:27 PDT 2009
//    Changed from sequential wizard style to a quicker list widget based gui.
//
//    Cyrus Harrison, Wed May 13 08:28:27 PDT 2009
//    Refactored to remove the "done page & transported guts of main page
//    into the QvisParallelCoordinatesPage class for better control of wizard
//    interaction. This wizard is no longer used when the user creates a plot
//    using an array variable.
//
// ****************************************************************************

class QvisParallelCoordinatesPlotWizard : public QvisWizard
{
    Q_OBJECT
public:
    QvisParallelCoordinatesPlotWizard(AttributeSubject *s,
                                      QWidget *parent,
                                      const std::string &varName,
                                      const avtDatabaseMetaData *md,
                                      const ExpressionList *exprList);
    ~QvisParallelCoordinatesPlotWizard();

protected:
    bool validateCurrentPage();
    void SetParallelCoordinatesAttributes();

    QvisParallelCoordinatesPlotWizardPage *mainPage;

};

#endif
