/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <JSONNode.h>

#include <visitstream.h>
#include <stack>
#include <cstdlib>

// ****************************************************************************
//  Method:  JSONNode::JSONNode
//
//  Purpose:
//
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************
JSONNode::JSONNode():type(JSONNULLVALUE)
{
}

// ****************************************************************************
//  Method:  JSONNode::JSONNode
//
//  Purpose:
//
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************
//JSONNode::JSONNode(const std::string &JSON_data):type(JSONNULLVALUE)
//{
//    Parse(JSON_data);
//}

// ****************************************************************************
//  Method:  JSONNode::JSONNode
//
//  Purpose:
//
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************
JSONNode::JSONNode(istream &iss):type(JSONNULLVALUE)
{
    Parse(iss);
}


// ****************************************************************************
//  Method:  JSONNode::JSONNode
//
//  Purpose:
//
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************
JSONNode::JSONNode(const JSONNode &node):type(JSONNULLVALUE)
{
    Init(node);
}

JSONNode::JSONNode(const JSONNode *node):type(JSONNULLVALUE)
{
    Init(*node);
}

JSONNode::JSONNode(const bool &v)
{
    *this = v;
}

JSONNode::JSONNode(const int& v)
{
    *this = v;
}

JSONNode::JSONNode(const long& v)
{
    *this = v;
}

JSONNode::JSONNode(const float& v)
{
    *this = v;
}

JSONNode::JSONNode(const double& v)
{
    *this = v;
}

JSONNode::JSONNode(const char* v)
{
    *this = std::string(v);
}

JSONNode::JSONNode(const std::string& v)
{
    *this = v;
}

JSONNode::JSONNode(const JSONNode::JSONArray& array)
{
    *this = array;
}

JSONNode::JSONNode(const JSONNode::JSONObject& obj)
{
    *this = obj;
}

// ****************************************************************************
//  Method:  JSONNode::operator=
//
//  Purpose:
//
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************
JSONNode &
JSONNode::operator=(const JSONNode &node)
{
    if(this != &node)
        Init(node);

    return *this;
}

// ****************************************************************************
//  Method:  JSONNode::~JSONNode
//
//  Purpose:
//     JSONNode destructor.
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************
JSONNode::~JSONNode()
{
    Cleanup();
}

// ****************************************************************************
//  Method:  JSONNode::ToString
//
//  Purpose:
//     Creates an JSON string from this node.
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************

std::string
JSONNode::ToString(const std::string &indent) const
{
    char buf[1024];

    if(type == JSONNULLVALUE)
        return "null";

    if(type == JSONBOOL)
        return json.num.boolValue ? "true" : "false";

    if(type == JSONSTRING)
    {
        std::string str = "\"" + EscapeString(json.str) + "\"";
        return str;
    }

    if(type == JSONINTEGER)
    {
        sprintf(buf,"%d",(int)json.num.lnumber);
        return buf;
    }

    if(type == JSONLONG)
    {
        sprintf(buf,"%ld",json.num.lnumber);
        return buf;
    }

    if(type == JSONFLOAT)
    {
        sprintf(buf,"%f",(float)json.num.dnumber);
        return buf;
    }

    if(type == JSONDOUBLE)
    {
        sprintf(buf,"%f",json.num.dnumber);
        return buf;
    }

    if(type == JSONARRAY)
    {
        std::string output = "[ ";
        for(int i = 0; i < json.array.size(); ++i)
        {
            output += json.array[i].ToString();

            if(i != json.array.size() - 1)
                output += ",";
        }
        output += "]";

        return output;
    }

    if(type == JSONOBJECT)
    {
        std::string output = "{";

        int index = 0;
        for(JSONObject::const_iterator itr = json.object.begin();
              itr != json.object.end(); ++itr)
        {
            std::string pair = "\"" + itr->first + "\"";
            pair += ":";
            pair += itr->second.ToString();

            output += pair;

            if(index != json.object.size() - 1)
                output += ",";

            ++index;
        }

        output += "}";
        return output;
    }

    /// default returns nothing..
    return "";
}


// ****************************************************************************
//  Method:  JSONNode::Parse
//
//  Purpose:
//     Parses an JSON string into a node.
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************

void 
JSONNode::Parse(const std::string &JSON_data)
{
    if(JSON_data == "")
        return;
    std::istringstream iss;
    iss.str(JSON_data);
    Parse(iss);
}

// ****************************************************************************
//  Method:  JSONNode::Parse
//
//  Purpose:
//     Parses an JSON string into a node.
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************

void 
JSONNode::Parse(std::istream &iss)
{
    iss >> std::ws;

    char val = (char)iss.peek();

    if(val == '{')
        ParseObject(iss);
    else if(val == '[')
        ParseArray(iss);
    else
        ParseVariant(iss);
}

