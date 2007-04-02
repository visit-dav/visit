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

#ifndef Q_FILE_SELECTION_LIST_BOX_ITEM_H
#define Q_FILE_SELECTION_LIST_BOX_ITEM_H

#include <qapplication.h>
#include <qcheckbox.h>
#include <qevent.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qsplitter.h>
#include <qvbox.h>

#include <QualifiedFilename.h>

// ****************************************************************************
// Class: QFileSelectionListBoxItem
//
// Purpose:
//   This class represents a list box item that gets inserted into the file
//   list.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 12:49:33 PDT 2003
//
// Modifications:
//    Jeremy Meredith, Mon Aug 28 17:33:55 EDT 2006
//    Moved from QvisColorGridWidget.C to its own file.
//
// ****************************************************************************

class QFileSelectionListBoxItem : public QListBoxItem
{
public:
    QFileSelectionListBoxItem(QListBox *listbox, const QString &name,
                              const QualifiedFilename &qf, QPixmap *pm = 0) :
        QListBoxItem(listbox), fileName(qf)
    {
        setText(name);
        pixmap = pm;
    }

    virtual ~QFileSelectionListBoxItem()
    {
    }

    virtual int height(const QListBox *lb) const
    {
        int textHeight = lb ? lb->fontMetrics().lineSpacing() + 2 : 0;
        if(pixmap)
            return QMAX(textHeight, pixmap->height() + 2);
        else
            return textHeight;
    }

    virtual int width(const QListBox *lb)  const
    {
        int textWidth = lb ? lb->fontMetrics().width(text()) + 6 : 0;
        textWidth += (pixmap ? (pixmap->width() + 6) : 0);
        return textWidth;
    }

    const QualifiedFilename &GetFilename() const
    {
        return fileName;
    }

protected:
    virtual void paintSpecial(QPainter *painter)
    {
    }

    virtual void paint(QPainter *painter)
    {
        QPen backupPen(painter->pen());
        QFontMetrics fm = painter->fontMetrics();

        // If we can't access the file, make it gray.
        if(!fileName.CanAccess())
            painter->setPen(Qt::gray);
        else if(listBox())
        {
            if(selected())
                painter->setPen(listBox()->colorGroup().highlightedText());
            else
                painter->setPen(listBox()->colorGroup().text());
        }
        else
            painter->setPen(Qt::black);

        // If the file is virtual, draw a database pixmap.
        int offset = 3;
        if(pixmap)
        {
            offset += (pixmap->width() + 3);
            painter->drawPixmap(3, 0, *pixmap);
        }

        // Draw the text.
        painter->drawText(offset, fm.ascent() + fm.leading()/2, text() );

        // Draw anything special.
        paintSpecial(painter);

        // Restore the old pen.
        painter->setPen(backupPen);
    }

protected:
    QualifiedFilename  fileName;
    QPixmap           *pixmap;
};

#endif
