#include "XMLDocument.h"

#include <Plugin.h>
#include <Attribute.h>
#include <XMLParser.h>
#include <Field.h>
#include <qfile.h>

vector<EnumType*> EnumType::enums;

class ErrorHandler : public QXmlErrorHandler
{
    bool error(const QXmlParseException & exception)
    {
        cerr << "Error (line "<< exception.lineNumber()
             << " column " << exception.columnNumber()
             << "): " << exception.message() << endl;
        return false;
    }
    bool warning(const QXmlParseException & exception)
    {
        cerr << "Warning (line "<< exception.lineNumber()
             << " column " << exception.columnNumber()
             << "): " << exception.message() << endl;
        return false;
    }
    bool fatalError(const QXmlParseException & exception)
    {
        cerr << "Fatal error (line "<< exception.lineNumber()
             << " column " << exception.columnNumber()
             << "): " << exception.message() << endl;
        return false;
    }
    QString errorString()
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
// ****************************************************************************

void
XMLDocument::open(const QString &file)
{
    filename = file;

    FieldFactory  *fieldFactory = new FieldFactory;
    XMLParser     parser(fieldFactory, file);

    ifstream test(file.latin1(),ios::in);
    if (!test)
    {
        docType = "Plugin";
        plugin = new Plugin("","","","","","", "");
        attribute = new Attribute("","",QString(),"","");
        plugin->atts = attribute;
        return;
    }
    else
        test.close();

    try
    {
        QFile             xmlFile(file);
        QXmlInputSource   source(xmlFile);
        QXmlSimpleReader  reader;
        ErrorHandler      errorhandler;
        
        reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
        reader.setContentHandler(&parser);
        reader.setErrorHandler(&errorhandler);
        bool success = reader.parse(source);

        if (!success)
        {
            cerr << "Error parsing input file " << file << endl;
            exit(-1);
        }

        docType   = parser.docType;
        plugin    = parser.plugin;
        if (docType == "Attribute")
            attribute = parser.attribute;
        else
            attribute = plugin->atts;
    }
    catch (const char *s)
    {
        cerr << "ERROR: " << s << endl;
        exit(-1);
    }
    catch (const QString &s)
    {
        cerr << "ERROR: " << s << endl;
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
// ****************************************************************************
void
XMLDocument::save(const QString &file)
{
    filename = file;

    ofstream out(file, ios::out);
    if (!out)
        throw "Could not open xml file for saving\n";

    out << "<?xml version=\"1.0\"?>\n";
    if (docType == "Attribute")
        attribute->SaveXML(out, "");
    else
        plugin->SaveXML(out, "");

    out.close();

    if (attribute->codeFile)
    {
        attribute->codeFile->filepath = FilePath(file);
        attribute->codeFile->filename = attribute->codeFile->filebase;
        if (! attribute->codeFile->filepath.isEmpty())
            attribute->codeFile->filename = attribute->codeFile->filepath +
                                            attribute->codeFile->filename;
    }
    attribute->SaveCodeFile();
}
