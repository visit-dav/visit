/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>

#include <QvisScatterWidget.h>
#include <QvisVariableButton.h>
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
// ****************************************************************************

QvisScatterPlotWizard::QvisScatterPlotWizard(AttributeSubject *s,
    QWidget *parent) : QvisWizard(s, parent)
{
    selectZCoord = true;
    selectColor = true;

    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::HaveHelpButton, false);
    setOption(QWizard::HaveNextButtonOnLastPage, false);

    // Set some defaults into the wizard's local copy of the plot attributes.
    scatterAtts->SetVar1Role(ScatterAttributes::Coordinate0);
    scatterAtts->SetVar2("default");
    scatterAtts->SetVar2Role(ScatterAttributes::Coordinate1);
    scatterAtts->SetVar3("default");
    scatterAtts->SetVar3Role(ScatterAttributes::Coordinate2);
    scatterAtts->SetVar4("default");
    scatterAtts->SetVar4Role(ScatterAttributes::Color);

    // Set the wizard's title.
    topLevelWidget()->setWindowTitle(tr("Scatter plot wizard"));

    //
    // Create the wizard's first page.
    //
    CreateVariablePage(&page0, &scatter[0],
        tr("Choose a variable to use for the Scatter\n"
        "plot's Y coordinate."),
        SLOT(choseYVariable(const QString &)),
        true, false, false);
    scatter[0]->setAllowAnimation(true);
    page0->setSubTitle(tr("Choose Y coordinate"));
    addPage(page0);

    CreateYesNoPage(&page1, &scatter[1], &bg2,
        tr("Would you like to choose a variable to use\n"
           "as the Scatter plot's Z coordinate?"),
        SLOT(decideZ(int)), true, true, false);
    page1->setSubTitle(tr("Choose Z coordinate"));
    addPage(page1);

    CreateVariablePage(&page2, &scatter[2],
        tr("Choose a variable to use for the Scatter\n"
           "plot's Z coordinate."), SLOT(choseZVariable(const QString &)),
        true, true, false);
    page2->setSubTitle(tr("Choose Z coordinate"));
    addPage(page2);

    CreateYesNoPage(&page3, &scatter[3], &bg4,
        tr("Would you like to choose a variable to use\n"
           "as the Scatter plot's color?"),
        SLOT(decideColor(int)), false, true /* depends on selectZCoord */, true);
    page3->setSubTitle(tr("Choose color variable"));
    addPage(page3);

    CreateVariablePage(&page4, &scatter[4],
        tr("Choose a variable to use for the Scatter\n"
        "plot's color."), SLOT(choseColorVariable(const QString &)),
        false, true /* could be false*/, true);
    page4->setSubTitle(tr("Choose color variable"));
    addPage(page4);

    CreateFinishPage(&page5, &scatter[5],
        tr("Click the %1 button to create a new Scatter plot").
            arg(buttonText(QWizard::FinishButton)),
        false, true /* could be false*/, true);
    page5->setSubTitle(tr("Click %1").arg(buttonText(QWizard::FinishButton)));
    page5->setFinalPage(true);
    addPage(page5);

    // Connect a slot that turns the animation flag on the scatter widgets
    // on and off as needed.
    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(updateAnimationFlags(int)));
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
    // Delete parentless widgets.
    delete page0;
    delete page1;
    delete page2;
    delete page3;
    delete page4;
    delete page5;
}

