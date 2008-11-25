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

#ifndef QVIS_LINE_STYLE_WIDGET_H
#define QVIS_LINE_STYLE_WIDGET_H
#include <gui_exports.h>
#include <QWidget>

class QComboBox;

// ****************************************************************************
// Class: QvisLineStyleWidget
//
// Purpose:
//   This widget contains a small option menu for selecting line styles.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 14:39:02 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Dec 1 16:31:36 PST 2000
//   Added a method to change the pixmap strings. It is used before the
//   pixmaps are created.
//
//   Brad Whitlock, Tue Jun  3 10:39:29 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisLineStyleWidget : public QWidget
{
    Q_OBJECT
public:
    QvisLineStyleWidget(int style_ = 0, QWidget *parent = 0);
    virtual ~QvisLineStyleWidget();
    void SetLineStyle(int style);
    int  GetLineStyle() const;
signals:
    void lineStyleChanged(int);
public slots:
    virtual void setEnabled(bool val);
private:
    void AugmentPixmap(const char *xpm[]);

    QComboBox         *lineStyleComboBox;
    static const char *style1[];
    static const char *style2[];
    static const char *style3[];
    static const char *style4[];
    static char       *augmentedData[];
    static char        augmentedForeground[];
};

#endif
