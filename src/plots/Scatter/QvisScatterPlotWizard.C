#include <QvisScatterPlotWizard.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>

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
//   
// ****************************************************************************

QvisScatterPlotWizard::QvisScatterPlotWizard(AttributeSubject *s,
    QWidget *parent, const char *name) : QvisWizard(s, parent, name)
{
    selectZCoord = true;
    selectColor = true;

    // Set some defaults into the wizard's local copy of the plot attributes.
    scatterAtts->SetVar1Role(ScatterAttributes::Coordinate0);
    scatterAtts->SetVar2("default");
    scatterAtts->SetVar2Role(ScatterAttributes::Coordinate1);
    scatterAtts->SetVar3("default");
    scatterAtts->SetVar3Role(ScatterAttributes::Coordinate2);
    scatterAtts->SetVar4("default");
    scatterAtts->SetVar4Role(ScatterAttributes::Color);

    // Set the wizard's title.
    topLevelWidget()->setCaption("Scatter plot wizard");

    //
    // Create the wizard's first page.
    //
    CreateVariablePage(&page1, &scatter1,
        "Choose a variable to use for the Scatter\n"
        "plot's Y coordinate.",
        SLOT(choseYVariable(const QString &)),
        true, false, false);
    addPage(page1, "Choose Y coordinate");
    setHelpEnabled(page1, false);

    CreateYesNoPage(&page2, &scatter2, &bg2,
        "Would you like to choose a variable to use\n"
        "as the Scatter plot's Z coordinate?.",
        SLOT(decideZ(int)), true, true, false);
    addPage(page2, "Choose Z coordinate");
    setHelpEnabled(page2, false);

    CreateVariablePage(&page3, &scatter3,
        "Choose a variable to use for the Scatter\n"
        "plot's Z coordinate.", SLOT(choseZVariable(const QString &)),
        true, true, false);
    addPage(page3, "Choose Z coordinate");
    setHelpEnabled(page3, false);

    CreateYesNoPage(&page4, &scatter4, &bg4,
        "Would you like to choose a variable to use\n"
        "as the Scatter plot's color?.",
        SLOT(decideColor(int)), false, true /* depends on selectZCoord */, true);
    addPage(page4, "Choose color variable");
    setHelpEnabled(page4, false);

    CreateVariablePage(&page5, &scatter5,
        "Choose a variable to use for the Scatter\n"
        "plot's color.", SLOT(choseColorVariable(const QString &)),
        false, true /* could be false*/, true);
    addPage(page5, "Choose color variable");
    setHelpEnabled(page5, false);

    CreateFinishPage(&page6, &scatter6,
        "Click the Finish button to create a new Scatter plot",
        false, true /* could be false*/, true);
    addPage(page6, "Click Finish");
    setHelpEnabled(page6, false);
    setFinishEnabled(page6, true);
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
    delete page1;
    delete page2;
    delete page3;
    delete page4;
    delete page5;
    delete page6;
    delete bg2;
    delete bg4;
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
//   
// ****************************************************************************

void
QvisScatterPlotWizard::CreateVariablePage(QFrame **f, QvisScatterWidget **s,
    const char *promptText,
    const char *slot, bool highlight, bool threeD, bool colorPoints)
{
    QFrame *frame = new QFrame(0, "frame");
    *f = frame;
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);
    QHBoxLayout *pageLRLayout = new QHBoxLayout(frameinnerLayout);
    pageLRLayout->setSpacing(10);

    QvisScatterWidget *scatter = new QvisScatterWidget(frame, "scatter");
    scatter->setHighlightAxis(highlight);
    scatter->setThreeD(threeD);
    scatter->setColoredPoints(colorPoints);
    *s = scatter;
    pageLRLayout->addWidget(scatter);
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
//   
// ****************************************************************************

void
QvisScatterPlotWizard::CreateYesNoPage(QFrame **f, QvisScatterWidget **s,
    QButtonGroup **b, const char *promptText,
    const char *slot, bool highlight, bool threeD, bool colorPoints)
{
    QFrame *frame = new QFrame(0, "frame");
    *f = frame;
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);
    QHBoxLayout *pageLRLayout = new QHBoxLayout(frameinnerLayout);
    pageLRLayout->setSpacing(10);

    QvisScatterWidget *scatter = new QvisScatterWidget(frame, "scatter");
    scatter->setHighlightAxis(highlight);
    scatter->setThreeD(threeD);
    scatter->setColoredPoints(colorPoints);
    *s = scatter;
    pageLRLayout->addWidget(scatter);
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

    QButtonGroup *btn = new QButtonGroup(0, "btn");
    *b = btn;
    connect(btn, SIGNAL(clicked(int)),
            this, slot);
    QRadioButton *r1 = new QRadioButton("Yes", frame, "r1");
    r1->setChecked(true);
    btn->insert(r1);
    pageVLayout->addWidget(r1);
    QRadioButton *r2 = new QRadioButton("No", frame, "r2");
    btn->insert(r2);
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
//   
// ****************************************************************************

void
QvisScatterPlotWizard::CreateFinishPage(QFrame **f, QvisScatterWidget **s,
    const char *promptText,
    bool highlight, bool threeD, bool colorPoints)
{
    QFrame *frame = new QFrame(0, "frame");
    *f = frame;
    frame->setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(frame);
    frameinnerLayout->setMargin(0);
    frameinnerLayout->addSpacing(10);
    QHBoxLayout *pageLRLayout = new QHBoxLayout(frameinnerLayout);
    pageLRLayout->setSpacing(10);

    QvisScatterWidget *scatter = new QvisScatterWidget(frame, "scatter");
    scatter->setHighlightAxis(highlight);
    scatter->setThreeD(threeD);
    scatter->setColoredPoints(colorPoints);
    *s = scatter;
    pageLRLayout->addWidget(scatter);
    pageLRLayout->addSpacing(10); // or a line?

    QVBoxLayout *pageRLayout = new QVBoxLayout(pageLRLayout);
    pageRLayout->setSpacing(10);
    QLabel *prompt = new QLabel(promptText, frame, "prompt");
    pageRLayout->addWidget(prompt);
    pageRLayout->addStretch(10);
}

// ****************************************************************************
// Method: QvisScatterPlotWizard::appropriate
//
// Purpose: 
//   This method determines which wizard pages can be viewed based on some
//   of the responses.
//
// Arguments:
//   page : The page being considered for its appropriateness.
//
// Returns:    True if the page can be displayed; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:59:36 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
QvisScatterPlotWizard::appropriate(QWidget *p) const
{
    bool retval = true;
    QWidget *prevPage = 0;
    QWidget *nextPage = 0;
    int index = 0;

    //
    // Figure out the previous and next pages for the current page.
    //
    if(currentPage() == page1)
    {
        // Choose Y coord
        index = 0;
        prevPage = page1;
        nextPage = page2;
    }
    else if(currentPage() == page2)
    {
        // Choose Z coord?
        index = 1;
        prevPage = page1;
        nextPage = selectZCoord ? page3 : page4;
    }
    else if(currentPage() == page3)
    {
        // Choose Z coord
        index = 2;
        prevPage = page2;
        nextPage = page4;
    }
    else if(currentPage() == page4)
    {
        // Choose color?
        index = 3;
        prevPage = selectZCoord ? page3 : page2;
        nextPage = selectColor ? page5 : page6;
    }
    else if(currentPage() == page5)
    {
        // Choose color
        index = 4;
        prevPage = page4;
        nextPage = page6;
    }
    else if(currentPage() == page6)
    {
        // Finish
        index = 5;
        prevPage = selectColor ? page5 : page4;
        nextPage = page6;
    }

    // Given the current page and the previous input, determine
    // which pages are okay to show.
    bool okayToShow[6];
    int i;
    for(i = 0; i < 6; ++i)
        okayToShow[i] = false;
    okayToShow[index] = true;
    for(i = 0; i < 6; ++i)
    {
        if(page(i) == prevPage)
            okayToShow[i] = true;
        if(page(i) == nextPage)
            okayToShow[i] = true;
    }

    // Determine whether the page being considered is okay to show.
    for(i = 0; i < 6; ++i)
    {
        if(page(i) == p)
        {
            retval = okayToShow[i];
            break;
        }
    }
    
    return retval;
}

//
// Qt slots
//

void
QvisScatterPlotWizard::choseYVariable(const QString &var)
{
    scatterAtts->SetVar2(var.latin1());
    scatterAtts->SetVar2Role(ScatterAttributes::Coordinate1);
}

void
QvisScatterPlotWizard::choseZVariable(const QString &var)
{
    scatterAtts->SetVar3(var.latin1());
    scatterAtts->SetVar3Role(ScatterAttributes::Coordinate2);
}

void
QvisScatterPlotWizard::choseColorVariable(const QString &var)
{
    scatterAtts->SetVar4(var.latin1());
    scatterAtts->SetVar4Role(ScatterAttributes::Color);
}

void
QvisScatterPlotWizard::decideZ(int index)
{
    selectZCoord = (index == 0);
    scatter4->setThreeD(selectZCoord);
    scatter5->setThreeD(selectZCoord);
    scatter6->setThreeD(selectZCoord);
    if(!selectZCoord)
        scatterAtts->SetVar3Role(ScatterAttributes::None);
    else
        scatterAtts->SetVar3Role(ScatterAttributes::Coordinate2);
}

void
QvisScatterPlotWizard::decideColor(int index)
{
    selectColor = (index == 0);
    scatter5->setColoredPoints(selectColor);
    scatter6->setColoredPoints(selectColor);
    if(!selectColor)
        scatterAtts->SetVar4Role(ScatterAttributes::None);
    else
        scatterAtts->SetVar3Role(ScatterAttributes::Color);
}
