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

#ifndef VIEWER_HOSTPROFILESELECTOR_WITHWIN_H
#define VIEWER_HOSTPROFILESELECTOR_WITHWIN_H
#include <viewer_exports.h>
#include <qdialog.h>
#include <ViewerHostProfileSelector.h>

#include <string>
#include <vector>
#include <map>

class QLineEdit;
class QLabel;
class QSpinBox;
class QListBox;
class QPushButton;
class HostProfileList;

// ****************************************************************************
//  Class:  ViewerHostProfileSelectorWithWin
//
//  Purpose:
//    Selects a host profile. 
//
//  Notes:  Extracted from ViewerEngineChooser.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
//  Modifications:
//    Jeremy Meredith, Wed Oct 27 13:56:37 PDT 2004
//    Added flag so we know when we are waiting on a user already so that
//    we don't try to ask them multiple times about launching the same engine.
//
//    Jeremy Meredith, Thu Sep 15 16:42:48 PDT 2005
//    Added machine file for some mpich implementations.
//
// ****************************************************************************
class VIEWER_API ViewerHostProfileSelectorWithWin : public QDialog, 
                                                    public ViewerHostProfileSelector
{
    Q_OBJECT
  public:
             ViewerHostProfileSelectorWithWin(QWidget *parent=NULL, const char *name=NULL);
    virtual ~ViewerHostProfileSelectorWithWin();

    virtual bool SelectProfile(HostProfileList*, const std::string&, 
                               bool skip);

  public slots:
    void   newProfileSelected();

  private:
    bool       waitingOnUser;

    QListBox  *profiles;
    QLabel    *numProcsLabel;
    QSpinBox  *numProcs;
    QLabel    *numNodesLabel;
    QSpinBox  *numNodes;
    QLabel    *bankNameLabel;
    QLineEdit *bankName;
    QLabel    *timeLimitLabel;
    QLineEdit *timeLimit;
    QLabel    *machinefileLabel;
    QLineEdit *machinefile;

    QPushButton *okayButton;
    QPushButton *cancelButton;
};

#endif