// ****************************************************************************
// Method: QvisScatterPlotWizard::CreateVariablePage
//
// Purpose: 
//   Creates a wizard page that lets us select a variable using a variable
//   button.
//
// Arguments:
//   f           : The return variable for the frame widget pointer.
//   s           : The return variable for the scatter widget pointer.
//   promptText  : The text to display.
//   slot        : The slot function to call when a variable is selected.
//   highlight   : Whether an axis should be highlighted.
//   threeD      : Whether the scatter widget should be 3D.
//   colorPoints : Whether the scatter widget's points are colored.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:56:01 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 11:09:58 PDT 2008
//   Added tr()
//
//   Brad Whitlock, Fri Aug  8 14:03:29 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisScatterPlotWizard::CreateVariablePage(QWizardPage **f, QvisScatterWidget **s,
    const QString &promptText,
    const char *slot, bool highlight, bool threeD, bool colorPoints)
{
    QWizardPage *frame = new QWizardPage(0);
    *f = frame;

    QHBoxLayout *pageLRLayout = new QHBoxLayout(frame);

    QvisScatterWidget *scatter = new QvisScatterWidget(frame);
    scatter->setHighlightAxis(highlight);
    scatter->setThreeD(threeD);
    scatter->setColoredPoints(colorPoints);
    *s = scatter;
    pageLRLayout->addWidget(scatter);
    pageLRLayout->addSpacing(10); // or a line?

    QVBoxLayout *pageRLayout = new QVBoxLayout(0);
    pageLRLayout->addLayout(pageRLayout);
    pageLRLayout->setStretchFactor(pageRLayout, 10);
    pageRLayout->setMargin(0);
    pageRLayout->setSpacing(10);
    QLabel *prompt = new QLabel(promptText, frame);
    pageRLayout->addWidget(prompt);
    pageRLayout->addSpacing(10);

    // add interior.
    QHBoxLayout *pageVLayout = new QHBoxLayout(0);
    pageRLayout->addLayout(pageVLayout);
    pageVLayout->setMargin(0);
    pageVLayout->setSpacing(10);
    pageVLayout->addStretch(5);
    QLabel *varlabel = new QLabel(tr("Variable"));
    QvisVariableButton *var = new QvisVariableButton(true, false, true,
        QvisVariableButton::Scalars, frame);
    var->setMinimumWidth(fontMetrics().boundingRect("really_really_long_var_name").width());
    connect(var, SIGNAL(activated(const QString &)),
            this, slot);
    pageVLayout->addWidget(varlabel);
    pageVLayout->addWidget(var);
    pageVLayout->addStretch(5);

    pageRLayout->addStretch(10);
}

// ****************************************************************************
// Method: QvisScatterPlotWizard::CreateYesNoPage
//
// Purpose: 
//   Creates a wizard page that lets us choose yes/no.
//
// Arguments:
//   f           : The return variable for the frame widget pointer.
//   s           : The return variable for the scatter widget pointer.
//   b           : The return variable for the button group.
//   promptText  : The text to display.
//   slot        : The slot function to call when a yes/no chosen.
//   highlight   : Whether an axis should be highlighted.
//   threeD      : Whether the scatter widget should be 3D.
//   colorPoints : Whether the scatter widget's points are colored.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:56:01 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 11:10:11 PDT 2008
//   Added t()
//
//   Brad Whitlock, Fri Aug  8 14:03:51 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisScatterPlotWizard::CreateYesNoPage(QWizardPage **f, QvisScatterWidget **s,
    QButtonGroup **b, const QString &promptText,
    const char *slot, bool highlight, bool threeD, bool colorPoints)
{
    QWizardPage *frame = new QWizardPage(0);
    *f = frame;
    QHBoxLayout *pageLRLayout = new QHBoxLayout(frame);

    QvisScatterWidget *scatter = new QvisScatterWidget(frame);
    scatter->setHighlightAxis(highlight);
    scatter->setThreeD(threeD);
    scatter->setColoredPoints(colorPoints);
    *s = scatter;
    pageLRLayout->addWidget(scatter);
    pageLRLayout->addSpacing(10); // or a line?

    QVBoxLayout *pageRLayout = new QVBoxLayout(0);
    pageLRLayout->addLayout(pageRLayout);
    pageLRLayout->setStretchFactor(pageRLayout, 10);
    pageRLayout->setMargin(0);
    pageRLayout->setSpacing(10);
    QLabel *prompt = new QLabel(promptText, frame);
    pageRLayout->addWidget(prompt);
    pageRLayout->addSpacing(10);

    // add interior.
    QHBoxLayout *pageVLayout = new QHBoxLayout(0);
    pageRLayout->addLayout(pageVLayout);
    pageRLayout->setMargin(0);
    pageVLayout->setSpacing(10);
    pageVLayout->addStretch(5);

    QButtonGroup *btn = new QButtonGroup(frame);
    *b = btn;
    connect(btn, SIGNAL(buttonClicked(int)),
            this, slot);
    QRadioButton *r1 = new QRadioButton(tr("Yes"), frame);
    r1->setChecked(true);
    btn->addButton(r1, 0);
    pageVLayout->addWidget(r1);
    QRadioButton *r2 = new QRadioButton(tr("No"), frame);
    btn->addButton(r2, 1);
    pageVLayout->addWidget(r2);
    pageVLayout->addStretch(5);

    pageRLayout->addStretch(10);
}

