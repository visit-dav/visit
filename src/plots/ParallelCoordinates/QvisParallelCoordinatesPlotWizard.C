/*****************************************************************************
*
* Copyright (c) 2000 - 2008, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisParallelCoordinatesPlotWizard.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include <QvisParallelCoordinatesWidget.h>
#include <QvisVariableButton.h>
#include <ParallelCoordinatesAttributes.h>

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
//   varNeme : Name of the plot's scalar variable
//   name    : Wizard's name
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
// ****************************************************************************

QvisParallelCoordinatesPlotWizard::QvisParallelCoordinatesPlotWizard(
    AttributeSubject *s, QWidget *parent, const std::string &varName,
    bool doNothing, const char *name) : QvisWizard(s, parent, name)
{
    for (int pageNum = 0; pageNum < MAX_WIZARD_SELECTABLE_AXES*2 - 2; pageNum++)
        pages[pageNum] = NULL;
    for (int groupNum = 0; groupNum < MAX_WIZARD_SELECTABLE_AXES - 2; groupNum++)
        yesNoButtonGroups[groupNum] = NULL;


    if (doNothing)
    {
        parAxisAtts->ResetAxes();
        CreateFinishPage(&pages[0], NULL, "The plot has successfully been "
                         "defined by use of an array variable.");
        addPage(pages[0], "Click Finish");
        setHelpEnabled(pages[0], false);
        setFinishEnabled(pages[0], true);
        return;    
    }

    InitializeParallelCoordinatesAttributes(varName);
    
    axisVarNames[0] = varName;
    axisYesNos[0] = true; axisYesNos[1] = true;
    
    curAxisCount = 1;
    
    // Set the wizard's title.
    topLevelWidget()->setCaption("ParallelCoordinates Plot Wizard");
    
    //
    // Create the wizard's pages.
    //
    CreateAxisVariablePage(&pages[0], &thumbnails[0], 1,
        "A ParallelCoordinates plot needs at least 2 axes.\n"
        "Choose a scalar variable for the second axis.");
    addPage(pages[0], "Choose second axis variable");

    CreateAxisYesNoPage(&pages[1], &thumbnails[1], &yesNoButtonGroups[0], 2,
        "Would you like to add a third axis to the plot?");
    addPage(pages[1], "Add third axis?");

    CreateAxisVariablePage(&pages[2], &thumbnails[2], 2,
        "Choose a scalar variable for the third axis.");
    addPage(pages[2], "Choose third axis variable");

    CreateAxisYesNoPage(&pages[3], &thumbnails[3], &yesNoButtonGroups[1], 3,
        "Would you like to add a fourth axis to the plot?");
    addPage(pages[3], "Add fourth axis?");

    CreateAxisVariablePage(&pages[4], &thumbnails[4], 3,
        "Choose a scalar variable for the fourth axis.");
    addPage(pages[4], "Choose fourth axis variable");

    CreateAxisYesNoPage(&pages[5], &thumbnails[5], &yesNoButtonGroups[2], 4,
        "Would you like to add a fifth axis to the plot?");
    addPage(pages[5], "Add fifth axis?");

    CreateAxisVariablePage(&pages[6], &thumbnails[6], 4,
        "Choose a scalar variable for the fifth axis.");
    addPage(pages[6], "Choose fifth axis variable");

    CreateAxisYesNoPage(&pages[7], &thumbnails[7], &yesNoButtonGroups[3], 5,
        "Would you like to add a sixth axis to the plot?");
    addPage(pages[7], "Add sixth axis?");

    CreateAxisVariablePage(&pages[8], &thumbnails[8], 5,
        "Choose a scalar variable for the sixth axis.");
    addPage(pages[8], "Choose sixth axis variable");

    CreateAxisYesNoPage(&pages[9], &thumbnails[9], &yesNoButtonGroups[4], 6,
        "Would you like to add a seventh axis to the plot?");
    addPage(pages[9], "Add seventh axis?");

    CreateAxisVariablePage(&pages[10], &thumbnails[10], 6,
        "Choose a scalar variable for the seventh axis.");
    addPage(pages[10], "Choose seventh axis variable");

    CreateAxisYesNoPage(&pages[11], &thumbnails[11], &yesNoButtonGroups[5], 7,
        "Would you like to add an eighth axis to the plot?");
    addPage(pages[11], "Add eighth axis?");

    CreateAxisVariablePage(&pages[12], &thumbnails[12], 7,
        "Choose a scalar variable for the eighth axis.");
    addPage(pages[12], "Choose eighth axis variable");

    CreateFinishPage(&pages[13], &thumbnails[13],
        "Click the Finish button to create a new ParallelCoordinates plot.");
    addPage(pages[13], "Click Finish");
    
    for (int pageNum = 0; pageNum < 14; pageNum++)
        setHelpEnabled(pages[pageNum], false);

    setFinishEnabled(pages[13], true);
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
//   
// ****************************************************************************

QvisParallelCoordinatesPlotWizard::~QvisParallelCoordinatesPlotWizard()
{
    // Delete parentless widgets.
    
    for (int pageNum = 0; pageNum < MAX_WIZARD_SELECTABLE_AXES*2 - 2; pageNum++)
    {
        delete pages[pageNum];
    }
    
    for (int groupNum = 0; groupNum < MAX_WIZARD_SELECTABLE_AXES - 2; groupNum++)
    {
        delete yesNoButtonGroups[groupNum];
    }
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::CreateAxisVariablePage
//
// Purpose: 
//   Creates a wizard page that enables the user to select a scalar variable
//   for a specified axis in the plot.
//
// Note: This is intended to emulate the style of the QvisScatterPlotWizard
//       for the Scatter plot, which came first.
//
// Arguments:
//   f           : Return variable for the frame widget pointer
//   s           : Return variable for the ParallelCoordinates widget pointer
//   axisOrdinal : Ordinal number of the axis whose variable is being chosen
//   promptText  : User prompt to display
//
// Programmer: Mark Blair
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Modifications:
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.  Added
//    visualAxisNames.
//   
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::CreateAxisVariablePage(QFrame **f,
    QvisParallelCoordinatesWidget **s, int axisOrdinal, const char *promptText)
{
    const std::vector<std::string> names = parAxisAtts->GetScalarAxisNames();
    std::string leftAxisVarName = names[names.size()-1];

    QFrame *frame = new QFrame(NULL, "frame");
    *f = frame;
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);
    QHBoxLayout *pageLRLayout = new QHBoxLayout(frameinnerLayout);
    pageLRLayout->setSpacing(10);

    QvisParallelCoordinatesWidget *thumbnail =
        new QvisParallelCoordinatesWidget(frame, "thumbnail");
    thumbnail->setNumberOfAxes(axisOrdinal + 1);
    thumbnail->setAxisTitles(axisVarNames);
    *s = thumbnail;
    pageLRLayout->addWidget(thumbnail);
    pageLRLayout->addSpacing(10); // or a line?

    QVBoxLayout *pageRLayout = new QVBoxLayout(pageLRLayout);
    pageRLayout->setSpacing(10);
    QLabel *prompt = new QLabel(promptText, frame, "prompt");
    pageRLayout->addWidget(prompt);
    pageRLayout->addSpacing(10);

    // add interior.
    QHBoxLayout *pageVLayout = new QHBoxLayout(pageRLayout);
    pageVLayout->setSpacing(10);
    pageVLayout->addStretch(5);
    QLabel *varlabel = new QLabel("Variable", frame);
    QvisVariableButton *var = new QvisVariableButton(false, false, true,
        QvisVariableButton::Scalars, frame);
    var->setMinimumWidth(fontMetrics().boundingRect("really_really_long_var_name").width());
    var->setText(QString(leftAxisVarName.c_str()));
    connect(var, SIGNAL(activated(const QString &)), this,
            SLOT(choseAxisVariable(const QString &)));
    pageVLayout->addWidget(varlabel);
    pageVLayout->addWidget(var);
    pageVLayout->addStretch(5);

    pageRLayout->addSpacing(25);
    dupVarMessages[axisOrdinal] = new QLabel(
        "Selected variable duplicates variable of another axis.\n"
        "Choose a unique scalar variable for the new axis.", frame, "errMsg");
    dupVarMessages[axisOrdinal]->setPaletteForegroundColor(QColor(255,0,0));
    pageRLayout->addWidget(dupVarMessages[axisOrdinal]);
    dupVarMessages[axisOrdinal]->hide();

    pageRLayout->addStretch(10);
    
    axisVarNames[axisOrdinal] = leftAxisVarName;
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::CreateAxisYesNoPage
//
// Purpose: 
//   Creates a wizard page that enables the user to click "Yes" or "No" to
//   indicate whether to add another axis to the plot.
//
// Note: This is intended to emulate the style of the QvisScatterPlotWizard
//       for the Scatter plot, which came first.
//
// Arguments:
//   f           : Return variable for the frame widget pointer.
//   s           : Return variable for the ParallelCoordinates widget pointer.
//   bg          : Return variable for the button group.
//   axisOrdinal : Ordinal number of the axis whose variable is being chosen
//   promptText  : User prompt to display.
//
// Programmer: Mark Blair
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::CreateAxisYesNoPage(QFrame **f, QvisParallelCoordinatesWidget **s,
    QButtonGroup **bg, int axisOrdinal, const char *promptText)
{
    QFrame *frame = new QFrame(NULL, "frame");
    *f = frame;
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);
    QHBoxLayout *pageLRLayout = new QHBoxLayout(frameinnerLayout);
    pageLRLayout->setSpacing(10);

    QvisParallelCoordinatesWidget *thumbnail =
        new QvisParallelCoordinatesWidget(frame, "thumbnail");
    thumbnail->setNumberOfAxes(axisOrdinal + 1);
    thumbnail->setAxisTitles(axisVarNames);
    *s = thumbnail;
    pageLRLayout->addWidget(thumbnail);
    pageLRLayout->addSpacing(10); // or a line?

    QVBoxLayout *pageRLayout = new QVBoxLayout(pageLRLayout);
    pageRLayout->setSpacing(10);
    QLabel *prompt = new QLabel(promptText, frame, "prompt");
    pageRLayout->addWidget(prompt);
    pageRLayout->addSpacing(10);

    // add interior.
    QHBoxLayout *pageVLayout = new QHBoxLayout(pageRLayout);
    pageVLayout->setSpacing(10);
    pageVLayout->addStretch(5);

    QButtonGroup *btn = new QButtonGroup(NULL, "btn");
    *bg = btn;
    connect(btn, SIGNAL(clicked(int)), this, SLOT(decideIfAnotherAxis(int)));
    QRadioButton *r1 = new QRadioButton("Yes", frame, "r1");
    r1->setChecked(false);
    btn->insert(r1);
    pageVLayout->addWidget(r1);
    QRadioButton *r2 = new QRadioButton("No", frame, "r2");
    r2->setChecked(true);
    btn->insert(r2);
    pageVLayout->addWidget(r2);
    pageVLayout->addStretch(5);

    pageRLayout->addStretch(10);
    
    axisYesNos[axisOrdinal] = false;
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::CreateFinishPage
//
// Purpose: 
//   Creates final wizard page
//
// Note: This is intended to emulate the style of the QvisScatterPlotWizard
//       for the Scatter plot, which came first.
//
// Arguments:
//   f           : Return variable for the frame widget pointer.
//   s           : Return variable for the ParallelCoordinates widget pointer.
//   promptText  : User prompt to display.
//
// Programmer: Mark Blair
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Modifications:
//    Jeremy Meredith, Thu Feb  7 12:59:42 EST 2008
//    Allow either the widget or prompt text to be NULL.
//   
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::CreateFinishPage(QFrame **f,
    QvisParallelCoordinatesWidget **s, const char *promptText)
{
    QFrame *frame = new QFrame(NULL, "frame");
    *f = frame;
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);
    QHBoxLayout *pageLRLayout = new QHBoxLayout(frameinnerLayout);
    pageLRLayout->setSpacing(10);

    if (s)
    {
        QvisParallelCoordinatesWidget *thumbnail =
            new QvisParallelCoordinatesWidget(frame, "thumbnail");
        *s = thumbnail;
        pageLRLayout->addWidget(thumbnail);
        pageLRLayout->addSpacing(10); // or a line?
    }

    if (promptText)
    {
        QVBoxLayout *pageRLayout = new QVBoxLayout(pageLRLayout);
        pageRLayout->setSpacing(10);
        QLabel *prompt = new QLabel(promptText, frame, "prompt");
        pageRLayout->addWidget(prompt);
        pageRLayout->addStretch(10);
    }
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::InitializeParallelCoordinatesAttributes
//
// Purpose: Initializes local copy of plot's attributes
//
// Arguments:
//    varName : Name of the scalar variable for the plot's left axis
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
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::InitializeParallelCoordinatesAttributes(
                                                         const std::string &varName)
{
    stringVector saxisNames;
    stringVector vaxisNames;
    doubleVector extMins;
    doubleVector extMaxs;
    
    saxisNames.push_back(varName);
    vaxisNames.push_back(varName);
    extMins.push_back(-1e+37);
    extMaxs.push_back(+1e+37);

    parAxisAtts->SetScalarAxisNames(saxisNames);
    parAxisAtts->SetVisualAxisNames(vaxisNames);
    parAxisAtts->SetExtentMinima(extMins);
    parAxisAtts->SetExtentMaxima(extMaxs);
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::UniqueAxisVariableName
//
// Purpose: Determines if a variable name is the same as that for an axis
//          already added to the plot.
//
// Arguments:
//    varName : Name of the scalar variable to be checked
//
// Returns: true if variable is unique, false otherwise
//
// Programmer: Mark Blair
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
QvisParallelCoordinatesPlotWizard::UniqueAxisVariableName(
                                                       const std::string &varName)
{
    int axisNum;

    for (axisNum = 0; axisNum < curAxisCount; axisNum++ )
    {
        if (varName == axisVarNames[axisNum]) break;
    }
    
    return ((axisNum >= curAxisCount));
}


//
// Qt slots
//

void
QvisParallelCoordinatesPlotWizard::choseAxisVariable(const QString &varName)
{
    int curPageIndex = indexOf(currentPage());
    std::string newVarName = varName.latin1();

    axisVarNames[curAxisCount] = newVarName;
    
    if (UniqueAxisVariableName(newVarName))
    {
        thumbnails[curPageIndex]->setAxisTitles(axisVarNames);
        thumbnails[curPageIndex]->redrawAllAxes(true);

        dupVarMessages[curAxisCount]->hide();
    }
    else
    {
        dupVarMessages[curAxisCount]->show();
    }
}


void
QvisParallelCoordinatesPlotWizard::decideIfAnotherAxis(int buttonIndex)
{
    axisYesNos[indexOf(currentPage())/2 + 2] = (buttonIndex == 0);
}


void
QvisParallelCoordinatesPlotWizard::next()
{
    int curPageIndex = indexOf(currentPage());
    int nextPageIndex = curPageIndex + 1;

    if ((curPageIndex & 1) == 0)   // Select axis variable name page
    {
        std::string newVarName = axisVarNames[curAxisCount];
        
        if (!UniqueAxisVariableName(newVarName))
        {
            dupVarMessages[curAxisCount]->show();
            return;
        }

        dupVarMessages[curAxisCount]->hide();
        
        parAxisAtts->InsertAxis(newVarName);
        curAxisCount++;
        
        if (nextPageIndex == MAX_WIZARD_SELECTABLE_AXES*2 - 3)   // Finish page
        {
            thumbnails[nextPageIndex]->setNumberOfAxes(curAxisCount);
            thumbnails[nextPageIndex]->setAxisTitles(axisVarNames);
            thumbnails[nextPageIndex]->redrawAllAxes(true);
        }
        else
        {
            thumbnails[nextPageIndex+1]->setAxisTitles(axisVarNames);
            thumbnails[nextPageIndex  ]->setAxisTitles(axisVarNames);
            thumbnails[nextPageIndex  ]->redrawAllAxes(false);
        }
    }
    else   // New axis "Yes or No" page
    {
        if (axisYesNos[curAxisCount])
        {
            thumbnails[nextPageIndex]->redrawAllAxes(false);
        }
        else    // Finish page
        {
            nextPageIndex = MAX_WIZARD_SELECTABLE_AXES*2 - 3;

            thumbnails[nextPageIndex]->setNumberOfAxes(curAxisCount);
            thumbnails[nextPageIndex]->setAxisTitles(axisVarNames);
            thumbnails[nextPageIndex]->redrawAllAxes(true);
        }
    }

    showPage(pages[nextPageIndex]);
}


void
QvisParallelCoordinatesPlotWizard::back()
{
    int curPageIndex = indexOf(currentPage());
    int prevPageIndex = curPageIndex - 1;
    
    if ((curPageIndex & 1) != 0)   // New axis "Yes or No" page, or Finish page
    {
        parAxisAtts->DeleteAxis(axisVarNames[--curAxisCount], 1);

        if (curPageIndex == MAX_WIZARD_SELECTABLE_AXES*2 - 3)   // Finish page
        {
            prevPageIndex = (curAxisCount == 1) ? 0 : curAxisCount*2 - 3;
        }
    }
    else
    {
        dupVarMessages[curAxisCount]->hide();
    }

    thumbnails[prevPageIndex]->redrawAllAxes(false);

    showPage(pages[prevPageIndex]);
}
