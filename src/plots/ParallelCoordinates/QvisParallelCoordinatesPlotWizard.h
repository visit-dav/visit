/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
