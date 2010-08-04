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

#include <QvisCMFEWizard.h>

#include <ExpressionList.h>
#include <GlobalAttributes.h>
#include <WindowInformation.h>

#include <DebugStream.h>
#include <FileServerList.h>
#include <NameSimplifier.h>
#include <QualifiedFilename.h>
#include <VariableMenuPopulator.h>

#include <avtExpressionTypeConversions.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTextEdit>
#include <QTimer>
#include <QToolTip>
#include <QTreeWidget>
#include <QWidget>

#include <QvisDialogLineEdit.h>
#include <QvisVariableButton.h>

#include <ViewerProxy.h>

#include <snprintf.h>

#define DONOR_FILE 0
#define DONOR_TIME 1
#define DONOR_WITHIN_FILE 2

#define INTERP_CONN_CMFE 0
#define INTERP_POS_CMFE  1

#define TIME_TYPE_SIMTIME  0
#define TIME_TYPE_SIMCYCLE 1
#define TIME_TYPE_INDEX    2

#define FILL_VAR       0
#define FILL_CONSTANT  1

#define EXPRESSION_SIMPLE      0
#define EXPRESSION_DIFF_FIRST  1
#define EXPRESSION_DIFF_SECOND 2
#define EXPRESSION_ABS_DIFF    3


int QvisCMFEWizard::timesCompleted = 0;


// ****************************************************************************
// Method: QvisCMFEWizard::QvisCMFEWizard
//
// Purpose: 
//   This is the constructor for the QvisCMFEWizard class.
//
// Arguments:
//   atts   : A pointer to the attribute subject that the wizard will set.
//   parent : The wizard's parent.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// ****************************************************************************

QvisCMFEWizard::QvisCMFEWizard(AttributeSubject *atts, QWidget *parent) : 
    QvisWizard(atts, parent)
{
    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::HaveHelpButton, false);
    setOption(QWizard::HaveNextButtonOnLastPage, false);

    decision_donorType = DONOR_TIME;
    decision_source = "";
    decision_absolute = true;
    decision_timeType = TIME_TYPE_INDEX;
    decision_time = 0.0;
    decision_cycle = 0;
    decision_index = 0;
    decision_variable = "";
    decision_mesh = "";
    decision_interp = INTERP_POS_CMFE;
    decision_fill = FILL_CONSTANT;
    decision_fillvar = "";
    decision_fillval = 0.0;
    decision_exprtype = EXPRESSION_SIMPLE;
    decision_diffvarname = "";

    char str[1024];
    SNPRINTF(str, 1024, "cmfe%d", timesCompleted);
    decision_exprname = str;
    timesCompleted++;

    globalAtts = NULL;
    windowInfo = NULL;
    exprList = NULL;

    // Set the wizard's title.
    setWindowTitle(tr("Data-Level Comparison Wizard"));

    // Create the wizard pages.
    CreateIntroPage();              // page0
    CreateDonorTypePage();          // page1
    CreateFileSelectionPage();      // page2
    CreateTimeSpecificationPage();  // page3
    CreateDonorAndTargetPage();     // page4
    CreateInterpSelectionPage();    // page5
    CreateActivityPage();           // page6

    setStartId(Page_Intro);
}

// ****************************************************************************
// Method: QvisCMFEWizard::~QvisCMFEWizard
//
// Purpose: 
//   Destructor for the QvisCMFEWizard class.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// ****************************************************************************

QvisCMFEWizard::~QvisCMFEWizard()
{
    ;
}

// ****************************************************************************
//  Method: QvisCMFEWizard::SetExpressionList
//
//  Purpose:
//      Sets the expression list.
//
//  Programmer: Hank Childs
//  Creation:   August 2, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::SetExpressionList(ExpressionList *e)
{
    exprList = e;
}


// ****************************************************************************
// Method: QvisCMFEWizard::Exec
//
// Purpose: 
//   This method executes the wizard.
//
// Returns:    A return value for the wizard, which will be used to tell the
//             user what to do.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// ****************************************************************************

int
QvisCMFEWizard::Exec()
{
    raise();
    activateWindow();
    restart();

    // Go through the wizard.
    int retval = exec();

    return retval;
}

// ****************************************************************************
// Method: QvisCMFEWizard::nextId
//
// Purpose: 
//   This method is called when we want to know the id of the next wizard page.
//
// Arguments:
//
// Returns:    The id of the next wizard page.
//
// Note:       
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// Modifications:
//   
// ****************************************************************************

int
QvisCMFEWizard::nextId() const
{
    int id;

    switch(currentId())
    {
    case Page_Intro:
        id = Page_DonorType;
        break;
    case Page_DonorType:
        if (decision_donorType == DONOR_FILE)
            id = Page_FileSelection;
        else if (decision_donorType == DONOR_TIME)
            id = Page_TimeSpecification;
        else 
            id = Page_DonorAndTargetSpecification;
        break;
    case Page_FileSelection:
        id = Page_DonorAndTargetSpecification;
        break;
    case Page_TimeSpecification:
        id = Page_DonorAndTargetSpecification;
        break;
    case Page_DonorAndTargetSpecification:
        id = Page_InterpSelection;
        break;
    case Page_InterpSelection:
        id = Page_ActivityDescription;
        break;
    case Page_ActivityDescription:
        id = -1;
        break;
    }

    return id;
}

// ****************************************************************************
// Method: QvisCMFEWizard::validateCurrentPage
//
// Purpose: 
//   This method is called before we go to the next page. If this method
//   returns false then the wizard won't go to the next page.
//
// Arguments:
//
// Returns:    True if the wizard can go to the next page.
//
// Note:       
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// Modifications:
//   
// ****************************************************************************

