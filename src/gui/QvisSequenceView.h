#ifndef QVIS_SEQUENCE_VIEW_H
#define QVIS_SEQUENCE_VIEW_H
/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
#include <qgridview.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qpixmap.h>

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
//   
// ****************************************************************************

class QvisSequenceView : public QGridView
{
    Q_OBJECT
public:
    QvisSequenceView(QWidget *parent, const char *name = 0);
    virtual ~QvisSequenceView();

    void clear();

    void addViewport(const QString &);
    void addSequenceToViewport(const QString &vpt, const QString &seqName, int seqType = 0);
    void addSequenceToViewport(const QString &vpt, const QString &seqName,
                               const QPixmap &pix, int seqType = 0);
    void removeSequence(const QString &name);

    int getNumberOfSequencesInViewport(const QString &vpt) const;
    bool getSequenceInViewport(const QString &vpName, int index,
                               QString &, int &) const;
signals:
    void updatedMapping(const QString &vp1,
                        const QStringList &seqList1);
    void updatedMapping(const QString &vp1,
                        const QStringList &seqList1,
                        const QString &vp2,
                        const QStringList &seqList2);
protected:
    virtual void paintCell(QPainter *p, int row, int col);

    virtual void contentsMouseMoveEvent(QMouseEvent* e);
    virtual void contentsMousePressEvent(QMouseEvent* e);
    virtual void contentsMouseReleaseEvent(QMouseEvent* e);

    virtual void drawContents(QPainter *p, int clipx, int clipy, 
                              int clipw, int cliph);

private:
    bool getCellInformation(int row, int col, QString &txt, QPixmap &pix, int &t) const;

    struct QSequenceData
    {
        QString name;
        QPixmap pixmap;
        int     seqType;
    };

    struct DropData
    {
        QString name;
        int     index;
        QRect   site;
    };
    typedef QValueList<QSequenceData> QSequenceDataList;
    typedef QValueList<DropData>      DropDataList;

    typedef QMap<QString, QSequenceDataList> QStringQSequenceDataListMap;

    void drawCellContents(QPainter *p, int row, int col);
    void drawCellContentsAt(QPainter *p, const QRect &r, int row, int col);
    void drawDropSitesThatIntersect(QPainter *p, const QRect &clip);
    void drawCellContentsOverlay(QPainter *p, const QRect &r, int row, int col);

    bool getViewportName(int row, QString &name) const;
    bool getSequenceInformation(const QString &vpName, int index,
                                QSequenceData&) const;
    void insertSequenceInViewport(const QString &vpt, int index,
                                  const QSequenceData &);
    void printContents() const;

    QStringQSequenceDataListMap sequencesPerViewport;
    int                         nRowsExist;
    int                         nColsExist;

    DropDataList                dropSites;
    int                         dropSiteIndex;
    int                         srcRow, srcCol;
};

#endif
