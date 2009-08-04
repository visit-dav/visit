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

#include "XMLEditStd.h"
#include "XMLDocument.h"

#include <Plugin.h>
#include <Attribute.h>
#include <XMLParser.h>
#include <Field.h>
#include <qfile.h>
#include <qmessagebox.h>

#if !defined(_WIN32)
#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <iostream>
#include <fstream>

using namespace std;

vector<EnumType*> EnumType::enums;

class ErrorHandler : public QXmlErrorHandler
{
    bool error(const QXmlParseException & exception)
    {
        cErr << "Error (line "<< exception.lineNumber()
             << " column " << exception.columnNumber()
             << "): " << exception.message() << Endl;
        return false;
    }
    bool warning(const QXmlParseException & exception)
    {
        cErr << "Warning (line "<< exception.lineNumber()
             << " column " << exception.columnNumber()
             << "): " << exception.message() << Endl;
        return false;
    }
    bool fatalError(const QXmlParseException & exception)
    {
        cErr << "Fatal error (line "<< exception.lineNumber()
             << " column " << exception.columnNumber()
             << "): " << exception.message() << Endl;
        return false;
    }
    QString errorString() const
    {
        return "No error string defined....";
    }
};

// ****************************************************************************
//  Method:  XMLDocument::open
//
//  Purpose:
//    Open an existing document, or create a new one if the file does
//    not exist.
//
//  Arguments:
//    file       the file name
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Nov 18 14:11:21 PST 2002
//    I renamed Parser to XMLParser to make it build on Windows.
//
//    Brad Whitlock, Thu Mar 13 12:58:03 PDT 2003
//    I added another argument to the Plugin constructor.
//
//    Hank Childs, Sat Sep 13 08:27:57 PDT 2003
//    Added another argument to the constructor.
//
//    Jeremy Meredith, Mon Sep 22 17:19:11 PDT 2003
//    Added check for writable xml/codefile files.
//    Create an Attribute if one was not in the loaded file.
//    Added a lot of error checks before writing a file, for example
//    for required fields that were not filled in.
//    Catch errors writing files and display the messages to the user.
//
//    Jeremy Meredith, Wed Aug 25 11:57:08 PDT 2004
//    Added the "no-engine" and "engine-only" options to the plugin
//    constructor.
//
//    Hank Childs, Tue May 24 10:19:40 PDT 2005
//    Added argument for hasoptions.
//
//    Brad Whitlock, Thu Mar 6 14:48:36 PST 2008
//    Adapted to updated CodeFile implementation.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************

void
XMLDocument::open(const QString &file)
{
    filename = file;

    FieldFactory  *fieldFactory = new FieldFactory;
    XMLParser     parser(fieldFactory, file);

    ifstream test(file.toStdString().c_str(),ios::in);
    if (!test)
    {
        docType = "Plugin";
        plugin = new Plugin("","","","","","", "", false, false, false, false);
        attribute = new Attribute("","",QString(),"","");
        plugin->atts = attribute;
        return;
    }
    else
    {
        test.close();
    }


    try
    {
        QFile             xmlFile(file);
        QXmlInputSource   source(&xmlFile);
        QXmlSimpleReader  reader;
        ErrorHandler      errorhandler;
        
        reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
        reader.setContentHandler(&parser);
        reader.setErrorHandler(&errorhandler);
        bool success = reader.parse(source);

        if (!success)
        {
            cerr << "Error parsing input file " << file.toStdString() << endl;
            exit(-1);
        }

        docType   = parser.docType;
        plugin    = parser.plugin;
        if (docType == "Attribute")
            attribute = parser.attribute;
        else
        {
            if (!plugin->atts)
            {
                plugin->atts = new Attribute(QString(),QString(),QString(),QString(),QString());
            }
            attribute = plugin->atts;
        }

#if !defined(_WIN32)
        struct stat s;
        stat(file.toStdString().c_str(), &s);
        if (!(s.st_mode & S_IWUSR))
        {
            QMessageBox::warning(0,"Warning","File is not writable.");
        }
        if (attribute && attribute->codeFile)
        {
            stat(attribute->codeFile->FileName().toStdString().c_str(), &s);
            if (!(s.st_mode & S_IWUSR))
            {
                QMessageBox::warning(0,"Warning","Code file is not writable.");
            }
        }
#endif

    }
    catch (const char *s)
    {
        cerr << "ERROR: " << s << endl;
        exit(-1);
    }
    catch (const QString &s)
    {
        cerr << "ERROR: " << s.toStdString() << endl;
        exit(-1);
    }
}

// ****************************************************************************
//  Method:  XMLDocument::save
//
//  Purpose:
//    Save the existing file and the associated codefile.
//
//  Arguments:
//    file       the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 14:50:21 PST 2008
//    Adapted to newer CodeFile implementation.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
void
XMLDocument::save(const QString &file)
{
    filename = file;

    //
    // Check for various errors before allowing a save
    //

    // Note: we're allowing unnamed attributes for databases right now
    // since their attributes are ignored.
    if (docType == "Attribute" || plugin->type != "database")
    {
        if (attribute->name.isEmpty())
        {
            QMessageBox::warning(0,"Error","You must name your attributes!");
            return;
        }
    }

    if (docType == "Plugin")
    {
        if (plugin->name.isEmpty())
        {
            QMessageBox::warning(0,"Error","You must name your plugin!");
            return;
        }
        if (plugin->type.isEmpty())
        {
            QMessageBox::warning(0,"Error","You must specify a type for your plugin!");
        }
    }

    if (attribute)
    {
        for (size_t i=0; i<attribute->fields.size(); i++)
        {
            if (attribute->fields[i]->enabler &&
                attribute->fields[i]->enableval.empty())
            {
                
                QMessageBox::warning(0,"Error",
                    QString("The enabler for field %1 must have at least one value.").arg(attribute->fields[i]->name));
                return;
            }
        }
    }

    if (attribute && !attribute->codeFile)
    {
        if (!attribute->functions.empty())
        {
            QMessageBox::warning(0,"Error","You have created functions "
                                 "but have not specified a code file!");
            return;
        }
        if (!attribute->constants.empty())
        {
            QMessageBox::warning(0,"Error","You have created constants "
                                 "but have not specified a code file!");
            return;
        }
        if (!attribute->includes.empty())
        {
            QMessageBox::warning(0,"Error","You have specified includes "
                                 "but have not specified a code file!");
            return;
        }
        if (!attribute->codes.empty())
        {
            QMessageBox::warning(0,"Error","You have created code pieces "
                                 "but have not specified a code file!");
            return;
        }
    }

    
    QFile outfile(file);

    if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(0,"Error","Could not open xml file for saving!");
        return;
    }
    
    QTextStream out(&outfile);

    out << "<?xml version=\"1.0\"?>\n";
    if (docType == "Attribute")
        attribute->SaveXML(out, "");
    else
        plugin->SaveXML(out, "");

    outfile.close();

    if (attribute->codeFile)
    {
        attribute->codeFile->SetFilePath(FilePath(file));
        attribute->codeFile->SetFileName(attribute->codeFile->FileBase());
        if (! attribute->codeFile->FilePath().isEmpty())
            attribute->codeFile->SetFileName(attribute->codeFile->FilePath() +
                                            attribute->codeFile->FileName());
    }

    try {
        attribute->SaveCodeFile();
    }
    catch (const char *s)
    {
        QMessageBox::warning(0,"Error",s);
        return;
    }
    catch (const QString &s)
    {
        QMessageBox::warning(0,"Error",s);
        return;
    }
}
