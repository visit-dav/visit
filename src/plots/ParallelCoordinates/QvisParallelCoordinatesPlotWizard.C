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
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>

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
//    Brad Whitlock, Wed Apr 23 10:12:44 PDT 2008
//    Added tr()
//  
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
// ****************************************************************************

QvisParallelCoordinatesPlotWizard::QvisParallelCoordinatesPlotWizard(
    AttributeSubject *s, QWidget *parent, const std::string &varName,
    bool doNothing) 
    : QvisWizard(s, parent)
{

    if (doNothing)
    {
        parAxisAtts->ResetAxes();
        AddFinishPage(tr("Click Finish"),
                      tr("The plot has successfully been "
                         "defined by use of an array variable."));
        return;    
    }

    InitializeParallelCoordinatesAttributes(varName);
    
    axisVarNames.push_back(varName);

    axisYesNos.push_back(true); 
    axisYesNos.push_back(true); 
    
    dupVarMessages.append(NULL);
    
    curAxisCount = 1;
    
    // Set the wizard title.
    topLevelWidget()->setWindowTitle(tr("ParallelCoordinates Plot Wizard"));
    
    //
    // Create wizard pages.
    //
    
    AddAxisVariablePage(1, tr("Choose second axis variable"),
                tr("A ParallelCoordinates plot needs at least 2 axes.\n"
                "Choose a scalar variable for the second axis."));
    
    AddAxisYesNoPage(2, tr("Add third axis?"),
                     tr("Would you like to add a third axis to the plot?"));
    AddAxisVariablePage(2, tr("Choose third axis variable"),
                        tr("Choose a scalar variable for the third axis."));
    
    AddAxisYesNoPage(3, tr("Add fourth axis?"),
                     tr("Would you like to add a fourth axis to the plot?"));
    AddAxisVariablePage(3, tr("Choose forth axis variable"),
                        tr("Choose a scalar variable for the forth axis."));
    
    AddAxisYesNoPage(4, tr("Add fifth axis?"),
                     tr("Would you like to add a fourth axis to the plot?"));
    AddAxisVariablePage(4, tr("Choose fifth axis variable"),
                        tr("Choose a scalar variable for the fifth axis."));
    
    AddAxisYesNoPage(5, tr("Add sixth axis?"),
                     tr("Would you like to add a fourth axis to the plot?"));
    AddAxisVariablePage(5, tr("Choose sixth axis variable"),
                        tr("Choose a scalar variable for the sixth axis."));
    
    AddAxisYesNoPage(6, tr("Add seventh axis?"),
                     tr("Would you like to add a seventh axis to the plot?"));
    AddAxisVariablePage(6, tr("Choose seventh axis variable"),
                        tr("Choose a scalar variable for the seventh axis."));

    AddAxisYesNoPage(7, tr("Add eigth axis?"),
                     tr("Would you like to add a eigth axis to the plot?"));
    AddAxisVariablePage(7, tr("Choose eigth axis variable"),
                        tr("Choose a scalar variable for the eigth axis."));

    AddFinishPage(tr("Click Finish"),
        tr("Click the Finish button to create a new ParallelCoordinates plot."));
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
// ****************************************************************************

QvisParallelCoordinatesPlotWizard::~QvisParallelCoordinatesPlotWizard()
{
    // Delete parentless widgets.
    
    foreach(QWizardPage *page, pages)
        delete page;
    pages.clear();
    
    yesNoGroups.clear();
}
// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::validateCurrentPage
//
// Purpose: 
//   Validates the current wizard page's inputs before moving 
//   to the next. It ensures that duplicate axis varibles are not selected, 
//   and refreshs the preview displays on the next page.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jul 21 12:32:18 PDT 2008
//
//  
// Modifications:
//
// ****************************************************************************

bool
QvisParallelCoordinatesPlotWizard::validateCurrentPage()
{
    int id = currentId();
    
    // if we are on the last page, we are golden
    if(id == pages.count() -1)
        return true;
    
    // find next page
    int nextId = GetNextPageId();
             
     if((id & 1) == 0)
     {
        // if a axis var selection page, check for dups
        std::string newVarName = axisVarNames[curAxisCount];

        if (!UniqueAxisVariableName(newVarName))
        {
            // dont let a dupe var pass
            dupVarMessages[curAxisCount]->show();
            return false;
        }
        else
        {
            // add a new var
            parAxisAtts->InsertAxis(newVarName);
            curAxisCount++;
            // make sure we have enough room for the next var.
            if(axisVarNames.size() < curAxisCount)
                axisVarNames.push_back("");
        }
    }
    
    // check if next page is final, update its thumbnail
    if(nextId == pages.count() -1) 
    {
        thumbnails[nextId]->setNumberOfAxes(curAxisCount);
        thumbnails[nextId]->setAxisTitles(axisVarNames);
        thumbnails[nextId]->redrawAllAxes(true);
    }
    else
    {
        // update the thumbnail for current page
        thumbnails[nextId]->setAxisTitles(axisVarNames);
        thumbnails[nextId]->redrawAllAxes(false);
    }
        
    return true;
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::cleanupPage
//
// Purpose: 
//   Called when the user clicks the back button. Keeps the currentAxisCount 
//   and atts  in sync.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jul 21 12:32:18 PDT 2008
//
//  
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::cleanupPage(int id)
{
    if((id & 1) != 0 && id != pages.count() -1)
        parAxisAtts->DeleteAxis(axisVarNames[--curAxisCount], 1);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::AddAxisVariablePage
//
// Purpose: 
//   Creates a wizard page that enables the user to select a scalar variable
//   for a specified axis in the plot.
//
// Note: This is intended to emulate the style of the QvisScatterPlotWizard
//       for the Scatter plot, which came first. It was Refactored for Qt4 
//       from Mark Blair's CreateAxisVariablePage().
//
//
// Arguments:
//   axisOrdinal : Ordinal number of the axis whose variable is being chosen
//   title       : Page title.
//   promptText  : User prompt to display
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jul 21 12:32:18 PDT 2008
//
//  
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::AddAxisVariablePage(int axisOrdinal, 
                                                       const QString &title,
                                                       const QString &promptText)
{
    const std::vector<std::string> names = parAxisAtts->GetScalarAxisNames();
    std::string leftAxisVarName = names[names.size()-1];

    QWizardPage *page = new QWizardPage();
    page->setTitle(title);
    pages.append(page);
        
    QHBoxLayout *pageLRLayout = new QHBoxLayout(page);

    QvisParallelCoordinatesWidget *thumbnail =
        new QvisParallelCoordinatesWidget(page);
    thumbnail->setNumberOfAxes(axisOrdinal + 1);
    thumbnail->setAxisTitles(axisVarNames);
    thumbnails.append(thumbnail);
    //*s = thumbnail;
    
    pageLRLayout->addWidget(thumbnail);

    QVBoxLayout *pageRLayout = new QVBoxLayout();
    pageLRLayout->addLayout(pageRLayout);
    QLabel *prompt = new QLabel(promptText, page);
    pageRLayout->addWidget(prompt);
    
    // add interior.
    QHBoxLayout *pageVLayout = new QHBoxLayout();
    pageRLayout->addLayout(pageVLayout);
        
    QLabel *varlabel = new QLabel(tr("Variable"), page);
    QvisVariableButton *var = new QvisVariableButton(false, false, true,
                                       QvisVariableButton::Scalars,page);
    var->setMinimumWidth(fontMetrics().boundingRect("really_really_long_var_name").width());
    var->setText(QString(leftAxisVarName.c_str()));
    connect(var, SIGNAL(activated(const QString &)), this,
            SLOT(choseAxisVariable(const QString &)));
    pageVLayout->addWidget(varlabel);
    pageVLayout->addWidget(var);
    pageVLayout->addStretch(5);

    QLabel *dupeLabel = new QLabel(
        tr("Selected variable duplicates variable of another axis.\n"
           "Choose a unique scalar variable for the new axis."), page);
    dupVarMessages.append(dupeLabel);
    QPalette palette;
    palette.setColor( QPalette::Foreground, QColor( Qt::red ) );
    dupeLabel->setPalette(palette);
    pageRLayout->addWidget(dupeLabel);
    dupeLabel->hide();
    pageRLayout->addStretch(5);
    
    axisVarNames.push_back(leftAxisVarName);
    addPage(page);
}

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::AddAxisYesNoPage
//
// Purpose: 
//   Creates a wizard page that enables the user to click "Yes" or "No" to
//   indicate whether to add another axis to the plot.
//
// Note: This is intended to emulate the style of the QvisScatterPlotWizard
//       for the Scatter plot, which came first. It was Refactored for Qt4 
//       from Mark Blair's CreateAxisVariablePage().
//
// Arguments:
//   axisOrdinal : Ordinal number of the axis whose variable is being chosen
//   title       : Page title.
//   promptText  : User prompt to display

//
// Programmer: Cyrus Harrison
// Creation:   Mon Jul 21 12:32:18 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::AddAxisYesNoPage(int axisOrdinal, 
                                                    const QString &title,
                                                    const QString &promptText)
{
    QWizardPage *page = new QWizardPage();
    page->setTitle(title);
    pages.append(page);
    
    QHBoxLayout *pageLRLayout = new QHBoxLayout(page);
    
    QvisParallelCoordinatesWidget *thumbnail =
        new QvisParallelCoordinatesWidget(page);
    thumbnail->setNumberOfAxes(axisOrdinal + 1);
    thumbnail->setAxisTitles(axisVarNames);
    pageLRLayout->addWidget(thumbnail);
    thumbnails.append(thumbnail);

    QVBoxLayout *pageRLayout = new QVBoxLayout();
    pageLRLayout->addLayout(pageRLayout);
    QLabel *prompt = new QLabel(promptText, page);
    pageRLayout->addWidget(prompt);

    // add interior.
    QHBoxLayout *pageVLayout = new QHBoxLayout();
    pageRLayout->addLayout(pageVLayout);

    QButtonGroup *btn = new QButtonGroup(page);
    yesNoGroups.append(btn);
    
    connect(btn, SIGNAL(buttonClicked(int)), 
            this, SLOT(decideIfAnotherAxis(int)));
    
    QRadioButton *r1 = new QRadioButton(tr("Yes"), page);
    r1->setChecked(false);
    btn->addButton(r1,0);
    pageVLayout->addStretch(5);
    pageVLayout->addWidget(r1);
    QRadioButton *r2 = new QRadioButton(tr("No"), page);
    r2->setChecked(true);
    btn->addButton(r2,1);
    pageVLayout->addWidget(r2);
    pageVLayout->addStretch(5);
    axisYesNos.push_back(false);
    pageRLayout->addStretch(5);
    addPage(page);
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::AddFinishPage
//
// Purpose: 
//   Creates final wizard page
//
// Note: This is intended to emulate the style of the QvisScatterPlotWizard
//       for the Scatter plot, which came first. It was Refactored for Qt4 
//       from Mark Blair's CreateAxisVariablePage().
//
//
// Arguments:
//   title       : Page title.
//   promptText  : User prompt to display.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Jul 21 12:32:18 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWizard::AddFinishPage(const QString &title,
                                                 const QString &promptText)
{
    QWizardPage *page= new QWizardPage();
    page->setTitle(title);
    page->setFinalPage(true);
    pages.append(page);
    
    QHBoxLayout *pageLRLayout = new QHBoxLayout(page);
    
    QvisParallelCoordinatesWidget *thumbnail = 
                new QvisParallelCoordinatesWidget(page);
    thumbnails.append(thumbnail);
    pageLRLayout->addWidget(thumbnail);

    if (!promptText.isEmpty())
    {
        QVBoxLayout *pageRLayout = new QVBoxLayout();
        pageLRLayout->addLayout(pageRLayout);
        QLabel *prompt = new QLabel(promptText, page);
        pageRLayout->addWidget(prompt);
        pageRLayout->addStretch(10);
    }
    addPage(page);
}
// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWizard::GetNextPageId()
//
// Purpose: Calculates the next page id.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Jul 22 09:32:03 PDT 2008
//
// Modifications:
//
// ****************************************************************************

int
QvisParallelCoordinatesPlotWizard::GetNextPageId() const
{    
    int id = currentId();
    int nextId = id + 1;
    // check for yes/no page
    if( (id & 1) != 0 )
        if (!axisYesNos[curAxisCount]) // if "no", go to final page
            nextId = pages.count()-1;
    return nextId;
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
    
    int curPageIndex = currentId();
    std::string newVarName = varName.toStdString();

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
    axisYesNos[curAxisCount] = (buttonIndex == 0);
}


int
QvisParallelCoordinatesPlotWizard::nextId() const
{
    return GetNextPageId();
}
