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

#include <XMLNode.h>

#include <visitstream.h>
#include <stack>

using namespace std;

string XMLNode::unset_str = "[unset]";


// ****************************************************************************
//  Method:  XMLNode::XMLNode
//
//  Purpose:
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************
XMLNode::XMLNode()
: name(""), text("")
{;}

// ****************************************************************************
//  Method:  XMLNode::XMLNode
//
//  Purpose:
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************
XMLNode::XMLNode(const string &xml_data)
: name(""), text("")
{
    Parse(xml_data);
}

// ****************************************************************************
//  Method:  XMLNode::XMLNode
//
//  Purpose:
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************
XMLNode::XMLNode(istream &iss)
: name(""), text("")
{
    Parse(iss);
}


// ****************************************************************************
//  Method:  XMLNode::XMLNode
//
//  Purpose:
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************
XMLNode::XMLNode(const XMLNode &node)
: name(""), text("")
{
    Init(node);
}


// ****************************************************************************
//  Method:  XMLNode::operator=
//
//  Purpose:
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************
XMLNode &
XMLNode::operator=(const XMLNode &node)
{
    if(this != &node)
        Init(node);
    return *this;
}

// ****************************************************************************
//  Method:  XMLNode::~XMLNode
//
//  Purpose:
//     XMLNode destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************
XMLNode::~XMLNode()
{
    Cleanup();
}

// ****************************************************************************
//  Method:  XMLNode::Attribute
//
//  Purpose:
//     Gets attribute in a const safe way. If attribute is not set, 
//     returns "[unset]".
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

const string  &
XMLNode::Attribute(const string &att_name) const
{
    map<string,string>::const_iterator itr = attributes.find(att_name);
    if(itr == attributes.end())
        return unset_str;
    else
        return itr->second;
}

// ****************************************************************************
//  Method:  XMLNode::HasAttribute
//
//  Purpose:
//     Checks if the node has an attribute with the given name.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

bool 
XMLNode::HasAttribute(const string &name) const
{
    return attributes.find(name) != attributes.end();
}

// ****************************************************************************
//  Method:  XMLNode::DeleteAttribute
//
//  Purpose:
//     Removes attribute with the given name, if it exists. Return result
//     indicates success.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

bool 
XMLNode::DeleteAttribute(const string &name)
{
    map<string,string>::iterator itr = attributes.find(name);
    if(itr != attributes.end())
    {
        attributes.erase(itr);
        return true;
    }
    else
    {
        return false;
    }
}

// ****************************************************************************
//  Method:  XMLNode::GetAttributeNames
//
//  Purpose:
//     Enumerates the attribute names of this node.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

void 
XMLNode::GetAttributeNames(stringVector &result) const
{
    result.clear();
    map<string,string>::const_iterator itr;
    for(itr =attributes.begin(); itr != attributes.end();++itr)
        result.push_back(itr->first);
}

// ****************************************************************************
//  Method:  XMLNode::GetAttributeNames
//
//  Purpose:
//     Gets pointer to child at given index, returns NULL if index is invalid.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

XMLNode *
XMLNode::GetChild(int index) const
{
    if(index < 0 || index >= children.size())
        return NULL;
    return children[index];
}

// ****************************************************************************
//  Method:  XMLNode::AddChild
//
//  Purpose:
//     Creates a new child the given name, and returns its pointer.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

XMLNode *
XMLNode::AddChild(const string &name)
{
    XMLNode *child = new XMLNode();
    child->Name() = name;
    children.push_back(child);
    return child;
}

// ****************************************************************************
//  Method:  XMLNode::AddChild
//
//  Purpose:
//     Creates copy of the passed node, adds it as a child and returns 
//     the new child's pointer.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

XMLNode *
XMLNode::AddChild(const XMLNode &node)
{
    XMLNode *child = new XMLNode(node);
    children.push_back(child);
    return child;
}

// ****************************************************************************
//  Method:  XMLNode::TakeChild
//
//  Purpose:
//     Removes the child node at given index from this node. After this
//     the caller assumes responsiblity for deleting the returned node.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

XMLNode *
XMLNode::TakeChild(int index)
{
    if(index < 0 || index >= children.size())
        return false;
        
    XMLNode *child = children[index];
    children.erase(children.begin()+index);
    return child;
}

