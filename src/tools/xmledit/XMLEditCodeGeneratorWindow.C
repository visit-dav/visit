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
#include "XMLEditCodeGeneratorWindow.h"
#include <qdir.h>
#include <qfontmetrics.h>
#include <qlayout.h>
#include <qprocess.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qtimer.h>

// ****************************************************************************
// Method: XMLEditCodeGeneratorWindow::XMLEditCodeGeneratorWindow
//
// Purpose: 
//   Constructor
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:21:46 PST 2008
//
// Modifications:
//   
// ****************************************************************************

XMLEditCodeGeneratorWindow::XMLEditCodeGeneratorWindow(QWidget *parent,
    const char *name) : QMainWindow(parent, name), xmlFile()
{
    toolIndex = 0;
    currentProcess = 0;
    for(int tool = 0; tool <= ID_XML2AVT; ++tool)
        useTools[tool] = false;

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *topLayout = new QVBoxLayout(central);
    topLayout->setSpacing(10);
    topLayout->setMargin(10);

    outputText = new QTextEdit(central, "outputText");
    outputText->setWordWrap(QTextEdit::NoWrap);
    outputText->setMinimumWidth(fontMetrics().width("X") * 70);
    outputText->setMinimumHeight(fontMetrics().lineSpacing() * 25);
    topLayout->addWidget(outputText, 10);

    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);
    buttonLayout->addStretch(10);

    QPushButton *dismiss = new QPushButton("Dismiss", central, "dismiss");
    buttonLayout->addWidget(dismiss);
    connect(dismiss, SIGNAL(clicked()), this, SLOT(hide()));
}

// ****************************************************************************
// Method: XMLEditCodeGeneratorWindow::~XMLEditCodeGeneratorWindow
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:21:56 PST 2008
//
// Modifications:
//   
// ****************************************************************************

XMLEditCodeGeneratorWindow::~XMLEditCodeGeneratorWindow()
{
    cleanupProcess();
}

// ****************************************************************************
// Method: XMLEditCodeGeneratorWindow::GenerateCode
//
// Purpose: 
//   Invokes the xml tools on the specified file.
//
// Arguments:
//   xml   : The XML file.
//   tools : An array of bools indicating which tools to run.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:22:09 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditCodeGeneratorWindow::GenerateCode(const QString &xml, 
    const bool *tools)
{
    xmlFile = xml;
    toolIndex = 0;

    bool anyTools = false;
    for(int tool = 0; tool < ID_XML_MAX; ++tool)
    {
        useTools[tool] = tools[tool];
        anyTools |= useTools[tool];
    }

    if(anyTools)
    {
        outputText->clear();
        show();
        generateOne();
    }
}

//
// Qt slots
//

QString
PathToVisIt()
{
    // Not sure if we'll need to detect the path to the xmledit script...
    return QString();
}

// ****************************************************************************
// Method: XMLEditCodeGeneratorWindow::generateOne
//
// Purpose: 
//   Invokes 1 tool and sets it up so the next one will be called when this
//   tool is done.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:22:52 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditCodeGeneratorWindow::generateOne()
{
    static const char *xmlTools[] = {"xml2atts", "xml2java", "xml2python",
    "xml2window",
#if defined(WIN32)
    "xml2projectfile",
#else
    "xml2makefile",
#endif
    "xml2info", "xml2avt"};

    cleanupProcess();

    if(useTools[toolIndex])
    {
        // Create a new process.
        currentProcess = new QProcess(this, xmlTools[toolIndex]);
        QString xmlTool(PathToVisIt() + xmlTools[toolIndex]);
        currentProcess->addArgument(xmlTool);
        currentProcess->addArgument("-clobber");
#if defined(WIN32)
        if(toolIndex == ID_XML2MAKEFILE)
            currentProcess->addArgument("-version7");
#endif
        currentProcess->addArgument(xmlFile);

        // Set the process's working directory.
        QDir d(xmlFile);
        QString fullName(d.filePath(d.absPath()));
        int slash = fullName.findRev("/");
        if(slash == -1)
            slash = fullName.findRev("\\");
        if(slash != -1)
        {
            QString dirName(fullName.left(slash));
            currentProcess->setWorkingDirectory(QDir(dirName));
        }

        connect(currentProcess, SIGNAL(readyReadStdout()), 
                this, SLOT(readProcessStdout()));
        connect(currentProcess, SIGNAL(readyReadStderr()), 
                this, SLOT(readProcessStderr()));
        if(toolIndex < ID_XML2AVT)
        {
            connect(currentProcess, SIGNAL(processExited()),
                    this, SLOT(generateOne()));
        }
        else
        {
            connect(currentProcess, SIGNAL(processExited()),
                    this, SLOT(cleanupProcess()));
        }
        // Run the process.
        currentProcess->start();

        toolIndex++;
    }
    else if(toolIndex < ID_XML2AVT)
    {
        toolIndex++;

        // We're not using the tool so schedule this method to run again.
        QTimer::singleShot(300, this, SLOT(generateOne()));
    }
}

// ****************************************************************************
// Method: XMLEditCodeGeneratorWindow::readProcessStdout
//
// Purpose: 
//   Reads stdout from the process.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:23:30 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditCodeGeneratorWindow::readProcessStdout()
{
    // The process has some data to read.
    while(currentProcess->canReadLineStdout())
    {
        QString s(currentProcess->readLineStdout());
        outputText->insert(s + "\n");
        outputText->scrollToBottom();
    }
}

// ****************************************************************************
// Method: XMLEditCodeGeneratorWindow::readProcessStderr
//
// Purpose: 
//   Reads stderr from the process.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:23:30 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditCodeGeneratorWindow::readProcessStderr()
{
    // The process has some data to read.
    while(currentProcess->canReadLineStderr())
    {
        QString s(currentProcess->readLineStderr());
        outputText->insert(s + "\n");
        outputText->scrollToBottom();
    }
}

// ****************************************************************************
// Method: XMLEditCodeGeneratorWindow::cleanupProcess
//
// Purpose: 
//   Cleans up the process.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:23:58 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
XMLEditCodeGeneratorWindow::cleanupProcess()
{
    if(currentProcess != 0)
    {
        if(currentProcess->isRunning())
            currentProcess->tryTerminate();
        delete currentProcess;
        currentProcess = 0;
    }
}
