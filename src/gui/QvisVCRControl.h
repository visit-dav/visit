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

#ifndef QVIS_VCR_CONTROLS_H
#define QVIS_VCR_CONTROLS_H
#include <gui_exports.h>
#include <qwidget.h>

class QPushButton;

// *******************************************************************
// Class: QvisVCRControl
//
// Purpose:
//   This is a widget that encapsulates the individual VCR buttons
//   into a new VCR controls widget.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 24 13:41:24 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Sep 1 10:15:50 PDT 2000
//   Changed the signal names and made it inherit directly from
//   the QWidget class.
//
//   Brad Whitlock, Fri Dec 1 16:31:36 PST 2000
//   Added a method to change the pixmap strings. It is used before the
//   pixmaps are created.
//
// *******************************************************************

class GUI_API QvisVCRControl : public QWidget
{
    Q_OBJECT
public:
    QvisVCRControl(QWidget * parent=0, const char * name=0);
    ~QvisVCRControl();
    void SetActiveButton(int btn);
signals:
    void prevFrame();
    void reversePlay();
    void stop();
    void play();
    void nextFrame();
private slots:
    void b0_clicked();
    void b1_clicked();
    void b2_clicked();
    void b3_clicked();
    void b4_clicked();
private:
    void AugmentPixmap(const char *xpm[]);

    int          activeButton;
    QPushButton *buttons[5];

    static char *augmentedData[];
    static char  augmentedForeground[];
};

#endif