// ****************************************************************************
//  Method:  XMLNode::DeleteChild
//
//  Purpose:
//     Removes the child node at given index. Result indicates success.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

bool 
XMLNode::DeleteChild(int index)
{
    if(index < 0 || index >= children.size())
        return false;
        
    XMLNode *child = children[index];
    delete child;
    children.erase(children.begin()+index);
    return true;
}

// ****************************************************************************
//  Method:  XMLNode::FindIndex
//
//  Purpose:
//     Finds the index of the first child with the given name, after the start
//     index. -1 == failure.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
//  Modifications:
//    Kathleen Bonnell, Wed Jan  2 08:15:07 PST 2008
//    Fix compile error. (start = 0, instead of start ==0).
//
// ****************************************************************************

int 
XMLNode::FindIndex(const string &name, int start) const
{
    if(start < 0)
        start = 0;
    if(start > children.size())
        return -1;
        
    for(int i=start; i<children.size(); i++)
    {
        if(name == children[i]->Name())
            return i;
    }
    return -1;
}

// ****************************************************************************
//  Method:  XMLNode::ToString
//
//  Purpose:
//     Creates an xml string from this node.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

string
XMLNode::ToString(const string &indent) const
{
    ostringstream oss;
    string esc_name =EscapeString(name);
    oss << indent <<"<" << esc_name;
    map<string,string>::const_iterator itr;
    for(itr = attributes.begin(); itr != attributes.end(); ++itr)
    {
        oss << " " << EscapeString(itr->first) << "=\"" 
            << EscapeString(itr->second) << "\"";
    }
    oss << ">";
    int nchildren = children.size();
    if(nchildren == 0) // assume text node
    {
       // keep text nodes on a single line.
       oss << EscapeString(text) << "</" << esc_name << ">" << endl;
    }
    else
    {
        oss << endl;
        string child_indent = indent + "    ";
        // if we have a text value, save it out as a "text" child
        // since nested & text xml nodes are not supposed to mix
        if(text != "")
            oss << child_indent << "<text>" << EscapeString(text) 
                << "</text>" <<endl;
        
        for(int i=0;i<nchildren;i++)
            oss << children[i]->ToString(child_indent);
        oss << indent << "</" << esc_name << ">" << endl;
    }
    
    return oss.str();
}


// ****************************************************************************
//  Method:  XMLNode::Parse
//
//  Purpose:
//     Parses an xml string into a node.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

void 
XMLNode::Parse(const string &xml_data)
{
    if(xml_data == "")
        return;
    istringstream iss;
    iss.str(xml_data);
    Parse(iss);
}

// ****************************************************************************
//  Method:  XMLNode::Parse
//
//  Purpose:
//     Parses an xml string into a node.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

void 
XMLNode::Parse(istream &iss)
{
    if(iss.eof())
        return;
    // eat any white space
    iss >> ws;
    Eat("<",iss);
    name = GrabName(iss);
    iss >> ws;
    while(!Check(">",iss))
    {
        string att_name  = GrabName(iss);
        Eat("=",iss);
        Eat("\"",iss);
        string att_value = GrabString('\"',iss);
        Eat("\"",iss);
        iss >> ws;
        attributes[att_name] = att_value;
    }
    Eat(">",iss);
    // dont keep white space at beginning of new tag
    iss >> ws;
   
    if(!Check("</",iss)) // only continue if node isnt empty
    {
        if(Check("<",iss)) // get child nodes
        {
            while(!Check("</",iss))
            {
                XMLNode *child = new XMLNode(iss);
                children.push_back(child);
                iss >> ws;
            }
        }
        else // text node
        {
            text = GrabString('<',iss);
        }
    }
    Eat("</",iss);
    GrabName(iss); // check?
    Eat(">",iss);
}

// ****************************************************************************
//  Method:  XMLNode::Check
//
//  Purpose:
//     Checks if the passed characters exist at the front of the input stream.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

bool 
XMLNode::Check(const string&str,istream &iss)
{
    // see of first part of the string buffer == the input string
    int i=0;
    int str_len = str.size();
    stack<unsigned char> buff;
    while( !iss.eof() && i < str_len && str[i]==iss.peek())
    {
        buff.push(iss.get());
        i++;
    }
    while(buff.size()>0)
    {
        iss.putback(buff.top());
        buff.pop();
    }
        
    return i == str_len;
}

