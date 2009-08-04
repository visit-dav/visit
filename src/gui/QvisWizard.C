/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <QvisWizard.h>
#include <AttributeSubject.h>

// ****************************************************************************
// Method: QvisWizard::QvisWizard
//
// Purpose: 
//   Constructor for the QvisWizard class.
//
// Arguments:
//   subj   : The subject that the wizard will use to send its data to
//            the viewer.
//   parent : The wizard's parent widget.
//   name   : The name of the wizard.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:32:56 PDT 2004
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 09:20:47 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

QvisWizard::QvisWizard(AttributeSubject *subj, QWidget *parent) 
: QWizard(parent)
{
    atts = subj;
    localCopy = atts->NewInstance(true);
}

// ****************************************************************************
// Method: QvisWizard::~QvisWizard
//
// Purpose: 
//   Destructor for the QvisWizard class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:35:31 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisWizard::~QvisWizard()
{
    atts = 0;
    delete localCopy;
}

// ****************************************************************************
// Method: QvisWizard::SendAttributes
//
// Purpose: 
//   Tells the wizard to send its attributes to the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:35:51 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisWizard::SendAttributes()
{
    if(atts->CopyAttributes(localCopy))
    {
        atts->SelectAll();
        atts->Notify();
    }
}

// ****************************************************************************
// Method: QvisWizard::UpdateAttributes
//
// Purpose: 
//   Writes over the wizard's local atts with the original atts.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 22 11:44:22 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
QvisWizard::UpdateAttributes()
{
    return localCopy->CopyAttributes(atts);
}
