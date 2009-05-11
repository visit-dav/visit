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
#include <SpreadsheetCurveViewer.h>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QTextEdit>
#include <QTextStream>

#include <snprintf.h>

#include <ViewerPlot.h>

#define PLOT_ENABLED
#ifdef PLOT_ENABLED
#include <PlotPluginManager.h>
#include <ViewerWindowManager.h>
#include <ViewerWindow.h>
#include <ViewerMethods.h>
#endif

int SpreadsheetCurveViewer::counter = -1;

// ****************************************************************************
// Method: SpreadsheetCurveViewer::SpreadsheetCurveViewer
//
// Purpose: 
//   constructor
//
// Arguments:
//   plot : The plot that gives us access to the viewer.
//   parent : The window's parent.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:48:45 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

SpreadsheetCurveViewer::SpreadsheetCurveViewer(ViewerPlot *p, QWidget *parent) : 
    QMainWindow(parent)
{
    counter++;
    plot = p;

    // Create widgets.
    setWindowTitle(tr("Spreadsheet Curve Data"));

    QWidget *top = new QWidget(this);
    setCentralWidget(top);
    QVBoxLayout *topLayout = new QVBoxLayout(top);
    topLayout->setSpacing(5);
    topLayout->setMargin(10);
#ifdef Q_WS_MAC
    QWidget *menuContainer = new QWidget(top);
    QHBoxLayout *menuLayout = new QHBoxLayout(menuContainer);
    topLayout->addWidget(menuContainer);
#endif

    curveText = new QTextEdit(top);
    curveText->setMinimumWidth(fontMetrics().width("X") * 55);
    curveText->setMinimumHeight(fontMetrics().height() * 20);
    topLayout->addWidget(curveText);

    QHBoxLayout *layout = new QHBoxLayout(0);
    topLayout->addLayout(layout);
    layout->setSpacing(5);
    layout->addStretch(5);

    QPushButton *dismiss = new QPushButton(tr("Dismiss"),top);
    connect(dismiss, SIGNAL(clicked()),
            this, SLOT(deleteLater()));
    layout->addWidget(dismiss);

    //
    // Do the main menu.
    //
    fileMenu = new QMenu(tr("&File"), this);
#ifdef Q_WS_MAC
    QPushButton *fileButton = new QPushButton(tr("&File"), menuContainer);
    menuLayout->addWidget(fileButton);
    fileButton->setMenu(fileMenu);
#else
    menuBar()->addMenu(fileMenu);
#endif
    fileMenu->addAction(tr("Save curve . . ."), this, SLOT(saveCurve()), Qt::CTRL+Qt::Key_S);

    editMenu = new QMenu(tr("&Edit"), this);
#ifdef Q_WS_MAC
    QPushButton *editButton = new QPushButton(tr("&Edit"), menuContainer);
    menuLayout->addWidget(editButton);
    editButton->setMenu(editMenu);
#else
    menuBar()->addMenu(editMenu);
#endif
    editMenu->addAction(tr("&Copy"), this, SLOT(copyCurve()), Qt::CTRL+Qt::Key_C);

#ifdef PLOT_ENABLED
    operationsMenu = new QMenu(tr("&Operations"), this);
#ifdef Q_WS_MAC
    QPushButton *opButton = new QPushButton(tr("&Operations"), menuContainer);
    menuLayout->addWidget(opButton);
    opButton->setMenu(operationsMenu);
#else
    menuBar()->addMenu(operationsMenu);
#endif
    operationsMenu->addAction(tr("Plot curve"), this, SLOT(operationPlot()));
#endif
}

// ****************************************************************************
// Method: SpreadsheetCurveViewer::~SpreadsheetCurveViewer
//
// Purpose: 
//   dtor
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:51:01 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

SpreadsheetCurveViewer::~SpreadsheetCurveViewer()
{
}

// ****************************************************************************
// Method: SpreadsheetCurveViewer::setData
//
// Purpose: 
//   Store the curve data as text in the window.
//
// Arguments:
//   vals : The values.
//   nvals : The number of values.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:50:25 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetCurveViewer::setData(const double *vals, int nvals)
{
    char tmp[200];
    curveText->clear();
    for(int i = 0; i < nvals; ++i)
    {
        SNPRINTF(tmp, 200, "%1.16e %1.16e", vals[i*2], vals[i*2+1]);
        curveText->append(tmp);
    }
}

//
// Qt slots
//

// ****************************************************************************
// Method: SpreadsheetCurveViewer::saveCurve
//
// Purpose: 
//   Save the curve data to a file.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:50:02 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetCurveViewer::saveCurve()
{
    filename = QFileDialog::getSaveFileName(this, tr("Select curve filename"), tr("spreadsheet%1.curve").arg(counter),
                                            tr("Curve (*.curve)"));

    if(!filename.isEmpty())
        saveCurveFile(filename);
}

// ****************************************************************************
// Method: SpreadsheetCurveViewer::saveCurveFile
//
// Purpose: 
//   Save the curve data as a file.
//
// Arguments:
//   name : The name of the file to save.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:49:42 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetCurveViewer::saveCurveFile(const QString &name)
{
    // Save the text to a file.
    QFile file(name);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream( &file );
        stream << "# curve\n" << curveText->toPlainText();
        file.close();
    }
    else
    {
        QString err(tr("Could not write file %1.").arg(name));
        plot->Error(err);
        filename = "";
    }
}


// ****************************************************************************
// Method: SpreadsheetCurveViewer::copyToClipboard
//
// Purpose: 
//   This slot saves the selected cells to the clipboard so they can be
//   pasted into other applications.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 15:40:06 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetCurveViewer::copyToClipboard()
{
    // Copy the text to the clipbard.
    QClipboard *cb = QApplication::clipboard();
    cb->setText(curveText->toPlainText());
}

// ****************************************************************************
// Method: SpreadsheetCurveViewer::operationPlot
//
// Purpose: 
//   Save the curve data and plot it in another VisIt window.
//
// Programmer: Brad Whitlock
// Creation:   Fri May  8 16:49:16 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
SpreadsheetCurveViewer::operationPlot()
{
#ifdef PLOT_ENABLED
    // Save the curve file
    saveCurve();
    if(!filename.isEmpty())
    {
        // Get a lineout window.
        ViewerWindow *win = ViewerWindowManager::Instance()->GetLineoutWindow();

        // Make the window active
        plot->GetViewerMethods()->SetActiveWindow(win->GetWindowId()+1);

        // Open the curve file and do a plot of it.
        PlotPluginManager *mgr = plot->GetPlotPluginManager();
        std::string plotName("Curve_1.0");
        if(mgr->PluginExists(plotName))
        {
            int idx = mgr->GetEnabledIndex(plotName);
            plot->GetViewerMethods()->OpenDatabase(filename.toStdString());
            plot->GetViewerMethods()->AddPlot(idx, "curve");
            plot->GetViewerMethods()->DrawPlots();
        }
    }
#endif
}
