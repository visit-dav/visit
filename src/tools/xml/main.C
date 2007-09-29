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

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Jun  8 16:45:00 PDT 2006
//    Add copyright string.
//
// ****************************************************************************

#include <qxml.h>
#include <visitstream.h>
#include "Field.h"
#include "Attribute.h"
#include "Enum.h"
#include "Plugin.h"

vector<EnumType*> EnumType::enums;

bool print   = true;
bool clobber = false;
bool installpublic  = false;
bool installprivate = false;
bool outputtoinputdir = false;
QString currentInputDir = "";

std::string copyright_str = 
"/*****************************************************************************\n"
"*\n"
"* Copyright (c) 2000 - 2007, The Regents of the University of California\n"
"* Produced at the Lawrence Livermore National Laboratory\n"
"* All rights reserved.\n"
"*\n"
"* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The\n"
"* full copyright notice is contained in the file COPYRIGHT located at the root\n"
"* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.\n"
"*\n"
"* Redistribution  and  use  in  source  and  binary  forms,  with  or  without\n"
"* modification, are permitted provided that the following conditions are met:\n"
"*\n"
"*  - Redistributions of  source code must  retain the above  copyright notice,\n"
"*    this list of conditions and the disclaimer below.\n"
"*  - Redistributions in binary form must reproduce the above copyright notice,\n"
"*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the\n"
"*    documentation and/or materials provided with the distribution.\n"
"*  - Neither the name of the UC/LLNL nor  the names of its contributors may be\n"
"*    used to  endorse or  promote products derived from  this software without\n"
"*    specific prior written permission.\n"
"*\n"
"* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
"* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE\n"
"* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE\n"
"* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF\n"
"* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR\n"
"* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL\n"
"* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR\n"
"* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER\n"
"* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT\n"
"* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY\n"
"* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH\n"
"* DAMAGE.\n"
"*\n"
"*****************************************************************************/\n";

std::string java_copyright_str = 
"// ***************************************************************************\n"
"//\n"
"// Copyright (c) 2000 - 2007, The Regents of the University of California\n"
"// Produced at the Lawrence Livermore National Laboratory\n"
"// All rights reserved.\n"
"//\n"
"// This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The\n"
"// full copyright notice is contained in the file COPYRIGHT located at the root\n"
"// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.\n"
"//\n"
"// Redistribution  and  use  in  source  and  binary  forms,  with  or  without\n"
"// modification, are permitted provided that the following conditions are met:\n"
"//\n"
"//  - Redistributions of  source code must  retain the above  copyright notice,\n"
"//    this list of conditions and the disclaimer below.\n"
"//  - Redistributions in binary form must reproduce the above copyright notice,\n"
"//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the\n"
"//    documentation and/or materials provided with the distribution.\n"
"//  - Neither the name of the UC/LLNL nor  the names of its contributors may be\n"
"//    used to  endorse or  promote products derived from  this software without\n"
"//    specific prior written permission.\n"
"//\n"
"// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
"// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE\n"
"// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE\n"
"// ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF\n"
"// CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR\n"
"// ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL\n"
"// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR\n"
"// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER\n"
"// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT\n"
"// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY\n"
"// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH\n"
"// DAMAGE.\n"
"//\n"
"// ***************************************************************************\n";

#ifdef TEST_ONLY
#endif
#ifdef GENERATE_ATTS
#include "GenerateAtts.h"
#endif
#ifdef GENERATE_WINDOW
#include "GenerateWindow.h"
#endif
#ifdef GENERATE_INFO
#include "GenerateInfo.h"
#endif
#ifdef GENERATE_MAKEFILE
#include "GenerateMakefile.h"
#endif
#ifdef GENERATE_AVT
#include "GenerateAVT.h"
#endif
#ifdef GENERATE_PYTHON
#include "GeneratePython.h"
#endif
#ifdef GENERATE_JAVA
#include "GenerateJava.h"
#endif
#ifdef GENERATE_PROJECTFILE
#include "GenerateProjectFile.h"
bool generateVersion7Projects = true;
#endif

#include "XMLParser.h"

void
PrintUsage(const char *prog)
{
    cerr << "usage: "<<prog<<" [options] <file.xml>" << endl;
    cerr << "    options:" << endl;
    cerr << "        -clobber       overwrite old files if possible" << endl;
    cerr << "        -noprint       no debug output" << endl;
    cerr << "        -public        (xml2makefile only) install publicly" << endl;
    cerr << "        -private       (xml2makefile only) install privately" << endl;
    cerr << "        -version6      (xml2projectfile only) make MSVC 6.0 projects" << endl;
    cerr << "        -version7      (xml2projectfile only) make MSVC .Net 2003 projects (default)" << endl;
}