bool
QvisCMFEWizard::validateCurrentPage()
{
    bool valid = true;

    switch(currentId())
    {
    case Page_FileSelection:
        valid = (decision_source != "");
        break;
    case Page_DonorAndTargetSpecification:
        if (decision_variable == "")
            valid = false;
        if (decision_mesh == "" && decision_donorType != DONOR_WITHIN_FILE)
            valid = false;
        break;
    case Page_InterpSelection:
        if (decision_interp == INTERP_POS_CMFE && decision_fill == FILL_VAR
            && decision_fillvar == "")
            valid = false;
    case Page_ActivityDescription:
        if (decision_diffvarname == "" && decision_exprtype != EXPRESSION_SIMPLE)
            valid = false;
        if (decision_exprname == "")
            valid = false;
    default:
        valid = true;
    }

    return valid;
}

// ****************************************************************************
// Method: QvisCMFEWizard::initializePage
//
// Purpose: 
//   This method is called when a page is shown and we call it so the page
//   gets updated with the right values from the decisions.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::initializePage(int pageId)
{
    char buff[1024];
    switch(pageId)
    {
    case Page_DonorType:
        // Initialize page1's button group with the donor type.
        page1_buttongroup->blockSignals(true);
        page1_buttongroup->button((decision_donorType == DONOR_TIME ? 0 : 
                                  (decision_donorType == DONOR_FILE ? 1 : 2)))->setChecked(true);
        page1_buttongroup->blockSignals(false);
        break;
    case Page_FileSelection:
        UpdateSourceList();
        break;
    case Page_TimeSpecification:
        // Initialize page3's button group with the time specification.
        page3_buttongroup1->blockSignals(true);
        page3_buttongroup1->button((decision_absolute?0:1))->setChecked(true);
        page3_buttongroup1->blockSignals(false);
        page3_buttongroup2->blockSignals(true);
        page3_buttongroup2->button(
                  (decision_timeType == TIME_TYPE_SIMTIME ? 0
                   : (decision_timeType == TIME_TYPE_SIMCYCLE ? 1 : 2)))->setChecked(true);
        page3_buttongroup2->blockSignals(false);
        page3_lineEditTime->blockSignals(true);
        SNPRINTF(buff, 1024, "%f", decision_time);
        page3_lineEditTime->setText(tr(buff));
        page3_lineEditTime->setEnabled(decision_timeType == TIME_TYPE_SIMTIME);
        page3_lineEditTime->blockSignals(false);
        page3_lineEditCycle->blockSignals(true);
        SNPRINTF(buff, 1024, "%d", decision_cycle);
        page3_lineEditCycle->setText(tr(buff));
        page3_lineEditCycle->setEnabled(decision_timeType == TIME_TYPE_SIMCYCLE);
        page3_lineEditCycle->blockSignals(false);
        page3_lineEditIndex->blockSignals(true);
        SNPRINTF(buff, 1024, "%d", decision_index);
        page3_lineEditIndex->setText(tr(buff));
        page3_lineEditIndex->setEnabled(decision_timeType == TIME_TYPE_INDEX);
        page3_lineEditIndex->blockSignals(false);
        break;
    case Page_DonorAndTargetSpecification:
        UpdateDonorFields();
        UpdateMeshField();
        break;
    case Page_InterpSelection:
        page5_buttongroup2->blockSignals(true);
        page5_buttongroup2->button(decision_interp == INTERP_CONN_CMFE ? 0 : 1)->setChecked(true);
        page5_buttongroup2->blockSignals(false);
        if (decision_interp == INTERP_POS_CMFE)
        {
            page5_button1->setEnabled(true);
            page5_button2->setEnabled(true);
            if (decision_fill == FILL_CONSTANT)
            {
                page5_lineEdit->setEnabled(true);
                page5_overlapVariable->setEnabled(false);
            }
            else
            {
                page5_lineEdit->setEnabled(false);
                page5_overlapVariable->setEnabled(true);
            }
            page5_buttongroup1->button(decision_fill == FILL_CONSTANT ? 0 : 1)->setChecked(true);
        }
        else
        {
            page5_button1->setEnabled(false);
            page5_button2->setEnabled(false);
            page5_lineEdit->setEnabled(false);
            page5_overlapVariable->setEnabled(false);
        }
        break;
    case Page_ActivityDescription:
        page6_buttongroup1->blockSignals(true);
        page6_buttongroup1->button(decision_exprtype == EXPRESSION_SIMPLE ? 0 : 1)->setChecked(true);
        page6_buttongroup1->blockSignals(false);
        if (decision_exprtype == EXPRESSION_SIMPLE)
        {
            page6_buttongroup2->button(0)->setEnabled(false);
            page6_buttongroup2->button(1)->setEnabled(false);
            page6_buttongroup2->button(2)->setEnabled(false);
        }
        else
        {
            page6_buttongroup2->button(0)->setEnabled(true);
            page6_buttongroup2->button(1)->setEnabled(true);
            page6_buttongroup2->button(2)->setEnabled(true);
            if (decision_exprtype == EXPRESSION_DIFF_FIRST)
                page6_buttongroup2->button(1)->setChecked(true);
            else if (decision_exprtype == EXPRESSION_DIFF_SECOND)
                page6_buttongroup2->button(2)->setChecked(true);
            else if (decision_exprtype == EXPRESSION_ABS_DIFF)
                page6_buttongroup2->button(0)->setChecked(true);
        }
        break;

    default:
        break;
    }
}


// ****************************************************************************
// Method: QvisCMFEWizard::CreateIntroPage
//
// Purpose:
//   Creates the intro page.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::CreateIntroPage(void)
{
    page0 = new QWizardPage(this);
    page0->setTitle(tr("Data-Level Comparisons"));
    page0->setSubTitle(tr("Cross-mesh field evaluations (CMFEs) take a field "
                "from a donor mesh and place that field on a target mesh.  "
                "This wizard will set up a CMFE."));

    // Add the page.
    setPage(Page_Intro, page0);
}


