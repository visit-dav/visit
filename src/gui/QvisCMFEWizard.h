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

#ifndef QVIS_CMFE_WIZARD_H
#define QVIS_CMFE_WIZARD_H
#include <QvisWizard.h>
#include <MovieUtility.h>
#include <map>
#include <vectortypes.h>
#include <Expression.h>
#include <SimpleObserver.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QListWidget;
class QLabel;
class QLineEdit;
class QListWidget;
class QTreeWidget;
class QPushButton;
class QRadioButton;
class QScrollArea;
class QSpinBox;
class QTextEdit;
class QVBoxLayout;
class QListWidgetItem;

class QvisDialogLineEdit;
class QvisVariableButton;
class QvisCustomSourceVariableButton;

class GlobalAttributes;
class WindowInformation;
class ExpressionList;

// ****************************************************************************
// Class: QvisCMFEWizard
//
// Purpose:
//   This class contains the Data-Level Comparisons/CMFE wizard that leads the 
//   user through all of the questions needed to set up a CMFE.
//
// Programmer: Hank Childs
// Creation:   August 1, 2010
//
// Modifications:
//   Cyrus Harrison, Mon Aug 30 11:59:25 PDT 2010
//   Simplify wizard & add ability to open new databases.
//
// ****************************************************************************

class QvisCMFEWizard : public QvisWizard, public SimpleObserver
{
    Q_OBJECT
public:
    QvisCMFEWizard(AttributeSubject *, QWidget *parent);
    virtual ~QvisCMFEWizard();

    int Exec();

    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);

    void    SetGlobalAttributes(GlobalAttributes *ga);
    void    SetWindowInformation(WindowInformation *wi);
    void    SetExpressionList(ExpressionList *el);

    void    AddCMFEExpression(void);

    virtual int nextId() const;
    virtual bool validateCurrentPage();
protected:
    virtual void initializePage(int id);
private slots:

    void donorTypeChanged(int);
    void targetDatabaseChanged(int);
    void donorDatabaseChanged(int);
    void absVsRelTimeChanged(int);
    void timeTypeChanged(int);
    void timeChanged(const QString &);
    void cycleChanged(const QString &);
    void indexChanged(const QString &);
    void donorFieldVarChanged(const QString &);
    void targetMeshVarChanged(const QString &);
    void interpChanged(int);
    void nonOverlapChanged(int);
    void nonOverlapTxtChanged(const QString &);
    void nonOverlapVarChanged(const QString &);
    void exprNameChanged(const QString &);
    void exprTypeChanged(int);
    void exprDiffVarChanged(const QString &);
    void exprDiffTypeChanged(int);

    void targetDatabaseOpenClicked();
    void donorDatabaseOpenClicked();

    void donorListClicked(QListWidgetItem*);
    void addDonor();
    void deleteDonor();
    void deleteDonors();

private:
    enum {
        Page_DonorType,                   // page0
        Page_DonorAndTargetSpecification, // page1
        Page_TimeSpecification,           // page2
        Page_InterpSelection,             // page3
        Page_ActivityDescription          // page4
    };

    void CreateDonorTypePage();          // page0
    void CreateDonorAndTargetPage();     // page1
    void CreateTimeSpecificationPage();  // page2
    void CreateInterpSelectionPage();    // page3
    void CreateActivityPage();           // page4


    int                     decision_donorType;
    std::string             decision_donorDatabase;
    std::string             decision_targetDatabase;
    bool                    decision_absolute;
    int                     decision_timeType;
    double                  decision_time;
    int                     decision_cycle;
    int                     decision_index;
    std::string             decision_variable;
    std::string             decision_mesh;
    int                     decision_interp;
    int                     decision_fill;
    std::string             decision_fillvar;
    double                  decision_fillval;
    int                     decision_exprtype;
    std::string             decision_diffvarname;
    std::string             decision_exprname;

    std::string             selectedTargetDatabase;
    std::string             selectedDonorDatabase;

    GlobalAttributes       *globalAtts;
    WindowInformation      *windowInfo;
    ExpressionList         *exprList;

    // Setup type of cmfe.
    QWizardPage            *page0;
    QButtonGroup           *donorTypeSelect;

    // Set up the donor and target
    QWizardPage            *page1;
    QLabel                             *targetDatabaseLabel;
    QComboBox                          *targetDatabase;
    QPushButton                        *targetDatabaseOpen;
    QvisCustomSourceVariableButton     *targetMeshVar;
    QLabel                             *donorDatabaseLabel;
    QComboBox                          *donorDatabase;
    QPushButton                        *donorDatabaseOpen;
    QvisCustomSourceVariableButton     *donorFieldVar;
    QListWidget *donorList;
    QPushButton *donorListDeleteDonor, *donorListDeleteAllDonors, *donorListAddDonor;

    // Set up the absolute vs relative time.
    QWizardPage            *page2;
    QButtonGroup           *absVsRelTimeSelect;
    QButtonGroup           *timeTypeSelect;
    QLineEdit              *timeTxt;
    QLineEdit              *cycleTxt;
    QLineEdit              *indexTxt;


    // Set up the CMFE type
    QWizardPage            *page3;
    QButtonGroup           *interpSelect;

    QButtonGroup           *nonOverlapSelect;
    QLineEdit              *nonOverlapTxt;
    QvisVariableButton     *nonOverlapVar;

    // Set up the expression
    QWizardPage            *page4;
    QLineEdit              *exprNameTxt;
    QButtonGroup           *exprTypeSelect;
    QButtonGroup           *exprDiffTypeSelect;
    QvisVariableButton     *exprDiffVar;

    static int              timesCompleted;

    // Helper methods.
    void   UpdateSourceList();
    void   UpdateDonorField();
    void   UpdateTargetMesh();
    Expression::ExprType GetVarType(const std::string &);
    std::string GetMeshForTargetDatabase(void);
};

#endif
