/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

