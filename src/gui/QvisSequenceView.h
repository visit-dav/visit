#ifndef QVIS_SEQUENCE_VIEW_H
#define QVIS_SEQUENCE_VIEW_H
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
#include <QTableView>

// ****************************************************************************
// Class: QvisSequenceView
//
// Purpose:
//   A widget for viewing sequences.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 12:33:24 PDT 2006
//
// Modifications:
//   Brad Whitlock, Fri Oct 17 14:27:20 PDT 2008
//   Total rewrite for Qt 4.
//
// ****************************************************************************

class QvisSequenceView : public QTableView
{
    Q_OBJECT
public:
    QvisSequenceView(QWidget *parent);
    virtual ~QvisSequenceView();

    void clear();

    void addViewport(const QString &);
    void addSequenceToViewport(const QString &vpt, const QString &seqName, int seqType = 0);
    void addSequenceToViewport(const QString &vpt, const QString &seqName,
                               const QPixmap &pix, int seqType = 0);
    void removeSequence(const QString &name);

    bool getSequenceInViewport(const QString &vpName, int index,
                               QString &, int &) const;
    void selectSequence(const QString &name);

signals:
    void updatedMapping(const QString &vp1,
                        const QStringList &seqList1);
    void updatedMapping(const QString &vp1,
                        const QStringList &seqList1,
                        const QString &vp2,
                        const QStringList &seqList2);
};

#endif