// ****************************************************************************
// Method: QvisCMFEWizard::CreateDonorTypePage
//
// Purpose:
//   Creates the donor type page.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::CreateDonorTypePage(void)
{
    page1 = new QWizardPage(this);
    page1->setTitle(tr("Donor Type"));
    page1->setSubTitle(tr("Describe the source of the data"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page1);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    hCenterLayout->addStretch(5);
    QVBoxLayout *buttonLayout = new QVBoxLayout(0);
    hCenterLayout->addLayout(buttonLayout);
    hCenterLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    page1_buttongroup = new QButtonGroup(page1);
    QRadioButton *r1 = new QRadioButton(tr("A different time slice from this database"), page1);
    r1->setChecked(true);
    page1_buttongroup->addButton(r1, 0);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("A different file"), page1);
    page1_buttongroup->addButton(r2, 1);
    buttonLayout->addWidget(r2);

    QRadioButton *r3 = new QRadioButton(tr("Between meshes in the same file"), page1);
    page1_buttongroup->addButton(r3, 2);
    buttonLayout->addWidget(r3);

    pageLayout->addStretch(10);
    connect(page1_buttongroup, SIGNAL(buttonClicked(int)),
            this, SLOT(page1_donorTypeChanged(int)));

    // Add the page.
    setPage(Page_DonorType, page1);
}


// ****************************************************************************
// Method: QvisCMFEWizard::CreateFileSelectionPage
//
// Purpose:
//   Creates the page to select the file.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::CreateFileSelectionPage(void)
{
    page2 = new QWizardPage(this);
    page2->setTitle(tr("Donor File"));
    page2->setSubTitle(tr("Which file will the field come from?  (If you don't"
                           " see the source you want, then cancel this wizard, open the"
                           " file and try again.)"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page2);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    hCenterLayout->addStretch(5);

    QWidget *page2_description_vbox = new QWidget(page2);
    QVBoxLayout *comboLayout = new QVBoxLayout(0);
    hCenterLayout->addLayout(comboLayout);
    hCenterLayout->addStretch(5);
    comboLayout->setSpacing(5);

    QLabel *sourceLabel = new QLabel(tr("Donor file"), page2_description_vbox);
    sourceLabel->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    comboLayout->addWidget(sourceLabel);
    page2_sourceComboBox = new QComboBox();
    comboLayout->addWidget(page2_sourceComboBox);

    connect(page2_sourceComboBox, SIGNAL(activated(int)),
            this, SLOT(page2_sourceChanged(int)));

    // Add the page.
    setPage(Page_FileSelection, page2);
}


// ****************************************************************************
// Method: QvisCMFEWizard::CreateTimeSpecificationPage
//
// Purpose:
//   Creates the page to specify the time.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::CreateTimeSpecificationPage(void)
{
    page3 = new QWizardPage(this);
    page3->setTitle(tr("Specify Donor Time Slice"));
    page3->setSubTitle(tr("Which time slice do you want to extract data from?"));

    QWidget *page3_widget = new QWidget(page3);

    QVBoxLayout *pageLayout = new QVBoxLayout(page3);
    //pageLayout->setSpacing(10);

    QLabel *timeLabel = new QLabel(tr("<u>Time Type and Value</u>"), page3_widget);
    //timeLabel->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    pageLayout->addWidget(timeLabel);

    QGridLayout *glayout = new QGridLayout(0);
    pageLayout->addLayout(glayout);

    page3_buttongroup2 = new QButtonGroup(page3);
    QRadioButton *r21 = new QRadioButton(tr("Simulation time"), page3);
    page3_buttongroup2->addButton(r21, 0);
    //pageLayout->addWidget(r21);
    glayout->addWidget(r21, 0, 0);

    page3_lineEditTime = new QLineEdit(page3);
    page3_lineEditTime->setText(tr("0"));
    //pageLayout->addWidget(page3_lineEdit);
    glayout->addWidget(page3_lineEditTime, 0, 1);
    connect(page3_lineEditTime, SIGNAL(textChanged(const QString &)),
            this, SLOT(page3_timeChanged(const QString &)));

    QRadioButton *r22 = new QRadioButton(tr("Simulation cycle"), page3);
    page3_buttongroup2->addButton(r22, 1);
    //pageLayout->addWidget(r22);
    glayout->addWidget(r22, 1, 0);

    page3_lineEditCycle = new QLineEdit(page3);
    page3_lineEditCycle->setText(tr("0"));
    //pageLayout->addWidget(page3_lineEdit);
    glayout->addWidget(page3_lineEditCycle, 1, 1);
    connect(page3_lineEditCycle, SIGNAL(textChanged(const QString &)),
            this, SLOT(page3_cycleChanged(const QString &)));

    QRadioButton *r23 = new QRadioButton(tr("Time index"), page3);
    r23->setChecked(true);
    page3_buttongroup2->addButton(r23, 2);
    //pageLayout->addWidget(r23);
    glayout->addWidget(r23, 2, 0);

    page3_lineEditIndex = new QLineEdit(page3);
    page3_lineEditIndex->setText(tr("0"));
    //pageLayout->addWidget(page3_lineEdit);
    glayout->addWidget(page3_lineEditIndex, 2, 1);
    connect(page3_lineEditIndex, SIGNAL(textChanged(const QString &)),
            this, SLOT(page3_indexChanged(const QString &)));

    QLabel *descLabel = new QLabel(tr("(Time index is the most robust and works in all circumstances)"), page3_widget);
    pageLayout->addWidget(descLabel);

    //pageLayout->addStretch(10);
    connect(page3_buttongroup2, SIGNAL(buttonClicked(int)),
            this, SLOT(page3_timeTypeChanged(int)));

    QFrame *hline1 = new QFrame(page3_widget);
    hline1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    pageLayout->addWidget(hline1);

    QLabel *arLabel = new QLabel(tr("<u>Absolute Versus Relative Time</u>"), page3_widget);
    //arLabel->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    pageLayout->addWidget(arLabel);

    page3_buttongroup1 = new QButtonGroup(page3);
    QRadioButton *r1 = new QRadioButton(tr("I want to specify an absolute time; the time slice I specify should not change as I change the time slider"), page3);
    r1->setChecked(true);
    page3_buttongroup1->addButton(r1, 0);
    pageLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("I want to specify a relative time; the time slice I want will change as I change the time slider"), page3);
    page3_buttongroup1->addButton(r2, 1);
    pageLayout->addWidget(r2);

    //pageLayout->addStretch(10);
    connect(page3_buttongroup1, SIGNAL(buttonClicked(int)),
            this, SLOT(page3_absoluteTimeChanged(int)));

    QFrame *hline2 = new QFrame(page3_widget);
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    pageLayout->addWidget(hline2);

    QLabel *exLabel = new QLabel(tr("<u>Examples</u>"), page3_widget);
    //exLabel->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    pageLayout->addWidget(exLabel);

    QLabel *ex1 = new QLabel(tr("#1: \"Simulation time\" with \"10.5\" and absolute time will always return the data from simulation time 10.5."), page3_widget);
    pageLayout->addWidget(ex1);
    QLabel *ex2 = new QLabel(tr("#2: \"Time index\" with \"-1\" and relative time will always return the data from the time slice immediately preceding the current one."), page3_widget);
    pageLayout->addWidget(ex2);

    // Add the page.
    setPage(Page_TimeSpecification, page3);
}


