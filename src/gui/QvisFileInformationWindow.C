#if defined(_WIN32)
#include <strstrea.h>
#else

#if defined(GCC3_3_X86_64) || defined(__APPLE__)
#include <strstream>
using std::ostrstream;
#else
#include <strstream.h>
#endif

#endif
#include <qmultilineedit.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qstring.h>

#include <FileServerList.h>
#include <QvisFileInformationWindow.h>

#include <avtDatabaseMetaData.h>

//
// Static constants
//
const char *QvisFileInformationWindow::titleSeparator =
"================================================";

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
//
// ****************************************************************************

QvisFileInformationWindow::QvisFileInformationWindow(FileServerList *fs,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) : 
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
//
// ****************************************************************************

void
QvisFileInformationWindow::CreateWindowContents()
{
    // Create a multi line edit to display the text.
    outputText = new QMultiLineEdit(central, "outputText");
    outputText->setMinimumWidth(fontMetrics().width(titleSeparator));
    outputText->setWordWrap(QMultiLineEdit::NoWrap);
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
// ****************************************************************************

void
QvisFileInformationWindow::UpdateWindow(bool doAll)
{
    if(fileServer->FileChanged() || doAll)
    {
        const avtDatabaseMetaData *md = fileServer->GetMetaData();

        if(md != 0)
        {

            // get MetaData directly from server if its not invariant
            if (md->GetMustRepopulateOnStateChange())
            {
                md = fileServer->GetMetaDataFromMDServer(
                                     fileServer->GetOpenFile(),
                                     fileServer->GetOpenFileTimeState());
            }

            ostrstream os;
            os << "File = " << fileServer->GetOpenFile().FullName().c_str()
               << endl;
            os << titleSeparator << endl;
            md->Print(os);
            outputText->setText(os.str());
        }
        else if(fileServer->GetOpenFile().Empty())
        {
            outputText->setText("There is no open file. File information is "
                                "not available until a file has been opened.");
        }
        else
            outputText->setText("VisIt could not read the file meta-data.");
        outputText->setCursorPosition(0, 0);
    }
}
