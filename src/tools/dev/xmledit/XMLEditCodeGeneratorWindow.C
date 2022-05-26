// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "XMLEditCodeGeneratorWindow.h"
#include <QDir>
#include <QFontMetrics>
#include <QLayout>
#include <QProcess>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>

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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
//    Kathleen Biagas, Wed Apr 6, 2022
//    Fix QT_VERSION test to use Qt's QT_VERSION_CHECK.
//
// ****************************************************************************

XMLEditCodeGeneratorWindow::XMLEditCodeGeneratorWindow(QWidget *parent)
: QMainWindow(parent), xmlFile()
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

    outputText = new QTextEdit(central);
    outputText->setWordWrapMode(QTextOption::NoWrap);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    outputText->setMinimumWidth(fontMetrics().horizontalAdvance("X") * 70);
#else
    outputText->setMinimumWidth(fontMetrics().width("X") * 70);
#endif
    outputText->setMinimumHeight(fontMetrics().lineSpacing() * 25);
    topLayout->addWidget(outputText, 10);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(10);

    QPushButton *dismiss = new QPushButton(tr("Dismiss"), central);
    buttonLayout->addWidget(dismiss);
    topLayout->addLayout(buttonLayout);

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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 2008
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

void
XMLEditCodeGeneratorWindow::generateOne()
{
    static const char *xmlTools[] = {"xml2atts", "xml2java", "xml2python",
       "xml2window", "xml2cmake", "xml2info", "xml2avt"};

    cleanupProcess();

    if(useTools[toolIndex])
    {
        // Create a new process.
        currentProcess = new QProcess(this);
        QString xmlTool(PathToVisIt() + xmlTools[toolIndex]);

        QStringList arguments;
        arguments << "-clobber";

        arguments << xmlFile;

        // Set the process's working directory.
        QDir d(xmlFile);
        QString fullName(d.filePath(d.absolutePath()));
        int slash = fullName.lastIndexOf("/");
        if(slash == -1)
            slash = fullName.lastIndexOf("\\");
        if(slash != -1)
        {
            QString dirName(fullName.left(slash));
            currentProcess->setWorkingDirectory(dirName);
        }

        connect(currentProcess, SIGNAL(readyReadStandardOutput()),
                this, SLOT(readProcessStdout()));
        connect(currentProcess, SIGNAL(readyReadStandardError()),
                this, SLOT(readProcessStderr()));
        if(toolIndex < ID_XML2AVT)
        {
            connect(currentProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
                    this, SLOT(generateOne()));
        }
        else
        {
            connect(currentProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
                    this, SLOT(cleanupProcess()));
        }
        // Run the process.
        currentProcess->start(xmlTool,arguments);

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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

void
XMLEditCodeGeneratorWindow::readProcessStdout()
{
    // The process has some data to read.
    // set the channel to standard out, and read
    currentProcess->setReadChannel(QProcess::StandardOutput);
    QByteArray res = currentProcess->readAll();
    outputText->append(QString(res));
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

void
XMLEditCodeGeneratorWindow::readProcessStderr()
{
    // The process has some data to read.
    // set the channel to standard err, and read
    currentProcess->setReadChannel(QProcess::StandardError);
    QByteArray res = currentProcess->readAll();
    outputText->append(QString(res));
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
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

void
XMLEditCodeGeneratorWindow::cleanupProcess()
{
    if(currentProcess != 0)
    {
        if(currentProcess->state() == QProcess::Running)
            currentProcess->terminate();
        delete currentProcess;
        currentProcess = 0;
    }
}