class ErrorHandler : public QXmlErrorHandler
{
    bool error(const QXmlParseException & exception)
    {
        cerr << "Error: " << exception.message() << endl;
        cerr << "Line: "   << exception.lineNumber() << endl;
        cerr << "Column: " << exception.columnNumber() << endl;
        return true;
    }
    bool warning(const QXmlParseException & exception)
    {
        cerr << "Warning: " << exception.message() << endl;
        cerr << "Line: "   << exception.lineNumber() << endl;
        cerr << "Column: " << exception.columnNumber() << endl;
        return true;
    }
    bool fatalError(const QXmlParseException & exception)
    {
        cerr << "Fatal error: " << exception.message() << endl;
        cerr << "Line: "   << exception.lineNumber() << endl;
        cerr << "Column: " << exception.columnNumber() << endl;
        return true;
    }
    QString errorString()
    {
        return "No error string defined....";
    }
};

// ****************************************************************************
//  Function:  Open
//
//  Purpose:
//    Open a file, checking for errors.
//
//  Arguments:
//    file           the ofstream to use for opening the file
//    name_nopath    the filename, without path, of the file to open
//
//  Modifications:
//    Jeremy Meredith, Wed Jun  6 12:37:34 EDT 2007
//    Added support for outputing files to the input directory.
//
// ****************************************************************************

bool
Open(ofstream &file, const QString &name_withoutpath)
{
    QString name;
    if (outputtoinputdir)
        name = currentInputDir + name_withoutpath;
    else
        name = name_withoutpath;

    bool alreadyexists = false;
    if (clobber)
        file.open(name.latin1(), ios::out);
    else
    {
        ifstream testopen(name.latin1(), ios::in);
        if (!testopen)
        {
            file.open(name.latin1(), ios::out);
        }
        else
        {
            testopen.close();
            alreadyexists = true;
        }
    }
    if (alreadyexists || !file)
    {
        cerr << "Warning: Could not create file '"<<name<<"' for writing." << endl;
        if (!clobber)
        {
            cerr << "Info: If you wish to overwrite file '"<<name<<"'," << endl;
            cerr << "Info: you might want to give the -clobber flag." << endl;
        }
    }
    return bool(file);
}

void ProcessFile(QString file);

