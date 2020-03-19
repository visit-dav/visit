// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerMessagingUI.h>
#include <ViewerText.h>

#include <ViewerState.h>
#include <MessageAttributes.h>
#include <StatusAttributes.h>

#include <UnicodeHelper.h>

#include <QString>
#include <QMessageBox>

// ****************************************************************************
// Method: ViewerMessagingUI::ViewerMessagingUI
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep  4 00:07:57 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerMessagingUI::ViewerMessagingUI() : ViewerMessaging()
{
}

// ****************************************************************************
// Method: ViewerMessagingUI::~ViewerMessagingUI
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep  4 00:07:57 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerMessagingUI::~ViewerMessagingUI()
{
}

// ****************************************************************************
// Method: QStringFromViewerText
//
// Purpose:
//   Converts ViewerText to QString.
//
// Arguments:
//   input : The input ViewerText string.
//
// Returns:    A QString representation of the ViewerText.
//
// Note:       We try and translate the input string with tr() since the string
//             exists in its original (un-evaluated) form. After translation,
//             we use the normal QString mechanism to fill in the wildcards.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep  4 00:05:32 PDT 2014
//
// Modifications:
//
// ****************************************************************************

static QString
QStringFromViewerText(const ViewerText &input)
{
    // Attempt to translate.
    QString trans = QObject::tr(input.GetText().c_str());
    QString retval;

    // Fill in the arguments.
    const std::vector<std::string> &a = input.GetArguments();
    switch(a.size())
    {
    case 0:
        retval = trans;
        break;
    case 1:
        retval = trans.arg(a[0].c_str());
        break;
    case 2:
        retval = trans.
                 arg(a[0].c_str()).
                 arg(a[1].c_str());
        break;
    case 3:
        retval = trans.
                 arg(a[0].c_str()).
                 arg(a[1].c_str()).
                 arg(a[2].c_str());
        break;
    case 4:
        retval = trans.
                 arg(a[0].c_str()).
                 arg(a[1].c_str()).
                 arg(a[2].c_str()).
                 arg(a[3].c_str());
        break;
    case 5:
        retval = trans.
                 arg(a[0].c_str()).
                 arg(a[1].c_str()).
                 arg(a[2].c_str()).
                 arg(a[3].c_str()).
                 arg(a[4].c_str());
        break;
    case 6:
        retval = trans.
                 arg(a[0].c_str()).
                 arg(a[1].c_str()).
                 arg(a[2].c_str()).
                 arg(a[3].c_str()).
                 arg(a[4].c_str()).
                 arg(a[5].c_str());
        break;
    default:
        retval = trans;
        break;
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerMessagingUI::InformationBox
//
// Purpose:
//   Create an information box and return its result.
//
// Arguments:
//   title : The title of the dialog.
//   msg   : The message in the dialog.
//
// Returns:    The result of the yes/no choice from the dialog.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep  4 00:04:29 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
ViewerMessagingUI::InformationBox(const ViewerText &title, const ViewerText &msg)
{
    QString qtitle(QStringFromViewerText(title));
    QString qmsg(QStringFromViewerText(msg));

    BlockClientInput(true);
    bool retval = (QMessageBox::information(0, qtitle,
                      qmsg, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) ==
                   QMessageBox::Yes);
    BlockClientInput(false);

    return retval;
}

// ****************************************************************************
// Method: ViewerMessagingUI::SetMessageAttributesText
//
// Purpose:
//   Helps set the ViewerText object into the message attributes.
//
// Arguments:
//   message : The message to store.
//
// Returns:    
//
// Note:       This is virtual so we can hook in translation in derived classes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 17:04:21 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerMessagingUI::SetMessageAttributesText(const ViewerText &message)
{
    QString qmsg(QStringFromViewerText(message));
    MessageAttributes_SetText(*GetViewerState()->GetMessageAttributes(),
                              qmsg, true);
}

// ****************************************************************************
// Method: ViewerMessagingUI::SetStatusAttributesMessage
//
// Purpose:
//   Helps set the ViewerText object into the status attributes.
//
// Arguments:
//   message : The message to store.
//
// Returns:    
//
// Note:       This is virtual so we can hook in translation in derived classes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 17:04:21 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerMessagingUI::SetStatusAttributesMessage(const ViewerText &message)
{
    QString qmsg(QStringFromViewerText(message));
    StatusAttributes_SetStatusMessage(*GetViewerState()->GetStatusAttributes(),
                                      qmsg, true);
}

