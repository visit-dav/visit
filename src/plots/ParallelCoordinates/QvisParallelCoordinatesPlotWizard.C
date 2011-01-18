/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

    int nvars = saxisNames.size();
    vaxisNames = saxisNames;

    for(int i=0;i < nvars;i++)
    {
        extMins.push_back(-1e+37);
        extMaxs.push_back(+1e+37);
    }

    parAxisAtts->SetScalarAxisNames(saxisNames);
    parAxisAtts->SetVisualAxisNames(vaxisNames);
    parAxisAtts->SetExtentMinima(extMins);
    parAxisAtts->SetExtentMaxima(extMaxs);
}


