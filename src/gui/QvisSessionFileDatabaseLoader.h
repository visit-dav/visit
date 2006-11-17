/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef QVIS_SESSION_FILE_DATABASE_LOADER_H
#define QVIS_SESSION_FILE_DATABASE_LOADER_H
#include <qobject.h>
#include <GUIBase.h>
#include <vectortypes.h>

class CancelledObserver;

// ****************************************************************************
// Class: QvisSessionFileDatabaseLoader
//
// Purpose:
//   Helps load session file databases before we tell the viewer to actually
//   load a session file. This class listens for mdserver cancelations, allowing
//   the mdserver cancel button to cancel session loading when done from the
//   GUI.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 14:08:04 PST 2005
//
// Modifications:
//   Brad Whitlock, Tue Nov 14 15:20:22 PST 2006
//   Added another complete signal.
//
// ****************************************************************************

class QvisSessionFileDatabaseLoader : public QObject, public GUIBase
{
    Q_OBJECT
public:
    QvisSessionFileDatabaseLoader(QObject *parent, const char *name = 0);
    virtual ~QvisSessionFileDatabaseLoader();

    void SetDatabases(const stringVector &s);
    void Start(const QString &sfn);
signals:
    void complete(const QString &);
    void complete(const QString &, const stringVector &);
    void loadFile(const QString &);
protected slots:
    void ProcessFile();

protected:
    int                 index;
    stringVector        databases;
    CancelledObserver  *cancelledObserver;
    QString             sessionFile;
};

#endif
