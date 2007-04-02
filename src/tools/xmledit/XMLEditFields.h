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

#ifndef XMLEDITFIELDS_H
#define XMLEDITFIELDS_H

#include <qframe.h>

struct XMLDocument;
class QButtonGroup;
class QGroupBox;
class QLineEdit;
class QNarrowLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QListBox;
class QTextEdit;
class QMultiLineEdit;
class QPushButton;

// ****************************************************************************
//  Class:  XMLEditFields
//
//  Purpose:
//    Field editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Dec 10 10:27:41 PDT 2004
//    Added controls for setting some variable name attributes.
//
//    Brad Whitlock, Wed Feb 28 18:41:56 PST 2007
//    Added public/private
//
// ****************************************************************************

class XMLEditFields : public QFrame
{
    Q_OBJECT
  public:
    XMLEditFields(QWidget *p, const QString &n);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void UpdateTypeList();
    void UpdateEnablerList();
    void nameTextChanged(const QString&);
    void labelTextChanged(const QString&);
    void typeChanged(int);
    void subtypeTextChanged(const QString&);
    void enablerChanged(int);
    void enablevalTextChanged(const QString&);
    void lengthTextChanged(const QString&);
    void internalChanged();
    void ignoreeqChanged();
    void initChanged();
    void valuesChanged();
    void fieldlistNew();
    void fieldlistDel();
    void fieldlistUp();
    void fieldlistDown();
    void variableTypeClicked(int);
    void accessChanged(int);
  private:
    XMLDocument     *xmldoc;

    QPushButton     *newButton;
    QPushButton     *delButton;

    QPushButton     *upButton;
    QPushButton     *downButton;

    QListBox        *fieldlist;
    QLineEdit       *name;
    QLineEdit       *label;
    QComboBox       *type;
    QLineEdit       *subtype;
    QComboBox       *enabler;
    QLineEdit       *enableval;
    QNarrowLineEdit *length;
    QCheckBox       *internal;
    QCheckBox       *ignoreeq;
    QButtonGroup    *access;
    QGroupBox       *variableNameGroup;
    QButtonGroup    *varNameButtons;
    QCheckBox       *init;
    QMultiLineEdit  *values;
};

#endif
