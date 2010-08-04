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

#ifndef QVIS_CMFE_WIZARD_H
#define QVIS_CMFE_WIZARD_H
#include <QvisWizard.h>
#include <MovieUtility.h>
#include <map>
#include <vectortypes.h>
#include <Expression.h>

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
// ****************************************************************************

class QvisCMFEWizard : public QvisWizard
{
    Q_OBJECT
public:
    QvisCMFEWizard(AttributeSubject *, QWidget *parent);
    virtual ~QvisCMFEWizard();

    int Exec();

    void    SetGlobalAttributes(GlobalAttributes *ga) { globalAtts = ga; };
    void    SetWindowInformation(WindowInformation *wi) {windowInfo = wi;};
    void    SetExpressionList(ExpressionList *el);

    void    AddCMFEExpression(void);

    virtual int nextId() const;
    virtual bool validateCurrentPage();
protected:
    virtual void initializePage(int id);
private slots:
    
    void page1_donorTypeChanged(int);
    void page2_sourceChanged(int);
    void page3_absoluteTimeChanged(int);
    void page3_timeTypeChanged(int);
    void page3_timeChanged(const QString &);
    void page3_cycleChanged(const QString &);
    void page3_indexChanged(const QString &);
    void page4_donorChanged(const QString &);
    void page4_targetChanged(const QString &);
    void page5_interpChanged(int);
    void page5_fillChanged(int);
    void page5_overlapConstChanged(const QString &);
    void page5_overlapVarChanged(const QString &);
    void page6_exprNameChanged(const QString &);
    void page6_exprTypeChanged(int);
    void page6_diffVariable1Changed(const QString &);
    void page6_diffTypeChanged(int);

private:
    enum {
        Page_Intro,                       // page0
        Page_DonorType,                   // page1
        Page_FileSelection,               // page2
        Page_TimeSpecification,           // page3
        Page_DonorAndTargetSpecification, // page4
        Page_InterpSelection,             // page5
        Page_ActivityDescription,         // page6
    };

    void CreateIntroPage();              // page0
    void CreateDonorTypePage();          // page1
    void CreateFileSelectionPage();      // page2
    void CreateTimeSpecificationPage();  // page3
    void CreateDonorAndTargetPage();     // page4
    void CreateInterpSelectionPage();    // page5
    void CreateActivityPage();           // page6

    int                     decision_donorType;
    std::string             decision_source;
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
   
    GlobalAttributes       *globalAtts;
    WindowInformation      *windowInfo;
    ExpressionList         *exprList;

    // Introduce CMFE.
    QWizardPage            *page0;

    // Set up the donor type.
    QWizardPage            *page1;
    QButtonGroup           *page1_buttongroup;

    // Set up the file source.
    QWizardPage            *page2;
    QComboBox              *page2_sourceComboBox;

    // Set up the absolute vs relative time.
    QWizardPage            *page3;
    QButtonGroup           *page3_buttongroup1;
    QButtonGroup           *page3_buttongroup2;
    QLineEdit              *page3_lineEditTime;
    QLineEdit              *page3_lineEditCycle;
    QLineEdit              *page3_lineEditIndex;
    
    // Set up the donor and target
    QWizardPage            *page4;
    QvisCustomSourceVariableButton     *page4_donorVariable;
    QvisVariableButton                 *page4_targetMesh;

    // Set up the CMFE type
    QWizardPage            *page5;
    QButtonGroup           *page5_buttongroup1;
    QButtonGroup           *page5_buttongroup2;
    QLineEdit              *page5_lineEdit;
    QvisVariableButton     *page5_overlapVariable;
    QRadioButton           *page5_button1;
    QRadioButton           *page5_button2;

    // Set up the expression
    QWizardPage            *page6;
    QButtonGroup           *page6_buttongroup1;
    QButtonGroup           *page6_buttongroup2;
    QvisVariableButton     *page6_diffVariable1;
    QLineEdit              *page6_lineEdit;

    static int              timesCompleted;

    // Helper methods.
    void   UpdateSourceList();
    void   UpdateDonorFields();
    void   UpdateMeshField();
    Expression::ExprType GetVarType(const std::string &);
    std::string GetMeshForActiveSource(void);
};

#endif