// ****************************************************************************
// Method: QvisCMFEWizard::CreateDonorAndTargetPage
//
// Purpose:
//   Creates the donor and target specification page.
//
// Programmer: Hank Childs
// Creation:   August 2, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::CreateDonorAndTargetPage(void)
{
    page4 = new QWizardPage(this);
    page4->setTitle(tr("Specify exact donor and target"));
    page4->setSubTitle(tr("Which field do you want from the donor?  Which mesh should that mesh be placed on?"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page4);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    hCenterLayout->addStretch(5);

    QWidget *page4_description_vbox = new QWidget(page4);
    QVBoxLayout *comboLayout = new QVBoxLayout(0);
    hCenterLayout->addLayout(comboLayout);
    hCenterLayout->addStretch(5);
    comboLayout->setSpacing(5);

    QLabel *donorLabel = new QLabel(tr("Donor field"), page4_description_vbox);
    donorLabel->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    comboLayout->addWidget(donorLabel);
    page4_donorVariable = new QvisCustomSourceVariableButton(false, false, NULL, 
                     QvisBaseVariableButton::Scalars |
                     QvisBaseVariableButton::Vectors |
                     QvisBaseVariableButton::Tensors,
                     page4_description_vbox);
    comboLayout->addWidget(page4_donorVariable);

    connect(page4_donorVariable, SIGNAL(activated(const QString &)),
            this, SLOT(page4_donorChanged(const QString &)));

    QHBoxLayout *hCenterLayout2 = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout2);
    hCenterLayout2->addStretch(5);

    QWidget *page4_description_vbox2 = new QWidget(page4);
    QVBoxLayout *comboLayout2 = new QVBoxLayout(0);
    hCenterLayout2->addLayout(comboLayout2);
    hCenterLayout2->addStretch(5);
    comboLayout2->setSpacing(5);

    QLabel *targetLabel = new QLabel(tr("Target mesh"), page4_description_vbox2);
    targetLabel->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    comboLayout2->addWidget(targetLabel);
    page4_targetMesh = new QvisVariableButton(false, false, false, 
                     QvisBaseVariableButton::Meshes,
                     page4_description_vbox2);
    comboLayout2->addWidget(page4_targetMesh);

    connect(page4_targetMesh, SIGNAL(activated(const QString &)),
            this, SLOT(page4_targetChanged(const QString &)));

    // Add the page.
    setPage(Page_DonorAndTargetSpecification, page4);
}