// ****************************************************************************
// Method: QvisScatterPlotWizard::CreateFinishPage
//
// Purpose: 
//   Creates final wizard page
//
// Arguments:
//   f           : The return variable for the frame widget pointer.
//   s           : The return variable for the scatter widget pointer.
//   promptText  : The text to display.
//   highlight   : Whether an axis should be highlighted.
//   threeD      : Whether the scatter widget should be 3D.
//   colorPoints : Whether the scatter widget's points are colored.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:56:01 PDT 2004
//
// Modifications:
//   Brad Whitlock, Fri Aug  8 14:08:17 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisScatterPlotWizard::CreateFinishPage(QWizardPage **f, QvisScatterWidget **s,
    const QString &promptText,
    bool highlight, bool threeD, bool colorPoints)
{
    QWizardPage *frame = new QWizardPage(0);
    *f = frame;
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);
    QHBoxLayout *pageLRLayout = new QHBoxLayout(0);
    frameinnerLayout->addLayout(pageLRLayout);
    pageLRLayout->setMargin(0);
    pageLRLayout->setSpacing(10);

    QvisScatterWidget *scatter = new QvisScatterWidget(frame);
    scatter->setHighlightAxis(highlight);
    scatter->setThreeD(threeD);
    scatter->setColoredPoints(colorPoints);
    *s = scatter;
    pageLRLayout->addWidget(scatter);
    pageLRLayout->addSpacing(10); // or a line?

    QVBoxLayout *pageRLayout = new QVBoxLayout(0);
    pageLRLayout->addLayout(pageRLayout);
    pageRLayout->setMargin(0);
    pageRLayout->setSpacing(10);

    QLabel *prompt = new QLabel(promptText, frame);
    pageRLayout->addWidget(prompt);
    pageRLayout->addStretch(10);
}

// ****************************************************************************
// Method: QvisScatterPlotWizard::nextid
//
// Purpose: 
//   This method returns the id of the next wizard page.
//
// Returns:    The id of the next wizard page.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  8 16:13:07 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
QvisScatterPlotWizard::nextId() const
{
    int id;

    //
    // Figure out the previous and next pages for the current page.
    //
    if(currentPage() == page0)
    {
        // Choose Y coord
        id = 1;
    }
    else if(currentPage() == page1)
    {
        // Choose Z coord?
        id = selectZCoord ? 2 : 3;
    }
    else if(currentPage() == page2)
    {
        // Choose Z coord
        id = 3;
    }
    else if(currentPage() == page3)
    {
        // Choose color?
        id = selectColor ? 4 : 5;
    }
    else if(currentPage() == page4)
    {
        // Choose color
        id = 5;
    }
    else if(currentPage() == page5)
    {
        // Finish
        id = 5;
    }

    // Turn off the current page's animation.
    scatter[currentId()]->setAllowAnimation(false);

    return id;
}

//
// Qt slots
//

void
QvisScatterPlotWizard::choseYVariable(const QString &var)
{
    scatterAtts->SetVar2(var.toStdString());
    scatterAtts->SetVar2Role(ScatterAttributes::Coordinate1);
}

void
QvisScatterPlotWizard::choseZVariable(const QString &var)
{
    scatterAtts->SetVar3(var.toStdString());
    scatterAtts->SetVar3Role(ScatterAttributes::Coordinate2);
}

void
QvisScatterPlotWizard::choseColorVariable(const QString &var)
{
    scatterAtts->SetVar4(var.toStdString());
    scatterAtts->SetVar4Role(ScatterAttributes::Color);
}

void
QvisScatterPlotWizard::decideZ(int index)
{
    selectZCoord = (index == 0);
    scatter[3]->setThreeD(selectZCoord);
    scatter[4]->setThreeD(selectZCoord);
    scatter[5]->setThreeD(selectZCoord);
    if(!selectZCoord)
        scatterAtts->SetVar3Role(ScatterAttributes::None);
    else
        scatterAtts->SetVar3Role(ScatterAttributes::Coordinate2);
}

void
QvisScatterPlotWizard::decideColor(int index)
{
    selectColor = (index == 0);
    scatter[4]->setColoredPoints(selectColor);
    scatter[5]->setColoredPoints(selectColor);
    if(!selectColor)
        scatterAtts->SetVar4Role(ScatterAttributes::None);
    else
        scatterAtts->SetVar3Role(ScatterAttributes::Color);
}

void
QvisScatterPlotWizard::updateAnimationFlags(int newId)
{
    if(newId >= 0 && newId <= 5)
    {
        // Turn on animation for the new page if it needs it.
        scatter[newId]->setAllowAnimation(true);

        // Disable the Continue/Next button if we're on the last page.
        QAbstractButton *b = button(QWizard::NextButton);
        if(newId == 5 && b != 0)
            b->setEnabled(false);
    }
}
