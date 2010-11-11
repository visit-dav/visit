/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <QvisScatterPlotWizard.h>
#include <QLayout>
#include <QvisScatterPlotWizardPage.h>
#include <ScatterAttributes.h>

#define scatterAtts ((ScatterAttributes *)localCopy)

// ****************************************************************************
// Method: QvisScatterPlotWizard::QvisScatterPlotWizard
//
// Purpose: 
//   Constructor for the Scatter plot wizard.
//
// Arguments:
//   s      : The subject to use when communicating with the viewer.
//   parent : The wizard's parent widget.
//   name   : The wizard's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:54:32 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 11:09:42 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Fri Aug  8 14:00:29 PDT 2008
//   Qt 4.
//
//   Cyrus Harrison, Thu Aug 19 13:28:53 PDT 2010
//   Refactored, most functionality moved into QvisScatterPlotWizardPage.
//
// ****************************************************************************

QvisScatterPlotWizard::QvisScatterPlotWizard(AttributeSubject *s,
                                             QWidget *parent,
                                             const std::string &var_name)
: QvisWizard(s, parent)
{

    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::HaveHelpButton, false);
    setOption(QWizard::HaveNextButtonOnLastPage, false);
    setOption(QWizard::NoBackButtonOnLastPage, true);

    // Set some defaults into the wizard's local copy of the plot attributes.
    scatterAtts->SetVar1Role(ScatterAttributes::Coordinate0);
    scatterAtts->SetVar2("default");
    scatterAtts->SetVar2Role(ScatterAttributes::Coordinate1);
    scatterAtts->SetVar3("default");
    scatterAtts->SetVar3Role(ScatterAttributes::Coordinate2);
    scatterAtts->SetVar4("default");
    scatterAtts->SetVar4Role(ScatterAttributes::Color);

    // Set the wizard's title.
    topLevelWidget()->setWindowTitle(tr("Scatter Plot Setup"));

    //
    // Create the main wizard page.
    //
    mainPage = new QvisScatterPlotWizardPage(s,this,var_name);
    addPage(mainPage);
    mainPage->setFinalPage(true);
}

// ****************************************************************************
// Method: QvisScatterPlotWizard::~QvisScatterPlotWizard
//
// Purpose:
//   Destructor for the QvisScatterPlotWizard class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:55:40 PDT 2004
//
// Modifications:
//
// ****************************************************************************

QvisScatterPlotWizard::~QvisScatterPlotWizard()
{
}

// ****************************************************************************
// Method: QvisScatterPlotWizard::validateCurrentPage
//
// Purpose: Used to trigger update of the local copy of plot's attributes.
//
//
// Programmer: Cyrus Harrison
// Creation:   Wed Aug 18 16:44:47 PDT 2010
//
// Modifications:
//
// ****************************************************************************
bool
QvisScatterPlotWizard::validateCurrentPage()
{
    SetScatterAttributes();
    return true;
}

// ****************************************************************************
// Method: QvisScatterPlotWizard::SetScatterAttributes
//
// Purpose: Sets attributes from wizard selections before the plot is created.
//
//
// Programmer: Cyrus Harrison
// Creation:   Wed Aug 18 16:44:47 PDT 2010
//
// Modifications:
//   Cyrus Harrison, Thu Aug 19 13:28:53 PDT 2010
//   Use color table mode if color var is selected.
//
// ****************************************************************************
void
QvisScatterPlotWizard::SetScatterAttributes()
{
    stringVector vars;
    mainPage->GetSelectedVars(vars);
    scatterAtts->SetVar1(vars[0]);
    scatterAtts->SetVar1Role(ScatterAttributes::Coordinate0);
    scatterAtts->SetVar2(vars[1]);
    scatterAtts->SetVar2Role(ScatterAttributes::Coordinate1);

    if(mainPage->ZEnabled())
    {
        scatterAtts->SetVar3(vars[2]);
        scatterAtts->SetVar3Role(ScatterAttributes::Coordinate2);
    }
    else
    {
        scatterAtts->SetVar3Role(ScatterAttributes::None);
    }

    if(mainPage->ColorEnabled())
    {
        scatterAtts->SetVar4(vars[3]);
        scatterAtts->SetVar4Role(ScatterAttributes::Color);
        scatterAtts->SetColorType(ScatterAttributes::ColorByColorTable);
    }
    else
    {
        scatterAtts->SetVar4Role(ScatterAttributes::None);
    }
}

