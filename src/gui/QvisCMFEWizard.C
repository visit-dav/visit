/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#include <StringHelpers.h>
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
#include <QvisFileOpenDialog.h>
#include <QvisVariableButton.h>

#include <ViewerProxy.h>

#include <snprintf.h>

#define DONOR_SINGLE_DATABASE    0
#define DONOR_TIME_SLICES        1
#define DONOR_MULTIPLE_DATABASES 2

#define INTERP_CONN_CMFE 0
#define INTERP_POS_CMFE  1

#define TIME_TYPE_SIMTIME  0
#define TIME_TYPE_SIMCYCLE 1
#define TIME_TYPE_INDEX    2

#define FILL_VAR       0
#define FILL_CONSTANT  1

#define EXPRESSION_SIMPLE      0x11
#define EXPRESSION_MINIMUM     0x12
#define EXPRESSION_MAXIMUM     0x14

#define EXPRESSION_SUM         0x21
#define EXPRESSION_AVERAGE     0x22
#define EXPRESSION_VARIANCE    0x24

#define EXPRESSION_ABS_DIFF    0x41
#define EXPRESSION_DIFF_FIRST  0x42
#define EXPRESSION_DIFF_SECOND 0x44


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
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

QvisCMFEWizard::QvisCMFEWizard(AttributeSubject *atts, QWidget *parent) :
    QvisWizard(atts, parent), SimpleObserver()
{
    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::HaveHelpButton, false);
    setOption(QWizard::HaveNextButtonOnLastPage, false);

    decision_donorType = DONOR_SINGLE_DATABASE;
    decision_targetDatabase ="";
    decision_donorDatabase ="";
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

    selectedTargetDatabase = "";
    selectedDonorDatabase  = "";

    char str[1024];
    SNPRINTF(str, 1024, "cmfe%d", timesCompleted);
    decision_exprname = str;
    timesCompleted++;

    globalAtts = NULL;
    windowInfo = NULL;
    exprList = NULL;

    // Set the wizard's title.
    setWindowTitle(tr("Data-Level Comparison Setup"));

    // Create the wizard pages.
    CreateDonorTypePage();          // page0
    CreateDonorAndTargetPage();     // page1
    CreateTimeSpecificationPage();  // page2
    CreateInterpSelectionPage();    // page3
    CreateActivityPage();           // page4

    setStartId(Page_DonorType);
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
//  Method: QvisCMFEWizard::SetGlobalAttributes
//
//  Purpose:
//      Sets the global attributes.
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon Aug 30 09:39:20 PDT 2010
//
// ****************************************************************************

void
QvisCMFEWizard::SetGlobalAttributes(GlobalAttributes *ga)
{
    globalAtts = ga;
    ga->Attach(this);
}

// ****************************************************************************
//  Method: QvisCMFEWizard::SetGlobalAttributes
//
//  Purpose:
//      Sets the global attributes.
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon Aug 30 09:39:20 PDT 2010
//
// ****************************************************************************

