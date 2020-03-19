// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XML_PARSER_H
#define XML_PARSER_H
#include <QTextStream>

#include <qxml.h>
#include "Field.h"
#include "XMLParserUtil.h"
#include "Attribute.h"
#include "Plugin.h"
#include "Function.h"
#include "Constant.h"
#include "CodeFile.h"
#include "Include.h"

#include <vector>

enum ComponentTypes
{
    COMP_NONE            = 0x000,
    COMP_GUI             = 0x001,
    COMP_SCRIPTING       = 0x002,
    COMP_VIEWER          = 0x004,
    COMP_MDSERVER        = 0x008,
    COMP_ENGINE          = 0x010,
    COMP_ENGINESER       = 0x020,
    COMP_ENGINEPAR       = 0x040,
    COMP_WIDGETS         = 0x080,
    COMP_VIEWER_WIDGETS  = 0x100,
    COMP_JAVA            = 0x200,
    COMP_ALL             = 0x400
};

inline std::vector<QString>
ParseCharacters(const QString &buff_input)
{
    std::vector<QString> output;

    // split one string into a list of strings when delimited by whitespace
    // or quotation marks, e.g.   <string1  "string two"  ""  string4>
    QString buff(buff_input.trimmed());
    bool quote=false;
    QString tmp="";
    for (int i=0; i<(int)buff.length(); i++)
    {
        if (!quote && buff[i].isSpace())
        {
            if (!tmp.isEmpty())
                output.push_back(tmp);
            tmp="";
        }
        else if (!quote && buff[i] == '"')
        {
            quote = true;
        }
        else if (quote && buff[i] == '"')
        {
            quote = false;
            output.push_back(tmp);
            tmp="";
        }
        else
        {
            tmp += buff[i];
        }
    }
    if (!tmp.isEmpty())
        output.push_back(tmp);

    return output;
}

// ****************************************************************************
//  Class:  XMLParser
//
//  Purpose:
//    Parse a plugin/attributesubject.
//
//  Note:
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 14:15:14 PDT 2001
//    Added vartype to a plugin.  This is so plot plugins can specify
//    whether they handle meshes, scalars, vectors, etc.
//
//    Jeremy Meredith, Fri Apr 19 15:34:19 PDT 2002
//    Major enhancements.  Added more attributes to Field, added constants,
//    includes.  Renamed and added some more stuff for the code file.
//
//    Jeremy Meredith, Wed May  8 15:29:14 PDT 2002
//    Added "persistent" tag to Attributes.
//
//    Brad Whitlock, Tue May 14 10:23:01 PDT 2002
//    Added support for exports.
//
//    Jeremy Meredith, Mon Jul 15 01:22:09 PDT 2002
//    Big enhancements so all our plugin makefile can be build automatically.
//
//    Jeremy Meredith, Fri Jul 19 17:34:21 PDT 2002
//    Added default value for tag Function attribute member (true).
//
//    Jeremy Meredith, Mon Aug 26 17:33:24 PDT 2002
//    Added database plugin support.
//
//    Jeremy Meredith, Thu Oct 17 15:57:13 PDT 2002
//    Added filepath.  Added some improvements for the XML editor.
//
//    Brad Whitlock, Mon Nov 18 14:06:14 PST 2002
//    I renamed the class to avoid naming conflicts on Windows.
//
//    Brad Whitlock, Thu Mar 13 12:23:35 PDT 2003
//    I added support for icon files.
//
//    Jeremy Meredith, Thu Mar 27 12:51:05 PST 2003
//    Stopped adding the class scope to the enum enabler values.
//    It is now added only before use in GenerateWindow.
//
//    Hank Childs, Tue Sep  9 10:04:41 PDT 2003
//    Added support for file writers.
//
//    Jeremy Meredith, Tue Sep 23 17:00:16 PDT 2003
//    Made haswriter be a true bool.
//
//    Jeremy Meredith, Wed Nov  5 13:28:03 PST 2003
//    Added ability to disable plugins by default.
//
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Jeremy Meredith, Wed Aug 25 11:50:14 PDT 2004
//    Added the concept of an engine-only or everything-but-the-engine plugin.
//
//    Hank Childs, Tue May 24 09:41:53 PDT 2005
//    Added hasoptions.
//
//    Hank Childs, Tue Sep 12 09:25:05 PDT 2006
//    Fix problem where "NULL" QStrings were getting used elsewhere in the
//    code where empty strings should be used.
//
//    Brad Whitlock, Fri Feb 23 17:09:30 PST 2007
//    Added support for viewer widgets.
//
//    Cyrus Harrison, Wed Mar  7 09:35:48 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Brad Whitlock, Wed Feb 28 18:34:33 PST 2007
//    Added support for fields that should be public. Added keyframe flag.
//
//    Hank Childs, Thu Jan 10 14:51:51 PST 2008
//    Added support for explicit filenames.
//
//    Brad Whitlock, Fri Apr 25 11:23:20 PDT 2008
//    Added support for setting access type on functions.
//
//    Cyrus Harrison, Fri Sep 19 13:56:30 PDT 2008
//    Added support for custom libs for gui,engine,mdserver,and viewer targets.
//
//    Brad Whitlock, Wed Oct 15 14:23:40 PDT 2008
//    Added support for additional Java files.
//
//    Jeremy Meredith, Mon Feb 23 17:37:33 EST 2009
//    Don't just check if init is nonnull, check if it's true.
//
//    Kathleen Bonnell, Wed May 27 9:01:52 MST 2009
//    Added support for windows-only mdserver and engine files (WM, WE).
//
//    Mark C. Miller, Wed Aug 26 11:03:19 PDT 2009
//    Added support for custom base class for derived state objects.
//
//    Jeremy Meredith, Tue Sep  8 15:11:35 EDT 2009
//    Split custom engine libs into serial and parallel versions.
//
//    Jeremy Meredith, Tue Dec 29 11:21:30 EST 2009
//    Replaced "Extensions" and "Filenames" with "FilePatterns".  Removed
//    specifiedFilenames.  Added filePatternsStrict and opensWholeDirectory.
//
//    Hank Childs, Thu Dec 30 13:33:19 PST 2010
//    Added support for expression-creating operators.
//
//    Kathleen Biagas, Thu Aug 25 13:23:07 MST 2011
//    Added persistent flag for fields.
//
//    Kathleen Biagas, Thu Nov  6 11:24:21 PST 2014
//    Add support for DEFINES tag.
//
//    Kathleen Biagas, Thu Jan  2 09:18:18 PST 2020
//    Added haslicense.
//
// ****************************************************************************

