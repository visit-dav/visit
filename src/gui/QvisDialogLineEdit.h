#ifndef QVIS_DIRECTORY_LINE_EDIT_H
#define QVIS_DIRECTORY_LINE_EDIT_H
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
#include <qhbox.h>
#include <gui_exports.h>

class QLineEdit;
class QPushButton;

// ****************************************************************************
// Class: QvisDialogLineEdit
//
// Purpose:
//   Incorporates a line edit with a file dialog.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 10 15:13:00 PST 2006
//
// Modifications:
//   Brad Whitlock, Fri Mar 16 15:01:10 PST 2007
//   Renamed the class and added the ChooseFont option.
//
// ****************************************************************************

class GUI_API QvisDialogLineEdit : public QHBox
{
    Q_OBJECT
public:
    typedef enum {
        ChooseFile, 
        ChooseLocalFile, 
        ChooseDirectory, 
        ChooseFont
    } DialogMode;

    QvisDialogLineEdit(QWidget *parent, const char *name);
    virtual ~QvisDialogLineEdit();

    void setText(const QString &);
    QString text();
    QString displayText();

    void setDialogFilter(const QString &);
    void setDialogMode(DialogMode m);
    void setDialogCaption(const QString &);

signals:
    void returnPressed();
    void textChanged(const QString &);
protected:
    void fontChange(const QFont &oldFont);
private slots:
    void pushButtonClicked();
private:
    DialogMode   dialogMode;
    QString      dialogFilter;
    QString      dialogCaption;
    QLineEdit   *lineEdit;
    QPushButton *pushButton;
};

#endif