// ****************************************************************************
//  Method:  XMLNode::Eat
//
//  Purpose:
//     Eats passed characters from the input stream.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

void 
XMLNode::Eat(const string &str,istream &iss)
{
    // eat string from stream
    int i=0;
    int str_len = str.size();
    while(!iss.eof() && i < str_len && str[i]==iss.peek())
    {
        iss.get();
        i++;
    }
}

// ****************************************************************************
//  Method:  XMLNode::GrabName
//
//  Purpose:
//     Grabs from input stream until [whitespace]=<> .
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

string
XMLNode::GrabName(istream &iss)
{
    string res = "";
    unsigned char c = iss.get();
    while( !iss.eof() && 
           c != ' ' && c != '\t' && c != '\n' &&
           c != '=' && c != '<'  && c != '>'  )
    {
        res += c;
        c = iss.get();
    }
    if(!iss.eof())
        iss.putback(c);
    return res;
}

// ****************************************************************************
//  Method:  XMLNode::GrabString
//
//  Purpose:
//     Grabs string data until it encounters passed terminating character.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

string 
XMLNode::GrabString(unsigned char term,istream &iss)
{
    string res = "";
    unsigned char c = iss.get();
    while( !iss.eof() && c != term)
    {
        // check for entity values
        if(c == '&')
        {res += GrabEntityValue(iss);}
        else
        {res.push_back(c);}
        c = iss.get();
    }
    if(!iss.eof())
        iss.putback(c);

    return res;
}

// ****************************************************************************
//  Method:  XMLNode::GrabEntityValue
//
//  Purpose:
//     Reads in special xml encoded values. Assumes '&' has already been 
//     removed.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************

string
XMLNode::GrabEntityValue(istream &iss)
{
    // handles standard xml entities:
    // &lt;    <
    // &gt;    >
    // &amp;   &
    // &apos;  '
    // $quot;  "

    string val = "";    
    unsigned char c = iss.get();
    while( !iss.eof() && c != ';')
    {
        val.push_back(c);
        c = iss.get();
    }

    if(val == "lt")
    {return "<";}
    else if(val == "gt")
    {return ">";}
    else if(val == "amp")
    {return "&";}
    else if(val == "apos")
    {return "'";}
    else if(val == "quot")
    {return "\"";}
    else
    {return "[<unsupported entity>]";}
}


// ****************************************************************************
//  Method:  XMLNode::EscapeString
//
//  Purpose:
//     Removes invalid xml characters and replaces them with proper xml 
//     entities values.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************

string
XMLNode::EscapeString(const string &val)
{
    // replace with standard xml entities:
    // < &lt;    
    // > &gt;    
    // & &amp;
    // ' &apos;
    // " $quot;
    
    string res="";
    int ssize = val.size();
    for(int i=0;i<ssize;i++)
    {
        if(val[i] == '<')
        {res += string("&lt;");}
        else if(val[i] == '>')
        {res += string("&gt;");}
        else if(val[i] == '&')
        {res += string("&amp;");}
        else if(val[i] == '\'')
        {res += string("&apos;");}
        else if(val[i] == '"')
        {res += string("&quote;");}
        else
        {res.push_back(val[i]);} 
    }
    return res;
}


// ****************************************************************************
//  Method:  XMLNode::Init
//
//  Purpose:
//     Replicates the passed node.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

void 
XMLNode::Init(const XMLNode &node)
{
    if(children.size() > 0 || attributes.size() >0)
        Cleanup();
        
    name = node.name;
    text = node.text;
    attributes = node.attributes;
    
    int nchildren = node.GetNumChildren();
    for(int i=0;i<nchildren;i++)
        children.push_back(new XMLNode(*node.children[i]));
}



// ****************************************************************************
//  Method:  XMLNode::Cleanup
//
//  Purpose:
//     Clean up child nodes.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

void 
XMLNode::Cleanup()
{
    int nchildren = GetNumChildren();
    for(int i=0;i<nchildren;i++)
        delete children[i];
    children.clear();
    attributes.clear();
}