class XMLParser : public QXmlDefaultHandler
{
  public:
    QString    docType;
    Plugin    *plugin;
    Attribute *attribute;
    QString    filepath;
  public:
    XMLParser(FieldFactory *fieldFactory_, QString filename)
    {
        filepath = FilePath(filename);
        currentPlugin = NULL;
        plugin    = NULL;
        attribute = NULL;
        currentConstants = NULL;
        currentFunctions = NULL;
        fieldFactory = fieldFactory_;
    }
    bool startDocument()
    {
        currentTag = "";
        tagStack.push_back("xml file");
        return true;
    }
    bool characters(const QString& buff)
    {
        std::vector<QString> strings = ParseCharacters(buff);

        for(size_t i=0; i<strings.size(); i++)
        {
            if (currentTag == "Enum")
                currentEnum->AddValue(strings[i]);
            else if (currentTag == "Field")
                currentField->SetValue(strings[i], (int)i);
            else if (currentTag == "Include")
                currentInclude->SetInclude(strings[i]);
            else if (currentTag == "Files")
            {
                if (currentFileComponents & COMP_GUI)
                    currentPlugin->gfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_SCRIPTING)
                    currentPlugin->sfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_VIEWER)
                    currentPlugin->vfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_MDSERVER)
                    currentPlugin->mfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_ENGINE ||
                    currentFileComponents & COMP_ENGINESER ||
                    currentFileComponents & COMP_ENGINEPAR)
                    currentPlugin->efiles.push_back(strings[i]);
                if (currentFileComponents & COMP_WIDGETS)
                    currentPlugin->wfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_VIEWER_WIDGETS)
                    currentPlugin->vwfiles.push_back(strings[i]);
                if (currentFileComponents & COMP_JAVA)
                    currentPlugin->jfiles.push_back(strings[i]);
            }
            else if (currentTag == "LIBS")
            {
                if (currentLibComponents & COMP_GUI)
                    currentPlugin->glibs.push_back(strings[i]);
                if (currentLibComponents & COMP_VIEWER)
                    currentPlugin->vlibs.push_back(strings[i]);
                if (currentLibComponents & COMP_MDSERVER)
                    currentPlugin->mlibs.push_back(strings[i]);
                if (currentLibComponents & COMP_ENGINE)
                {
                    currentPlugin->elibsSer.push_back(strings[i]);
                    currentPlugin->elibsPar.push_back(strings[i]);
                }
                if (currentLibComponents & COMP_ENGINESER)
                    currentPlugin->elibsSer.push_back(strings[i]);
                if (currentLibComponents & COMP_ENGINEPAR)
                    currentPlugin->elibsPar.push_back(strings[i]);
                // case with no flags (libs for all components)
                if (currentLibComponents & COMP_ALL)
                    currentPlugin->libs.push_back(strings[i]);
            }
            else if (currentTag == "CXXFLAGS")
            {
                currentPlugin->cxxflags.push_back(strings[i]);
            }
            else if (currentTag == "DEFINES")
            {
                currentPlugin->defs.push_back(strings[i]);
            }
            else if (currentTag == "LDFLAGS")
            {
                currentPlugin->ldflags.push_back(strings[i]);
            }
            else if (currentTag == "FilePatterns")
            {
                currentPlugin->filePatterns.push_back(strings[i]);
            }
        }
        return true;
    }
    bool startElement(const QString&,
                      const QString&,
                      const QString& tag,
                      const QXmlAttributes& atts)
    {
        if (currentTag == "")
            docType = tag;

        QString name    = atts.value("name");
        if (tag == "Plugin")
        {
            QString label     = atts.value("label");
            QString type      = atts.value("type");
            QString vartype   = atts.value("vartype");
            QString dbtype    = atts.value("dbtype");
            QString haswriter = atts.value("haswriter");
            QString hasoptions= atts.value("hasoptions");
            QString haslicense= atts.value("haslicense");
            QString version   = atts.value("version");
            QString iconFile  = atts.value("iconFile");
            QString enabled   = atts.value("enabled");
            QString mdspecific= atts.value("mdspecificcode");
            QString engspecific= atts.value("engspecificcode");
            QString onlyengine= atts.value("onlyengine");
            QString noengine  = atts.value("noengine");
            QString filePatternsStrict = atts.value("filePatternsStrict");
            QString opensWholeDirectory = atts.value("opensWholeDirectory");
            QString category  = atts.value("category");
            QString createExpression = atts.value("createExpression");
            QString exprInType = atts.value("exprInType");
            QString exprOutType = atts.value("exprOutType");
            currentPlugin = new Plugin(name, label, type, vartype,
                                       dbtype, version, iconFile,
                                       haswriter.isNull() ? false : Text2Bool(haswriter),
                                       hasoptions.isNull() ? false : Text2Bool(hasoptions),
                                       haslicense.isNull() ? false : Text2Bool(haslicense),
                                       onlyengine.isNull() ? false : Text2Bool(onlyengine),
                                       noengine.isNull() ? false : Text2Bool(noengine));
            if (!enabled.isNull())
            {
                currentPlugin->enabledByDefault = Text2Bool(enabled);
            }
            if (!mdspecific.isNull())
            {
                currentPlugin->has_MDS_specific_code = Text2Bool(mdspecific);
            }
            if (!engspecific.isNull())
            {
                currentPlugin->hasEngineSpecificCode = Text2Bool(engspecific);
            }
            if (!filePatternsStrict.isNull())
            {
                currentPlugin->filePatternsStrict = Text2Bool(filePatternsStrict);
            }
            if (!opensWholeDirectory.isNull())
            {
                currentPlugin->opensWholeDirectory = Text2Bool(opensWholeDirectory);
            }
            if (!category.isNull())
            {
                currentPlugin->category = category;
            }
            if (!createExpression.isNull())
            {
                currentPlugin->createExpression = Text2Bool(createExpression);
            }
            if (!exprInType.isNull())
            {
                currentPlugin->exprInType = exprInType;
            }
            if (!exprOutType.isNull())
            {
                currentPlugin->exprOutType = exprOutType;
            }
        }
        else if (tag == "Attribute")
        {
            QString purpose       = atts.value("purpose");
            QString codefile      = atts.value("codefile");
            QString persistent    = atts.value("persistent");
            QString keyframe      = atts.value("keyframe");
            QString exportAPI     = atts.value("exportAPI");
            QString baseClass     = atts.value("baseClass");
            if (exportAPI.isNull())
                exportAPI = "";
            QString exportInclude = atts.value("exportInclude");
            if (exportInclude.isNull())
                exportInclude = "";
            if (!filepath.isNull() && !codefile.isNull())
                codefile = filepath + codefile;
            currentAttribute = new Attribute(name, purpose, codefile,
                                             exportAPI, exportInclude,
                                             baseClass);

            if (persistent.isNull())
                currentAttribute->persistent = true;
            else
                currentAttribute->persistent = Text2Bool(persistent);

            if (keyframe.isNull())
                currentAttribute->keyframe = true;
            else
                currentAttribute->keyframe = Text2Bool(keyframe);

            if (currentAttribute->codeFile)
            {
                QStringList targets, names, first, second;
                currentAttribute->codeFile->GetAllCodes(targets, names, first, second);
                for(int i = 0; i < targets.size(); ++i)
                {
                    currentAttribute->codes.push_back(new Code(names[i],
                                                               first[i],
                                                               second[i],
                                                               targets[i]));
                }
            }

            if (baseClass.isNull())
                currentAttribute->custombase = false;
            else
                currentAttribute->custombase = (baseClass != "AttributeSubject");
        }
        else if (tag == "Enum")
        {
            currentEnum = new EnumType(name);
        }
        else if (tag == "Include")
        {
            QString file   = atts.value("file");
            QString quoted = atts.value("quoted");
            QString target = atts.value("target");
            bool    quote = false;
            if (!quoted.isNull())
                quote = Text2Bool(quoted);
            if (target.isNull())
                target = "xml2atts";

            currentInclude = new Include(file, quote, target);
        }
        else if (tag == "Constant")
        {
            QString member = atts.value("member");

            if (!currentAttribute)
                throw QString("No current attribute when specifying constant %1").arg(name);
            CodeFile *codeFile = currentAttribute->codeFile;
            if (!codeFile)
                throw QString("No codefile found for constant %1.").arg(name);
            if (!codeFile->HasConstant(name))
                throw QString("no constant %1 in codefile.").arg(name);

            // Get the constant definitions
            QStringList targets, first, second;
            codeFile->GetConstant(name, targets, first, second);
            currentConstants = new Constant *[targets.size() + 1];
            for(int i = 0; i < targets.size(); ++i)
            {
                currentConstants[i] = new Constant(name,
                                                   first[i],
                                                   second[i],
                                                   Text2Bool(member),
                                                   targets[i]);
            }
            currentConstants[targets.size()] = NULL;
        }
        else if (tag == "Function")
        {
            QString user   = atts.value("user");
            QString member = atts.value("member");
            if (member.isNull())
                member = "true";
            QString access = atts.value("access");
            Function::AccessType a;
            if(access == "protected")
                a = Function::AccessProtected;
            else if(access == "private")
                a = Function::AccessPrivate;
            else
                a = Function::AccessPublic;

            if (!currentAttribute)
                throw QString("No current attribute when specifying function %1").arg(name);
            CodeFile *codeFile = currentAttribute->codeFile;
            if (!codeFile)
                throw QString("No codefile found for function %1.").arg(name);
            if (!codeFile->HasFunction(name))
                throw QString("no function %1 in codefile.").arg(name);

            // Get the function definitions
            QStringList targets, first, second;
            codeFile->GetFunction(name, targets, first, second);
            currentFunctions = new Function*[targets.size()+1];
            for(int i = 0; i < targets.size(); ++i)
            {
                currentFunctions[i] = new Function(name,
                                                   first[i],
                                                   second[i],
                                                   Text2Bool(user), Text2Bool(member),
                                                   targets[i]);
                currentFunctions[i]->accessType = a;
            }
            currentFunctions[targets.size()] = NULL;
        }
        else if (tag == "CXXFLAGS")
        {
        }
        else if (tag == "DEFINES")
        {
        }
        else if (tag == "LDFLAGS")
        {
        }
        else if (tag == "FilePatterns")
        {
        }
        else if (tag == "Files")
        {
            QString         comps1 = atts.value("components");
            std::vector<QString> comps2 = SplitValues(comps1);
            int             comps3 = COMP_NONE;

            for (size_t i=0; i<comps2.size(); i++)
            {
                if (comps2[i] == "G")
                {
                    currentPlugin->gfiles.clear();
                    comps3 |= COMP_GUI;
                    currentPlugin->customgfiles = true;
                }
                else if (comps2[i] == "S")
                {
                    currentPlugin->sfiles.clear();
                    comps3 |= COMP_SCRIPTING;
                    currentPlugin->customsfiles = true;
                }
                else if (comps2[i] == "V")
                {
                    currentPlugin->vfiles.clear();
                    comps3 |= COMP_VIEWER;
                    currentPlugin->customvfiles = true;
                }
                else if (comps2[i] == "M")
                {
                    currentPlugin->mfiles.clear();
                    comps3 |= COMP_MDSERVER;
                    currentPlugin->custommfiles = true;
                }
                else if (comps2[i] == "E")
                {
                    currentPlugin->efiles.clear();
                    comps3 |= COMP_ENGINE;
                    currentPlugin->customefiles = true;
                }
                else if (comps2[i] == "W")
                {
                    currentPlugin->wfiles.clear();
                    comps3 |= COMP_WIDGETS;
                    currentPlugin->customwfiles = true;
                }
                else if (comps2[i] == "VW")
                {
                    currentPlugin->vwfiles.clear();
                    comps3 |= COMP_VIEWER_WIDGETS;
                    currentPlugin->customvwfiles = true;
                }
                else if (comps2[i] == "J")
                {
                    currentPlugin->jfiles.clear();
                    comps3 |= COMP_JAVA;
                    currentPlugin->customjfiles = true;
                }
                else if (comps2[i] == "WM")
                {
                    currentPlugin->wmfiles.clear();
                    comps3 |= COMP_MDSERVER;
                    currentPlugin->customwmfiles = true;
                }
                else if (comps2[i] == "WE")
                {
                    currentPlugin->wefiles.clear();
                    comps3 |= COMP_ENGINE;
                    currentPlugin->customwefiles = true;
                }
                else
                    throw QString("invalid file '%1' for components attribute of Files tag").arg(comps2[i]);
            }
            currentFileComponents = comps3;
        }
        else if (tag == "LIBS")
        {
            currentLibComponents = COMP_NONE;
            // if we have a "components" attribute, we need to find out
            // which component the libs are for.
            // if not, we have libs for all comps
            if(atts.index("components") == -1)
            {
                currentLibComponents = COMP_ALL;
            }
            else
            {
                QString         comps         = atts.value("components");
                std::vector<QString> comps_split   = SplitValues(comps);
                int             comps_current = COMP_NONE;

                for (size_t i=0; i<comps_split.size(); i++)
                {
                    if (comps_split[i] == "G")
                    {
                        currentPlugin->glibs.clear();
                        comps_current |= COMP_GUI;
                        currentPlugin->customglibs = true;
                    }
                    else if (comps_split[i] == "V")
                    {
                        currentPlugin->vlibs.clear();
                        comps_current |= COMP_VIEWER;
                        currentPlugin->customvlibs = true;
                    }
                    else if (comps_split[i] == "M")
                    {
                        currentPlugin->mlibs.clear();
                        comps_current |= COMP_MDSERVER;
                        currentPlugin->custommlibs = true;
                    }
                    else if (comps_split[i] == "ESer")
                    {
                        currentPlugin->elibsSer.clear();
                        comps_current |= COMP_ENGINESER;
                        currentPlugin->customelibsSer = true;
                    }
                    else if (comps_split[i] == "EPar")
                    {
                        currentPlugin->elibsPar.clear();
                        comps_current |= COMP_ENGINEPAR;
                        currentPlugin->customelibsPar = true;
                    }
                    else
                        throw QString("invalid file '%1' for components attribute of LIBS tag").arg(comps_split[i]);
                }
                currentLibComponents = comps_current;
            }
        }
        else if (tag == "Field")
        {
            if (!currentAttribute)
                throw QString("No current attribute when specifying field %1").arg(name);

            QString type      = atts.value("type");
            QString subtype   = atts.value("subtype");
            QString length    = atts.value("length");
            QString label     = atts.value("label");

            currentField = fieldFactory->createField(name,type,subtype,length,label);
            currentField->codeFile = currentAttribute->codeFile;

            QString enabler   = atts.value("enabler");
            QString internal  = atts.value("internal");
            QString persistent  = atts.value("persistent");

            QString ignoreeq  = atts.value("ignoreeq");
            if (!ignoreeq.isNull())
            {
                currentField->ignoreEquality = Text2Bool(ignoreeq);
            }

            QString access    = atts.value("access");
            if (!access.isNull())
            {
                if(access == "public")
                    currentField->SetPublicAccess();
                else if(access == "protected")
                    currentField->SetProtectedAccess();
                else
                    currentField->SetPrivateAccess();
            }
            else
                currentField->SetPrivateAccess();

            QString init      = atts.value("init");
            if (!init.isNull() && Text2Bool(init)==true)
            {
                if (!currentAttribute->codeFile)
                    throw QString("No codefile found for initializer for %1.").arg(name);
                if (!currentAttribute->codeFile->HasInit(name))
                    throw QString("no initializer for %1 in codefile.").arg(name);

                QStringList targets, defs;
                currentAttribute->codeFile->GetInit(name, targets, defs);
                for(int i = 0; i < targets.size(); ++i)
                    currentField->SetInitCode(targets[i], defs[i]);
            }

            if (!enabler.isNull())
            {
                std::vector<QString> vals = SplitValues(enabler);
                if (vals.size() < 2)
                    throw QString("enabler for field %1 requires a value").arg(name);

                Field *enablerField = NULL;
                for (size_t i=0; i<currentAttribute->fields.size(); i++)
                {
                    if (currentAttribute->fields[i]->name == vals[0])
                        enablerField = currentAttribute->fields[i];
                }

                if (!enablerField)
                    throw QString("enabler field %1 was not defined before current field %2").arg(vals[0]).arg(name);

                currentField->SetEnabler(enablerField);
                for (size_t i=1; i<vals.size(); i++)
                {
                    currentField->SetEnablerValue(vals[i]);
                }
            }

            if (!internal.isNull())
                currentField->SetInternal(internal);

            if (!persistent.isNull())
                currentField->SetPersistent(persistent);

            for (int j=0; j<atts.length(); j++)
            {
                if (atts.qName(j) != "name"    &&
                    atts.qName(j) != "type"    &&
                    atts.qName(j) != "subtype" &&
                    atts.qName(j) != "label"   &&
                    atts.qName(j) != "length"  &&
                    atts.qName(j) != "enabler" &&
                    atts.qName(j) != "internal"&&
                    atts.qName(j) != "persistent"  &&
                    atts.qName(j) != "ignoreeq"&&
                    atts.qName(j) != "access"  &&
                    atts.qName(j) != "init"    )
                {
                    currentField->SetAttrib(atts.qName(j),atts.value(j));
                }
            }
        }
        else
        {
            throw QString("unknown tag for startElement: %1").arg(tag);
        }
        currentTag = tag;
        tagStack.push_back(currentTag);
        return true;
    }
    bool endElement( const QString&, const QString&, const QString &tag)
    {
        // NOTE: If you need to add a new tag, make sure you add a case here (even if empty)
        // so the parser will except it. Default behavior is to throw an exception.

        if (tagStack.back() != tag)
            throw QString("ending tag (%1) does not match latest tag started (%2)").arg(tagStack.back()).arg(tag);
        tagStack.pop_back();

        if (tag == "Attribute")
        {
            attribute = currentAttribute;
            if (currentPlugin)
            {
                currentPlugin->atts = currentAttribute;
                currentAttribute = NULL;
            }
        }
        else if (tag == "Field")
        {
            currentField->index = (int)currentAttribute->fields.size();
            currentAttribute->fields.push_back(currentField);
            currentField = NULL;
        }
        else if (tag == "Enum")
        {
            EnumType::enums.push_back(currentEnum);
            currentEnum = NULL;
        }
        else if (tag == "Include")
        {
            currentAttribute->includes.push_back(currentInclude);
            currentInclude = NULL;
        }
        else if (tag == "Plugin")
        {
            plugin = currentPlugin;
        }
        else if (tag == "Function")
        {
            for(Function **f = currentFunctions; *f != NULL; ++f)
                currentAttribute->functions.push_back(*f);
            delete [] currentFunctions;
            currentFunctions = NULL;
        }
        else if (tag == "CXXFLAGS")
        {
        }
        else if (tag == "DEFINES")
        {
        }
        else if (tag == "LDFLAGS")
        {
        }
        else if (tag == "LIBS")
        {
            currentLibComponents = COMP_NONE;
        }
        else if (tag == "FilePatterns")
        {
        }
        else if (tag == "Files")
        {
            currentFileComponents = COMP_NONE;
        }
        else if (tag == "Constant")
        {
            for(Constant **c = currentConstants; *c != NULL; ++c)
                currentAttribute->constants.push_back(*c);
            delete [] currentConstants;
            currentConstants = NULL;
        }
        else
        {
            throw QString("unknown tag for endElement: %1").arg(tag);
        }

        currentTag = tagStack.back();
        return true;
    }
  private:
    int             currentFileComponents;
    int             currentLibComponents;
    Include        *currentInclude;
    Constant      **currentConstants;
    Function      **currentFunctions;
    Field          *currentField;
    EnumType       *currentEnum;
    Attribute      *currentAttribute;
    Plugin         *currentPlugin;
    QString         currentTag;
    std::vector<QString> tagStack;
    FieldFactory   *fieldFactory;
};

#endif
