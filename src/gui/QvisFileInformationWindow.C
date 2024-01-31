// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <visitstream.h>
#include <QTextEdit>
#include <QTabWidget>
#include <QPushButton>
#include <QLayout>
#include <QString>

#include <FileServerList.h>
#include <QvisFileInformationWindow.h>

#include <avtDatabaseMetaData.h>

//
// Static constants
//
const char *QvisFileInformationWindow::titleSeparator =
"==============================================================================";

// ****************************************************************************
// Method: QvisFileInformationWindow::QvisFileInformationWindow
//
// Purpose: 
//   This is the constructor for the QvisFileInformationWindow class.
//
// Arguments:
//   msgAtts : The MessageAttributes object that the window observes.
//   caption : The name of the window that appears in the window
//             decoration.
//   notepad : A pointer to the notepad area to which the window will post.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 13:56:24 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 10:47:21 PDT 2008
//   QString for caption and shortName.
//
// ****************************************************************************

QvisFileInformationWindow::QvisFileInformationWindow(FileServerList *fs,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) : 
    QvisPostableWindowObserver(fs, caption, shortName, notepad,
                               QvisPostableWindowObserver::NoExtraButtons,
                               false)
{
    // nothing here
}

// ****************************************************************************
// Method: QvisFileInformationWindow::~QvisFileInformationWindow
//
// Purpose: 
//   Destructor for the QvisFileInformationWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 13:56:24 PST 2002
//
// Modifications:
//   
// ****************************************************************************

QvisFileInformationWindow::~QvisFileInformationWindow()
{
    // nothing here
}

// ****************************************************************************
// Method: QvisFileInformationWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 13:56:24 PST 2002
//
// Modifications:
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//   Mark C. Miller, Wed May  6 09:47:52 PDT 2009
//   Made it set min 'size' instead of just width. Also, increased length of
//   titleSeparator to achieve a bit larger size.
//
//   Kathleen Biagas, Wed Apr 6, 2022
//   Fix QT_VERSION test to use Qt's QT_VERSION_CHECK.
//
// ****************************************************************************

void
QvisFileInformationWindow::CreateWindowContents()
{
    // Create a multi line edit to display the text.
    outputText = new QTextEdit(central);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int w = fontMetrics().horizontalAdvance(titleSeparator);
#else
    int w = fontMetrics().width(titleSeparator);
#endif
    outputText->setMinimumSize(w, w);
    outputText->setLineWrapMode(QTextEdit::NoWrap);
    outputText->setReadOnly(true);
    topLayout->addWidget(outputText);
}

// ****************************************************************************
// Method: QvisFileInformationWindow::UpdateWindow
//
// Purpose: 
//   Updates the output window when the FileServerList object that the
//   window watches is updated.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 20 13:56:24 PST 2002
//
// Modifications:
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interfaces to GetMetaData and GetSIL
//
//   Tom Fogal, Sat Aug 25 11:11:14 PDT 2007
//   Changed ostrstream to ostringstream
//
//   Cyrus Harrison, Mon Sep 10 10:08:31 PDT 2007
//   Added call to c_str() from ostringstream output to fix std:string
//   to QString linking error on AIX. 
//   (Also added modification message for Tom's recent change above)
//
//   Brad Whitlock, Tue Apr  8 12:26:44 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisFileInformationWindow::UpdateWindow(bool doAll)
{
    if(fileServer->FileChanged() || doAll)
    {
        const QualifiedFilename &qf = fileServer->GetOpenFile();
        const avtDatabaseMetaData *md =
            fileServer->GetMetaData(qf, GetStateForSource(qf),
                                   !FileServerList::ANY_STATE,
                                    FileServerList::GET_NEW_MD);

        if(md != 0)
        {
            std::ostringstream os;
            os << "File = " << fileServer->GetOpenFile().FullName().c_str()
               << endl;
            os << titleSeparator << endl;
            md->Print(os);
            outputText->setText(os.str().c_str());
        }
        else if(fileServer->GetOpenFile().Empty())
        {
            outputText->setText(tr("There is no open file. File information is "
                                "not available until a file has been opened."));
        }
        else
            outputText->setText(tr("VisIt could not read the file meta-data."));
    
        outputText->moveCursor(QTextCursor::Start);
    }
}
