#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <qxml.h>
#include <visitstream.h>
#include "Field.h"
#include "XMLParserUtil.h"
#include "Attribute.h"
#include "Plugin.h"
#include "Function.h"
#include "Constant.h"
#include "CodeFile.h"
#include "Include.h"

#include <vector>
using std::vector;

enum ComponentTypes
{
    COMP_NONE      = 0x00,
    COMP_GUI       = 0x01,
    COMP_SCRIPTING = 0x02,
    COMP_VIEWER    = 0x04,
    COMP_MDSERVER  = 0x08,
    COMP_ENGINE    = 0x10,
    COMP_WIDGETS   = 0x20
};

inline vector<QString>
ParseCharacters(const QString &buff)
{
    vector<QString> output;

    // split one string into a list of strings when delimited by whitespace
    // or quotation marks, e.g.   <string1  "string two"  ""  string4>
    buff.stripWhiteSpace();
    bool quote=false;
    QString tmp="";
    for (int i=0; i<buff.length(); i++)
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
        vector<QString> strings = ParseCharacters(buff);

        for(int i=0; i<strings.size(); i++)
        {
            if (currentTag == "Enum")
                currentEnum->AddValue(strings[i]);
            else if (currentTag == "Field")
                currentField->SetValue(strings[i], i);
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
                if (currentFileComponents & COMP_ENGINE)
                    currentPlugin->efiles.push_back(strings[i]);
                if (currentFileComponents & COMP_WIDGETS)
                    currentPlugin->wfiles.push_back(strings[i]);
            }
            else if (currentTag == "CXXFLAGS")
            {
                currentPlugin->cxxflags.push_back(strings[i]);
            }
            else if (currentTag == "LDFLAGS")
            {
                currentPlugin->ldflags.push_back(strings[i]);
            }
            else if (currentTag == "LIBS")
            {
                currentPlugin->libs.push_back(strings[i]);
            }
            else if (currentTag == "Extensions")
            {
                currentPlugin->extensions.push_back(strings[i]);
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
            QString version   = atts.value("version");
            QString iconFile  = atts.value("iconFile");
            QString enabled   = atts.value("enabled");
            QString mdspecific= atts.value("mdspecificcode");
            QString onlyengine= atts.value("onlyengine");
            QString noengine  = atts.value("noengine");
            currentPlugin = new Plugin(name, label, type, vartype,
                                       dbtype, version, iconFile, 
                                       haswriter.isNull() ? false : Text2Bool(haswriter),
                                       hasoptions.isNull() ? false : Text2Bool(hasoptions),
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
        }
        else if (tag == "Attribute")
        {
            QString purpose       = atts.value("purpose");
            QString codefile      = atts.value("codefile");
            QString persistent    = atts.value("persistent");
            QString exportAPI     = atts.value("exportAPI");
            QString exportInclude = atts.value("exportInclude");
            if (!filepath.isNull() && !codefile.isNull())
                codefile = filepath + codefile;
            currentAttribute = new Attribute(name, purpose, codefile,
                                             exportAPI, exportInclude);
            if (persistent.isNull())
                currentAttribute->persistent = true;
            else
                currentAttribute->persistent = Text2Bool(persistent);

            if (currentAttribute->codeFile)
            {
                std::map<QString, std::pair<QString,QString> >::iterator it;
                for (it = currentAttribute->codeFile->code.begin();
                     it != currentAttribute->codeFile->code.end();
                     it++)
                {
                    currentAttribute->codes.push_back(new Code(it->first,
                                                               it->second.first,
                                                               it->second.second));
                }
            }
        }
        else if (tag == "Enum")
        {
            currentEnum = new EnumType(name);
        }
        else if (tag == "Include")
        {
            QString file   = atts.value("file");
            QString quoted = atts.value("quoted");
            bool    quote = false;
            if (!quoted.isNull()) 
                quote = Text2Bool(quoted);

            currentInclude = new Include(file, quote);
        }
        else if (tag == "Constant")
        {
            QString member = atts.value("member");
            if (!currentAttribute)
                throw QString().sprintf("No current attribute when specifying constant %s",name.latin1());
            if (!currentAttribute->codeFile)
                throw QString().sprintf("No codefile found for constant %s.",name.latin1());
            if (!currentAttribute->codeFile->constant.count(name))
                throw QString().sprintf("no constant %s in codefile.",name.latin1());

            CodeFile *codeFile = currentAttribute->codeFile;

            currentConstant = new Constant(name,
                                           codeFile->constant[name].first,
                                           codeFile->constant[name].second,
                                           Text2Bool(member));
        }
        else if (tag == "Function")
        {
            QString user   = atts.value("user");
            QString member = atts.value("member");
            if (!currentAttribute)
                throw QString().sprintf("No current attribute when specifying function %s",name.latin1());
            if (!currentAttribute->codeFile)
                throw QString().sprintf("No codefile found for function %s.",name.latin1());
            if (!currentAttribute->codeFile->func.count(name))
                throw QString().sprintf("no function %s in codefile.",name.latin1());

            CodeFile *codeFile = currentAttribute->codeFile;

            if (member.isNull())
                member = "true";
            currentFunction = new Function(name,
                                           codeFile->func[name].first,
                                           codeFile->func[name].second,
                                           Text2Bool(user), Text2Bool(member));
        }
        else if (tag == "CXXFLAGS")
        {
        }
        else if (tag == "LDFLAGS")
        {
        }
        else if (tag == "LIBS")
        {
        }
        else if (tag == "Extensions")
        {
        }
        else if (tag == "Files")
        {
            QString         comps1 = atts.value("components");
            vector<QString> comps2 = SplitValues(comps1);
            int             comps3 = COMP_NONE;

            for (int i=0; i<comps2.size(); i++)
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
                else
                    throw QString().sprintf("invalid file '%s' for components attribute of Files tag", comps2[i].latin1());
            }
            currentFileComponents = comps3;
        }
        else if (tag == "Field")
        {
            if (!currentAttribute)
                throw QString().sprintf("No current attribute when specifying field %s",name.latin1());

            QString type      = atts.value("type");
            QString subtype   = atts.value("subtype");
            QString length    = atts.value("length");
            QString label     = atts.value("label");

            currentField = fieldFactory->createField(name,type,subtype,length,label);
            currentField->codeFile = currentAttribute->codeFile;

            QString enabler   = atts.value("enabler");
            QString internal  = atts.value("internal");

            QString ignoreeq  = atts.value("ignoreeq");
            if (!ignoreeq.isNull())
            {
                currentField->ignoreEquality = Text2Bool(ignoreeq);
            }

            QString init      = atts.value("init");
            if (!init.isNull())
            {
                if (!currentAttribute->codeFile)
                    throw QString().sprintf("No codefile found for initializer for %s.",name.latin1());
                if (!currentAttribute->codeFile->init.count(name))
                    throw QString().sprintf("no initializer for %s in codefile.",name.latin1());
                currentField->SetInitCode(currentAttribute->codeFile->init[name]);
            }

            int i;
            if (!enabler.isNull())
            {
                vector<QString> vals = SplitValues(enabler);
                if (vals.size() < 2)
                    throw QString().sprintf("enabler for field %s requires a value",name.latin1());

                Field *enablerField = NULL;
                for (i=0; i<currentAttribute->fields.size(); i++)
                {
                    if (currentAttribute->fields[i]->name == vals[0])
                        enablerField = currentAttribute->fields[i];
                }

                if (!enablerField)
                    throw QString().sprintf("enabler field %s was not defined before current field %s",vals[0].latin1(),name.latin1());

                currentField->SetEnabler(enablerField);
                for (i=1; i<vals.size(); i++)
                {
                    currentField->SetEnablerValue(vals[i]);
                }
            }

            if (!internal.isNull())
                currentField->SetInternal(internal);

            for (i=0; i<atts.length(); i++)
            {
                if (atts.qName(i) != "name"    &&
                    atts.qName(i) != "type"    &&
                    atts.qName(i) != "subtype" &&
                    atts.qName(i) != "label"   &&
                    atts.qName(i) != "length"  &&
                    atts.qName(i) != "enabler" &&
                    atts.qName(i) != "internal"&&
                    atts.qName(i) != "ignoreeq"&&
                    atts.qName(i) != "init"    )
                {
                    currentField->SetAttrib(atts.qName(i),atts.value(i));
                }
            }
        }
        else
        {
            throw QString().sprintf("unknown tag for startElement: %s",tag.latin1());
        }
        currentTag = tag;
        tagStack.push_back(currentTag);

        return true;
    }
    bool endElement( const QString&, const QString&, const QString &tag)
    {
        if (tagStack.back() != tag)
            throw QString().sprintf("ending tag (%s) does not match latest tag started (%s)", tagStack.back().latin1(), tag.latin1());
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
            currentField->index = currentAttribute->fields.size();
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
            currentAttribute->functions.push_back(currentFunction);
            currentFunction = NULL;
        }
        else if (tag == "CXXFLAGS")
        {
        }
        else if (tag == "LDFLAGS")
        {
        }
        else if (tag == "LIBS")
        {
        }
        else if (tag == "Extensions")
        {
        }
        else if (tag == "Files")
        {
            currentFileComponents = COMP_NONE;
        }
        else if (tag == "Constant")
        {
            currentAttribute->constants.push_back(currentConstant);
            currentConstant = NULL;
        }
        else
        {
            throw QString().sprintf("unknown tag for endElement: %s",tag.latin1());
        }

        currentTag = tagStack.back();
        return true;
    }
  private:
    int             currentFileComponents;
    Function       *currentFunction;
    Constant       *currentConstant;
    Include        *currentInclude;
    Field          *currentField;
    EnumType       *currentEnum;
    Attribute      *currentAttribute;
    Plugin         *currentPlugin;
    QString         currentTag;
    vector<QString> tagStack;
    FieldFactory   *fieldFactory;
};

#endif
