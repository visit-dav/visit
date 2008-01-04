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

#ifndef QVIS_SCATTER_PLOT_WIZARD_H
#define QVIS_SCATTER_PLOT_WIZARD_H
#include <QvisWizard.h>

class QButtonGroup;
class QFrame;
class QvisScatterWidget;

// ****************************************************************************
// Class: QvisScatterPlotWizard
//
// Purpose:
//   This class is a wizard that helps the user choose initial parameters for
//   the Scatter plot.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:53:27 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class QvisScatterPlotWizard : public QvisWizard
{
    Q_OBJECT
public:
    QvisScatterPlotWizard(AttributeSubject *s, QWidget *parent,
        const char *name = 0);
    virtual ~QvisScatterPlotWizard();

    virtual bool appropriate(QWidget *) const;
private slots:
    void choseYVariable(const QString &);
    void choseZVariable(const QString &);
    void choseColorVariable(const QString &);
    void decideZ(int);
    void decideColor(int);
protected:
    void CreateVariablePage(QFrame **, QvisScatterWidget **,
        const char *prompt, const char *slot,
        bool highlight, bool threeD, bool colorPoints);
    void CreateYesNoPage(QFrame **, QvisScatterWidget **, QButtonGroup **,
        const char *prompt, const char *slot,
        bool highlight, bool threeD, bool colorPoints);
    void CreateFinishPage(QFrame **, QvisScatterWidget **,
        const char *prompt,
        bool highlight, bool threeD, bool colorPoints);

    QFrame            *page1;
    QFrame            *page2;
    QFrame            *page3;
    QFrame            *page4;
    QFrame            *page5;
    QFrame            *page6;

    QvisScatterWidget *scatter1;
    QvisScatterWidget *scatter2;
    QvisScatterWidget *scatter3;
    QvisScatterWidget *scatter4;
    QvisScatterWidget *scatter5;
    QvisScatterWidget *scatter6;

    QButtonGroup      *bg2;
    QButtonGroup      *bg4;

    bool selectZCoord;
    bool selectColor;
};

#endif