// ****************************************************************************
// Method: QvisCMFEWizard::CreateInterpSelectionPage
//
// Purpose:
//   Creates the page to specify how to do the interpolation.
//
// Programmer: Hank Childs
// Creation:   August 2, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::CreateInterpSelectionPage(void)
{
    page5 = new QWizardPage(this);
    page5->setTitle(tr("Specify Evaluation Scheme"));
    page5->setSubTitle(tr("Do you want to use indexing or interpolation?  How will you handle regions of non-overlap?"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page5);
    //pageLayout->setSpacing(10);

    QWidget *page5_widget = new QWidget(page5);

    QLabel *evalLabel = new QLabel(tr("<u>Evaluation Scheme</u>"), page5_widget);
    pageLayout->addWidget(evalLabel);

    QGridLayout *glayout = new QGridLayout(0);
    pageLayout->addLayout(glayout);
    glayout->setColumnStretch(0, 0);
    glayout->setColumnStretch(1, 100);

    page5_buttongroup2 = new QButtonGroup(page5);
    QRadioButton *r21 = new QRadioButton(tr("Connectivity-based CMFE"));
    page5_buttongroup2->addButton(r21, 0);
    glayout->addWidget(r21, 0, 0);
    QLabel *connLabel1 = new QLabel(tr("The value at index 'i' of the target is assigned"), page5_widget);
    connLabel1->setAlignment(Qt::AlignLeft);
    glayout->addWidget(connLabel1, 0, 1);
    QLabel *connLabel2 = new QLabel(tr("the value of index 'i' from the donor.  This is efficient,"), page5_widget);
    connLabel2->setAlignment(Qt::AlignLeft);
    glayout->addWidget(connLabel2, 1, 1);
    QLabel *connLabel3 = new QLabel(tr("but requires that the meshes are congruent."), page5_widget);
    connLabel3->setAlignment(Qt::AlignLeft);
    glayout->addWidget(connLabel3, 2, 1);

    QFrame *hline2 = new QFrame(page5_widget);
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    glayout->addWidget(hline2, 3, 1);

    QRadioButton *r22 = new QRadioButton(tr("Position-based CMFE"));
    page5_buttongroup2->addButton(r22, 1);
    glayout->addWidget(r22, 4, 0);

    QLabel *posLabel1 = new QLabel(tr("The value at index 'i' from the target is evaluated"), page5_widget);
    posLabel1->setAlignment(Qt::AlignLeft);
    glayout->addWidget(posLabel1, 4, 1);
    QLabel *posLabel2 = new QLabel(tr("by first determining its location and then determining the"), page5_widget);
    posLabel2->setAlignment(Qt::AlignLeft);
    glayout->addWidget(posLabel2, 5, 1);
    QLabel *posLabel3 = new QLabel(tr("value at that location from the donor mesh.  This is less"), page5_widget);
    posLabel3->setAlignment(Qt::AlignLeft);
    glayout->addWidget(posLabel3, 6, 1);
    QLabel *posLabel4 = new QLabel(tr("efficient, but is robust and works on non-congruent meshes."), page5_widget);
    posLabel4->setAlignment(Qt::AlignLeft);
    glayout->addWidget(posLabel4, 7, 1);

    connect(page5_buttongroup2, SIGNAL(buttonClicked(int)),
            this, SLOT(page5_interpChanged(int)));

    QFrame *hline1 = new QFrame(page5_widget);
    hline1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    pageLayout->addWidget(hline1);

    QLabel *olLabel = new QLabel(tr("<u>Handling For Non-Overlapping Regions</u>"), page5_widget);
    pageLayout->addWidget(olLabel);

    QGridLayout *glayout2 = new QGridLayout(0);
    pageLayout->addLayout(glayout2);

    page5_buttongroup1 = new QButtonGroup(page5);
    QRadioButton *r11 = new QRadioButton(tr("Use a constant"));
    page5_buttongroup1->addButton(r11, 0);
    glayout2->addWidget(r11, 0, 0);
    page5_button1 = r11;

    page5_lineEdit = new QLineEdit(page5);
    page5_lineEdit->setText(tr("0"));
    glayout2->addWidget(page5_lineEdit, 0, 1);
    connect(page5_lineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page5_overlapConstChanged(const QString &)));

    QRadioButton *r12 = new QRadioButton(tr("Use a variable"));
    page5_buttongroup1->addButton(r12, 1);
    glayout2->addWidget(r12, 1, 0);
    page5_button2 = r12;

    page5_overlapVariable = new QvisVariableButton(false, false, false, 
                     QvisBaseVariableButton::Scalars |
                     QvisBaseVariableButton::Vectors |
                     QvisBaseVariableButton::Tensors,
                     page5_widget);
    glayout2->addWidget(page5_overlapVariable, 1, 1);
    connect(page5_overlapVariable, SIGNAL(activated(const QString &)),
            this, SLOT(page5_overlapVarChanged(const QString &)));

    connect(page5_buttongroup1, SIGNAL(buttonClicked(int)),
            this, SLOT(page5_fillChanged(int)));

    // Add the page.
    setPage(Page_InterpSelection, page5);
}


