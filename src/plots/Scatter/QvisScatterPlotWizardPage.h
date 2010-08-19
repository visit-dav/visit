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

#ifndef QVIS_SCATTER_PLOT_WIZARD_PAGE_H
#define QVIS_SCATTER_PLOT_WIZARD_PAGE_H
#include <QvisWizard.h>

#include <vectortypes.h>
#include <QMap>


class QvisScatterWidget;
class QCheckBox;
class QvisVariableButton;


// ****************************************************************************
// Class: QvisScaterPlotWizardPage
//
// Purpose:
//   Wizard page that provides the interface setting up a scatter plot.
//
// Notes: Refactored from QvisScatterPlotWizard.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Jul  8 09:02:28 PDT 2010
//
// Modifications:
//
// ****************************************************************************



class QvisScatterPlotWizardPage : public QWizardPage
{
    Q_OBJECT
public:
    QvisScatterPlotWizardPage(AttributeSubject *s,
                             QWidget *parent,
                             const std::string &xvar_name);

    virtual ~QvisScatterPlotWizardPage();

    void GetSelectedVars(stringVector &res) const;
    bool ZEnabled() const;
    bool ColorEnabled() const;

protected slots:
    void choseXVariable(const QString &);
    void choseYVariable(const QString &);
    void choseZVariable(const QString &);
    void choseColorVariable(const QString &);

    void decideZ(int);
    void decideColor(int);
protected:
    virtual bool        isComplete() const;
    void                checkComplete();

    bool                ready;
    QvisScatterWidget  *preview;
    QvisVariableButton *xVarButton;
    QvisVariableButton *yVarButton;
    QvisVariableButton *zVarButton;
    QCheckBox          *zVarCheck;
    QvisVariableButton *colorVarButton;
    QCheckBox          *colorVarCheck;

    std::string         xVarName;
    std::string         yVarName;
    std::string         zVarName;
    std::string         colorVarName;

};

#endif
