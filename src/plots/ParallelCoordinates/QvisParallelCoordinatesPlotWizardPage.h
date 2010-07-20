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

#ifndef QVIS_PARALLEL_COORDINATES_PLOT_WIZARD_PAGE_H
#define QVIS_PARALLEL_COORDINATES_PLOT_WIZARD_PAGE_H
#include <QvisWizard.h>

#include <vectortypes.h>
#include <QMap>

class QListWidget;
class QPushButton;
class QButtonGroup;
class QFrame;
class QLabel;
class QvisParallelCoordinatesWidget;
class avtDatabaseMetaData;
class ExpressionList;


// ****************************************************************************
// Class: QvisParallelCoordinatesPlotWizardPage
//
// Purpose:
//   Wizard page that provides the interface for selecting axis variables.
//
// Notes: Refactored from QvisParallelCoordinatesPlotWizard.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul  8 09:02:28 PDT 2010
//
// Modifications:
//
// ****************************************************************************

class QvisParallelCoordinatesPlotWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    QvisParallelCoordinatesPlotWizardPage(AttributeSubject *s,
                                QWidget *parent,
                                const std::string &varName,
                                const avtDatabaseMetaData *md,
                                const ExpressionList *exprList);

    virtual ~QvisParallelCoordinatesPlotWizardPage();
    void GetAxisNames(stringVector &res);


protected slots:
    void OnScalarVarSelectionChanged();
    void OnAxisVarSelectionChanged();
    void OnAddButtonPressed();
    void OnUpButtonPressed();
    void OnDownButtonPressed();
    void OnRemoveButtonPressed();

protected:

    virtual bool isComplete() const;

    void  InitScalarVarNames(const avtDatabaseMetaData *md,
                             const ExpressionList *expList);

    void SetupAxisVariableSelectionPage();

    void UpdateVarsList();
    void UpdateAxisVarsList();
    void UpdatePreview(QvisParallelCoordinatesWidget *preview);


    bool                                  ready;
    std::string                           varName;

    QLabel                               *infoLbl;
    QListWidget                          *varsList;
    QListWidget                          *axisVarsList;
    QPushButton                          *addButton;
    QPushButton                          *upButton;
    QPushButton                          *downButton;
    QPushButton                          *removeButton;

    QvisParallelCoordinatesWidget        *selectionPreview;

    QStringList                           scalarVarNames;
    QStringList                           scalarExprNames;
    QStringList                           scalarDBExprNames;
    QMap<QString,bool>                    usedVars;

};

#endif
