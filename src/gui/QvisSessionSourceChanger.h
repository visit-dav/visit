#ifndef QVIS_SESSION_SOURCE_CHANGER_H
#define QVIS_SESSION_SOURCE_CHANGER_H
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
#include <QWidget>
#include <vectortypes.h>
#include <map>

class QListWidget;
class QvisDialogLineEdit;

// ****************************************************************************
// Class: QvisSessionSourceChanger
//
// Purpose:
//   This is a widget that allows the user to alter the sources associated
//   with source ids.
//
// Notes:      We use this class in the movie wizard and in session file
//             restoration so we can allow the user to use different
//             sources with restored sessions.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 10:57:19 PDT 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

class QvisSessionSourceChanger : public QWidget
{
    Q_OBJECT
public:
    QvisSessionSourceChanger(QWidget *parent);
    virtual ~QvisSessionSourceChanger();

    void setSources(const stringVector &keys, const stringVector &values,
                    const std::map<std::string, stringVector> &uses);
    const stringVector &getSources() const;

private slots:
    void selectedSourceChanged();
    void sourceChanged();
    void sourceChanged(const QString &);
private:
    void updateControls(int ci);

    QListWidget        *sourceList;
    QvisDialogLineEdit *fileLineEdit;
    QListWidget        *useList;

    stringVector sourceIds;
    stringVector sources;
    std::map<std::string, stringVector> sourceUses;
};

#endif
