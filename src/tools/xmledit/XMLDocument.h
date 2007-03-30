#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

#include <qstring.h>
class Plugin;
class Attribute;

// ****************************************************************************
//  Class:  XMLDocument
//
//  Purpose:
//    Encapsulates a plugin/attribute XML file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// ****************************************************************************
struct XMLDocument
{
    QString     filename;
    QString     docType;
    Plugin     *plugin;
    Attribute  *attribute;
  public:
    void open(const QString &file);
    void save(const QString &file);
};

#endif
