// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

#include <vector>

using namespace std;

vector<EnumType*> EnumType::enums;

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
//    Mark C. Miller, Wed Aug 19 15:59:00 PDT 2009
//    Made it clear EnumType::enums at the very start.
//
//    Mark C. Miller, Wed Aug 26 11:03:19 PDT 2009
//    Added support for custom base class for derived state objects.
//
//    Kathleen Biagas, Wed Nov 30 18:59:22 PST 2016
//    Changed location for report whitespace feature, fixes problems with QT5.
//
//    Kathleen Biagas, Thu Jan  2 09:26:23 PST 2020
//    Added new argument to plugin constructor, for hasLicense.
//
// ****************************************************************************

void
XMLDocument::open(const QString &file)
{
    EnumType::enums.clear();
    filename = file;

    FieldFactory  *fieldFactory = new FieldFactory;
    XMLParser     parser(fieldFactory, file);

    ifstream test(file.toStdString().c_str(),ios::in);
    if (!test)
    {
        docType = "Plugin";
        plugin = new Plugin("","","","","","", "", false, false, false, false, false);
        attribute = new Attribute("","",QString(),"","","");
        plugin->atts = attribute;
        return;
    }
    else
    {
        test.close();
    }


    try
    {
        if (!parser.parse())
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
                plugin->atts = new Attribute(QString(),QString(),QString(),QString(),QString(),QString());
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
