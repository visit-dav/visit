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

#ifndef XMLEDIT_H
#define XMLEDIT_H

#include <QMainWindow>
#include <QString>

class QTabWidget;
class XMLDocument;

class XMLEditAttribute;
class XMLEditMakefile;
class XMLEditPlugin;
class XMLEditEnums;
class XMLEditFields;
class XMLEditFunctions;
class XMLEditConstants;
class XMLEditIncludes;
class XMLEditCode;
class XMLEditCodeGeneratorWindow;

// ****************************************************************************
//  Class:  XMLEdit
//
//  Purpose:
//    Main window for the XML Editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Mar 7 15:02:37 PST 2008
//    Added code generation window.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
class XMLEdit : public QMainWindow
{
    Q_OBJECT
  public:
    XMLEdit(const QString &file, QWidget *p);
    void OpenFile(const QString &file);
    void SaveFile(const QString &file);
  public slots:
    void newdoc();
    void open();
    void save();
    void saveAs();
    void updateTab(int tab);
    void generateCode();
  private:
    QString  filename;
    QTabWidget  *tabs;
    XMLDocument *xmldoc;

    XMLEditPlugin    *plugintab;
    XMLEditMakefile  *makefiletab;
    XMLEditAttribute *attributetab;
    XMLEditEnums     *enumstab;
    XMLEditFields    *fieldstab;
    XMLEditFunctions *functionstab;
    XMLEditConstants *constantstab;
    XMLEditIncludes  *includestab;
    XMLEditCode      *codetab;

    XMLEditCodeGeneratorWindow *codeGenerationWindow;
};

#endif
