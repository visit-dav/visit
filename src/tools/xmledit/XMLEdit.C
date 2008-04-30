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
#include <XMLToolIds.h>
#include <XMLEditCodeGeneratorWindow.h>
#include <XMLEditCodeSelectionDialog.h>

#include <visitstream.h>

// ****************************************************************************
//  Constructor: XMLEdit::XMLEdit
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Mar 7 13:19:55 PST 2008
//    Added "Generate code" menu option.
//
// ****************************************************************************
XMLEdit::XMLEdit(const QString &file, QWidget *p, const QString &n)
    : QMainWindow(p,n)
{
    setCaption(tr("XMLEdit: %1").arg(file));

    QPopupMenu *filemenu = new QPopupMenu( this );
    menuBar()->insertItem(tr("&File"),filemenu);
    filemenu->insertItem( tr("&New"),  this, SLOT(newdoc()),  CTRL+Key_N );
    filemenu->insertItem( tr("&Open"),  this, SLOT(open()),  CTRL+Key_O );
    filemenu->insertItem( tr("&Save"),  this, SLOT(save()),  CTRL+Key_S );
    filemenu->insertItem( tr("Save &as"),  this, SLOT(saveAs()),  CTRL+Key_A );
    filemenu->insertSeparator();
    filemenu->insertItem( tr("&Generate code"),  this, SLOT(generateCode()),  CTRL+Key_G );
    filemenu->insertSeparator();
    filemenu->insertItem( tr("E&xit"), this, SLOT(close()),  CTRL+Key_X );

    tabs = new QTabWidget(this, "tabs");
    tabs->setMargin(5);

    plugintab = new XMLEditPlugin(this, "plugintab");
    tabs->insertTab(plugintab, tr("Plugin"));

    makefiletab = new XMLEditMakefile(this, "makefiletab");
    tabs->insertTab(makefiletab, tr("Makefile"));

    attributetab = new XMLEditAttribute(this, "attributetab");
    tabs->insertTab(attributetab, tr("Attribute"));

    enumstab = new XMLEditEnums(this, "enumstab");
    tabs->insertTab(enumstab, tr("Enums"));

    fieldstab = new XMLEditFields(this, "fieldstab");
    tabs->insertTab(fieldstab, tr("Fields"));

    functionstab = new XMLEditFunctions(this, "functionstab");
    tabs->insertTab(functionstab, tr("Functions"));

    constantstab = new XMLEditConstants(this, "constantstab");
    tabs->insertTab(constantstab, tr("Constants"));

    includestab = new XMLEditIncludes(this, "includestab");
    tabs->insertTab(includestab, tr("Includes"));

    codetab = new XMLEditCode(this, "codetab");
    tabs->insertTab(codetab, tr("Code"));

    connect(tabs, SIGNAL(currentChanged(QWidget*)),
            this, SLOT(updateTab(QWidget*)));

    setCentralWidget(tabs);
    codeGenerationWindow = 0;

    OpenFile(file);
}

void
XMLEdit::newdoc()
{
    OpenFile(tr("untitled.xml"));
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
                                     tr("XML files (*.xml)"),
                                     NULL, tr("XMLOpen"), tr("Open file..."));
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
                                     tr("XML files (*.xml)"),
                                     NULL, tr("XMLSave"), tr("Save file..."));
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
    if (xmldoc->filename == tr("untitled.xml"))
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

    setCaption(tr("XMLEdit: %1").arg(file));
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

    setCaption(tr("XMLEdit: ")+file);
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

// ****************************************************************************
// Method: XMLEdit::generateCode
//
// Purpose: 
//   Invokes the code generation window to call various XML tools on the 
//   XML file that we saved.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 14:23:15 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
XMLEdit::generateCode()
{
    // First save the file.
    bool firstGeneration = false;
    if(xmldoc->filename == tr("untitled.xml"))
    {
        saveAs();
        firstGeneration = true;
    }
    else
        SaveFile(xmldoc->filename);

    // Set up initial
    bool plugin   = (xmldoc->docType == "Plugin");
    bool useTool[ID_XML_MAX], toolEnabled[ID_XML_MAX];
    useTool[ID_XML2ATTS] = true;
    useTool[ID_XML2JAVA] = true;
    useTool[ID_XML2PYTHON] = true;
    useTool[ID_XML2WINDOW] = firstGeneration;
    useTool[ID_XML2MAKEFILE] = firstGeneration;
    useTool[ID_XML2INFO] = firstGeneration;
    useTool[ID_XML2AVT] = firstGeneration;

    toolEnabled[ID_XML2ATTS] = true;
    toolEnabled[ID_XML2JAVA] = true;
    toolEnabled[ID_XML2PYTHON] = true;
    toolEnabled[ID_XML2WINDOW] = plugin;
    toolEnabled[ID_XML2MAKEFILE] = plugin;
    toolEnabled[ID_XML2INFO] = plugin;
    toolEnabled[ID_XML2AVT] = plugin;

    // Call the dialog that lets the user pick which items to regenerate.
    if(XMLEditCodeSelectionDialog::selectTools(tr("Select items to generate"),
       useTool, toolEnabled))
    {
        if(codeGenerationWindow == 0)
            codeGenerationWindow = new XMLEditCodeGeneratorWindow(0,"codeGeneratorWindow");
        codeGenerationWindow->setCaption(tr("Generate code for %1").arg(xmldoc->filename));

        // Generate the code for the selected tools.
        codeGenerationWindow->GenerateCode(xmldoc->filename, useTool);
    }
}