void
JSONNode::ParseObject(std::istream &iss)
{

//    std::cout << "ParseObject" << std::endl;
    iss >> std::ws;

    /// object = { key1 : value1, ..., keyN : valueN}
    char os;
    iss >> os; /// start object
    iss >> std::ws;

    if((char)iss.peek() != '}')
    {
        do
        {
            /// Parse the Key
            std::string key = ParseKey(iss);
            //std::cout << "Key : " << key << std::endl;

            iss >> std::ws; // consume white space..

            /// if successful parse value..
            if((char)iss.peek() == ':')
            {
                iss >> os;
                json.object.insert(std::pair<std::string,JSONNode*>(key,new JSONNode(iss)));
            }
            else
            {
                std::cerr << "Invalid JSON text: " << key << std::endl;
            }
            iss >> std::ws;
            iss >> os; /// read
        }
        while( !iss.eof() && os == ',');
    }

    if(os != '}')
    {
        std::cout << "JSON ParseObject Failed " << os << std::endl;
    }

    iss >> std::ws;
    type = JSONOBJECT;
}

std::string
JSONNode::ParseKey(std::istream & iss)
{
    //eat up any whitespace
//    std::cout << "ParseKey" << std::endl;
    iss >> std::ws;

    /// key = "key"
    char os;
    iss >> os; /// eat quote..

    std::string key = "";
    //read values as long as '"' is not found and if it is
    //found then it cannot be preceded by '\'
    while(!iss.eof() && ((char)iss.peek()) != '\"' && os != '\\')
    {
        iss >> os; /// eat char
        key += os;
    }
    iss >> std::ws;
    iss >> os; /// eat end quote..

    return key;
}

void
JSONNode::ParseArray(std::istream &iss)
{

//    std::cout << "ParseArray" << std::endl;
    iss >> std::ws;

    /// [ object1, ... object2 ]
    char os;
    iss >> os; //start bracket

    iss >> std::ws;

    /// check for empty array..
    if( ((char)iss.peek()) != ']' )
    {
        do
        {
            JSONNode node(iss);
            json.array.push_back(node);
            iss >> std::ws;
            iss >> os;
        } while(!iss.eof() && os == ',');
    }
    //iss >> os; //end bracket
    if(os != ']')
    {
        std::cout << "JSON ParseArray Failed " << os << std::endl;
    }
    iss >> std::ws;
    type = JSONARRAY;
}

void
JSONNode::ParseVariant(std::istream &iss)
{
    /// if quote then read between quotes..
    std::string key = "";

    iss >> std::ws;

    /// if string
    if(((char)iss.peek()) == '\"')
    {
        json.str = ParseKey(iss);
        //std::cout << "Got Value: " << json.str << std::endl;
        type = JSONSTRING;
        return;
    }

    /// so values are not a string then they
    /// can be true, false, null, or a number...
    /// store everything until
    /// either a "," or "}" or "]" or " " or eof...
    /// am I missing any other possible character?
    char os;
    while(!iss.eof() &&
          (char)iss.peek() != ',' &&
          (char)iss.peek() != ']' &&
          (char)iss.peek() != '}' &&
          (char)iss.peek() != ' ')
    {
        iss >> os;
        key += os;
    }

    /// now check to see if it is true, false or null
    //std::cout << "Got Value:" << key << ":" << ((char)iss.peek()) << std::endl;
    if(key == "true" || key == "false")
    {
        json.num.boolValue = ( key == "true" ? true : false);
        type = JSONBOOL;
    }
    else if(key == "null")
    {
        json.num.nullValue = true;
        type = JSONNULLVALUE;
    }
    else /// must now be a number..
    {
        if(key.find('.') != std::string::npos) /// float or double
        {
            /// store as double..
            std::istringstream tmp(key);
            tmp >> json.num.dnumber;
            type = JSONDOUBLE;
        }
        else
        {
            /// store as long..
            std::istringstream tmp(key);
            tmp >> json.num.lnumber;
            type = JSONLONG;
        }
    }

    /// read rest of potential whitespaces for this tag..
    iss >> std::ws;
}


std::string
JSONNode::EscapeString(const std::string &val) const
{
    // replace with standard JSON entities:
    // \"
    // \\
    // \/
    // \b
    // \f
    // \n
    // \r
    // \t
    // \u four-hex-digits
    
    std::string res="";
    int ssize = val.size();
    for(int i=0;i<ssize;i++)
    {
        if(val[i] == '"')
        {res += std::string("\\\"");}
        else
        {res.push_back(val[i]);} 
    }
    return res;
}


// ****************************************************************************
//  Method:  JSONNode::Init
//
//  Purpose:
//     Replicates the passed node.
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************

void 
JSONNode::Init(const JSONNode &node)
{
    JSONType type = node.type;
    if(type == JSONSTRING)
        *this = node.json.str;
    if(type == JSONBOOL)
        *this = node.json.num.boolValue;
    if(type == JSONINTEGER)
        *this = (int)node.json.num.lnumber;
    if(type == JSONLONG)
        *this = node.json.num.lnumber;
    if(type == JSONFLOAT)
        *this = (float)node.json.num.dnumber;
    if(type == JSONDOUBLE)
        *this = node.json.num.dnumber;

    if(type == JSONARRAY)
        *this = node.json.array;
    if(type == JSONOBJECT)
        *this = node.json.object;
}



