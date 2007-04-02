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

#include "XMLEdit.h"

#include <qapplication.h>
#include <qfiledialog.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>

#include <XMLEditAttribute.h>
#include <XMLEditMakefile.h>
#include <XMLEditPlugin.h>
#include <XMLEditEnums.h>
#include <XMLEditFields.h>
#include <XMLEditFunctions.h>
#include <XMLEditConstants.h>
#include <XMLEditIncludes.h>
#include <XMLEditCode.h>
#include <XMLDocument.h>

#include <visitstream.h>

// ****************************************************************************
//  Constructor: XMLEdit::XMLEdit
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
XMLEdit::XMLEdit(const QString &file, QWidget *p, const QString &n)
    : QMainWindow(p,n)
{
    setCaption("XMLEdit: "+file);

    QPopupMenu *filemenu = new QPopupMenu( this );
    menuBar()->insertItem(tr("&File"),filemenu);
    filemenu->insertItem( "&New",  this, SLOT(newdoc()),  CTRL+Key_N );
    filemenu->insertItem( "&Open",  this, SLOT(open()),  CTRL+Key_O );
    filemenu->insertItem( "&Save",  this, SLOT(save()),  CTRL+Key_S );
    filemenu->insertItem( "Save &as",  this, SLOT(saveAs()),  CTRL+Key_A );
    filemenu->insertSeparator();
    filemenu->insertItem( "E&xit", this, SLOT(close()),  CTRL+Key_X );

    tabs = new QTabWidget(this, "tabs");
    tabs->setMargin(5);

    plugintab = new XMLEditPlugin(this, "plugintab");
    tabs->insertTab(plugintab, "Plugin");

    makefiletab = new XMLEditMakefile(this, "makefiletab");
    tabs->insertTab(makefiletab, "Makefile");

    attributetab = new XMLEditAttribute(this, "attributetab");
    tabs->insertTab(attributetab, "Attribute");

    enumstab = new XMLEditEnums(this, "enumstab");
    tabs->insertTab(enumstab, "Enums");

    fieldstab = new XMLEditFields(this, "fieldstab");
    tabs->insertTab(fieldstab, "Fields");

    functionstab = new XMLEditFunctions(this, "functionstab");
    tabs->insertTab(functionstab, "Functions");

    constantstab = new XMLEditConstants(this, "constantstab");
    tabs->insertTab(constantstab, "Constants");

    includestab = new XMLEditIncludes(this, "includestab");
    tabs->insertTab(includestab, "Includes");

    codetab = new XMLEditCode(this, "codetab");
    tabs->insertTab(codetab, "Code");

    connect(tabs, SIGNAL(currentChanged(QWidget*)),
            this, SLOT(updateTab(QWidget*)));

    setCentralWidget(tabs);

    OpenFile(file);
}

void
XMLEdit::newdoc()
{
    OpenFile("untitled.xml");
}

// ****************************************************************************
//  Method:  XMLEdit::open
//
//  Purpose:
//    Menu callback: Open a new file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEdit::open()
{
    QString file = 
        QFileDialog::getOpenFileName(QString(),
                                     "XML files (*.xml)",
                                     NULL, "XMLOpen", "Open file...");
    if (file.isNull())
        return;

    OpenFile(file);
}

// ****************************************************************************
//  Method:  XMLEdit::saveAs
//
//  Purpose:
//    Menu callback: Save file with a new name.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEdit::saveAs()
{
    QString file = 
        QFileDialog::getSaveFileName(QString(),
                                     "XML files (*.xml)",
                                     NULL, "XMLSave", "Save file...");
    if (file.isNull())
        return;

    SaveFile(file);
}

// ****************************************************************************
//  Method:  XMLEdit::save
//
//  Purpose:
//    Menu callback: Save the file with its existing name, or prompt if 
//    it was still unnamed.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEdit::save()
{
    if (xmldoc->filename == "untitled.xml")
    {
        saveAs();
        return;
    }

    SaveFile(xmldoc->filename);
}

// ****************************************************************************
//  Method:  XMLEdit::OpenFile
//
//  Purpose:
//    Opens a file.
//
//  Arguments:
//    file       the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEdit::OpenFile(const QString &file)
{
    filename = file;

    xmldoc = new XMLDocument;
    xmldoc->open(filename);

    plugintab->SetDocument(xmldoc);
    makefiletab->SetDocument(xmldoc);
    attributetab->SetDocument(xmldoc);
    enumstab->SetDocument(xmldoc);
    fieldstab->SetDocument(xmldoc);
    functionstab->SetDocument(xmldoc);
    constantstab->SetDocument(xmldoc);
    includestab->SetDocument(xmldoc);
    codetab->SetDocument(xmldoc);

    plugintab->UpdateWindowContents();
    makefiletab->UpdateWindowContents();
    attributetab->UpdateWindowContents();
    enumstab->UpdateWindowContents();
    fieldstab->UpdateWindowContents();
    functionstab->UpdateWindowContents();
    constantstab->UpdateWindowContents();
    includestab->UpdateWindowContents();
    codetab->UpdateWindowContents();

    setCaption("XMLEdit: "+file);
}

// ****************************************************************************
//  Method:  XMLEdit::SaveFile
//
//  Purpose:
//    Saves a file.
//
//  Arguments:
//    file       the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEdit::SaveFile(const QString &file)
{
    xmldoc->save(file);

    setCaption("XMLEdit: "+file);
}

// ****************************************************************************
//  Method:  XMLEdit::updateTab
//
//  Purpose:
//    Update the contents of the tab when the user switches to it.
//
//  Arguments:
//    tab        the new tab being displayed
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
void
XMLEdit::updateTab(QWidget *tab)
{
    if (tab == plugintab)
        plugintab->UpdateWindowContents();
    else if (tab == makefiletab)
        makefiletab->UpdateWindowContents();
    else if (tab == attributetab)
        attributetab->UpdateWindowContents();
    else if (tab == enumstab)
        enumstab->UpdateWindowContents();
    else if (tab == fieldstab)
        fieldstab->UpdateWindowContents();
    else if (tab == functionstab)
        functionstab->UpdateWindowContents();
    else if (tab == constantstab)
        constantstab->UpdateWindowContents();
    else if (tab == includestab)
        includestab->UpdateWindowContents();
    else if (tab == codetab)
        codetab->UpdateWindowContents();
    else
        cerr << "UNKNOWN TAB IN " << __FILE__ << " LINE " << __LINE__ << endl;
}
