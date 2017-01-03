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

#ifndef JSON_NODE_H
#define JSON_NODE_H
#include <state_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include <vector>
#include <map>

// ****************************************************************************
//  Class:  JSONNode
//
//  Purpose:
//    Very basic tree based JSON generator and parser.
//
//  Programmer:  Hari Krishnan
//  Creation:    December 13, 2007
//
//  Modifications:
//    Kathleen Biagas Fri Nov 16 04:45:15 MST 2012
//    Renamed 'GetObject' to 'GetJsonObject' to avoid name conflict on Windows.
//
// ****************************************************************************

class STATE_API JSONNode
{
  public:

    enum JSONType
    {
        JSONLONG = 0x1,
        JSONDOUBLE = 0x2,
        JSONSTRING = 0x4,
        JSONARRAY = 0x8,
        JSONOBJECT = 0x10,
        JSONBOOL = 0x20,
        JSONNULLVALUE = 0x40,
        JSONINTEGER = JSONLONG,
        JSONFLOAT = JSONDOUBLE,
        JSONNUMBER = JSONLONG | JSONDOUBLE
    };

    union number
    {
        bool nullValue;
        bool boolValue;
        long lnumber;
        double dnumber;
    };

    struct JSONStruct
    {
        /// a JSON object can be any of these types..
        number num;
        std::string str;
        std::vector<JSONNode> array;
        std::map<std::string,JSONNode> object;

        JSONStruct() { num.nullValue = true; }
    };

    typedef std::vector<JSONNode> JSONArray;
    typedef std::map<std::string, JSONNode> JSONObject;

    JSONNode();
    JSONNode(const JSONNode&);
    JSONNode(const JSONNode*);
    JSONNode(std::istream &);
    JSONNode(const char*);
    JSONNode(const std::string&);
    JSONNode(const bool &v);
    JSONNode(const int& v);
    JSONNode(const long& v);
    JSONNode(const float& v);
    JSONNode(const double& v);
    JSONNode(const JSONNode::JSONArray& array);
    JSONNode(const JSONNode::JSONObject& obj);
    JSONNode(const boolVector& v);
    JSONNode(const charVector& v);
    JSONNode(const unsignedCharVector& v);
    JSONNode(const intVector& v);
    JSONNode(const longVector& v);
    JSONNode(const floatVector& v);
    JSONNode(const doubleVector& v);
    JSONNode(const stringVector& v);
    virtual  ~JSONNode();

    JSONType GetType() const { return type; }

    bool isNullValue()   const { return json.num.nullValue; }
    bool   GetBool()    const { return json.num.boolValue; }
    int    GetInt()       const { return (int)GetLong(); }
    long   GetLong()      const { return json.num.lnumber; }
    float  GetFloat()     const { return (float)GetDouble(); }
    double GetDouble() const { return type == JSONINTEGER || type == JSONLONG ?
                                        (double)json.num.lnumber :
                                                json.num.dnumber; }

    std::string GetString() const { return json.str; }
    JSONArray& GetArray() { return json.array; }
    JSONObject& GetJsonObject() { return json.object; }

    bool               AsBool()               const;
    char               AsChar()               const;
    unsigned char      AsUnsignedChar()       const;
    int                AsInt()                const;
    long               AsLong()               const;
    float              AsFloat()              const;
    double             AsDouble()             const;
    std::string        AsString()             const;
    boolVector         AsBoolVector()         const;
    charVector         AsCharVector()         const;
    unsignedCharVector AsUnsignedCharVector() const;
    intVector          AsIntVector()          const;
    longVector         AsLongVector()         const;
    floatVector        AsFloatVector()        const;
    doubleVector       AsDoubleVector()       const;
    stringVector       AsStringVector()       const;

//    bool isNullValue()  { return json.num.nullValue; }
//    bool   GetBool()     { return json.num.boolValue; }
//    int    GetInt()      { return json.num.integer; }
//    long   GetLong()     { return json.num.lnumber; }
//    float  GetFloat()    { return json.num.fnumber; }
//    double GetDouble()   { return json.num.dnumber; }
//    std::string GetString() { return json.str; }
    const JSONArray& GetArray() const { return json.array; }
    const JSONObject& GetJsonObject() const { return json.object; }

    void SetNullValue();
    //JSONNode  *operator=(const void*& v);
    JSONNode  &operator=(const bool& v);
    JSONNode  &operator=(const int& v);
    JSONNode  &operator=(const long& v);
    JSONNode  &operator=(const float& v);
    JSONNode  &operator=(const double& v);
    JSONNode  &operator=(const char* v);
    JSONNode  &operator=(const std::string& v);
    JSONNode  &operator=(const JSONArray& v);
    JSONNode  &operator=(const JSONObject& v);
    JSONNode  &operator=(const JSONNode& v);
    JSONNode  &operator=(const boolVector& v);
    JSONNode  &operator=(const charVector& v);
    JSONNode  &operator=(const unsignedCharVector& v);
    JSONNode  &operator=(const intVector& v);
    JSONNode  &operator=(const longVector& v);
    JSONNode  &operator=(const floatVector& v);
    JSONNode  &operator=(const doubleVector& v);
    JSONNode  &operator=(const stringVector& v);

    /// for JSONArray
    JSONNode  &operator[](int index);

    /// for JSONObject
    JSONNode  &operator[](const std::string& key);

    // for JSON Const
    //const JSONNode  &operator[](int index) const;

    /// for JSONObject
    //JSONNode  operator[](const std::string& key) const;

    std::string   ToString(const std::string &indent="") const;

    /// size..
    size_t Length();

    /// array values
    bool Append(const JSONNode& node);
    bool Append(const JSONArray& array);

    ///object values..
    bool HasKey(std::string key);

    // JSON parsing methods
    size_t Parse(istream &);
    size_t Parse(const std::string &);

private:
    std::string EscapeString(const std::string &val) const;
    template<typename T>
    JSONNode& convertArray(const T& v);

    // init & destroy helpers
    void    Init(const JSONNode&);
    void    Cleanup();
    


    void    ParseObject(std::istream &iss);
    void    ParseArray(std::istream &iss);
    void    ParseVariant(std::istream &iss);
    std::string ParseKey(std::istream &iss);

    JSONType type;
    JSONStruct json;
};

#endif


