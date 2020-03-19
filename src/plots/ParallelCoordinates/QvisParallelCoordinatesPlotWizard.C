// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisParallelCoordinatesPlotWizard.h>
#include <QvisParallelCoordinatesPlotWizardPage.h>
#include <ParallelCoordinatesAttributes.h>

#include <avtDatabaseMetaData.h>
#include <ExpressionList.h>
#include <QDebug>

#define parAxisAtts ((ParallelCoordinatesAttributes *)localCopy)

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::QvisParallelCoordinatesPlotWizard
//
// Purpose: 
//   Constructor for the ParallelCoordinates plot wizard.
//
// Note: This is intended to emulate the style of the QvisScatterPlotWizard
//       for the Scatter plot, which came first.
//
// Arguments:
//   s       : Subject to use when communicating with the viewer
//   parent  : Wizard's parent widget
//   varName : Name of the plot's scalar variable
//   md      : Metadata for current database
//   expList : Current set of expressions
//
// Programmer: Mark Blair
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Modifications:
//    Jeremy Meredith, Thu Feb  7 12:58:15 EST 2008
//    A wizard is needed because you can't reset the default plot attributes
//    without a wizard's accept action having been called.  If you don't, then
//    you'll have the wrong number of axes defined in the plot attributes.
//    As such, I extended the wizard to support a "no-op" mode.
//
//    Brad Whitlock, Wed Apr 23 10:12:44 PDT 2008
//    Added tr()
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port.
//
//    Cyrus Harrison, Wed May 13 08:30:56 PDT 2009
//    Overhauled to create a simpler list widget based interface.
//
//    Cyrus Harrison, Wed Jul  7 13:20:47 PDT 2010
//    Refactored so wizard is no longer used when an array var is passed.
//    The guts of the wizard were moved to the
//    QvisParallelCoordinatesPlotWizardPage class.
//
// ****************************************************************************

QvisParallelCoordinatesPlotWizard::QvisParallelCoordinatesPlotWizard(
                                   AttributeSubject *s,
                                   QWidget *parent,
                                   const std::string &varName,
                                   const avtDatabaseMetaData *md,
                                   const ExpressionList *expList)
: QvisWizard(s, parent)
{
    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::HaveHelpButton, false);
    setOption(QWizard::HaveNextButtonOnLastPage, false);
    setOption(QWizard::NoBackButtonOnStartPage, true);

    // Set the wizard title.
    topLevelWidget()->setWindowTitle(tr("Parallel Coordinates Plot Setup"));

    mainPage = new QvisParallelCoordinatesPlotWizardPage(s,
                                                          parent,
                                                          varName,
                                                          md,
                                                          expList);
    addPage(mainPage);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::~QvisParallelCoordinatesPlotWizard
//
// Purpose: Destructor for the QvisParallelCoordinatesPlotWizard class.
//
// Programmer: Mark Blair
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Modifications:
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port.
//
//    Cyrus Harrison, Wed May 13 08:30:56 PDT 2009
//    Added parent widget to all wizard pages, so we no longer need to
//    explicitly delete these wizard page widgets.
//
// ****************************************************************************
QvisParallelCoordinatesPlotWizard::~QvisParallelCoordinatesPlotWizard()
{

}

// ****************************************************************************
// Method: QvisParallelCoordinatesPage::validateCurrentPage
//
// Purpose: Used to trigger update of the local copy of plot's attributes.
//
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul  8 09:21:52 PDT 2010
//
// Modifications:
//
// ****************************************************************************
bool
QvisParallelCoordinatesPlotWizard::validateCurrentPage()
{
    SetParallelCoordinatesAttributes();
    return true;
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPage::SetParallelCoordinatesAttributes
//
// Purpose: Sets local copy of plot's attributes
//
// Arguments:
//
// Programmer: Jeremy Meredith
// Creation:   January 31, 2008
//
// Modifications:
//    Jeremy Meredith, Fri Feb  1 17:56:59 EST 2008
//    Made limits be in terms of actual values, not normalized 0..1.
//
//    Jeremy Meredith, Mon Feb  4 16:06:57 EST 2008
//    Drop the axis extents in the plot attributes -- they were not used.
//
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.  Added
//    visualAxisNames.
//
//    Cyrus Harrison, Wed Jul  7 13:20:47 PDT 2010
//    Refactored b/c wizard is no longer used when an array var is passed.
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::SetParallelCoordinatesAttributes()
{
    stringVector saxisNames;
    stringVector vaxisNames;
    doubleVector extMins;
    doubleVector extMaxs;

    mainPage->GetAxisNames(saxisNames);

    size_t nvars = saxisNames.size();
    vaxisNames = saxisNames;

    for(size_t i=0;i < nvars;i++)
    {
        extMins.push_back(-1e+37);
        extMaxs.push_back(+1e+37);
    }

    parAxisAtts->SetScalarAxisNames(saxisNames);
    parAxisAtts->SetVisualAxisNames(vaxisNames);
    parAxisAtts->SetExtentMinima(extMins);
    parAxisAtts->SetExtentMaxima(extMaxs);
}