// ****************************************************************************
//  Method:  JSONNode::Cleanup
//
//  Purpose:
//     Clean up child nodes.
//
//  Programmer:  Hari Krishnan
//  Creation:    October 13, 2012
//
// ****************************************************************************

void 
JSONNode::Cleanup()
{
    type = JSONNULLVALUE;
    json.num.nullValue = true;
    json.array.clear();
    json.object.clear();
}

//void
//JSONNode::SetNullValue()
//{
//    Cleanup();
//}


/// Pointers only accepts NULL;

//JSONNode&
//JSONNode::operator=(const void*& v)
//{
//    Cleanup();
//}

JSONNode&
JSONNode::operator=(const bool& v)
{
    Cleanup();
    type = JSONBOOL;
    json.num.boolValue = v;
    return *this;
}

JSONNode&
JSONNode::operator=(const int& v)
{
    Cleanup();
    type = JSONLONG;
    json.num.lnumber =(long) v;
    return *this;
}


JSONNode&
JSONNode::operator=(const long& v)
{
    Cleanup();
    type = JSONLONG;
    json.num.lnumber = v;
    return *this;
}

JSONNode&
JSONNode::operator=(const float& v)
{
    Cleanup();
    type = JSONDOUBLE;
    json.num.dnumber = (double)v;
    return *this;
}

JSONNode&
JSONNode::operator=(const double& v)
{
    Cleanup();
    type = JSONDOUBLE;
    json.num.dnumber = v;
    return *this;
}

JSONNode&
JSONNode::operator=(const char* v)
{
    Cleanup();
    type = JSONSTRING;
    json.str = v;
    return *this;
}


JSONNode&
JSONNode::operator=(const std::string& v)
{
    Cleanup();
    type = JSONSTRING;
    json.str = v;
    return *this;
}


//JSONNode&
//JSONNode::operator=(const Variant& v)
//{
//    Cleanup();

//    if(v.Type() == Variant::BOOL_TYPE)
//        type = JSONBOOL;
//    else if(v.Type() == Variant::STRING_TYPE)
//        type = JSONSTRING;
//    else if(v.Type() == Variant::INT_TYPE ||
//            v.Type() == Variant::FLOAT_TYPE ||
//            v.Type() == Variant::DOUBLE_TYPE)
//        type = JSONNUMBER;
//    else //no other type is allowed..
//        type = JSONNULLVALUE;

//    json.value = v;

//    return *this;
//}

JSONNode&
JSONNode::operator=(const JSONArray& v)
{
    Cleanup();
    type = JSONARRAY;

    /// empty array..
    if(v.size() == 0) return *this;

    json.array.resize(v.size());

    for(int i = 0; i < v.size(); ++i)
        json.array[i] = v[i];

    return *this;
}

JSONNode&
JSONNode::operator=(const JSONObject& v)
{
    Cleanup();

    type = JSONOBJECT;

    if(v.size() == 0) return *this;

    for(JSONObject::const_iterator itr = v.begin();
        itr != v.end(); ++itr)
    {
        std::string key = itr->first;
        json.object[key] = itr->second;
    }
    return *this;
}

/// for JSONArray or JSONObject
JSONNode&
JSONNode::operator[](int index)
{
    if(type == JSONARRAY)
    {
        if(index < 0)
        {
            index = json.array.size() - index;
            /// if index is still less than 0..
            if(index < 0) index = 0;
        }

        if(index < json.array.size())
            return json.array[index];
        else
        {
            json.array.resize(index+1);
            return json.array[index];
        }
    }

    Cleanup();

    *this = JSONArray();
    JSONNode& node = *this;

    return node[index];
}

/// for JSONObject
JSONNode&
JSONNode::operator[](const std::string& key)
{
    if(type == JSONOBJECT)
       return json.object[key];

    /// give warning if types are different?
    Cleanup();

    /// switch to object
    *this = JSONObject();

    return json.object[key];
}

bool
JSONNode::HasKey(std::string key)
{
    if(type != JSONOBJECT) return false;
    return json.object.find(key) != json.object.end();
}

/// array values
bool
JSONNode::Append(const JSONNode &node)
{
    if(type != JSONARRAY) return false;

    json.array.push_back(node);
    return true;
}

bool
JSONNode::Append(const JSONArray &array)
{
    if(type != JSONARRAY) return false;

    json.array.push_back(array);
    return true;
}

/// size..
size_t
JSONNode::Length()
{
    size_t len = 1;
    if(type == JSONOBJECT)
        len = json.object.size();
    else if(type == JSONARRAY)
        len = json.array.size();

    return len;
}

