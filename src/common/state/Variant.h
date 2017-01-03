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

#ifndef VARIANT_H
#define VARIANT_H
#include <state_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include <XMLNode.h>
#include <JSONNode.h>

class Connection;

// ****************************************************************************
//  Class:  Variant
//
//  Purpose:
//    Basic variant class.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 10, 2007
//
//  Modifications:
//    Brad Whitlock, Tue Jan  6 15:16:19 PST 2009
//    I added methods that let it read/write itself using Connection.
//
//    Kathleen Biagas, Wed Jul 13 10:15:33 PDT 2011
//    Added ConvertToString, takes a basic type and returns a string
//    respresentation (useful for logging).
//
//    Cyrus Harrison, Wed Jan  9 14:16:38 PST 2013
//    Added conversion helpers for numeric types (ToFloat,ToFloatVector,...etc)
//
// ****************************************************************************

class STATE_API Variant
{
  public:
    typedef enum
    {
        EMPTY_TYPE = 0,
        BOOL_TYPE, CHAR_TYPE, UNSIGNED_CHAR_TYPE, INT_TYPE, LONG_TYPE,
        FLOAT_TYPE, DOUBLE_TYPE, STRING_TYPE,
        BOOL_VECTOR_TYPE, CHAR_VECTOR_TYPE, UNSIGNED_CHAR_VECTOR_TYPE,
        INT_VECTOR_TYPE, LONG_VECTOR_TYPE, FLOAT_VECTOR_TYPE,
        DOUBLE_VECTOR_TYPE, STRING_VECTOR_TYPE, ID__LAST
    } VariantTypeEnum;

    Variant();
    Variant(const Variant &);
    Variant(const XMLNode &,bool decodeString = true);
    Variant(const XMLNode *,bool decodeString = true);
    Variant(const JSONNode &,const JSONNode &,bool decodeString = true);
    Variant(const JSONNode *,const JSONNode *,bool decodeString = true);
    Variant(bool);
    Variant(char);
    Variant(unsigned char);
    Variant(const char*); // interp as string
    Variant(int);
    Variant(long);
    Variant(float);
    Variant(double);
    Variant(const std::string&);
    Variant(const charVector&);
    Variant(const unsignedCharVector&);
    Variant(const intVector&);
    Variant(const longVector&);
    Variant(const floatVector&);
    Variant(const doubleVector&);
    Variant(const boolVector&);
    Variant(const stringVector&);
    virtual ~Variant();

    Variant                  &operator=(const Variant&);
    Variant                  &operator=(const XMLNode&);
    //Variant                  &operator=(const JSONNode&,const JSONNode&);
    Variant                  &operator=(bool);
    Variant                  &operator=(char);
    Variant                  &operator=(unsigned char);
    Variant                  &operator=(const char *); // interp as string
    Variant                  &operator=(int);
    Variant                  &operator=(long);
    Variant                  &operator=(float);
    Variant                  &operator=(double);
    Variant                  &operator=(const std::string &);
    Variant                  &operator=(const boolVector &);
    Variant                  &operator=(const charVector &);
    Variant                  &operator=(const unsignedCharVector &);
    Variant                  &operator=(const intVector &);
    Variant                  &operator=(const longVector &);
    Variant                  &operator=(const floatVector &);
    Variant                  &operator=(const doubleVector &);

    Variant                  &operator=(const stringVector &);

    bool                      operator ==(const Variant &obj) const;

    int                       Type()     const { return dataType;}
    std::string               TypeName() const;

    bool                     &AsBool();
    char                     &AsChar();
    unsigned char            &AsUnsignedChar();
    int                      &AsInt();
    long                     &AsLong();
    float                    &AsFloat();
    double                   &AsDouble();
    std::string              &AsString();
    boolVector               &AsBoolVector();
    charVector               &AsCharVector();
    unsignedCharVector       &AsUnsignedCharVector();
    intVector                &AsIntVector();
    longVector               &AsLongVector();
    floatVector              &AsFloatVector();
    doubleVector             &AsDoubleVector();
    stringVector             &AsStringVector();