// ****************************************************************************
//  Function:  main
//
//  Purpose:
//    Parse an xml file and generate code if asked.
//
//  Arguments:
//    <file.xml> :     File name
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 13:25:08 PDT 2001
//    Added Common plugin info.
//
//    Jeremy Meredith, Wed Feb  6 16:53:39 PST 2002
//    Added code so it will not clobber existing files unless the 
//    "-clobber" flag is given, and so it will not print if
//    the "-noprint" option is given.
//
//    Brad Whitlock, Thu Feb 21 16:40:54 PST 2002
//    Added some more top level try/catch code.
//
//    Jeremy Meredith, Fri Apr 19 15:35:59 PDT 2002
//    Pulled printing code to a common place.  Added check to make sure
//    a file exists before trying to parse it.
//
//    Jeremy Meredith, Mon Jul 15 01:22:09 PDT 2002
//    Big enhancements so all our plugin makefile can be build automatically.
//
//    Jeremy Meredith, Fri Jul 19 17:35:32 PDT 2002
//    Fixed an if test that was missing braces.
//
//    Brad Whitlock, Thu Aug 8 17:54:21 PST 2002
//    I added conditionally compiled code to generate Java source code.
//
//    Jeremy Meredith, Tue Aug 27 14:32:40 PDT 2002
//    Added support for database plugins.
//
//    Brad Whitlock, Mon Nov 18 14:07:54 PST 2002
//    I renamed the Parser class to XMLParser to make it build on Windows. I
//    also changed some exception handling so giving non-plugin files to
//    xml2info and xml2avt does not crash.
//
//    Jeremy Meredith, Tue Sep 23 17:02:38 PDT 2003
//    Moved most of the functionality here into a separate function, and
//    allow for multiple XML files on the command line.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added avt files for databases.
//
//    Brad Whitlock, Tue Dec 16 11:06:47 PDT 2003
//    Added code to generate Windows project files.
//
//    Jeremy Meredith, Fri Mar 19 15:17:31 PST 2004
//    Added code to clear enums in between input files.
//
//    Jeremy Meredith, Tue Mar 30 10:16:00 PST 2004
//    I added support for database plugins with only a mdserver or engine
//    component.  This was critical for simulation support.
//
//    Brad Whitlock, Mon Aug 16 16:50:29 PST 2004
//    I added the -version7 flag.
//
//    Jeremy Meredith, Wed Aug 11 14:56:02 PDT 2004
//    I made the mdserver/engine only support be specified by the xml file.
//
//    Jeremy Meredith, Wed Aug 25 11:51:33 PDT 2004
//    Moved the engine-only concept into the XML file, not a main argument.
//
//    Jeremy Meredith, Tue Feb  8 08:54:08 PST 2005
//    Added a new file generated by xml2makefile.  This contains a reference
//    to the version of visit used to generate the plugin.  Really it only
//    uses visit-config.h, but xml2makefile ensures it picks up the
//    visit-config.h from the same version as the plugin tools used.
//
//    Brad Whitlock, Thu Mar 3 09:01:29 PDT 2005
//    I removed the code to write a version file since the version is now
//    stored in the common plugin info to make it easier for us on Windows.
//
//    Hank Childs, Tue May 24 09:41:53 PDT 2005
//    Added hasoptions.
//
//    Jeremy Meredith, Wed Jun  6 12:36:30 EDT 2007
//    Added -outputtoinputdir.
//    Check for any nonexistent files before starting the processing.
//
// ****************************************************************************

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        PrintUsage(argv[0]);
        exit(1);
    }

    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-noprint") == 0)
        {
            print = false;
            argc--;
            for (int j=i; j<argc; j++)
                argv[j] = argv[j+1];
            i--;
        }
        else if (strcmp(argv[i], "-clobber") == 0)
        {
            clobber = true;
            argc--;
            for (int j=i; j<argc; j++)
                argv[j] = argv[j+1];
            i--;
        }
        else if (strcmp(argv[i], "-public") == 0)
        {
            installpublic = true;
            argc--;
            for (int j=i; j<argc; j++)
                argv[j] = argv[j+1];
            i--;
        }
        else if (strcmp(argv[i], "-private") == 0)
        {
            installprivate = true;
            argc--;
            for (int j=i; j<argc; j++)
                argv[j] = argv[j+1];
            i--;
        }
#ifdef GENERATE_PROJECTFILE
        else if (strcmp(argv[i], "-version6") == 0)
        {
            generateVersion7Projects = false;
            argc--;
            for (int j=i; j<argc; j++)
                argv[j] = argv[j+1];
            i--;
        }
        else if (strcmp(argv[i], "-version7") == 0)
        {
            generateVersion7Projects = true;
            argc--;
            for (int j=i; j<argc; j++)
                argv[j] = argv[j+1];
            i--;
        }
#endif
        else if (strcmp(argv[i], "-outputtoinputdir") == 0)
        {
            outputtoinputdir = true;
            argc--;
            for (int j=i; j<argc; j++)
                argv[j] = argv[j+1];
            i--;
        }
    }

    if (installpublic && installprivate)
    {
        cerr << "Cannot specify -public and -private at the same time!\n";
        PrintUsage(argv[0]);
        exit(1);
    }

    for (int f = 1 ; f < argc ; f++)
    {
        ifstream testfile(argv[f],ios::in);
        if (!testfile)
        {
            cerr << "File '"<<argv[1]<<"' doesn't exist!\n";
            exit(1);
        }
        testfile.close();
    }

    for (int f = 1 ; f < argc ; f++)
    {
        ProcessFile(argv[f]);
    }

    return 0;
}