void
QvisCMFEWizard::SetWindowInformation(WindowInformation *wi)
{
    windowInfo = wi;
    wi->Attach(this);
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
//    initializePage(Page_DonorAndTargetSpecification);

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
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

int
QvisCMFEWizard::nextId() const
{
    int id = -1;

    switch(currentId())
    {
    case Page_DonorType:
            id = Page_DonorAndTargetSpecification;
        break;
    case Page_DonorAndTargetSpecification:
        if (decision_donorType == DONOR_TIME_SLICES)
            id = Page_TimeSpecification;
        else
            id = Page_InterpSelection;
        break;
    case Page_TimeSpecification:
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
    switch(currentId())
    {
    case Page_DonorAndTargetSpecification:
        if (decision_targetDatabase == "")
            return false;
        if (decision_mesh == "")
            return false;

        if( decision_donorType == DONOR_SINGLE_DATABASE )
        {
          if (decision_variable == "" && donorList->count() == 0)
            return false;
        }
        else if( decision_donorType == DONOR_TIME_SLICES )
        {
          if( decision_variable == "" )
            return false;
        }
        else if( decision_donorType == DONOR_MULTIPLE_DATABASES )
        {
          if (decision_donorDatabase == "")
            return false;

          if (decision_variable == "" && donorList->count() == 0)
            return false;
        }

        return true;

    case Page_InterpSelection:
        if (decision_interp == INTERP_POS_CMFE && decision_fill == FILL_VAR
            && decision_fillvar == "")
            return false;

        return true;
    
    case Page_ActivityDescription:
        if (decision_exprname == "")
            return false;

        if( decision_exprtype != EXPRESSION_SIMPLE)
        {
          // Check for multiple donors.
          if( ((decision_donorType == DONOR_SINGLE_DATABASE ||
               decision_donorType == DONOR_MULTIPLE_DATABASES) &&
              donorList->count() > 1) || // Multiple donors
              // One donor in the list and one ready to be added
              (donorList->count() == 1 &&
               decision_donorDatabase != "" && decision_variable != "" ) )
          {
          }
          // Single donor so need a variable
          else if( decision_diffvarname == "" )
            return false;
        }

        return true;
        
    default:
        return true;
    }
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
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
//   Cyrus Harrison, Mon Aug 30 13:53:32 PDT 2010
//   Proper enable/disable for exprDiffVar button.
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
        donorTypeSelect->blockSignals(true);

        donorTypeSelect->button( decision_donorType )->setChecked(true);
        donorTypeSelect->blockSignals(false);
        break;
    case Page_DonorAndTargetSpecification:
        UpdateSourceList();

        if(decision_donorType == DONOR_SINGLE_DATABASE)
        {
            targetDatabaseLabel->setText(tr("Database:"));
            donorDatabaseLabel->setVisible(false);
            donorDatabase->setVisible(false);
            donorDatabaseOpen->setVisible(false);
       
            donorList->setVisible(true);
            donorListAddDonor->setVisible(true);
            donorListDeleteDonor->setVisible(true);
            donorListDeleteAllDonors->setVisible(true);
        }
        else if(decision_donorType == DONOR_TIME_SLICES)
        {
            targetDatabaseLabel->setText(tr("Database:"));
            donorDatabaseLabel->setVisible(false);
            donorDatabase->setVisible(false);
            donorDatabaseOpen->setVisible(false);

            donorList->setVisible(false);
            donorListAddDonor->setVisible(false);
            donorListDeleteDonor->setVisible(false);
            donorListDeleteAllDonors->setVisible(false);
        }
        else if(decision_donorType == DONOR_MULTIPLE_DATABASES)
        {
            targetDatabaseLabel->setText(tr("Target Database:"));
            donorDatabaseLabel->setVisible(true);
            donorDatabase->setVisible(true);
            donorDatabaseOpen->setVisible(true);

            donorList->setVisible(true);
            donorListAddDonor->setVisible(true);
            donorListDeleteDonor->setVisible(true);
            donorListDeleteAllDonors->setVisible(true);
        }
        break;
    case Page_TimeSpecification:
        // Initialize page3's button group with the time specification.
        absVsRelTimeSelect->blockSignals(true);
        absVsRelTimeSelect->button((decision_absolute?0:1))->setChecked(true);
        absVsRelTimeSelect->blockSignals(false);
        timeTypeSelect->blockSignals(true);
        timeTypeSelect->button( (decision_timeType == TIME_TYPE_SIMTIME ? 0
                                : (decision_timeType == TIME_TYPE_SIMCYCLE ? 1 : 2)))->setChecked(true);
        timeTypeSelect->blockSignals(false);
        timeTxt->blockSignals(true);
        SNPRINTF(buff, 1024, "%f", decision_time);
        timeTxt->setText(tr(buff));
        timeTxt->setEnabled(decision_timeType == TIME_TYPE_SIMTIME);
        timeTxt->blockSignals(false);
        cycleTxt->blockSignals(true);
        SNPRINTF(buff, 1024, "%d", decision_cycle);
        cycleTxt->setText(tr(buff));
        cycleTxt->setEnabled(decision_timeType == TIME_TYPE_SIMCYCLE);
        cycleTxt->blockSignals(false);
        cycleTxt->blockSignals(true);
        SNPRINTF(buff, 1024, "%d", decision_index);
        indexTxt->setText(tr(buff));
        indexTxt->setEnabled(decision_timeType == TIME_TYPE_INDEX);
        indexTxt->blockSignals(false);
        break;
    case Page_InterpSelection:
        interpSelect->blockSignals(true);
        interpSelect->button(decision_interp == INTERP_CONN_CMFE ? 0 : 1)->setChecked(true);
        interpSelect->blockSignals(false);
        if (decision_interp == INTERP_POS_CMFE)
        {
            nonOverlapSelect->button(0)->setEnabled(true);
            nonOverlapSelect->button(1)->setEnabled(true);
            if (decision_fill == FILL_CONSTANT)
            {
                nonOverlapTxt->setEnabled(true);
                nonOverlapVar->setEnabled(false);
            }
            else
            {
                nonOverlapTxt->setEnabled(false);
                nonOverlapVar->setEnabled(true);
            }
            nonOverlapSelect->button(decision_fill == FILL_CONSTANT ? 0 : 1)->setChecked(true);
        }
        else
        {
            nonOverlapSelect->button(0)->setEnabled(false);
            nonOverlapSelect->button(1)->setEnabled(false);
            nonOverlapTxt->setEnabled(false);
            nonOverlapVar->setEnabled(false);
        }
        break;
    case Page_ActivityDescription:

        // more than one donor so force the expression to be something
        // other than simple.

        bool singleDonor;

        // Multiple donors
        if( ((decision_donorType == DONOR_SINGLE_DATABASE ||
             decision_donorType == DONOR_MULTIPLE_DATABASES) &&
            donorList->count() > 1) || // Multiple donors
            // One donor in the list and one ready to be added
            (donorList->count() == 1 &&
             decision_donorDatabase != "" && decision_variable != "" ) )
        {
          singleDonor = false;

          if (decision_exprtype == EXPRESSION_SIMPLE)
            decision_exprtype = EXPRESSION_SUM;

          exprTypeSelect->button(0)->setEnabled(false);
          exprTypeSelect->button(1)->setEnabled(false);

          exprDiffVar->setEnabled(false);
          exprDiffVar->setVisible(false);
        }
        // Single donor
        else
        {
          singleDonor = true;

          exprTypeSelect->button(0)->setEnabled(true);
          exprTypeSelect->button(1)->setEnabled(true);

          exprDiffVar->setEnabled(true);
          exprDiffVar->setVisible(true);
        }

        if (decision_exprtype == EXPRESSION_SIMPLE)
        {
            exprTypeSelect->blockSignals(true);
            exprTypeSelect->button(0)->setChecked(true);
            exprTypeSelect->blockSignals(false);

            exprDiffVar->setEnabled(false);
            exprDiffTypeSelect->button(0)->setEnabled(false);
            exprDiffTypeSelect->button(1)->setEnabled(false);
            exprDiffTypeSelect->button(2)->setEnabled(false);
            exprDiffTypeSelect->button(3)->setEnabled(false);
            exprDiffTypeSelect->button(4)->setEnabled(false);
            exprDiffTypeSelect->button(5)->setEnabled(false);
            exprDiffTypeSelect->button(6)->setEnabled(false);
            exprDiffTypeSelect->button(7)->setEnabled(false);
        }
        else
        {
            exprTypeSelect->blockSignals(true);
            exprTypeSelect->button(1)->setChecked(true);
            exprTypeSelect->blockSignals(false);

            exprDiffTypeSelect->button(0)->setEnabled(true);
            exprDiffTypeSelect->button(1)->setEnabled(true);
            exprDiffTypeSelect->button(2)->setEnabled(true);
            exprDiffTypeSelect->button(3)->setEnabled(true);
            exprDiffTypeSelect->button(4)->setEnabled(true);
            exprDiffTypeSelect->button(5)->setEnabled(true);
            exprDiffTypeSelect->button(6)->setEnabled(singleDonor);
            exprDiffTypeSelect->button(7)->setEnabled(singleDonor);

            if (!singleDonor && decision_exprtype >= EXPRESSION_DIFF_FIRST)
              decision_exprtype = EXPRESSION_SUM;

            if (decision_exprtype == EXPRESSION_MINIMUM)
                exprDiffTypeSelect->button(0)->setChecked(true);
            else if (decision_exprtype == EXPRESSION_MAXIMUM)
                exprDiffTypeSelect->button(1)->setChecked(true);
            else if (decision_exprtype == EXPRESSION_SUM)
                exprDiffTypeSelect->button(2)->setChecked(true);
            else if (decision_exprtype == EXPRESSION_AVERAGE)
                exprDiffTypeSelect->button(3)->setChecked(true);
            else if (decision_exprtype == EXPRESSION_VARIANCE)
                exprDiffTypeSelect->button(4)->setChecked(true);
            else if (decision_exprtype == EXPRESSION_ABS_DIFF)
                exprDiffTypeSelect->button(5)->setChecked(true);
            else if (decision_exprtype == EXPRESSION_DIFF_FIRST)
                exprDiffTypeSelect->button(6)->setChecked(true);
            else if (decision_exprtype == EXPRESSION_DIFF_SECOND)
                exprDiffTypeSelect->button(7)->setChecked(true);
        }
        break;

    default:
        break;
    }
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
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::CreateDonorTypePage(void)
{
    page0 = new QWizardPage(this);
    page0->setTitle(tr("Data-Level Comparisons"));
    page0->setSubTitle(tr("Cross-mesh field evaluations (CMFEs) take a field "
                "from a donor mesh and place that field on a target mesh.  "
                "To begin select the type of evaluation to setup:"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page0);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    hCenterLayout->addStretch(5);
    QVBoxLayout *buttonLayout = new QVBoxLayout(0);
    hCenterLayout->addLayout(buttonLayout);
    hCenterLayout->addStretch(5);
    buttonLayout->setSpacing(5);
    donorTypeSelect = new QButtonGroup(page0);

    QRadioButton *r0 = new QRadioButton(tr("Between two or more meshes in a single database"), page0);
    donorTypeSelect->addButton(r0, 0);
    buttonLayout->addWidget(r0);

    QRadioButton *r1 = new QRadioButton(tr("Between different time slices on the same mesh"), page0);
    r1->setChecked(true);
    donorTypeSelect->addButton(r1, 1);
    buttonLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("Between meshes in two or more separate databases"), page0);
    donorTypeSelect->addButton(r2, 2);
    buttonLayout->addWidget(r2);


    pageLayout->addStretch(10);
    connect(donorTypeSelect, SIGNAL(buttonClicked(int)),
            this, SLOT(donorTypeChanged(int)));

    // Add the page.
    setPage(Page_DonorType, page0);
}


// ****************************************************************************
// Method: QvisCMFEWizard::CreateDonorAndTargetPage
//
// Purpose:
//   Creates the page to select the file.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::CreateDonorAndTargetPage(void)
{
    page1 = new QWizardPage(this);
    page1->setTitle(tr("Donor & Target Setup"));

    page1->setSubTitle(tr("Which mesh should be used as the evaluation target? "
                          "What is the desired donor field?"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page1);
    pageLayout->setSpacing(10);

    QHBoxLayout *hCenterLayout = new QHBoxLayout(0);
    pageLayout->addLayout(hCenterLayout);
    hCenterLayout->addStretch(5);

    QWidget *main_widget = new QWidget(page1);

    QGridLayout *main_layout= new QGridLayout();
    hCenterLayout->addLayout(main_layout);
    hCenterLayout->addStretch(5);
    main_layout->setSpacing(5);

    targetDatabaseLabel = new QLabel(tr("Target Database:"), main_widget);
    targetDatabase = new QComboBox(main_widget);

    targetDatabaseOpen = new QPushButton("...", this);
#if !(defined(Q_WS_MACX) || defined(Q_OS_MAC))
    targetDatabaseOpen->setMaximumWidth(fontMetrics().boundingRect("...").width() + 6);
#endif

    targetDatabaseOpen->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum));

    connect(targetDatabase,SIGNAL(activated(int)),
            this, SLOT(targetDatabaseChanged(int)));


    connect(targetDatabaseOpen, SIGNAL(clicked()),
            this, SLOT(targetDatabaseOpenClicked()));


    main_layout->addWidget(targetDatabaseLabel,0,0);
    main_layout->addWidget(targetDatabase,0,1);
    main_layout->addWidget(targetDatabaseOpen,0,2);

    QLabel *lbl_tmesh = new QLabel(tr("Target Mesh:"), main_widget);
    targetMeshVar = new QvisCustomSourceVariableButton(false, false, NULL,
                                                       QvisBaseVariableButton::Meshes,
                                                       main_widget);

    connect(targetMeshVar, SIGNAL(activated(const QString &)),
            this, SLOT(targetMeshVarChanged(const QString &)));


    main_layout->addWidget(lbl_tmesh,1,0);
    main_layout->addWidget(targetMeshVar,1,1);

    QFrame *hline = new QFrame(main_widget);
    hline->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    pageLayout->addWidget(hline);
    main_layout->addWidget(hline,2,0,1,3);


    donorDatabaseLabel = new QLabel(tr("Donor Database:"), main_widget);
    donorDatabase = new QComboBox(main_widget);
    donorDatabaseOpen = new QPushButton("...", this);
#if !(defined(Q_WS_MACX) || defined(Q_OS_MAC))
    donorDatabaseOpen->setMaximumWidth(fontMetrics().boundingRect("...").width() + 6);
#endif

    donorDatabaseOpen->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum));

    connect(donorDatabase,SIGNAL(activated(int)),
            this, SLOT(donorDatabaseChanged(int)));

    connect(donorDatabaseOpen, SIGNAL(clicked()),
            this, SLOT(donorDatabaseOpenClicked()));

    main_layout->addWidget(donorDatabaseLabel,3,0);
    main_layout->addWidget(donorDatabase,3,1);
    main_layout->addWidget(donorDatabaseOpen,3,2);

    donorDatabaseLabel->setVisible(false);
    donorDatabase->setVisible(false);
    donorDatabaseOpen->setVisible(false);


    QLabel *lbl_dvar = new QLabel(tr("Donor Field:"), main_widget);

    donorFieldVar = new QvisCustomSourceVariableButton(false, false, NULL,
                                                       QvisBaseVariableButton::Scalars |
                                                       QvisBaseVariableButton::Vectors |
                                                       QvisBaseVariableButton::Tensors,
                                                       main_widget);

    connect(donorFieldVar, SIGNAL(activated(const QString &)),
            this, SLOT(donorFieldVarChanged(const QString &)));

    main_layout->addWidget(lbl_dvar,4,0);
    main_layout->addWidget(donorFieldVar,4,1);



    // Donor database list.
    donorList = new QListWidget(main_widget);
    main_layout->addWidget(donorList, 5, 0, 3, 3);

    connect(donorList, SIGNAL(itemClicked(QListWidgetItem*)), this,
            SLOT(donorListClicked(QListWidgetItem*)));

    donorListAddDonor = new QPushButton(tr("Add Donor"), main_widget);
    donorListDeleteDonor = new QPushButton(tr("Delete Donor"), main_widget);
    donorListDeleteAllDonors = new QPushButton(tr("Delete All Donors"), main_widget);
    connect(donorListAddDonor, SIGNAL(clicked()), this, SLOT(addDonor()));
    connect(donorListDeleteDonor, SIGNAL(clicked()), this, SLOT(deleteDonor()));
    connect(donorListDeleteAllDonors, SIGNAL(clicked()), this, SLOT(deleteDonors()));
    main_layout->addWidget(donorListAddDonor, 8, 0);
    main_layout->addWidget(donorListDeleteDonor, 8, 1);
    main_layout->addWidget(donorListDeleteAllDonors, 8, 2);

    donorList->setVisible(false);
    donorListAddDonor->setVisible(false);
    donorListDeleteDonor->setVisible(false);
    donorListDeleteAllDonors->setVisible(false);

    // Add the page.
    setPage(Page_DonorAndTargetSpecification, page1);
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
//
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::CreateTimeSpecificationPage(void)
{
    page2 = new QWizardPage(this);
    page2->setTitle(tr("Specify Donor Time Slice"));
    page2->setSubTitle(tr("Which time slice do you want to extract data from?"));

    QWidget *main_widget= new QWidget(page2);

    QVBoxLayout *pageLayout = new QVBoxLayout(page2);
    //pageLayout->setSpacing(10);

    QLabel *arLabel = new QLabel(tr("<u>Absolute Versus Relative Time</u>"), main_widget);
    //arLabel->setAlignment(Qt::AlignLeft|Qt::AlignCenter);
    pageLayout->addWidget(arLabel);

    absVsRelTimeSelect = new QButtonGroup(main_widget);
    QRadioButton *r1 = new QRadioButton(tr("I want to specify an absolute time.\nThe time slice I specify should not change as I change the time slider"), main_widget);
    r1->setChecked(true);
    absVsRelTimeSelect->addButton(r1, 0);
    pageLayout->addWidget(r1);

    QRadioButton *r2 = new QRadioButton(tr("I want to specify a relative time.\nThe time slice I want will change as I change the time slider"), main_widget);
    absVsRelTimeSelect->addButton(r2, 1);
    pageLayout->addWidget(r2);

    connect(absVsRelTimeSelect, SIGNAL(buttonClicked(int)),
            this, SLOT(absVsRelTimeChanged(int)));

    QFrame *hline2 = new QFrame(main_widget);
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    pageLayout->addWidget(hline2);

    QLabel *timeLabel = new QLabel(tr("<u>Time Type and Value</u>"), main_widget);
    pageLayout->addWidget(timeLabel);

    QGridLayout *glayout = new QGridLayout(0);
    pageLayout->addLayout(glayout);

    timeTypeSelect = new QButtonGroup(main_widget);
    QRadioButton *r21 = new QRadioButton(tr("Simulation time"), main_widget);
    timeTypeSelect->addButton(r21, 0);
    glayout->addWidget(r21, 0, 0);

    timeTxt = new QLineEdit(main_widget);
    timeTxt->setText(tr("0"));

    glayout->addWidget(timeTxt, 0, 1);
    connect(timeTxt, SIGNAL(textChanged(const QString &)),
            this, SLOT(timeChanged(const QString &)));

    QRadioButton *r22 = new QRadioButton(tr("Simulation cycle"), main_widget);
    timeTypeSelect->addButton(r22, 1);
    glayout->addWidget(r22, 1, 0);

    cycleTxt = new QLineEdit(main_widget);
    cycleTxt->setText(tr("0"));
    glayout->addWidget(cycleTxt, 1, 1);
    connect(cycleTxt, SIGNAL(textChanged(const QString &)),
            this, SLOT(cycleChanged(const QString &)));

    QRadioButton *r23 = new QRadioButton(tr("Time index"), main_widget);
    r23->setChecked(true);
    timeTypeSelect->addButton(r23, 2);
    glayout->addWidget(r23, 2, 0);

    indexTxt = new QLineEdit(main_widget);
    indexTxt->setText(tr("0"));
    glayout->addWidget(indexTxt, 2, 1);
    connect(indexTxt, SIGNAL(textChanged(const QString &)),
            this, SLOT(indexChanged(const QString &)));

    QLabel *descLabel = new QLabel(tr("(Time index is the most robust and works in all circumstances)"), main_widget);
    pageLayout->addWidget(descLabel);

    connect(timeTypeSelect, SIGNAL(buttonClicked(int)),
            this, SLOT(timeTypeChanged(int)));

    QFrame *hline1 = new QFrame(main_widget);
    hline1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    pageLayout->addWidget(hline1);

    QLabel *exLabel = new QLabel(tr("<u>Examples</u>"), main_widget);
    pageLayout->addWidget(exLabel);

    QLabel *ex1 = new QLabel(tr("#1: \"Simulation time\" with \"10.5\" and absolute time will always return the data from simulation time 10.5."), main_widget);
    pageLayout->addWidget(ex1);
    QLabel *ex2 = new QLabel(tr("#2: \"Time index\" with \"-1\" and relative time will always return the data from the time slice immediately preceding the current one."), main_widget);
    pageLayout->addWidget(ex2);

    // Add the page.
    setPage(Page_TimeSpecification, page2);
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
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::CreateInterpSelectionPage(void)
{
    page3 = new QWizardPage(this);
    page3->setTitle(tr("Specify Evaluation Scheme"));
    page3->setSubTitle(tr("Do you want to use indexing or interpolation?  How will you handle regions of non-overlap?"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page3);

    QWidget *main_widget= new QWidget(page3);

    QLabel *evalLabel = new QLabel(tr("<u>Evaluation Scheme</u>"), main_widget);
    pageLayout->addWidget(evalLabel);

    QGridLayout *glayout = new QGridLayout(0);
    pageLayout->addLayout(glayout);
    glayout->setColumnStretch(0, 0);
    glayout->setColumnStretch(1, 100);

    interpSelect = new QButtonGroup(main_widget);
    QRadioButton *r21 = new QRadioButton(tr("Connectivity-based CMFE"));
    interpSelect->addButton(r21, 0);
    glayout->addWidget(r21, 0, 0);
    QLabel *connLabel1 = new QLabel(tr("The value at index 'i' of the target is assigned"), main_widget);
    connLabel1->setAlignment(Qt::AlignLeft);
    glayout->addWidget(connLabel1, 0, 1);
    QLabel *connLabel2 = new QLabel(tr("the value of index 'i' from the donor.  This is efficient,"), main_widget);
    connLabel2->setAlignment(Qt::AlignLeft);
    glayout->addWidget(connLabel2, 1, 1);
    QLabel *connLabel3 = new QLabel(tr("but requires that the meshes are congruent."), main_widget);
    connLabel3->setAlignment(Qt::AlignLeft);
    glayout->addWidget(connLabel3, 2, 1);

    QFrame *hline2 = new QFrame(main_widget);
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    glayout->addWidget(hline2, 3, 1);

    QRadioButton *r22 = new QRadioButton(tr("Position-based CMFE"));
    interpSelect->addButton(r22, 1);
    glayout->addWidget(r22, 4, 0);

    QLabel *posLabel1 = new QLabel(tr("The value at index 'i' from the target is evaluated"), main_widget);
    posLabel1->setAlignment(Qt::AlignLeft);
    glayout->addWidget(posLabel1, 4, 1);
    QLabel *posLabel2 = new QLabel(tr("by first determining its location and then determining the"), main_widget);
    posLabel2->setAlignment(Qt::AlignLeft);
    glayout->addWidget(posLabel2, 5, 1);
    QLabel *posLabel3 = new QLabel(tr("value at that location from the donor mesh.  This is less"), main_widget);
    posLabel3->setAlignment(Qt::AlignLeft);
    glayout->addWidget(posLabel3, 6, 1);
    QLabel *posLabel4 = new QLabel(tr("efficient, but is robust and works on non-congruent meshes."), main_widget);
    posLabel4->setAlignment(Qt::AlignLeft);
    glayout->addWidget(posLabel4, 7, 1);

    connect(interpSelect, SIGNAL(buttonClicked(int)),
            this, SLOT(interpChanged(int)));

    QFrame *hline1 = new QFrame(main_widget);
    hline1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    pageLayout->addWidget(hline1);

    QLabel *olLabel = new QLabel(tr("<u>Handling For Non-Overlapping Regions</u>"), main_widget);
    pageLayout->addWidget(olLabel);

    QGridLayout *glayout2 = new QGridLayout(0);
    pageLayout->addLayout(glayout2);

    nonOverlapSelect = new QButtonGroup(main_widget);
    QRadioButton *r11 = new QRadioButton(tr("Use a constant"));
    nonOverlapSelect->addButton(r11, 0);
    glayout2->addWidget(r11, 0, 0);

    nonOverlapTxt = new QLineEdit(main_widget);
    nonOverlapTxt->setText(tr("0"));
    glayout2->addWidget(nonOverlapTxt, 0, 1);
    connect(nonOverlapTxt, SIGNAL(textChanged(const QString &)),
            this, SLOT(nonOverlapTxtChanged(const QString &)));

    QRadioButton *r12 = new QRadioButton(tr("Use a variable"));
    nonOverlapSelect->addButton(r12, 1);
    glayout2->addWidget(r12, 1, 0);

    nonOverlapVar = new QvisVariableButton(false, false, false,
                                           QvisBaseVariableButton::Scalars |
                                           QvisBaseVariableButton::Vectors |
                                           QvisBaseVariableButton::Tensors,
                                            main_widget);
    glayout2->addWidget(nonOverlapVar, 1, 1);
    connect(nonOverlapVar, SIGNAL(activated(const QString &)),
            this, SLOT(nonOverlapVarChanged(const QString &)));

    connect(nonOverlapSelect, SIGNAL(buttonClicked(int)),
            this, SLOT(nonOverlapChanged(int)));

    // Add the page.
    setPage(Page_InterpSelection, page3);
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
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
//   Cyrus Harrison, Fri Oct 21 13:57:57 PDT 2011
//   Proper init for the expression diff var button.
//
// ****************************************************************************

void
QvisCMFEWizard::CreateActivityPage(void)
{
    page4 = new QWizardPage(this);
    page4->setTitle(tr("Setting Up The Expression"));
    page4->setSubTitle(tr("How do you want to use your new CMFE expression?  What is its name?"));

    QVBoxLayout *pageLayout = new QVBoxLayout(page4);

    QWidget *main_widget = new QWidget(page4);

    QGridLayout *glayout1 = new QGridLayout(0);
    pageLayout->addLayout(glayout1);
    QLabel *expLabel = new QLabel(tr("Name of Expression"), main_widget);
    glayout1->addWidget(expLabel, 0, 0);

    exprNameTxt = new QLineEdit(main_widget);
    exprNameTxt->setText(tr(decision_exprname.c_str()));
    glayout1->addWidget(exprNameTxt, 0, 1);
    connect(exprNameTxt, SIGNAL(textChanged(const QString &)),
            this, SLOT(exprNameChanged(const QString &)));

    QFrame *hline2 = new QFrame(main_widget);
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    pageLayout->addWidget(hline2);

    QLabel *quesLabel = new QLabel(tr("<u>What do you want to do with the donor field?<u>"), main_widget);
    pageLayout->addWidget(quesLabel);

    // Transfer
    exprTypeSelect = new QButtonGroup(main_widget);
    QRadioButton *r0 = new QRadioButton(tr("Simply place it on the target mesh (you can then use this field in the expression editor to do more complex things)"));
    exprTypeSelect->addButton(r0, 0);
    pageLayout->addWidget(r0);

    QGridLayout *glayout2 = new QGridLayout(0);
    pageLayout->addLayout(glayout2);
    glayout2->setColumnStretch(0, 0);
    glayout2->setColumnStretch(1, 100);

    // Expression
    QRadioButton *r1 = new QRadioButton(tr("Expression with "));
    exprTypeSelect->addButton(r1, 1);
    glayout2->addWidget(r1, 0, 0);

    connect(exprTypeSelect, SIGNAL(buttonClicked(int)),
            this, SLOT(exprTypeChanged(int)));

    exprDiffVar = new QvisVariableButton(false, false, false,
                                         QvisBaseVariableButton::Scalars |
                                         QvisBaseVariableButton::Vectors |
                                         QvisBaseVariableButton::Tensors,
                                         main_widget);
    exprDiffVar->setText(tr("<Select>"));
    glayout2->addWidget(exprDiffVar, 0, 1, Qt::AlignLeft);
    connect(exprDiffVar, SIGNAL(activated(const QString &)),
            this, SLOT(exprDiffVarChanged(const QString &)));

    exprDiffTypeSelect = new QButtonGroup(main_widget);

    // Min/Max
    QRadioButton *r01 = new QRadioButton(tr("Minimum of valves"));
    exprDiffTypeSelect->addButton(r01, 0);
    glayout2->addWidget(r01, 1, 1);
    QRadioButton *r02 = new QRadioButton(tr("Maximum of valves"));
    exprDiffTypeSelect->addButton(r02, 1);
    glayout2->addWidget(r02, 2, 1);

    // Summation
    QRadioButton *r11 = new QRadioButton(tr("Sum of values"));
    exprDiffTypeSelect->addButton(r11, 2);
    glayout2->addWidget(r11, 1, 2);
    QRadioButton *r12 = new QRadioButton(tr("Average of values"));
    exprDiffTypeSelect->addButton(r12, 3);
    glayout2->addWidget(r12, 2, 2);
    QRadioButton *r13 = new QRadioButton(tr("Variance of values"));
    exprDiffTypeSelect->addButton(r13, 4);
    glayout2->addWidget(r13, 3, 2);


    // Difference
    QRadioButton *r21 = new QRadioButton(tr("Absolute value of difference"));
    exprDiffTypeSelect->addButton(r21, 5);
    glayout2->addWidget(r21, 1, 3);
    QRadioButton *r22 = new QRadioButton(tr("Donor field minus variable"));
    exprDiffTypeSelect->addButton(r22, 6);
    glayout2->addWidget(r22, 2, 3);
    QRadioButton *r23 = new QRadioButton(tr("Variable minus donor field"));
    exprDiffTypeSelect->addButton(r23, 7);
    glayout2->addWidget(r23, 3, 3);

    connect(exprDiffTypeSelect, SIGNAL(buttonClicked(int)),
            this, SLOT(exprDiffTypeChanged(int)));

    // Add the page.
    setPage(Page_ActivityDescription, page4);
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
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
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
    const stringVector &sources = globalAtts->GetSources();

    targetDatabase->blockSignals(true);
    donorDatabase->blockSignals(true);

    const std::string &active_src = windowInfo->GetActiveSource();

    std::string tsrc = selectedTargetDatabase;
    if(tsrc == "")
        tsrc = decision_targetDatabase;
    if(tsrc == "")
        tsrc = active_src;

    std::string dsrc = selectedDonorDatabase;
    if(dsrc == "")
        dsrc = decision_donorDatabase;
    if(dsrc == "")
        dsrc = active_src;

    //
    // Simplify the current source names and put the short names into
    // the source combo box.
    //
    int target_index =-1;
    int donor_index  =-1;
    NameSimplifier simplifier;
    for(size_t i = 0; i < sources.size(); ++i)
    {
        if(sources[i] == tsrc)
            target_index = i;
        if(sources[i] == dsrc)
            donor_index = i;

        simplifier.AddName(sources[i]);
    }

    selectedTargetDatabase = "";
    selectedDonorDatabase  = "";

    stringVector shortSources;
    simplifier.GetSimplifiedNames(shortSources);

    targetDatabase->clear();
    donorDatabase->clear();
    for(size_t i = 0; i < shortSources.size(); ++i)
    {
        targetDatabase->addItem(shortSources[i].c_str());
        donorDatabase->addItem(shortSources[i].c_str());
    }

    //
    // Set the current item.
    //

    if(target_index != -1 && target_index != targetDatabase->currentIndex())
        targetDatabase->setCurrentIndex(target_index);

    if(donor_index != -1 && donor_index != donorDatabase->currentIndex())
        donorDatabase->setCurrentIndex(donor_index);

    donorDatabase->blockSignals(false);
    targetDatabase->blockSignals(false);

    // initialize the field, in case the user clicks next without
    // changing the selection.
    targetDatabaseChanged(target_index);
    donorDatabaseChanged(donor_index);
}


// ****************************************************************************
//  Method: QvisCMFEWizard::UpdateDonorField
//
//  Purpose:
//      Updates the variable drop down with the fields from the donor file.
//
//  Programmer: Hank Childs
//  Creation:   August 2, 2010
//
// Modifications:
//
//   Rob Sisneros, Sun Aug 29 20:13:10 CDT 2010
//   Add support for operators that create expressions.
//
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void 
QvisCMFEWizard::UpdateDonorField(void)
{
    VariableMenuPopulator *populator = new VariableMenuPopulator;


    bool useAltFile = decision_donorType == DONOR_MULTIPLE_DATABASES;

    std::string filename = (useAltFile ? decision_donorDatabase
                            : decision_targetDatabase);

    if(filename == "")
        filename = windowInfo->GetActiveSource();

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

    populator->PopulateVariableLists(filename, md, sil, exprList,NULL,false);

    donorFieldVar->ResetPopulator(populator);
    donorFieldVar->setText(tr("<Select>"));
    decision_variable = "";

//    donorList->clear();
}


// ****************************************************************************
//  Method: QvisCMFEWizard::UpdateTargetMesh
//
//  Purpose:
//      Sets the target mesh, in the case there is only one mesh.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2010
//
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::UpdateTargetMesh(void)
{
    std::string target_source = decision_targetDatabase;
    std::string meshname = GetMeshForTargetDatabase();

    if(target_source == "" || target_source =="notset")
        return;

    VariableMenuPopulator *populator = new VariableMenuPopulator;


    // we need metadata and sil for current state
    const avtDatabaseMetaData *md = fileServer->GetMetaData(target_source,
                                 GetStateForSource(target_source),
                                !FileServerList::ANY_STATE,
                                 FileServerList::GET_NEW_MD);

    const avtSIL *sil =
        fileServer->GetSIL(target_source,
                           GetStateForSource(target_source),
                          !FileServerList::ANY_STATE,
                           FileServerList::GET_NEW_MD);

    populator->PopulateVariableLists(target_source, md, sil, exprList,NULL,false);

    targetMeshVar->ResetPopulator(populator);

    if(meshname == "")
    {
        targetMeshVar->setText(tr("<Select>"));
        decision_mesh = "";
    }
    else
    {
        QString qmeshname = meshname.c_str();
        targetMeshVar->setDefaultVariable(qmeshname);
        targetMeshVar->setText(qmeshname);
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
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
//   Brad Whitlock, Mon Dec 13 10:46:08 PST 2010
//   Check for metadata==NULL.
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

    bool useAltFile = decision_donorType == DONOR_MULTIPLE_DATABASES;

    std::string filename = (useAltFile ? decision_donorDatabase
                            : decision_targetDatabase);

    if (filename == "notset")
        return Expression::ScalarMeshVar;

    // we need metadata and sil for current state
    const avtDatabaseMetaData *md = fileServer->GetMetaData(filename,
                                 GetStateForSource(filename),
                                !FileServerList::ANY_STATE,
                                 FileServerList::GET_NEW_MD);
    avtVarType vt = AVT_SCALAR_VAR;
    if(md != 0)
    {
        TRY
        {
            vt = md->DetermineVarType(str, true);
        }
        CATCH (VisItException)
        {
        }
        ENDTRY
    }

    return avtVarType_To_ExprType(vt);
}


// ****************************************************************************
//  Method: QvisCMFEWizard::GetMeshForTargetDatabase
//
//  Purpose:
//      Gets the mesh for the active source.  Returns the empty string if the
//      there are multiple meshes.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2010
//
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
//   Brad Whitlock, Mon Dec 13 10:48:26 PST 2010
//   Check for NULL metadata.
//
// ****************************************************************************

std::string
QvisCMFEWizard::GetMeshForTargetDatabase(void)
{
    if (windowInfo == NULL || globalAtts == NULL)
        return "";

    const stringVector &sources  = globalAtts->GetSources();
    int idx = targetDatabase->currentIndex();

    // get full name
    QualifiedFilename qfilename(sources[idx]);
    std::string filename = qfilename.FullName();

    if(filename == "")
        filename = windowInfo->GetActiveSource();

    if (filename == "notset")
        return "";

    // we need metadata and sil for current state
    const avtDatabaseMetaData *md = fileServer->GetMetaData(filename,
                                 GetStateForSource(filename),
                                !FileServerList::ANY_STATE,
                                 FileServerList::GET_NEW_MD);
    std::string mesh;
    if(md != 0 && md->GetNumMeshes() == 1)
        mesh = md->GetMeshes(0).name;
    return mesh;
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
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
//   Brad Whitlock, Wed Jun 1 18:10:23 PST 2011
//   Escape Windows-style filenames so the drive letter does not mess up the
//   expression. Also account for difference variables that contain special
//   characters.
//
//   Hank Childs, Mon Oct 10 10:54:45 PDT 2011
//   Increment the expression name after creating the CMFE.  (cmfe0->cmfe1)
//
// ****************************************************************************

void
QvisCMFEWizard::AddCMFEExpression(void)
{
    char file_var_part[1024] = "\0";

    if( decision_donorType == DONOR_SINGLE_DATABASE ||
        decision_donorType == DONOR_MULTIPLE_DATABASES )
    {
      // Flush the donar list.
      if( decision_variable != "" )
        addDonor();

      QListWidgetItem *item = donorList->item(0);

      // Get the first donor from the list.
      if( item )
        SNPRINTF(file_var_part, 1024, "%s", item->text().toLatin1().data() );
    }
    else if (decision_donorType == DONOR_TIME_SLICES)
    {
        if (decision_timeType == TIME_TYPE_SIMTIME)
            SNPRINTF(file_var_part, 1024, "[%f]t%s:%s", decision_time, 
                     (decision_absolute ? "" : "d"), decision_variable.c_str());
        else if (decision_timeType == TIME_TYPE_SIMCYCLE)
            SNPRINTF(file_var_part, 1024, "[%d]c%s:%s", decision_cycle, 
                     (decision_absolute ? "" : "d"), decision_variable.c_str());
        else if (decision_timeType == TIME_TYPE_INDEX)
            SNPRINTF(file_var_part, 1024, "[%d]i%s:%s", decision_index, 
                     (decision_absolute ? "" : "d"), decision_variable.c_str());
    }

    char cmfe_part[1024];
    char cmfe_part_var[1024];

    if (decision_interp == INTERP_CONN_CMFE)
    {
      SNPRINTF(cmfe_part, 1024, "conn_cmfe(<%s>, <%s>)", 
               file_var_part, decision_mesh.c_str());
    }
    else
    {
      char fillstr[1024];

      if (decision_fill == FILL_CONSTANT)
        SNPRINTF(fillstr, 1024, "%f", decision_fillval);
      else
        SNPRINTF(fillstr, 1024, "<%s>", decision_fillvar.c_str());
      
      SNPRINTF(cmfe_part, 1024, "pos_cmfe(<%s>, <%s>, %s)", 
               file_var_part, decision_mesh.c_str(), fillstr);
    }

    char defn[1024];
    char defn_var[1024];

    // Single donor variable
    if( (decision_donorType == DONOR_SINGLE_DATABASE && donorList->count() == 1) ||
        (decision_donorType == DONOR_TIME_SLICES) ||
        (decision_donorType == DONOR_MULTIPLE_DATABASES && donorList->count() == 1) )
    {
      // Account for some variables that have slashes or spaces.
      std::string filteredVar(decision_diffvarname);

      if (decision_exprtype != EXPRESSION_SIMPLE)
      {
        if(filteredVar.find("/") != std::string::npos ||
           filteredVar.find(" ") != std::string::npos)
        {
          filteredVar = std::string("<") + filteredVar + std::string(">");
        }
      }

      if (decision_exprtype == EXPRESSION_SIMPLE)
        strcpy(defn, cmfe_part);
      else if (decision_exprtype == EXPRESSION_MINIMUM)
        SNPRINTF(defn, 1024, "min(%s, %s)", cmfe_part, filteredVar.c_str());
      else if (decision_exprtype == EXPRESSION_MAXIMUM)
        SNPRINTF(defn, 1024, "max(%s, %s)", cmfe_part, filteredVar.c_str());
      else if (decision_exprtype == EXPRESSION_SUM)
        SNPRINTF(defn, 1024, "(%s+%s)", cmfe_part, filteredVar.c_str());
      else if (decision_exprtype == EXPRESSION_AVERAGE)
        SNPRINTF(defn, 1024, "(%s+%s)/2.0", cmfe_part, filteredVar.c_str());
      else  if (decision_exprtype == EXPRESSION_VARIANCE)
        SNPRINTF(defn, 1024, "(%s - (%s+%s)/2.0) * (%s - (%s+%s)/2.0) + (%s - (%s+%s)/2.0) * (%s - (%s+%s)/2.0)",
                 cmfe_part, cmfe_part, filteredVar.c_str(),
                 cmfe_part, cmfe_part, filteredVar.c_str(),
                 filteredVar.c_str(), cmfe_part, cmfe_part, 
                 filteredVar.c_str(), cmfe_part, cmfe_part);

      else if (decision_exprtype == EXPRESSION_ABS_DIFF)
        SNPRINTF(defn, 1024, "abs(%s-%s)", cmfe_part, filteredVar.c_str());
      else if (decision_exprtype == EXPRESSION_DIFF_FIRST)
        SNPRINTF(defn, 1024, "(%s-%s)", cmfe_part, filteredVar.c_str());
      else if (decision_exprtype == EXPRESSION_DIFF_SECOND)
        SNPRINTF(defn, 1024, "(%s-%s)", filteredVar.c_str(), cmfe_part);
    }

    // Multiple donor variables
    else
    {
      char cmfe_part_tmp[1024];
      cmfe_part_tmp[0] = '\0';

      char cmfe_part_tmp_var[1024];
      cmfe_part_tmp_var[0] = '\0';

      // Get the donors from the list.
      for (int i = 0; i < donorList->count(); i++)
      {
        QListWidgetItem *item = donorList->item(i);

        if( item )
        {
          std::string file_var_part( item->text().toLatin1().data() );

          if (decision_interp == INTERP_CONN_CMFE)
          {
            SNPRINTF(cmfe_part, 1024, "%s conn_cmfe(<%s>, <%s>)", 
                     cmfe_part_tmp, file_var_part.c_str(),
                     decision_mesh.c_str());


            if(decision_exprtype == EXPRESSION_VARIANCE)
              SNPRINTF(cmfe_part_var, 1024,
                       "%s "
                       "((conn_cmfe(<%s>, <%s>) - %s_average) *"
                       " (conn_cmfe(<%s>, <%s>) - %s_average))", 
                       cmfe_part_tmp_var,
                       file_var_part.c_str(), decision_mesh.c_str(),
                       decision_exprname.c_str(),
                       file_var_part.c_str(), decision_mesh.c_str(),
                       decision_exprname.c_str());
          }
          else
          {
            char fillstr[1024];
            
            if (decision_fill == FILL_CONSTANT)
              SNPRINTF(fillstr, 1024, "%f", decision_fillval);
            else
              strcpy(fillstr, decision_fillvar.c_str());
            
            SNPRINTF(cmfe_part, 1024, "%s pos_cmfe(<%s>, <%s>, <%s>)", 
                     cmfe_part_tmp, file_var_part.c_str(),
                     decision_mesh.c_str(), fillstr);

            if(decision_exprtype == EXPRESSION_VARIANCE)
              SNPRINTF(cmfe_part_var, 1024,
                       "%s "
                       "((pos_cmfe(<%s>, <%s>, <%s>) - %s_average) *"
                       " (pos_cmfe(<%s>, <%s>, <%s>) - %s_average))", 
                       cmfe_part_tmp_var,
                       file_var_part.c_str(), decision_mesh.c_str(), fillstr,
                       decision_exprname.c_str(),
                       file_var_part.c_str(), decision_mesh.c_str(), fillstr,
                       decision_exprname.c_str());
          }
        }

        // Add the plus (+), minus (-), or comma (,)
        if( i < donorList->count() - 1 )
        {
          if (decision_exprtype == EXPRESSION_MINIMUM ||
              decision_exprtype == EXPRESSION_MAXIMUM)
            SNPRINTF(cmfe_part_tmp, 1024, "%s, ", cmfe_part);
          
          else if (decision_exprtype == EXPRESSION_SUM ||
                   decision_exprtype == EXPRESSION_AVERAGE ||
                   decision_exprtype == EXPRESSION_VARIANCE)
            SNPRINTF(cmfe_part_tmp, 1024, "%s + ", cmfe_part);

          else if (decision_exprtype == EXPRESSION_ABS_DIFF)
            SNPRINTF(cmfe_part_tmp, 1024, "%s - ", cmfe_part);

          if(decision_exprtype == EXPRESSION_VARIANCE)
            SNPRINTF(cmfe_part_tmp_var, 1024, "%s + ", cmfe_part_var);

          cmfe_part[0] = '\0';
          cmfe_part_var[0] = '\0';
        }
      }

      if (decision_exprtype == EXPRESSION_MINIMUM)
        SNPRINTF(defn, 1024, "min(%s)", cmfe_part);
      else if (decision_exprtype == EXPRESSION_MAXIMUM)
        SNPRINTF(defn, 1024, "max(%s)", cmfe_part);
      else if (decision_exprtype == EXPRESSION_SUM)
        SNPRINTF(defn, 1024, "(%s)", cmfe_part);
      else if (decision_exprtype == EXPRESSION_ABS_DIFF)
        SNPRINTF(defn, 1024, "abs(%s)", cmfe_part);
      else if (decision_exprtype == EXPRESSION_AVERAGE ||
               decision_exprtype == EXPRESSION_VARIANCE)
        SNPRINTF(defn, 1024, "(%s) / %d", cmfe_part, donorList->count() );

      if (decision_exprtype == EXPRESSION_VARIANCE)
        SNPRINTF(defn_var, 1024, "(%s) / %d", cmfe_part_var, donorList->count()-1 );
    }


    if( (decision_donorType == DONOR_SINGLE_DATABASE ||
         decision_donorType == DONOR_MULTIPLE_DATABASES) &&
        decision_exprtype == EXPRESSION_VARIANCE && donorList->count() > 1)
    {
      Expression *e = new Expression();
      e->SetType(GetVarType(decision_variable));

      e->SetName(decision_exprname + "_average");
      e->SetDefinition(defn);
      exprList->AddExpressions(*e);

      e->SetName(decision_exprname);
      e->SetDefinition(defn_var);
      exprList->AddExpressions(*e);
      delete e;
    }
    else
    {
      Expression *e = new Expression();
      e->SetType(GetVarType(decision_variable));

      e->SetName(decision_exprname);
      e->SetDefinition(defn);
      exprList->AddExpressions(*e);
      delete e;
    }

    // Tell the viewer that the expressions have changed.
    exprList->Notify();
    GetViewerMethods()->ProcessExpressions();

    // change active source if the user selected another database
    const std::string &active_src = windowInfo->GetActiveSource();
    if(active_src != decision_targetDatabase)
        GetViewerMethods()->ActivateDatabase(decision_targetDatabase);

    char str[1024];
    SNPRINTF(str, 1024, "cmfe%d", timesCompleted);
    decision_exprname = str;
    exprNameTxt->setText(tr(decision_exprname.c_str()));
    timesCompleted++;
}


// ****************************************************************************
// Method: QvisCMFEWizard::donorTypeChanged
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
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::donorTypeChanged(int val)
{
  if( val == 0 )
    decision_donorType = DONOR_SINGLE_DATABASE;
  else if( val == 1 )
    decision_donorType = DONOR_TIME_SLICES;
  else //if( val == 2 )
    decision_donorType = DONOR_MULTIPLE_DATABASES;

  initializePage(Page_DonorAndTargetSpecification);
}


// ****************************************************************************
// Method: QvisCMFEWizard::targetDatabaseChanged
//
// Purpose:
//   This Qt slot function is called when the user picks the source
//
// Arguments:
//   val : The index into the source combo box.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Aug 30 09:10:15 PDT 2010
//
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::targetDatabaseChanged(int val)
{
    if (globalAtts == NULL || windowInfo == NULL)
    {
        debug1 << "Cannot set sources up because of internal error" << endl;
        return;
    }
    const stringVector &sources = globalAtts->GetSources();
    if(val >= 0 && (size_t)val < sources.size())
    {
        //
        // Make the file that we reopened be the new open file. Since we're
        // reopening, this will take care of freeing the old metadata and SIL.
        //
        QualifiedFilename fileName(sources[val]);
        decision_targetDatabase = fileName.FullName();
        UpdateTargetMesh();

        if(decision_donorType != DONOR_MULTIPLE_DATABASES)
            UpdateDonorField();
    }
    else
        debug1 << "Error with file name" << endl;
}


// ****************************************************************************
// Method: QvisCMFEWizard::donorDatabaseChanged
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
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::donorDatabaseChanged(int val)
{
    if (globalAtts == NULL || windowInfo == NULL)
    {
        debug1 << "Cannot set sources up because of internal error" << endl;
        return;
    }
    const stringVector &sources = globalAtts->GetSources();
    if(val >= 0 && (size_t)val < sources.size())
    {
        //
        // Make the file that we reopened be the new open file. Since we're
        // reopening, this will take care of freeing the old metadata and SIL.
        //
        QualifiedFilename fileName(sources[val]);
        decision_donorDatabase = fileName.FullName();
        UpdateDonorField();
    }
    else
        debug1 << "Error with file name" << endl;
}


// ****************************************************************************
// Method: QvisCMFEWizard::absVsRelTimeChanged
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
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::absVsRelTimeChanged(int val)
{
    decision_absolute = (val == 0 ? true : false);
}

// ****************************************************************************
// Method: QvisCMFEWizard::timeTypeChanged
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
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::timeTypeChanged(int val)
{
    if (val == 0)
        decision_timeType = TIME_TYPE_SIMTIME;
    else if (val == 1)
        decision_timeType = TIME_TYPE_SIMCYCLE;
    else //if( val == 2 )
        decision_timeType = TIME_TYPE_INDEX;

    initializePage(Page_TimeSpecification);
}


// ****************************************************************************
// Method: QvisCMFEWizard::timeChanged
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
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::timeChanged(const QString &s)
{
    bool okay = true;
    decision_time = s.toDouble(&okay);
}

// ****************************************************************************
// Method: QvisCMFEWizard::cycleChanged
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
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::cycleChanged(const QString &s)
{
    bool okay = true;
    decision_cycle = s.toInt(&okay);
}

// ****************************************************************************
// Method: QvisCMFEWizard::indexChanged
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
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

void
QvisCMFEWizard::indexChanged(const QString &s)
{
    bool okay = true;
    decision_index = s.toInt(&okay);
}

// ****************************************************************************
// Method: QvisCMFEWizard::donorFieldVarChanged
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
QvisCMFEWizard::donorFieldVarChanged(const QString &str)
{
    decision_variable = str.toStdString();
}


// ****************************************************************************
// Method: QvisCMFEWizard::targetMeshVarChanged
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
QvisCMFEWizard::targetMeshVarChanged(const QString &str)
{
    decision_mesh = str.toStdString();
}


// ****************************************************************************
// Method: QvisCMFEWizard::interpChanged
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
QvisCMFEWizard::interpChanged(int val)
{
    if (val == 0)
        decision_interp = INTERP_CONN_CMFE;
    else
        decision_interp = INTERP_POS_CMFE;
    initializePage(Page_InterpSelection);
}

// ****************************************************************************
// Method: QvisCMFEWizard::nonOverlapChanged
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
QvisCMFEWizard::nonOverlapChanged(int val) 
{ 
    if (val == 0)
        decision_fill = FILL_CONSTANT;
    else
        decision_fill = FILL_VAR;
    initializePage(Page_InterpSelection);
}

// ****************************************************************************
// Method: QvisCMFEWizard::nonOverlapTxtChanged
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
QvisCMFEWizard::nonOverlapTxtChanged(const QString &s)
{
    bool okay = false;
    decision_fillval = s.toDouble(&okay);
}

// ****************************************************************************
// Method: QvisCMFEWizard::nonOverlapVarChanged
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
QvisCMFEWizard::nonOverlapVarChanged(const QString &s) 
{
    decision_fillvar = s.toStdString();
}

// ****************************************************************************
// Method: QvisCMFEWizard::exprNameChanged
//
// Purpose:f
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
QvisCMFEWizard::exprNameChanged(const QString &s)
{
    decision_exprname = s.toStdString();
}

// ****************************************************************************
// Method: QvisCMFEWizard::exprTypeChanged
//
// Purpose:
//   This Qt slot function is called when the way to study the expression
//   is changed.
//
// Programmer: Hank Childs
// Creation:   August 4, 2010
//
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
//   Cyrus Harrison, Fri Oct 21 11:34:46 PDT 2011
//   Disable the finish button until we have a valid diff var name.
//
// ****************************************************************************

void 
QvisCMFEWizard::exprTypeChanged(int v)
{
    if (v == 0)
    {
        decision_exprtype = EXPRESSION_SIMPLE;
        button(QWizard::FinishButton)->setEnabled(true);
    }
    else
    {
        if( decision_diffvarname == "" && donorList->count() == 1 )
            button(QWizard::FinishButton)->setEnabled(false);

        // Single donor or multiple donors
        if( ((decision_donorType == DONOR_SINGLE_DATABASE ||
             decision_donorType == DONOR_MULTIPLE_DATABASES) &&
            donorList->count() > 1) || // Multiple donors
            // One donor in the list and one ready to be added
            (donorList->count() == 1 &&
             decision_donorDatabase != "" && decision_variable != "" ) )
        {
          exprDiffVar->setEnabled(false);
          exprDiffVar->setVisible(false);
        }
        else
        {
          exprDiffVar->setEnabled(true);
          exprDiffVar->setVisible(true);
        }

        if (exprDiffTypeSelect->button(0)->isChecked())
            decision_exprtype = EXPRESSION_MINIMUM;
        else if (exprDiffTypeSelect->button(1)->isChecked())
            decision_exprtype = EXPRESSION_MAXIMUM;
        else if (exprDiffTypeSelect->button(2)->isChecked())
            decision_exprtype = EXPRESSION_SUM;
        else if (exprDiffTypeSelect->button(3)->isChecked())
            decision_exprtype = EXPRESSION_AVERAGE;
        else if (exprDiffTypeSelect->button(4)->isChecked())
            decision_exprtype = EXPRESSION_VARIANCE;
        else if (exprDiffTypeSelect->button(5)->isChecked())
            decision_exprtype = EXPRESSION_ABS_DIFF;
        else if (exprDiffTypeSelect->button(6)->isChecked())
            decision_exprtype = EXPRESSION_DIFF_FIRST;
        else if (exprDiffTypeSelect->button(7)->isChecked())
            decision_exprtype = EXPRESSION_DIFF_SECOND;
        else
            decision_exprtype = EXPRESSION_SUM;
    }
    initializePage(Page_ActivityDescription);
}


// ****************************************************************************
// Method: QvisCMFEWizard::exprDiffVarChanged
//
// Purpose:
//   This Qt slot function is called when the variable to contrast the
//   CMFE variable is changed.
//
// Programmer: Hank Childs
// Creation:   August 4, 2010
//
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
//   Cyrus Harrison, Fri Oct 21 11:34:46 PDT 2011
//   If we have a valid diff var name, reenable the Finish button.
//
// ****************************************************************************

void 
QvisCMFEWizard::exprDiffVarChanged(const QString &s)
{
    decision_diffvarname = s.toStdString();
    button(QWizard::FinishButton)->setEnabled(true);
}

// ****************************************************************************
// Method: QvisCMFEWizard::exprDiffTypeChanged
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
QvisCMFEWizard::exprDiffTypeChanged(int v)
{
    if (v == 0)
        decision_exprtype = EXPRESSION_MINIMUM;
    else if (v == 1)
        decision_exprtype = EXPRESSION_MAXIMUM;
    else if (v == 2)
        decision_exprtype = EXPRESSION_SUM;
    else if (v == 3)
        decision_exprtype = EXPRESSION_AVERAGE;
    else if (v == 4)
        decision_exprtype = EXPRESSION_VARIANCE;
    else if (v == 5)
        decision_exprtype = EXPRESSION_ABS_DIFF;
    else if (v == 6)
        decision_exprtype = EXPRESSION_DIFF_FIRST;
    else if (v == 7)
        decision_exprtype = EXPRESSION_DIFF_SECOND;
}

// ****************************************************************************
// Method: QvisCMFEWizard::targetDatabaseOpen
//
// Purpose:
//   This Qt slot function is called when user wants to open a new target
//   database.
//
// Programmer: Cyrus Harrison
// Creation:   Fri Aug 27 16:39:44 PDT 2010
//
// Modifications:
//
// ****************************************************************************

void 
QvisCMFEWizard::targetDatabaseOpenClicked()
{
    selectedTargetDatabase = "";
    const std::string &active_src = windowInfo->GetActiveSource();
    QString name = QvisFileOpenDialog::getOpenFileName("", tr("Select Target Database"));

    if(name != QString(""))
    {
        selectedTargetDatabase  = name.toStdString();
        GetViewerMethods()->OpenDatabase(selectedTargetDatabase);
        // change active source back...
        if(active_src != "notset")
            GetViewerMethods()->ActivateDatabase(active_src);
    }
}

// ****************************************************************************
// Method: QvisCMFEWizard::donorDatabaseOpen
//
// Purpose:
//   This Qt slot function is called when user wants to open a new donor
//   database.
//
// Programmer: Cyrus Harrison
// Creation:   Fri Aug 27 16:39:44 PDT 2010
//
//
// Modifications:
//
// ****************************************************************************

void 
QvisCMFEWizard::donorDatabaseOpenClicked()
{
    selectedDonorDatabase  = "";

    const std::string &active_src= windowInfo->GetActiveSource();
    QString name = QvisFileOpenDialog::getOpenFileName("", tr("Select Donor Database"));

    if(name != QString(""))
    {
        selectedDonorDatabase  = name.toStdString();
        GetViewerMethods()->OpenDatabase(selectedDonorDatabase);
        // change active source back...
        if(active_src != "notset")
            GetViewerMethods()->ActivateDatabase(active_src);
    }
}


void
QvisCMFEWizard::donorListClicked(QListWidgetItem *item)
{
}

void
QvisCMFEWizard::addDonor()
{
  if ( (decision_variable == "") ||
       (decision_donorDatabase == "" && decision_donorType == DONOR_MULTIPLE_DATABASES))
    return;

  std::string filteredDB(decision_donorDatabase);
#ifdef _WIN32
  // Escape back slashes and drive letter punctuation so it can exist
  // in the generated expression.
  filteredDB = StringHelpers::Replace(filteredDB, "\\", "\\\\");
  filteredDB = StringHelpers::Replace(filteredDB, ":\\", "\\:\\");
#endif
  const char *src = filteredDB.c_str();
  const char *after_colon = strstr(src, ":");
  if (after_colon != NULL)
    src = after_colon+1;

  std::string donor;

  if( decision_donorType == DONOR_SINGLE_DATABASE )
    donor = std::string("[0]id:") + decision_variable;
  else if( decision_donorType == DONOR_MULTIPLE_DATABASES )  
    donor = std::string(src) + std::string(":") + decision_variable ;

  QListWidgetItem *item;

  // Do not add a duplicate donor.
  for (int i = 0; i < donorList->count(); i++)
  {
    if( (item = donorList->item(i)) )
    {
      if( donor == std::string( item->text().toLatin1().data() ) )
      {
        donorFieldVar->setText(tr("<Select>"));
        decision_variable = "";
        
          return;
      }
    }
  }

  item = new QListWidgetItem(donor.c_str(), donorList);
  
  donorList->setCurrentItem(item);
  
  donorFieldVar->setText(tr("<Select>"));
  decision_variable = "";
}

void
QvisCMFEWizard::deleteDonor()
{
    if (!donorList->selectedItems().empty())
    {
        qDeleteAll(donorList->selectedItems());
    }
}

void
QvisCMFEWizard::deleteDonors()
{
    donorList->clear();
}


// ****************************************************************************
// Method: QvisCMFEWizard::Update
//
// Purpose:
//   This method is called when the Global Atts or FileServerList that
//   the widget watches is updated.
//
//
// Programmer: Cyrus Harrison
// Creation:   Fri Mar 12 11:18:50 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::Update(Subject *subject)
{
    if(globalAtts == 0 || windowInfo == 0)
        return;

    if(subject == globalAtts &&
       globalAtts->IsSelected(GlobalAttributes::ID_sources))
    {
        UpdateSourceList();
    }
}

// ****************************************************************************
// Method: QvisCMFEWizard::SubjectRemoved
//
// Purpose:
//   Removes the subjects that this wizard observes.
//
// Arguments:
//   subject: The subject that is being removed.
//
//
// Programmer: Cyrus Harrison
// Creation:   Fri Aug 27 16:39:44 PDT 2010

// Modifications:
//
// ****************************************************************************

void
QvisCMFEWizard::SubjectRemoved(Subject *subject)
{
    if(subject == globalAtts)
        globalAtts = NULL;
    else if(subject == windowInfo)
        windowInfo = NULL;
}
