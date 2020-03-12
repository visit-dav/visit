// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XML_NODE_H
#define XML_NODE_H
#include <state_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include <vector>
#include <map>


// ****************************************************************************
//  Class:  XMLNode
//
//  Purpose:
//    Very basic tree based xml generator and parser.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
//  Modifications:
//
// ****************************************************************************

class STATE_API XMLNode
{
  public:
    XMLNode();
    XMLNode(std::istream &);
    XMLNode(const std::string&);
    XMLNode(const XMLNode&);
    virtual  ~XMLNode();
    XMLNode  &operator=(const XMLNode&);

    std::string  &Name() { return name;}
    std::string  &Text() { return text;}

    const std::string  &Name() const { return name;}
    const std::string  &Text() const { return text;}

    std::string  &Attribute(const std::string &att_name)
                    { return attributes[att_name];}
    const std::string  &Attribute(const std::string &) const;
    bool     HasAttribute(const std::string &) const;
    bool     DeleteAttribute(const std::string &);
    void     GetAttributeNames(stringVector &) const;

    bool     HasChildren()    const { return !children.empty(); }
    int      GetNumChildren() const { return (int)children.size(); }

    XMLNode *AddChild(const std::string  &);
    XMLNode *AddChild(const XMLNode &);

    XMLNode *GetChild(int index) const;
    XMLNode *TakeChild(int index);
    bool     DeleteChild(int index);

    int           FindIndex(const std::string &name,
                            int index_start = 0) const;
    std::string   ToString(const std::string &indent="") const;

 private:
    // init & destroy helpers
    void    Init(const XMLNode&);
    void    Cleanup();

    // xml parsing methods
    void    Parse(istream &);
    void    Parse(const std::string &);

    // Parse helpers
    static bool        Check(const std::string&,std::istream &);
    static void        Eat(const std::string&,std::istream &);
    static std::string GrabName(std::istream &);
    static std::string GrabString(unsigned char,std::istream &);
    static std::string GrabEntityValue(std::istream &);
    static std::string EscapeString(const std::string&);

    static std::string unset_str;

    std::string name;
    std::string text;
    std::map<std::string,std::string> attributes;
    std::vector<XMLNode *>  children;

};

#endif