// ****************************************************************************
//
//    Kathleen Bonnell, Thu Jul  5 17:58:22 PDT 2007 
//    For GeneratePython, if plugin, call plugin->WriteHeader instead of
//    attribute->WriteHeader.  There is a flag that needs to be turned off
//    for plugins.  The plugin->WriteHeader will turn off the flag then call
//    the WriteHeader for the atts.
//    
//
// ****************************************************************************
void
ProcessFile(QString file)
{
    QString    docType;
    Plugin    *plugin    = NULL;
    Attribute *attribute = NULL;

    EnumType::enums.clear();

    currentInputDir = "";
    int lastslash = file.findRev("/");
    if (lastslash < 0)
        lastslash = file.findRev("\\");
    if (lastslash >= 0)
        currentInputDir = file.left(lastslash+1);

    FieldFactory  *fieldFactory = new FieldFactory;
    XMLParser     parser(fieldFactory, file);
    try
    {
        QFile             xmlFile(file);
        QXmlInputSource   source(xmlFile);
        QXmlSimpleReader  reader;
        ErrorHandler      errorhandler;
        
        reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
        reader.setContentHandler(&parser);
        reader.setErrorHandler(&errorhandler);
        reader.parse(source);

        docType   = parser.docType;
        plugin    = parser.plugin;

        if (docType == "Attribute")
            attribute = parser.attribute;
        else if(plugin != NULL)
            attribute = plugin->atts;

#ifndef GENERATE_MAKEFILE
        if(attribute == NULL && plugin->type != "database")
            throw "Cannot generate code for this XML file.";
#endif
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

    if (docType.isNull())
    {
        cerr << "Error in parsing " << file << endl;
        exit(-1);
    }

    if (print)
    {
        cerr << "-----------------------------------------------------------------" << endl;
        cerr << "               Parsed document of type " << docType << endl;
        cerr << "-----------------------------------------------------------------" << endl;
        cerr << endl;
    }

    // test mode
    try
    {
        if (print)
        {
            for (int i=0; i<EnumType::enums.size(); i++)
            {
                EnumType::enums[i]->Print(cout);
                cout << endl;
            }
            if (docType == "Plugin")
                plugin->Print(cout);
            else if (docType == "Attribute")
                attribute->Print(cout);
            else
            {
                cerr << "Document type " << docType << "not supported" << endl;
                exit(-1);
            }
            cout << endl;
        }

#ifdef GENERATE_ATTS
        if (docType == "Plugin" && plugin->type == "database")
        {
            cerr << "No attributes to generate for database plugins\n";
        }
        else
        {
            // atts writer mode
            ofstream h;
            if (Open(h, attribute->name+".h"))
            {
                attribute->WriteHeader(h);
                h.close();
            }

            ofstream c;
            if (Open(c, attribute->name+".C"))
            {
                attribute->WriteSource(c);
                c.close();
            }
        }
#endif
#ifdef GENERATE_WINDOW
        if (docType == "Plugin" && plugin->type == "database")
        {
            cerr << "No window to generate for database plugins\n";
        }
        else
        {
            // window writer mode
            if (docType == "Plugin")
            {
                attribute->windowname = plugin->windowname;
                attribute->plugintype = plugin->type;
            }

            ofstream h;
            if (Open(h, attribute->windowname+".h"))
            {
                attribute->WriteHeader(h);
                h.close();
            }

            ofstream c;
            if (Open(c, attribute->windowname+".C"))
            {
                attribute->WriteSource(c);
                c.close();
            }
        }
#endif
#ifdef GENERATE_INFO
        // info writer mode
        if (docType == "Plugin" && plugin->type == "database")
        {
            ofstream ih;
            if (Open(ih, plugin->name+"PluginInfo.h"))
            {
                plugin->WriteInfoHeader(ih);
                ih.close();
            }

            ofstream ic;
            if (Open(ic, plugin->name+"PluginInfo.C"))
            {
                plugin->WriteInfoSource(ic);
                ic.close();
            }

            ofstream cc;
            if (Open(cc, plugin->name+"CommonPluginInfo.C"))
            {
                plugin->WriteCommonInfoSource(cc);
                cc.close();
            }

            ofstream mc;
            if (Open(mc, plugin->name+"MDServerPluginInfo.C"))
            {
                plugin->WriteMDServerInfoSource(mc);
                mc.close();
            }

            ofstream ec;
            if (Open(ec, plugin->name+"EnginePluginInfo.C"))
            {
                plugin->WriteEngineInfoSource(ec);
                ec.close();
            }
        }
        else
        {
            ofstream ih;
            if (Open(ih, plugin->name+"PluginInfo.h"))
            {
                plugin->WriteInfoHeader(ih);
                ih.close();
            }

            ofstream ic;
            if (Open(ic, plugin->name+"PluginInfo.C"))
            {
                plugin->WriteInfoSource(ic);
                ic.close();
            }

            ofstream cc;
            if (Open(cc, plugin->name+"CommonPluginInfo.C"))
            {
                plugin->WriteCommonInfoSource(cc);
                cc.close();
            }

            ofstream gc;
            if (Open(gc, plugin->name+"GUIPluginInfo.C"))
            {
                plugin->WriteGUIInfoSource(gc);
                gc.close();
            }

            ofstream vc;
            if (Open(vc, plugin->name+"ViewerPluginInfo.C"))
            {
                plugin->WriteViewerInfoSource(vc);
                vc.close();
            }

            ofstream ec;
            if (Open(ec, plugin->name+"EnginePluginInfo.C"))
            {
                plugin->WriteEngineInfoSource(ec);
                ec.close();
            }

            ofstream sc;
            if (Open(sc, plugin->name+"ScriptingPluginInfo.C"))
            {
                plugin->WriteScriptingInfoSource(sc);
                sc.close();
            }
        }
#endif
#ifdef GENERATE_MAKEFILE
        // makefile writer mode
        ofstream out;
        if (Open(out, "Makefile"))
        {
            plugin->WriteMakefile(out);
            out.close();
        }
#endif
#ifdef GENERATE_AVT
        if (docType == "Plugin" && plugin->type == "database")
        {
            // avt writer mode
            ofstream fh;
            if (Open(fh, QString("avt") + plugin->name + "FileFormat.h"))
            {
                plugin->WriteFileFormatReaderHeader(fh);
                fh.close();
            }

            ofstream fc;
            if (Open(fc, QString("avt") + plugin->name + "FileFormat.C"))
            {
                plugin->WriteFileFormatReaderSource(fc);
                fc.close();
            }

            if (plugin->haswriter)
            {
                // avt writer mode
                ofstream wh;
                if (Open(wh, QString("avt") + plugin->name + "Writer.h"))
                {
                    plugin->WriteFileFormatWriterHeader(wh);
                    wh.close();
                }

                ofstream wc;
                if (Open(wc, QString("avt") + plugin->name + "Writer.C"))
                {
                    plugin->WriteFileFormatWriterSource(wc);
                    wc.close();
                }
            }
            if (plugin->hasoptions)
            {
                // DB options mode.
                ofstream wh;
                if (Open(wh, QString("avt") + plugin->name + "Options.h"))
                {
                    plugin->WriteFileFormatOptionsHeader(wh);
                    wh.close();
                }

                ofstream wc;
                if (Open(wc, QString("avt") + plugin->name + "Options.C"))
                {
                    plugin->WriteFileFormatOptionsSource(wc);
                    wc.close();
                }
            }
        }
        else
        {
            // avt filters
            ofstream fh;
            if (Open(fh, QString("avt") + plugin->name + "Filter.h"))
            {
                plugin->WriteFilterHeader(fh);
                fh.close();
            }

            ofstream fc;
            if (Open(fc, QString("avt") + plugin->name + "Filter.C"))
            {
                plugin->WriteFilterSource(fc);
                fc.close();
            }

            if (plugin->type=="plot")
            {
                ofstream ph;
                if (Open(ph, QString("avt") + plugin->name + "Plot.h"))
                {
                    plugin->WritePlotHeader(ph);
                    ph.close();
                }

                ofstream pc;
                if (Open(pc, QString("avt") + plugin->name + "Plot.C"))
                {
                    plugin->WritePlotSource(pc);
                    pc.close();
                }
            }
        }
#endif
#ifdef GENERATE_PYTHON
        if (docType == "Plugin" && plugin->type == "database")
        {
            cerr << "No python to generate for database plugins\n";
        }
        else
        {
            // scripting writer mode
            QString prefix("Py");
            ofstream h;
            if (Open(h, prefix+attribute->name+".h"))
            {
                if (docType == "Plugin")
                {
                    plugin->WriteHeader(h);
                }
                else 
                {
                    attribute->WriteHeader(h);
                }
                h.close();
            }

            ofstream s;
            if (Open(s, prefix+attribute->name+".C"))
            {
                attribute->WriteSource(s);
                s.close();
            }
        }
#endif
#ifdef GENERATE_JAVA
        if (docType == "Plugin" && plugin->type == "database")
        {
            cerr << "No java to generate for database plugins\n";
        }
        else
        {
            if (docType == "Plugin")
            {
                attribute->pluginVersion = plugin->version;
                attribute->pluginName = plugin->name;
                attribute->pluginType = plugin->type;
            }

            // java atts writer mode
            ofstream j;
            if (Open(j, attribute->name+".java"))
            {
                attribute->WriteSource(j);
                j.close();
            }
        }
#endif
#ifdef GENERATE_PROJECTFILE
        if (docType == "Plugin")
        {
            // project file writer mode
            plugin->WriteProjectFiles(Open, generateVersion7Projects);
        }
        else
        {
            cerr << "No project files to generate for non-plugins." << endl;
        }
#endif

        delete attribute;
        delete plugin;
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