    const bool               &AsBool()               const;
    const char               &AsChar()               const;
    const unsigned char      &AsUnsignedChar()       const;
    const int                &AsInt()                const;
    const long               &AsLong()               const;
    const float              &AsFloat()              const;
    const double             &AsDouble()             const;
    const std::string        &AsString()             const;
    const boolVector         &AsBoolVector()         const;
    const charVector         &AsCharVector()         const;
    const unsignedCharVector &AsUnsignedCharVector() const;
    const intVector          &AsIntVector()          const;
    const longVector         &AsLongVector()         const;
    const floatVector        &AsFloatVector()        const;
    const doubleVector       &AsDoubleVector()       const;
    const stringVector       &AsStringVector()       const;


    bool                      ToBool()         const;
    char                      ToChar()         const;
    unsigned char             ToUnsignedChar() const;
    int                       ToInt()          const;
    long                      ToLong()         const;
    float                     ToFloat()        const;
    double                    ToDouble()       const;

    void                      ToBoolVector(boolVector &)                 const;
    void                      ToCharVector(charVector &)                 const;
    void                      ToUnsignedCharVector(unsignedCharVector &) const;
    void                      ToIntVector(intVector &)                   const;
    void                      ToLongVector(longVector &)                 const;
    void                      ToFloatVector(floatVector &)               const;
    void                      ToDoubleVector(doubleVector &)             const;

    bool                      IsNumeric()       const;
    bool                      IsNumericVector() const;

    std::string              &ConvertToString();


    void                      SetValue(const Variant&);
    void                      SetValue(const XMLNode&,bool decodeString = true);
    void                      SetValue(const XMLNode*,bool decodeString = true);
    virtual void              SetValue(const JSONNode&,const JSONNode&, bool decodeString = true);
    virtual void              SetValue(const JSONNode*,const JSONNode* meta, bool decodeString = true);
    void                      SetValue(bool);
    void                      SetValue(char);
    void                      SetValue(unsigned char);
    void                      SetValue(const char*); // interp as string
    void                      SetValue(int);
    void                      SetValue(long);
    void                      SetValue(float);
    void                      SetValue(double);
    void                      SetValue(const std::string&);
    void                      SetValue(const boolVector&);
    void                      SetValue(const charVector&);
    void                      SetValue(const unsignedCharVector&);
    void                      SetValue(const intVector&);
    void                      SetValue(const longVector&);
    void                      SetValue(const floatVector&);
    void                      SetValue(const doubleVector&);
    void                      SetValue(const stringVector&);

    void                      Reset() {Cleanup();} // set to empty

    virtual std::string       ToXML(const std::string &indent, bool encodeString) const;
    virtual std::string       ToJSON(const std::string &indent, bool encodeString) const;
    virtual XMLNode           ToXMLNode(bool encodeString) const;
    virtual JSONNode          ToJSONNode(bool encodeString, bool id) const;

 protected:
    void                      Write(Connection &conn) const;
    void                      Write(Connection *conn) const;
    void                      Read(Connection &conn);
    int                       CalculateMessageSize(Connection &conn) const;
    int                       CalculateMessageSize(Connection *conn) const;
    void                      Init(int);
    virtual JSONNode          ToJSONNodeMetaData(bool id) const;
    static std::string        TypeIDToName(int);
    static int                 NameToTypeID(const std::string &);
private:
    static void               Tokenize(const std::string&,
                                       stringVector &tokens);
    static void               TokenizeQuotedString(const std::string&,
                                                   stringVector &tokens);
    static std::string        EscapeQuotedString(const std::string &);

    static bool               unsetBool;
    static char               unsetChar;
    static unsigned char      unsetUnsignedChar;
    static int                unsetInt;
    static long               unsetLong;
    static float              unsetFloat;
    static double             unsetDouble;
    static std::string        unsetString;
    static boolVector         unsetBoolVector;
    static charVector         unsetCharVector;
    static unsignedCharVector unsetUnsignedCharVector;
    static intVector          unsetIntVector;
    static longVector         unsetLongVector;
    static floatVector        unsetFloatVector;
    static doubleVector       unsetDoubleVector;
    static stringVector       unsetStringVector;

    void                      Cleanup();

    int                       dataType;
    void                     *dataValue;
    std::string               tmp;
};

#endif

