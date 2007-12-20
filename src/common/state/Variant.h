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

#ifndef VARIANT_H
#define VARIANT_H
#include <state_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include <XMLNode.h>

typedef enum
{
    EMPTY_TYPE = 0,
    BOOL_TYPE, CHAR_TYPE, UNSIGNED_CHAR_TYPE, INT_TYPE, LONG_TYPE, FLOAT_TYPE, 
    DOUBLE_TYPE, STRING_TYPE, 
    BOOL_VECTOR_TYPE, CHAR_VECTOR_TYPE, UNSIGNED_CHAR_VECTOR_TYPE, 
    INT_VECTOR_TYPE, LONG_VECTOR_TYPE, FLOAT_VECTOR_TYPE,
    DOUBLE_VECTOR_TYPE, STRING_VECTOR_TYPE
} VariantTypeEnum;

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
//
// ****************************************************************************

class STATE_API Variant
{
  public:
    Variant();
    Variant(const Variant &);
    Variant(const XMLNode &);
    Variant(const XMLNode *);
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

    
    void                      SetValue(const Variant&);
    void                      SetValue(const XMLNode&);
    void                      SetValue(const XMLNode*);
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
    
    virtual std::string       ToXML(const std::string &indent="") const;
    virtual XMLNode           ToXMLNode() const;

 private:
    static std::string        TypeIDToName(int);
    static int                NameToTypeID(const std::string &);
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
    
    void                      Init(int);
    void                      Cleanup();

    int                       dataType;
    void                     *dataValue;
};

#endif

