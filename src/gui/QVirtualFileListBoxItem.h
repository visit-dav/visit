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

#ifndef Q_VIRTUAL_FILE_LIST_BOX_ITEM_H
#define Q_VIRTUAL_FILE_LIST_BOX_ITEM_H

#include <QFileSelectionListBoxItem.h>
#include <DebugStream.h>
#include <vectortypes.h>

// ****************************************************************************
// Class: QVirtualFileListBoxItem
//
// Purpose: 
//   This class represents a list box item that corresponds to a virtual file.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 31 11:06:59 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Dec 15 13:19:50 PST 2005
//   I shrank large virtual databases to 10 time states. The rest are omitted
//   to make displaying them work better.
//
//   Jeremy Meredith, Mon Aug 28 17:33:55 EDT 2006
//   Moved from QvisColorGridWidget.C to its own file.
//
// ****************************************************************************

class QVirtualFileListBoxItem : public QFileSelectionListBoxItem
{
public:
    static const int MAX_DISPLAYED_NAMES;

    QVirtualFileListBoxItem(QListBox *lb, const QString &name,
                            const QualifiedFilename &qf, const stringVector &n,
                            QPixmap *pm = 0) :
        QFileSelectionListBoxItem(lb, name, qf, pm), names(n)
    {
    }

    virtual ~QVirtualFileListBoxItem()
    {
    }

    virtual int height(const QListBox *lb) const
    {
        int textHeight = 0;

        if(lb)
        {
            if(names.size() > (MAX_DISPLAYED_NAMES*2))
                textHeight = ((MAX_DISPLAYED_NAMES*2) + 1 + 1 + 1) * lb->fontMetrics().lineSpacing() + 2;
            else
                textHeight = (names.size() + 1) * lb->fontMetrics().lineSpacing() + 2;
        }

        if(pixmap)
            return QMAX(textHeight, pixmap->height() + 2);
        else
            return textHeight;
    }

    virtual int width(const QListBox *lb)  const
    {
        int textWidth = 0;
        if(lb)
        {
            for(int i = 0; i < names.size(); ++i)
            {
                int w = lb->fontMetrics().width(names[i].c_str());
                textWidth = QMAX(w, textWidth);
            }
        }

        textWidth += (pixmap ? (pixmap->width() + 6) : 0);
        return textWidth;
    }

protected:
    virtual void paintSpecial(QPainter *painter)
    {
        QFileSelectionListBoxItem::paintSpecial(painter);

        // Draw the virtual filenames.
        if(fileName.IsVirtual())
        {
            QFontMetrics fm = painter->fontMetrics();
            int offset = pixmap ? (pixmap->width() + 6) : 3;
            int yIncr = fm.lineSpacing(); // + 2; //fm.ascent() + fm.leading()/2;
            int y = yIncr + fm.ascent() + fm.leading()/2;
            int i;

            if(names.size() > (MAX_DISPLAYED_NAMES*2))
            {
                QString nFilesString;
                nFilesString.sprintf("(%d total files)", names.size());
                painter->drawText(offset, y, nFilesString);
                y += yIncr;

                for(i = 0; i < MAX_DISPLAYED_NAMES; ++i)
                {
                    painter->drawText(offset, y, names[i].c_str());
                    y += yIncr;
                }

                painter->drawText(offset, y, "...");
                y += yIncr;

                for(i = names.size()-MAX_DISPLAYED_NAMES; i < names.size(); ++i)
                {
                    painter->drawText(offset, y, names[i].c_str());
                    y += yIncr;
                }
            }
            else
            {
                for(i = 0; i < names.size(); ++i)
                {
                    painter->drawText(offset, y, names[i].c_str());
                    y += yIncr;
                }
            }
        }
    }

private:
    stringVector names;
};

#endif
