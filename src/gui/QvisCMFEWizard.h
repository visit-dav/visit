// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_CMFE_WIZARD_H
#define QVIS_CMFE_WIZARD_H
#include <QvisWizard.h>
#include <MovieUtility.h>
#include <map>
#include <vectortypes.h>
#include <Expression.h>
#include <SimpleObserver.h>

class QButtonGroup;
class QComboBox;
class QListWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class QListWidgetItem;

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
