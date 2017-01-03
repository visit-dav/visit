/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
#include <limits>

#include <VisItException.h>

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
    iss >> std::noskipws; /// don't skip white spaces..
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

JSONNode::JSONNode(const boolVector& v)
{
    *this = v;
}

JSONNode::JSONNode(const charVector& v)
{
    *this = v;
}

JSONNode::JSONNode(const unsignedCharVector& v)
{
    *this = v;
}

JSONNode::JSONNode(const intVector& v)
{
    *this = v;
}

JSONNode::JSONNode(const longVector& v)
{
    *this = v;
}

JSONNode::JSONNode(const floatVector& v)
{
    *this = v;
}

JSONNode::JSONNode(const doubleVector& v)
{
    *this = v;
}

JSONNode::JSONNode(const stringVector& v)
{
    *this = v;
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
    //char buf[1024];
    std::ostringstream ostr;
    if(type == JSONNULLVALUE)
        return "null";

    if(type == JSONBOOL)
        return GetBool() ? "true" : "false";

    if(type == JSONSTRING)
    {
        std::string str = "\"" + EscapeString(json.str) + "\"";
        return str;
    }

//    if(type == JSONINTEGER)
//    {
//        ostr << GetInt();
//        return ostr.str();
//    }
    if(type == JSONLONG)
    {
        ostr << GetLong();
        return ostr.str();
    }
//    if(type == JSONFLOAT)
//    {
//        ostr << GetFloat();
//        return ostr.str();
//    }
    if(type == JSONDOUBLE)
    {
        ostr << GetDouble();
        return ostr.str();
    }

    if(type == JSONARRAY)
    {
        std::string output = "[";
        for(size_t i = 0; i < json.array.size(); ++i)
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

        size_t index = 0;
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

size_t JSONNode::Parse(const std::string &JSON_data)
{
    if(JSON_data == "")
        return 0;
    std::istringstream iss;
    iss >> std::noskipws; /// don't skip white spaces..
    iss.str(JSON_data);
    return Parse(iss);
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

size_t
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

    //std::cout << iss.tellg() << std::endl;
    return (size_t)iss.tellg();
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
        throw VisItException("JSON ParseObject Failed");
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

    iss >> os; /// read first character..
    std::string key = "";
    //read values as long as '"' is not found and if it is
    //found then it cannot be preceded by '\'

    while(!iss.eof())
    {
        if(os == '\\')
        {
            key += os; /// store backslash
            iss >> os;

            key += os; /// consume the backslashed character..
            iss >> os; /// eat char
        }
        else
        {
            if(os == '"')
                break;

            key += os;
            iss >> os; /// eat char
        }
    }

    //std::cout << key << " " << os << " " << std::endl;
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
    if( ((char)iss.peek()) == ']' )
    {
        iss >> os; // consume end bracket..
    }
    else
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
        throw VisItException("JSON ParseObject Failed");
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
    if(((char)iss.peek()) == '"')
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
    /*
     replace with standard JSON entities:
     \"
     \\
     \/
     \b
     \f
     \n
     \r
     \t
     \u four-hex-digits
    */
    
    std::string res="";
    size_t ssize = val.size();
    for(size_t i=0;i<ssize;i++)
    {
        if(val[i] == '"')
        {res += std::string("\\\"");}
        else if(val[i] == '\\')
        {res += std::string("\\");}
        else if(val[i] == '/')
        {res += std::string("\\/");}
        else if(val[i] == '\b')
        {res += std::string("\\b");}
        else if(val[i] == '\f')
        {res += std::string("\\f");}
        else if(val[i] == '\n')
        {res += std::string("\\n");}
        else if(val[i] == '\r')
        {res += std::string("\\r");}
        else if(val[i] == '\t')
        {res += std::string("\\t");}
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

template<typename T>
JSONNode&
JSONNode::convertArray(const T& v) {
    Cleanup();
    type = JSONARRAY;

    /// empty array..
    if(v.size() == 0) return *this;

    json.array.resize(v.size());

    for(size_t i = 0; i < v.size(); ++i)
        json.array[i] = v[i];

    return *this;
}

JSONNode&
JSONNode::operator=(const JSONArray& v)
{
    return convertArray<JSONArray>(v);
}

JSONNode&
JSONNode::operator=(const boolVector& v)
{
    return convertArray<boolVector>(v);
}

JSONNode&
JSONNode::operator=(const charVector& v)
{
    return convertArray<charVector>(v);
}

JSONNode&
JSONNode::operator=(const unsignedCharVector& v)
{
    return convertArray<unsignedCharVector>(v);
}

JSONNode&
JSONNode::operator=(const intVector& v)
{
    return convertArray<intVector>(v);
}

JSONNode&
JSONNode::operator=(const longVector& v)
{
    return convertArray<longVector>(v);
}

JSONNode&
JSONNode::operator=(const floatVector& v)
{
    return convertArray<floatVector>(v);
}

JSONNode&
JSONNode::operator=(const doubleVector& v)
{
    return convertArray<doubleVector>(v);
}

JSONNode&
JSONNode::operator=(const stringVector& v)
{
    return convertArray<stringVector>(v);
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
            index = (int)json.array.size() - index;
            /// if index is still less than 0..
            if(index < 0) index = 0;
        }

        if((size_t)index < json.array.size())
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

bool
JSONNode::AsBool() const{
    bool res;

    JSONType type = GetType();
    if(type == JSONBOOL)
        res = GetBool();
    else if(type == JSONNUMBER)
        res = (bool) GetInt();
    else if(type == JSONSTRING)
        res = GetString().size() == 0;
    else /// array or object is true..
        res = false;

    return res;
}

char
JSONNode::AsChar() const{
    return (char) AsUnsignedChar();
}

unsigned char
JSONNode::AsUnsignedChar() const{
    unsigned char res;

    JSONType type = GetType();

    if(type == JSONBOOL)
        res = (unsigned char)GetBool();
    else if(type == JSONNULLVALUE)
        res = ' ';
    else if(type == JSONNUMBER)
        res = (unsigned char) GetInt();
    else if(type == JSONSTRING)
        res = GetString().size() == 0 ? ' ' : GetString()[0];
    else
        res = ' ';

    return res;
}

int
JSONNode::AsInt() const{
    return (int)AsLong();
}

long
JSONNode::AsLong() const{
    long res;

    JSONType type = GetType();

    if(type == JSONBOOL)
        res = (long)GetBool();
    else if(type == JSONINTEGER)
        res = GetLong();
    else if(type == JSONDOUBLE)
        res = (long) GetDouble();
    else if(type == JSONSTRING)
        res = atol(GetString().c_str());
    else
        res = 0;

    return res;
}

float
JSONNode::AsFloat() const{
    return (float) AsDouble();
}

double
JSONNode::AsDouble() const{
    double res;

    JSONType type = GetType();

    if(type == JSONBOOL)
        res = (double)GetBool();
    else if(type == JSONINTEGER)
        res = (double) GetLong();
    else if(type == JSONDOUBLE)
        res = (double) GetDouble();
    else if(type == JSONSTRING)
        res = (double)atof(GetString().c_str());
    else
        res = 0;

    return res;
}

std::string
JSONNode::AsString() const{
    std::ostringstream res;

    JSONType type = GetType();

    if(type == JSONBOOL)
        res << std::string(GetBool() ? "true" : "false");
    else if(type == JSONINTEGER)
        res << (double) GetLong();
    else if(type == JSONDOUBLE)
        res << (double) GetDouble();
    else if(type == JSONSTRING)
        res << GetString();
    else
        res << "";

    return res.str();
}

boolVector
JSONNode::AsBoolVector() const{
    boolVector v;

    if(GetType() == JSONARRAY)
    {
        v.resize(json.array.size());

        for(size_t i = 0; i < json.array.size(); ++i)
            v[i] = json.array[i].AsBool();
    }
    else {
        v.push_back(AsBool());
    }
    return v;
}

charVector
JSONNode::AsCharVector() const{
    charVector v;

    if(GetType() == JSONARRAY)
    {
        v.resize(json.array.size());

        for(size_t i = 0; i < json.array.size(); ++i)
            v[i] = json.array[i].AsChar();
    }
    else {
        v.push_back(AsChar());
    }
    return v;
}

unsignedCharVector
JSONNode::AsUnsignedCharVector() const{
    unsignedCharVector v;

    if(GetType() == JSONARRAY)
    {
        v.resize(json.array.size());

        for(size_t i = 0; i < json.array.size(); ++i)
            v[i] = json.array[i].AsUnsignedChar();
    }
    else {
        v.push_back(AsUnsignedChar());
    }
    return v;
}

intVector
JSONNode::AsIntVector() const{
    intVector v;

    if(GetType() == JSONARRAY)
    {
        v.resize(json.array.size());

        for(size_t i = 0; i < json.array.size(); ++i)
            v[i] = json.array[i].AsInt();
    }
    else {
        v.push_back(AsInt());
    }
    return v;
}

longVector
JSONNode::AsLongVector() const{
    longVector v;

    if(GetType() == JSONARRAY)
    {
        v.resize(json.array.size());

        for(size_t i = 0; i < json.array.size(); ++i)
            v[i] = json.array[i].AsLong();
    }
    else {
        v.push_back(AsLong());
    }
    return v;
}

floatVector
JSONNode::AsFloatVector() const{
    floatVector v;

    if(GetType() == JSONARRAY)
    {
        v.resize(json.array.size());

        for(size_t i = 0; i < json.array.size(); ++i)
            v[i] = json.array[i].AsFloat();
    }
    else {
        v.push_back(AsFloat());
    }
    return v;
}

doubleVector
JSONNode::AsDoubleVector() const{
    doubleVector v;

    if(GetType() == JSONARRAY)
    {
        v.resize(json.array.size());

        for(size_t i = 0; i < json.array.size(); ++i)
            v[i] = json.array[i].AsDouble();
    }
    else {
        v.push_back(AsDouble());
    }
    return v;
}

stringVector
JSONNode::AsStringVector() const{
    stringVector v;

    if(GetType() == JSONARRAY)
    {
        v.resize(json.array.size());

        for(size_t i = 0; i < json.array.size(); ++i)
            v[i] = json.array[i].AsString();
    }
    else {
        v.push_back(AsString());
    }
    return v;
}
