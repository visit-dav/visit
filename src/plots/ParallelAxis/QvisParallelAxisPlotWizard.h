/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#ifndef QVIS_PARALLEL_AXIS_PLOT_WIZARD_H
#define QVIS_PARALLEL_AXIS_PLOT_WIZARD_H
#include <QvisWizard.h>

#define MAX_WIZARD_SELECTABLE_AXES      8

class QButtonGroup;
class QFrame;
class QLabel;
class QvisParallelAxisWidget;


// ****************************************************************************
// Class: QvisParallelAxisPlotWizard
//
// Purpose:
//   This class is a wizard that helps the user choose variables for initial
//   axes of a ParallelAxis plot.
//
// Note: This is intended to emulate the style of the QvisScatterPlotWizard
//       for the Scatter plot, which came first.
//
// Programmer: Mark Blair
// Creation:   Mon Jun 19 15:16:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

class QvisParallelAxisPlotWizard : public QvisWizard
{
    Q_OBJECT
public:
    QvisParallelAxisPlotWizard(AttributeSubject *s, QWidget *parent,
        const std::string &varName, const char *name = NULL);
    virtual ~QvisParallelAxisPlotWizard();

private slots:
    void choseAxisVariable(const QString &varName);
    void decideIfAnotherAxis(int buttonIndex);
    
protected slots:
    virtual void next();
    virtual void back();

protected:
    void CreateAxisVariablePage(QFrame **, QvisParallelAxisWidget **,
        int axisOrdinal, const char *prompt);
    void CreateAxisYesNoPage(QFrame **, QvisParallelAxisWidget **,
        QButtonGroup **, int axisOrdinal, const char *prompt);
    void CreateFinishPage(QFrame **, QvisParallelAxisWidget **, const char *prompt);

    void InitializeParallelAxisAttributes(const std::string &varName);
    bool UniqueAxisVariableName(const std::string &varName);
    void CompleteFinalAxisSequence();
    
    QFrame                 *pages[MAX_WIZARD_SELECTABLE_AXES*2 - 2];
    QvisParallelAxisWidget *thumbnails[MAX_WIZARD_SELECTABLE_AXES*2 - 2];
    QButtonGroup           *yesNoButtonGroups[MAX_WIZARD_SELECTABLE_AXES - 2];
    QLabel                 *dupVarMessages[MAX_WIZARD_SELECTABLE_AXES];
    
    std::string            axisVarNames[MAX_WIZARD_SELECTABLE_AXES];
    bool                   axisYesNos[MAX_WIZARD_SELECTABLE_AXES];
    
    int                    curAxisCount;
};

#endif