// ****************************************************************************
// Method: QvisCMFEWizard::CreateActivityPage
//
// Purpose:
//   Creates the details of the expression.
//
// Programmer: Hank Childs
// Creation:   August 3, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::CreateActivityPage(void)
{
    page6 = new QWizardPage(this);
    page6->setTitle(tr("Setting Up The Expression"));
    page6->setSubTitle(tr("How do you want to use your new CMFE expression?  What is its name?"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page6);

    QWidget *page6_widget = new QWidget(page6);

    QGridLayout *glayout2 = new QGridLayout(0);
    pageLayout->addLayout(glayout2);
    QLabel *expLabel = new QLabel(tr("Name of Expression"), page6_widget);
    glayout2->addWidget(expLabel, 0, 0);

    page6_lineEdit = new QLineEdit(page6);
    page6_lineEdit->setText(tr(decision_exprname.c_str()));
    glayout2->addWidget(page6_lineEdit, 0, 1);
    connect(page6_lineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(page6_exprNameChanged(const QString &)));

    QFrame *hline2 = new QFrame(page6_widget);
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    pageLayout->addWidget(hline2);

    QLabel *quesLabel = new QLabel(tr("<u>What do you want to do with the donor field?<u>"), page6_widget);
    pageLayout->addWidget(quesLabel);
    page6_buttongroup1 = new QButtonGroup(page6);
    QRadioButton *r1 = new QRadioButton(tr("Simply place it on the target mesh (you can then use this field in the expression editor to do more complex things)"));
    page6_buttongroup1->addButton(r1, 0);
    pageLayout->addWidget(r1);

    QGridLayout *glayout = new QGridLayout(0);
    pageLayout->addLayout(glayout);
    glayout->setColumnStretch(0, 0);
    glayout->setColumnStretch(1, 100);

    QRadioButton *r00 = new QRadioButton(tr("Difference with "));
    page6_buttongroup1->addButton(r00, 1);
    glayout->addWidget(r00, 0, 0);
    connect(page6_buttongroup1, SIGNAL(buttonClicked(int)),
            this, SLOT(page6_exprTypeChanged(int)));

    page6_diffVariable1 = new QvisVariableButton(false, false, false, 
                     QvisBaseVariableButton::Scalars |
                     QvisBaseVariableButton::Vectors |
                     QvisBaseVariableButton::Tensors,
                     page6_widget);
    glayout->addWidget(page6_diffVariable1, 0, 1, Qt::AlignLeft);
    connect(page6_diffVariable1, SIGNAL(activated(const QString &)),
            this, SLOT(page6_diffVariable1Changed(const QString &)));

    page6_buttongroup2 = new QButtonGroup(page6);
    QRadioButton *r11 = new QRadioButton(tr("Absolute value of difference"));
    page6_buttongroup2->addButton(r11, 0);
    glayout->addWidget(r11, 1, 1);
    QRadioButton *r21 = new QRadioButton(tr("Donor field minus variable"));
    page6_buttongroup2->addButton(r21, 1);
    glayout->addWidget(r21, 2, 1);
    QRadioButton *r31 = new QRadioButton(tr("Variable minus donor field"));
    page6_buttongroup2->addButton(r31, 2);
    glayout->addWidget(r31, 3, 1);
    connect(page6_buttongroup2, SIGNAL(buttonClicked(int)),
            this, SLOT(page6_diffTypeChanged(int)));

    // Add the page.
    setPage(Page_ActivityDescription, page6);
}

// ****************************************************************************
// Method: QvisCMFEWizard::UpdateSourceList
//
// Purpose:
//     Updates the source combo box when the donor type is another file.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::UpdateSourceList()
{
    if (globalAtts == NULL || windowInfo == NULL)
    {
        debug1 << "Cannot set sources up because of internal error" << endl;
        return;
    }
    const stringVector &sources     = globalAtts->GetSources();
    const std::string &activeSource = windowInfo->GetActiveSource();

    // See if the active source is in the list.
    int i, sourceIndex = -1;
    for(i = 0; i < sources.size(); ++i)
    {
        if(activeSource == sources[i])
        {
            sourceIndex = i;
            break;
        }
    }

    page2_sourceComboBox->blockSignals(true);

    //
    // Simplify the current source names and put the short names into
    // the source combo box.
    //
    NameSimplifier simplifier;
    for(i = 0; i < sources.size(); ++i)
        simplifier.AddName(sources[i]);

    stringVector shortSources;
    simplifier.GetSimplifiedNames(shortSources);

    page2_sourceComboBox->clear();
    for(i = 0; i < shortSources.size(); ++i)
        page2_sourceComboBox->addItem(shortSources[i].c_str());

    //
    // Set the current item.
    //
    if(sourceIndex != -1 && sourceIndex != page2_sourceComboBox->currentIndex())
        page2_sourceComboBox->setCurrentIndex(sourceIndex);

    page2_sourceComboBox->blockSignals(false);

    // initialize the field, in case the user clicks next without
    // changing the selection.
    page2_sourceChanged(sourceIndex);
}


// ****************************************************************************
//  Method: QvisCMFEWizard::UpdateDonorFields
//
//  Purpose:
//      Updates the variable drop down with the fields from the donor file.
//
//  Programmer: Hank Childs
//  Creation:   August 2, 2010
//
// ****************************************************************************

void 
QvisCMFEWizard::UpdateDonorFields(void)
{
    VariableMenuPopulator *populator = new VariableMenuPopulator;

    bool useAltFile = decision_donorType == DONOR_FILE;
    std::string filename = (useAltFile ? decision_source
                            : windowInfo->GetActiveSource());
    if (filename == "notset")
        return;

    // we need metadata and sil for current state
    const avtDatabaseMetaData *md = fileServer->GetMetaData(filename,
                                 GetStateForSource(filename),
                                !FileServerList::ANY_STATE,
                                 FileServerList::GET_NEW_MD);

    const avtSIL *sil =
        fileServer->GetSIL(filename,
                           GetStateForSource(filename),
                          !FileServerList::ANY_STATE,
                           FileServerList::GET_NEW_MD);

    populator->PopulateVariableLists(filename, md, sil, exprList,false);

    page4_donorVariable->ResetPopulator(populator);
}


// ****************************************************************************
//  Method: QvisCMFEWizard::UpdateMeshField
//
//  Purpose:
//      Sets the target mesh, in the case there is only one mesh.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::UpdateMeshField(void)
{
    std::string meshname = GetMeshForActiveSource();
    if (meshname != "")
    {
        QString qmeshname = meshname.c_str();
        page4_targetMesh->setDefaultVariable(qmeshname);
        page4_targetMesh->setText(qmeshname);
        decision_mesh = meshname;
    }
}


// ****************************************************************************
//  Method: QvisCMFEWizard::GetVarType
//
//  Purpose:
//      Gets the variable of the donor field.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2010
//
// ****************************************************************************

Expression::ExprType
QvisCMFEWizard::GetVarType(const std::string &str)
{
    int numExpressions = exprList->GetNumExpressions();
    for (int i = 0 ; i < numExpressions ; i++)
    {
        Expression &e = exprList->GetExpressions(i);
        if (str == e.GetName())
            return e.GetType();
    }

    bool useAltFile = decision_donorType == DONOR_FILE;
    std::string filename = (useAltFile ? decision_source
                            : windowInfo->GetActiveSource());
    if (filename == "notset")
        return Expression::ScalarMeshVar;

    // we need metadata and sil for current state
    const avtDatabaseMetaData *md = fileServer->GetMetaData(filename,
                                 GetStateForSource(filename),
                                !FileServerList::ANY_STATE,
                                 FileServerList::GET_NEW_MD);
    avtVarType vt = AVT_UNKNOWN_TYPE;
    TRY
    {
        vt = md->DetermineVarType(str, true);
    }
    CATCH (VisItException)
    {
        vt = AVT_SCALAR_VAR;
    }
    ENDTRY

    return avtVarType_To_ExprType(vt);
}


// ****************************************************************************
//  Method: QvisCMFEWizard::GetMeshForActiveSource
//
//  Purpose:
//      Gets the mesh for the active source.  Returns the empty string if the
//      there are multiple meshes.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2010
//
// ****************************************************************************

std::string
QvisCMFEWizard::GetMeshForActiveSource(void)
{
    if (windowInfo == NULL)
        return "";

    std::string filename = windowInfo->GetActiveSource();
    if (filename == "notset")
        return "";

    // we need metadata and sil for current state
    const avtDatabaseMetaData *md = fileServer->GetMetaData(filename,
                                 GetStateForSource(filename),
                                !FileServerList::ANY_STATE,
                                 FileServerList::GET_NEW_MD);
    int nMeshes = md->GetNumMeshes();
    if (nMeshes != 1)
        return "";
    return md->GetMeshes(0).name;
}


// ****************************************************************************
//  Method: QvisCMFEWizard::AddCMFEExpression
//
//  Purpose:
//      We've made all the decisions through the wizard.  Now build the
//      expression and add it to the list.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2010
//
// ****************************************************************************

void
QvisCMFEWizard::AddCMFEExpression(void)
{
    Expression *e = new Expression();
    e->SetName(decision_exprname);

    char filepart[1024];
    if (decision_donorType == DONOR_FILE)
    {
        const char *src = decision_source.c_str();
        const char *after_colon = strstr(src, ":");
        if (after_colon != NULL)
            src = after_colon+1;
        strcpy(filepart, src);
    }
    else if (decision_donorType == DONOR_TIME)
    {
        if (decision_timeType == TIME_TYPE_SIMTIME)
            SNPRINTF(filepart, 1024, "[%f]t%s", decision_time, 
                                               (decision_absolute ? "" : "d"));
        else if (decision_timeType == TIME_TYPE_SIMCYCLE)
            SNPRINTF(filepart, 1024, "[%d]c%s", decision_cycle, 
                                               (decision_absolute ? "" : "d"));
        else if (decision_timeType == TIME_TYPE_INDEX)
            SNPRINTF(filepart, 1024, "[%d]i%s", decision_index, 
                                               (decision_absolute ? "" : "d"));
    }
    else
        strcpy(filepart, "[0]id"); // hack for the current time slice

    char cmfe_part[1024];
    if (decision_interp == INTERP_CONN_CMFE)
    {
        SNPRINTF(cmfe_part, 1024, "conn_cmfe(<%s:%s>, %s)", 
                                          filepart, decision_variable.c_str(), 
                                          decision_mesh.c_str());
    }
    else
    {
        char fillstr[1024];
        if (decision_fill == FILL_CONSTANT)
            SNPRINTF(fillstr, 1024, "%f", decision_fillval);
        else
            strcpy(fillstr, decision_fillvar.c_str());

        SNPRINTF(cmfe_part, 1024, "pos_cmfe(<%s:%s>, %s, %s)", 
                                          filepart, decision_variable.c_str(), 
                                          decision_mesh.c_str(), fillstr);
    }

    char defn[1024];
    if (decision_exprtype == EXPRESSION_SIMPLE)
        strcpy(defn, cmfe_part);
    else if (decision_exprtype == EXPRESSION_DIFF_FIRST)
        SNPRINTF(defn, 1024, "%s-%s", cmfe_part, decision_diffvarname.c_str());
    else if (decision_exprtype == EXPRESSION_DIFF_SECOND)
        SNPRINTF(defn, 1024, "%s-%s", decision_diffvarname.c_str(), cmfe_part);
    else
        SNPRINTF(defn, 1024, "abs(%s-%s)", decision_diffvarname.c_str(), cmfe_part);

    e->SetDefinition(defn);
    e->SetType(GetVarType(decision_variable));
    exprList->AddExpressions(*e);
    delete e;

    // Tell the viewer that the expressions have changed.
    exprList->Notify();
    GetViewerMethods()->ProcessExpressions();
}


// ****************************************************************************
// Method: QvisCMFEWizard::page1_donorTypeChanged
//
// Purpose:
//   This Qt slot function is called when the user picks the donor type.
//
// Arguments:
//   val : The new donor type.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page1_donorTypeChanged(int val)
{
    decision_donorType = (val == 0 ? DONOR_TIME : (val == 1 ? DONOR_FILE
                                                            : DONOR_WITHIN_FILE));
}


// ****************************************************************************
// Method: QvisCMFEWizard::page2_sourceChanged
//
// Purpose:
//   This Qt slot function is called when the user picks the source
//
// Arguments:
//   val : The index into the source combo box.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page2_sourceChanged(int val)
{
    if (globalAtts == NULL || windowInfo == NULL)
    {
        debug1 << "Cannot set sources up because of internal error" << endl;
        return;
    }
    const stringVector &sources = globalAtts->GetSources();
    if(val >= 0 && val < sources.size())
    {
        //
        // Make the file that we reopened be the new open file. Since we're
        // reopening, this will take care of freeing the old metadata and SIL.
        //
        QualifiedFilename fileName(sources[val]);
        decision_source = fileName.FullName();
    }
    else
        debug1 << "Error with file name" << endl;
}


// ****************************************************************************
// Method: QvisCMFEWizard::page3_absoluteChanged
//
// Purpose:
//   This Qt slot function is called when the user picks whether to specify
//   absolute or relative time
//
// Arguments:
//   val : The new index into the button group.
//
// Programmer: Hank Childs
// Creation:   August 2, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page3_absoluteTimeChanged(int val)
{
    decision_absolute = (val == 0 ? true : false);
}

// ****************************************************************************
// Method: QvisCMFEWizard::page3_timeTypeChanged
//
// Purpose:
//   This Qt slot function is called when the time type changes.
//
// Arguments:
//   val : The new index into the button group.
//
// Programmer: Hank Childs
// Creation:   August 2, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page3_timeTypeChanged(int val)
{
    if (val == 0)
        decision_timeType = TIME_TYPE_SIMTIME;
    else if (val == 1)
        decision_timeType = TIME_TYPE_SIMCYCLE;
    else 
        decision_timeType = TIME_TYPE_INDEX;

    initializePage(Page_TimeSpecification);
}


// ****************************************************************************
// Method: QvisCMFEWizard::page3_timeChanged
//
// Purpose:
//   This Qt slot function is called when the time changes.
//
// Arguments:
//   val : The string describing the time.
//
// Programmer: Hank Childs
// Creation:   August 2, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page3_timeChanged(const QString &s)
{
    bool okay = true;
    decision_time = s.toDouble(&okay);
/*
    if (!okay)
        page3_lineEdit->setText(tr("0"));
 */
}

// ****************************************************************************
// Method: QvisCMFEWizard::page3_cycleChanged
//
// Purpose:
//   This Qt slot function is called when the cycle changes.
//
// Arguments:
//   val : The string describing the cycle.
//
// Programmer: Hank Childs
// Creation:   August 2, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page3_cycleChanged(const QString &s)
{
    bool okay = true;
    decision_cycle = s.toInt(&okay);

/*
    if (!okay)
        page3_lineEdit->setText(tr("0"));
 */
}

// ****************************************************************************
// Method: QvisCMFEWizard::page3_indexChanged
//
// Purpose:
//   This Qt slot function is called when the index changes.
//
// Arguments:
//   val : The string describing the index.
//
// Programmer: Hank Childs
// Creation:   August 2, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page3_indexChanged(const QString &s)
{
    bool okay = true;
    decision_index = s.toInt(&okay);

/*
    if (!okay)
        page3_lineEdit->setText(tr("0"));
 */
}

// ****************************************************************************
// Method: QvisCMFEWizard::page4_donorChanged
//
// Purpose:
//   This Qt slot function is called when the user picks the donor
//
// Arguments:
//   val : The index into the donor combo box.
//
// Programmer: Hank Childs
// Creation:   August 2, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page4_donorChanged(const QString &str)
{
    decision_variable = str.toStdString();
}


// ****************************************************************************
// Method: QvisCMFEWizard::page4_targetChanged
//
// Purpose:
//   This Qt slot function is called when the user picks the target
//
// Arguments:
//   val : The index into the target combo box.
//
// Programmer: Hank Childs
// Creation:   August 2, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page4_targetChanged(const QString &str)
{
    decision_mesh = str.toStdString();
}


// ****************************************************************************
// Method: QvisCMFEWizard::page5_interpChanged
//
// Purpose:
//   This Qt slot function is called when the user picks the interpolation type
//
// Arguments:
//   val : The index in the button group.
//
// Programmer: Hank Childs
// Creation:   August 3, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page5_interpChanged(int val)
{
    if (val == 0)
        decision_interp = INTERP_CONN_CMFE;
    else
        decision_interp = INTERP_POS_CMFE;
    initializePage(Page_InterpSelection);
}

// ****************************************************************************
// Method: QvisCMFEWizard::page5_fillChanged
//
// Purpose:
//   This Qt slot function is called when the method for determining how to
//   fill non-overlapping regions is changed.
//
// Arguments:
//   val : The index in the button group.
//
// Programmer: Hank Childs
// Creation:   August 3, 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::page5_fillChanged(int val) 
{ 
    if (val == 0)
        decision_fill = FILL_CONSTANT;
    else
        decision_fill = FILL_VAR;
    initializePage(Page_InterpSelection);
}

// ****************************************************************************
// Method: QvisCMFEWizard::page5_overlapConstChanged
//
// Purpose:
//   This Qt slot function is called when the constant used to
//   fill non-overlapping regions is changed.
//
// Programmer: Hank Childs
// Creation:   August 4, 2010
//
// Modifications:
//
// ****************************************************************************

void 
QvisCMFEWizard::page5_overlapConstChanged(const QString &s)
{
    bool okay = false;
    decision_fillval = s.toDouble(&okay);
}

// ****************************************************************************
// Method: QvisCMFEWizard::page5_overlapVarChanged
//
// Purpose:
//   This Qt slot function is called when the variable used to
//   fill non-overlapping regions is changed.
//
// Programmer: Hank Childs
// Creation:   August 4, 2010
//
// Modifications:
//
// ****************************************************************************

void 
QvisCMFEWizard::page5_overlapVarChanged(const QString &s) 
{
    bool okay = false;
    decision_fillvar = s.toStdString();
}

// ****************************************************************************
// Method: QvisCMFEWizard::page6_exprNameChanged
//
// Purpose:
//   This Qt slot function is called when the name of the CMFE expression
//   out is changed.
//
// Programmer: Hank Childs
// Creation:   August 4, 2010
//
// Modifications:
//
// ****************************************************************************

void 
QvisCMFEWizard::page6_exprNameChanged(const QString &s)
{
    decision_exprname = s.toStdString();
}

// ****************************************************************************
// Method: QvisCMFEWizard::page6_exprTypeChanged
//
// Purpose:
//   This Qt slot function is called when the way to study the expression
//   is changed.
//
// Programmer: Hank Childs
// Creation:   August 4, 2010
//
// Modifications:
//
// ****************************************************************************

void 
QvisCMFEWizard::page6_exprTypeChanged(int v)
{
    if (v == 0)
        decision_exprtype = EXPRESSION_SIMPLE;
    else
    {
        if (page6_buttongroup2->button(0)->isChecked())
            decision_exprtype = EXPRESSION_ABS_DIFF;
        else if (page6_buttongroup2->button(1)->isChecked())
            decision_exprtype = EXPRESSION_DIFF_FIRST;
        else if (page6_buttongroup2->button(2)->isChecked())
            decision_exprtype = EXPRESSION_DIFF_SECOND;
        else
            decision_exprtype = EXPRESSION_ABS_DIFF;
    }
    initializePage(Page_ActivityDescription);
}


// ****************************************************************************
// Method: QvisCMFEWizard::page6_diffVariable1Changed
//
// Purpose:
//   This Qt slot function is called when the variable to contrast the
//   CMFE variable is changed.
//
// Programmer: Hank Childs
// Creation:   August 4, 2010
//
// Modifications:
//
// ****************************************************************************

void 
QvisCMFEWizard::page6_diffVariable1Changed(const QString &s)
{
    decision_diffvarname = s.toStdString();
}

// ****************************************************************************
// Method: QvisCMFEWizard::page6_diffTypeChanged
//
// Purpose:
//   This Qt slot function is called when the type of differencing with the
//   CMFE variable is changed.
//
// Programmer: Hank Childs
// Creation:   August 4, 2010
//
// Modifications:
//
// ****************************************************************************

void 
QvisCMFEWizard::page6_diffTypeChanged(int v)
{
    if (v == 0)
        decision_exprtype = EXPRESSION_ABS_DIFF;
    else if (v == 1)
        decision_exprtype = EXPRESSION_DIFF_FIRST;
    else
        decision_exprtype = EXPRESSION_DIFF_SECOND;
}


