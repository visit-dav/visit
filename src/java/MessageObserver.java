// ****************************************************************************
//
// Copyright (c) 2000 - 2006, The Regents of the University of California
// Produced at the Lawrence Livermore National Laboratory
// All rights reserved.
//
// This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or materials provided with the distribution.
//  - Neither the name of the UC/LLNL nor  the names of its contributors may be
//    used to  endorse or  promote products derived from  this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
// CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
// ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ****************************************************************************

package llnl.visit;

// ****************************************************************************
// Class: MessageObserver
//
// Purpose:
//   This class is used to observe MessageAttributes in the ViewerProxy. It is
//   used to detect messages coming from the viewer and print them out if
//   desired.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:50:24 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class MessageObserver implements SimpleObserver
{
    public MessageObserver()
    {
        doUpdate = true;
        messageAtts = null;
        errorFlag = false;
        lastError = new String("");
        verbose = true;
    }

    public void Attach(MessageAttributes atts)
    {
        messageAtts = atts;
        messageAtts.Attach(this);
    }

    public void SetVerbose(boolean val)
    {
        verbose = val;
    }

    public void Update(AttributeSubject s)
    {
        if(messageAtts == null)
            return;

        if(messageAtts.GetSeverity() == MessageAttributes.SEVERITY_ERROR)
        {
            errorFlag = true;
            lastError = new String(messageAtts.GetText());
            if(verbose)
               System.out.println("VisIt: Error - " + messageAtts.GetText());
        }
        else if(verbose)
        {
            if(messageAtts.GetSeverity() == MessageAttributes.SEVERITY_WARNING)
                System.out.println("VisIt: Warning - " + messageAtts.GetText());
            else
                System.out.println("VisIt: Message - " + messageAtts.GetText());
        }
    }

    public void SetUpdate(boolean val) { doUpdate = val; }
    public boolean GetUpdate() { return doUpdate; }

    public void ClearError()
    {
        errorFlag = false;
    }

    public String  GetLastError() { return lastError; }
    public boolean GetErrorFlag() { return errorFlag; }
  
    private boolean           doUpdate;
    private MessageAttributes messageAtts;
    private boolean           errorFlag;
    private String            lastError;
    private boolean           verbose;
}
