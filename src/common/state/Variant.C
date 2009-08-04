/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <Variant.h>
#include <Connection.h>

using namespace std;

// ****************************************************************************
// Method: LittleEndian
//
// Purpose: 
//   Returns true if the machine is little endian.
//
// Returns:    true if the machine is little endian; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan  5 14:11:32 PST 2009
//
// Modifications:
//   
// ****************************************************************************

inline bool
LittleEndian()
{
    const int one = 1;
    unsigned char *ptr = (unsigned char *)&one;
    return (ptr[0] == 1);
}

// ****************************************************************************
// Method: EncodeToString
//
// Purpose: 
//   Encodes an object as a string of bytes in little endian order.
//
// Arguments:
//   buf : The destination string buffer.
//   val : The object to convert to bytes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan  5 14:12:06 PST 2009
//
// Modifications:
//   
// ****************************************************************************

template <class T>
void
EncodeToString(char *buf, T val)
{
    char *dest = buf;
    unsigned char *ptr = (unsigned char *)&val;

    if(LittleEndian())
    {
        for(int i = 0; i < sizeof(T); i++)
        {
            sprintf(dest, "%02x", int(ptr[i]));
            dest += 2;
        }
    }
    else
    {
        for(int i = sizeof(T)-1; i >= 0; i--)
        {
            sprintf(dest, "%02x", int(ptr[i]));
            dest += 2;
        }
    }
}

// ****************************************************************************
// Method: DecodeFromString
//
// Purpose: 
//   Decodes an object from a string of little endian bytes encoded by
//   the EncodeToString function.
//
// Arguments:
//   buf : The string that encodes the object.
//   val : The destination object.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan  5 14:14:26 PST 2009
//
// Modifications:
//   
// ****************************************************************************

template <class T>
void
DecodeFromString(const char *buf, T &val)
{
    char hex[3] = {'\0','\0','\0'};
    unsigned char *dest = (unsigned char *)&val;
    const char *src = buf;
    int b;

    if(LittleEndian())
    {
        for(int i = 0; i < sizeof(T); i++)
        {
            hex[0] = *src++;
            hex[1] = *src++;
            sscanf(hex, "%02x", &b);
            dest[i] = (unsigned char)b;
        }
    }
    else
    {
        for(int i = sizeof(T)-1; i >= 0; i--)
        {
            hex[0] = *src++;
            hex[1] = *src++;
            sscanf(hex, "%02x", &b);
            dest[i] = (unsigned char)b;
        }
    }
}


// static members (needed for const access to unset vars)
bool                Variant::unsetBool         = false;
char                Variant::unsetChar         = 0x0;
unsigned char       Variant::unsetUnsignedChar = 0x0;
int                 Variant::unsetInt          = 0;
long                Variant::unsetLong         = 0L;
float               Variant::unsetFloat        = 0.0f;
double              Variant::unsetDouble        = 0.0;
string              Variant::unsetString       = "";
boolVector          Variant::unsetBoolVector;
charVector          Variant::unsetCharVector;
unsignedCharVector  Variant::unsetUnsignedCharVector;
intVector           Variant::unsetIntVector;
longVector          Variant::unsetLongVector;
floatVector         Variant::unsetFloatVector;
doubleVector        Variant::unsetDoubleVector;
stringVector        Variant::unsetStringVector;

// ****************************************************************************
//  Method:  Variant::Variant
//
//  Purpose:
//     Initialize a variant from various data.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
Variant::Variant()
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{;}

// Construct from Variant
Variant::Variant(Variant const &var)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(var);}

// Construct from XML Node Ref
Variant::Variant(const XMLNode &node)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(node);}

// Construct from XML Node Ptr
Variant::Variant(const XMLNode *node)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(*node);}

// Construct from Bool
Variant::Variant(bool val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from Char
Variant::Variant(char val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from Unsigned Char
Variant::Variant(unsigned char val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from Unsigned Char
Variant::Variant(const char *val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from Int
Variant::Variant(int val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from Long
Variant::Variant(long val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from float
Variant::Variant(float val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from Double
Variant::Variant(double val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from String
Variant::Variant(const string &val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from BoolVector
Variant::Variant(const boolVector &val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from CharVector
Variant::Variant(const charVector &val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from UnsignedCharVector
Variant::Variant(const unsignedCharVector &val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from IntVector
Variant::Variant(const intVector &val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from LongVector
Variant::Variant(const longVector &val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from FloatVector
Variant::Variant(const floatVector &val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from DoubleVector
Variant::Variant(const doubleVector &val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// Construct from StringVector
Variant::Variant(const stringVector &val)
: dataType(EMPTY_TYPE), 
  dataValue(NULL)
{SetValue(val);}

// ****************************************************************************
//  Method:  Variant::operator=
//
//  Purpose:
//     Variant assignment operator. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
Variant &
Variant::operator=(const Variant &var)
{
    if(this != &var)
        SetValue(var);
    return *this;
}

Variant &
Variant::operator=(const XMLNode &node)
{
    SetValue(node);
    return *this;
}

Variant &
Variant::operator=(bool val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(char val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(unsigned char val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const char *val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(int val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(long val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(float val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(double val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const string &val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const boolVector &val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const charVector &val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const unsignedCharVector &val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const intVector &val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const longVector &val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const floatVector &val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const doubleVector &val)
{
    SetValue(val);
    return *this;
}

Variant &
Variant::operator=(const stringVector &val)
{
    SetValue(val);
    return *this;
}

// ****************************************************************************
//  Method:  Variant::~Variant
//
//  Purpose:
//     Variant destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
Variant::~Variant()
{
    Cleanup();
}


// ****************************************************************************
//  Method:  Variant::GetTypeName
//
//  Purpose:
//     Get type of this entry.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
string        
Variant::TypeName() const
{
    return TypeIDToName(dataType);
}

// ****************************************************************************
//  Method:  Variant::AsBool
//
//  Purpose:
//     Use this variant as a boolean. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
bool &
Variant::AsBool()
{
    if(dataType != BOOL_TYPE)
        Init(BOOL_TYPE);
    return *((bool *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsChar
//
//  Purpose:
//     Use this variant as a char.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
char &
Variant::AsChar()
{
    if(dataType != CHAR_TYPE)
        Init(CHAR_TYPE);
    return *((char *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsUnsignedChar
//
//  Purpose:
//     Use this variant as an unsigned char.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
unsigned char &
Variant::AsUnsignedChar()
{
    if(dataType != UNSIGNED_CHAR_TYPE)
        Init(UNSIGNED_CHAR_TYPE);
    return *((unsigned char *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsInt
//
//  Purpose:
//     Use this variant as an int.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
int &
Variant::AsInt()
{
    if(dataType != INT_TYPE)
        Init(INT_TYPE);
    return *((int *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsLong
//
//  Purpose:
//     Use this variant as a long.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
long &
Variant::AsLong()
{
    if(dataType != LONG_TYPE)
        Init(LONG_TYPE);
    return *((long *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsFloat
//
//  Purpose:
//     Use this variant as a float.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
float &
Variant::AsFloat()
{
    if(dataType != FLOAT_TYPE)
        Init(FLOAT_TYPE);
    return *((float *)dataValue);
}


// ****************************************************************************
//  Method:  Variant::AsDouble
//
//  Purpose:
//     Use this variant as a double. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
double &
Variant::AsDouble()
{
    if(dataType != DOUBLE_TYPE)
        Init(DOUBLE_TYPE);
    return *((double *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsString
//
//  Purpose:
//     Use this variant as a string. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
string &
Variant::AsString()
{
    if(dataType != STRING_TYPE)
        Init(STRING_TYPE);
    return *((string *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsBoolVector
//
//  Purpose:
//     Use this variant as a boolVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
boolVector &
Variant::AsBoolVector()
{
    if(dataType != BOOL_VECTOR_TYPE)
        Init(BOOL_VECTOR_TYPE);
    return *((boolVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsCharVector
//
//  Purpose:
//     Use this variant as a charVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
charVector &
Variant::AsCharVector()
{
    if(dataType != CHAR_VECTOR_TYPE)
        Init(CHAR_VECTOR_TYPE);
    return *((charVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsUnsignedCharVector
//
//  Purpose:
//     Use this variant as an unsignedCharVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
unsignedCharVector &
Variant::AsUnsignedCharVector()
{
    if(dataType != UNSIGNED_CHAR_VECTOR_TYPE)
        Init(UNSIGNED_CHAR_VECTOR_TYPE);
    return *((unsignedCharVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsIntVector
//
//  Purpose:
//     Use this variant as an intVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
intVector &
Variant::AsIntVector()
{
    if(dataType != INT_VECTOR_TYPE)
        Init(INT_VECTOR_TYPE);
    return *((intVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsLongVector
//
//  Purpose:
//     Use this variant as a longVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
longVector &
Variant::AsLongVector()
{
    if(dataType != LONG_VECTOR_TYPE)
        Init(LONG_VECTOR_TYPE);
    return *((longVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsFloatVector
//
//  Purpose:
//     Use this variant as a floatVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
floatVector &
Variant::AsFloatVector()
{
    if(dataType != FLOAT_VECTOR_TYPE)
        Init(FLOAT_VECTOR_TYPE);
    return *((floatVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsDoubleVector
//
//  Purpose:
//     Use this variant as a doubleVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
doubleVector &
Variant::AsDoubleVector()
{
    if(dataType != DOUBLE_VECTOR_TYPE)
        Init(DOUBLE_VECTOR_TYPE);
    return *((doubleVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsStringVector
//
//  Purpose:
//     Use this variant as a stringVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
stringVector &
Variant::AsStringVector()
{
    if(dataType != STRING_VECTOR_TYPE)
        Init(STRING_VECTOR_TYPE);
    return *((stringVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsBool
//
//  Purpose:
//     Use this variant as a boolean. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const bool &
Variant::AsBool() const
{
    if(dataType != BOOL_TYPE)
        return unsetBool;
    else        
        return *((bool *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsChar
//
//  Purpose:
//     Use this variant as a char.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const char &
Variant::AsChar() const
{
    if(dataType != CHAR_TYPE)
        return unsetChar;
    else
        return *((char *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsUnsignedChar
//
//  Purpose:
//     Use this variant as an unsigned char.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const unsigned char &
Variant::AsUnsignedChar() const
{
    if(dataType != UNSIGNED_CHAR_TYPE)
        return unsetUnsignedChar;
    else
        return *((unsigned char *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsInt
//
//  Purpose:
//     Use this variant as an int.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const int &
Variant::AsInt() const
{
    if(dataType != INT_TYPE)
        return unsetInt;
    else
        return *((int *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsLong
//
//  Purpose:
//     Use this variant as a long.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const long &
Variant::AsLong() const
{
    if(dataType != LONG_TYPE)
        return unsetLong;
    else
        return *((long *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsFloat
//
//  Purpose:
//     Use this variant as a float.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const float &
Variant::AsFloat() const
{
    if(dataType != FLOAT_TYPE)
        return unsetFloat;
    else
        return *((float *)dataValue);
}


// ****************************************************************************
//  Method:  Variant::AsDouble
//
//  Purpose:
//     Use this variant as a double. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const double &
Variant::AsDouble() const
{
    if(dataType != DOUBLE_TYPE)
        return unsetDouble;
    else
        return *((double *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsString
//
//  Purpose:
//     Use this variant as a string. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
const string &
Variant::AsString() const
{
    if(dataType != STRING_TYPE)
        return unsetString;
    else
        return *((string *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsBoolVector
//
//  Purpose:
//     Use this variant as a boolVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const boolVector &
Variant::AsBoolVector() const
{
    if(dataType != BOOL_VECTOR_TYPE)
        return unsetBoolVector;
    else
        return *((boolVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsCharVector
//
//  Purpose:
//     Use this variant as a charVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const charVector &
Variant::AsCharVector() const
{
    if(dataType != CHAR_VECTOR_TYPE)
        return unsetCharVector;
    else
        return *((charVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsUnsignedCharVector
//
//  Purpose:
//     Use this variant as an unsignedCharVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const unsignedCharVector &
Variant::AsUnsignedCharVector() const
{
    if(dataType != UNSIGNED_CHAR_VECTOR_TYPE)
        return unsetUnsignedCharVector;
    else
        return *((unsignedCharVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsIntVector
//
//  Purpose:
//     Use this variant as an intVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const intVector &
Variant::AsIntVector() const
{
    if(dataType != INT_VECTOR_TYPE)
        return unsetIntVector;
    else
        return *((intVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsLongVector
//
//  Purpose:
//     Use this variant as a longVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const longVector &
Variant::AsLongVector() const
{
    if(dataType != LONG_VECTOR_TYPE)
        return unsetLongVector;
    else
        return *((longVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsFloatVector
//
//  Purpose:
//     Use this variant as a floatVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const floatVector &
Variant::AsFloatVector() const
{
    if(dataType != FLOAT_VECTOR_TYPE)
        return unsetFloatVector;
    else
        return *((floatVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsDoubleVector
//
//  Purpose:
//     Use this variant as a doubleVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const doubleVector &
Variant::AsDoubleVector() const
{
    if(dataType != DOUBLE_VECTOR_TYPE)
        return unsetDoubleVector;
    else
        return *((doubleVector *)dataValue);
}

// ****************************************************************************
//  Method:  Variant::AsStringVector
//
//  Purpose:
//     Use this variant as a stringVector. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
const stringVector &
Variant::AsStringVector() const
{
    if(dataType != STRING_VECTOR_TYPE)
        return unsetStringVector;
    else
        return *((stringVector *)dataValue);
}


// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to value of given variant.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const Variant &var)
{
    if(var.dataType == EMPTY_TYPE)
    {Cleanup();}
    else if(var.dataType == BOOL_TYPE)
    {SetValue(*((bool*)var.dataValue));}
    else if(var.dataType == CHAR_TYPE)
    {SetValue(*((char*)var.dataValue));}
    else if(var.dataType == UNSIGNED_CHAR_TYPE)
    {SetValue(*((unsigned char*)var.dataValue));}
    else if(var.dataType == INT_TYPE)
    {SetValue(*((int*)var.dataValue));}
    else if(var.dataType == LONG_TYPE)
    {SetValue(*((long*)var.dataValue));}
    else if(var.dataType == FLOAT_TYPE)
    {SetValue(*((float*)var.dataValue));}
    else if(var.dataType == DOUBLE_TYPE)
    {SetValue(*((double*)var.dataValue));}
    else if(var.dataType == STRING_TYPE)
    {SetValue(*((string*)var.dataValue));}
    else if(var.dataType == BOOL_VECTOR_TYPE)
    {SetValue(*((boolVector*)var.dataValue));}
    else if(var.dataType == CHAR_VECTOR_TYPE)
    {SetValue(*((charVector*)var.dataValue));}
    else if(var.dataType == UNSIGNED_CHAR_VECTOR_TYPE)
    {SetValue(*((unsignedCharVector*)var.dataValue));}
    else if(var.dataType == INT_VECTOR_TYPE)
    {SetValue(*((intVector*)var.dataValue));}
    else if(var.dataType == LONG_VECTOR_TYPE)
    {SetValue(*((longVector*)var.dataValue));}
    else if(var.dataType == FLOAT_VECTOR_TYPE)
    {SetValue(*((floatVector*)var.dataValue));}
    else if(var.dataType == DOUBLE_VECTOR_TYPE)
    {SetValue(*((doubleVector*)var.dataValue));}
    else if(var.dataType == STRING_VECTOR_TYPE)
    {SetValue(*((stringVector*)var.dataValue));}
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Sets this variant's value from an xml node.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
//  Modifications:
//    Brad Whitlock, Mon Jan  5 14:21:22 PST 2009
//    Decode float and doubles from byte strings so we preserve the number.
//
// ****************************************************************************
void 
Variant::SetValue(const XMLNode &node)
{
    // if we dont have a valid xml rep, set to an empty variant
    if( node.Name() !="variant" || 
       !node.HasAttribute("type") || 
       node.Text() == "")
    {
        Init(EMPTY_TYPE);
        return;
    }

    // loop index
    size_t i = 0;
    int data_type = NameToTypeID(node.Attribute("type"));
    const char *txt_val = node.Text().c_str();
    
    // get tokens ready
    stringVector tokens;
    if(data_type == STRING_VECTOR_TYPE)
        TokenizeQuotedString(node.Text(),tokens); 
    else
        Tokenize(node.Text(),tokens);
    
    
    if(data_type == BOOL_TYPE)
    {
        SetValue(node.Text() == "true");
    }
    else if(data_type == CHAR_TYPE)
    {
        char val;
        sscanf(txt_val,"%c",&val);
        SetValue(val);
    }
    else if(data_type == UNSIGNED_CHAR_TYPE)
    {
        int val;
        sscanf(txt_val,"%d",&val);
        SetValue((unsigned char)val);
    }
    else if(data_type == INT_TYPE)
    {
        int val;
        sscanf(txt_val,"%d",&val);
        SetValue(val);
    }
    else if(data_type == LONG_TYPE)
    {
        long val;
        sscanf(txt_val,"%ld",&val);
        SetValue(val);
    }
    else if(data_type == FLOAT_TYPE)
    {
        float val;
        DecodeFromString(txt_val, val);
        SetValue(val);
    }
    else if(data_type == DOUBLE_TYPE)
    {
        double val;
        DecodeFromString(txt_val, val);
        SetValue(val);
    }
    else if(data_type == STRING_TYPE)
    {
        SetValue(node.Text());
    }
    else if(data_type == BOOL_VECTOR_TYPE)
    {
        boolVector &vec = AsBoolVector();
        for(i = 0; i < tokens.size(); i++)
        {
            if(tokens[i] == "1")
                vec.push_back(true);
            else if(tokens[i] == "0")
                vec.push_back(false);
        }
    }
    else if(data_type == CHAR_VECTOR_TYPE)
    {
        charVector &vec = AsCharVector();
        for(i = 0; i < tokens.size(); i++)
            vec.push_back(tokens[i][0]);
    }         
    else if(data_type == UNSIGNED_CHAR_VECTOR_TYPE)
    {
        int val = 0;
        unsignedCharVector &vec = AsUnsignedCharVector();
        for(i = 0; i < tokens.size(); i++)
        {
            sscanf(tokens[i].c_str(),"%d",&val);
            vec.push_back((unsigned char)val);
        }
    }         
    else if(data_type == INT_VECTOR_TYPE)
    {
        int val = 0;
        intVector &vec = AsIntVector();
        for(i = 0; i < tokens.size(); i++)
        {
            sscanf(tokens[i].c_str(),"%d",&val);
            vec.push_back(val);
        }
    } 
    else if(data_type == LONG_VECTOR_TYPE)
    {
        long val = 0L;
        longVector &vec = AsLongVector();
        for(i = 0; i < tokens.size(); i++)
        {
            sscanf(tokens[i].c_str(),"%ld",&val);
            vec.push_back(val);
        }
    }
    else if(data_type == FLOAT_VECTOR_TYPE)
    {
        float val = 0.0f;
        floatVector &vec = AsFloatVector();
        for(i = 0; i < tokens.size(); i++)
        {
            DecodeFromString(tokens[i].c_str(), val);
            vec.push_back(val);
        }
    }
    else if(data_type == DOUBLE_VECTOR_TYPE)
    {
        double val = 0.;
        doubleVector &vec = AsDoubleVector();
        for(i = 0; i < tokens.size(); i++)
        {
            DecodeFromString(tokens[i].c_str(), val);
            vec.push_back(val);
        }
    }
    else if(data_type == STRING_VECTOR_TYPE)
    {
        AsStringVector() = tokens;
    }
        
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Sets this variant's value from an xml node pointer.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const XMLNode *node)
{
    SetValue(*node);
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given boolean value.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(bool val)
{
    if(dataType != BOOL_TYPE)
        Init(BOOL_TYPE);
    *((bool *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given char value.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(char val)
{
    if(dataType != CHAR_TYPE)
        Init(CHAR_TYPE);
    *((char *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given unsigned char value.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(unsigned char val)
{
    if(dataType != UNSIGNED_CHAR_TYPE)
        Init(UNSIGNED_CHAR_TYPE);
    *((unsigned char *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given string.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const char *val)
{
    if(dataType != STRING_TYPE)
        Init(STRING_TYPE);
    string s((const char*)val);
    *((string *)dataValue) = s;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given int value.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(int val)
{
    if(dataType != INT_TYPE)
        Init(INT_TYPE);
    *((int *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given long value.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(long val)
{
    if(dataType != LONG_TYPE)
        Init(LONG_TYPE);
    *((long *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given float value.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(float val)
{
    if(dataType != FLOAT_TYPE)
        Init(FLOAT_TYPE);
    *((float *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given double value.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(double val)
{
    if(dataType != DOUBLE_TYPE)
        Init(DOUBLE_TYPE);
    *((double *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given string value.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const string &val)
{
    if(dataType != STRING_TYPE)
        Init(STRING_TYPE);
    *((string *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given boolVector.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const boolVector &val)
{
    if(dataType != BOOL_VECTOR_TYPE)
        Init(BOOL_VECTOR_TYPE);
    *((boolVector *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given charVector.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const charVector &val)
{
    if(dataType != CHAR_VECTOR_TYPE)
        Init(CHAR_VECTOR_TYPE);
    *((charVector *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given unsignedCharVector.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const unsignedCharVector &val)
{
    if(dataType != UNSIGNED_CHAR_VECTOR_TYPE)
        Init(UNSIGNED_CHAR_VECTOR_TYPE);
    *((unsignedCharVector *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given intVector.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const intVector &val)
{
    if(dataType != INT_VECTOR_TYPE)
        Init(INT_VECTOR_TYPE);
    *((intVector *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given longVector.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const longVector &val)
{
    if(dataType != LONG_VECTOR_TYPE)
        Init(LONG_VECTOR_TYPE);
    *((longVector *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given floatVector.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const floatVector &val)
{
    if(dataType != FLOAT_VECTOR_TYPE)
        Init(FLOAT_VECTOR_TYPE);
    *((floatVector *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given doubleVector.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const doubleVector &val)
{
    if(dataType != DOUBLE_VECTOR_TYPE)
        Init(DOUBLE_VECTOR_TYPE);
    *((doubleVector *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::SetValue
//
//  Purpose:
//     Set variant to given stringVector.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
// ****************************************************************************
void 
Variant::SetValue(const stringVector &val)
{
     if(dataType != STRING_VECTOR_TYPE)
        Init(STRING_VECTOR_TYPE);
     *((stringVector *)dataValue) = val;
}

// ****************************************************************************
//  Method:  Variant::ToXML
//
//  Purpose:
//     Create xml represenation of this variant.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
string 
Variant::ToXML(const string &indent) const
{
    XMLNode node;
    node.Name() = "variant";
    node.Attribute("type") = TypeName();
    return ToXMLNode().ToString(indent);
}

// ****************************************************************************
//  Method:  Variant::ToXML
//
//  Purpose:
//     Create xml string represenation of this variant.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 15, 2007
//
//  Modifications:
//    Brad Whitlock, Mon Jan  5 14:17:19 PST 2009
//    Encode the floats and doubles to preserve accuracy.
//
// ****************************************************************************

XMLNode 
Variant::ToXMLNode() const
{
    XMLNode node;
    node.Name() = "variant";
    node.Attribute("type") = TypeName();

    string res_str="";
    char   buff[1024];
    
    if(dataType == BOOL_TYPE)
    {
        if(*((bool *)dataValue))
            res_str = "true";
        else
            res_str = "false";
    }
    else if(dataType == CHAR_TYPE)
    {
        sprintf(buff,"%c",*((char *)dataValue));
        res_str = buff;
    }
    else if(dataType == UNSIGNED_CHAR_TYPE)
    {
        sprintf(buff,"%d", (int)(*((unsigned char *)dataValue)));
        res_str = buff;
    }
    else if(dataType == INT_TYPE)
    {
        sprintf(buff,"%d",*((int *)dataValue));
        res_str = buff;
    }
    else if(dataType == LONG_TYPE)
    {
        sprintf(buff,"%ld",*((long *)dataValue));
        res_str = buff;
    }
    else if(dataType == FLOAT_TYPE)
    {
        EncodeToString(buff, *((float *)dataValue));
        res_str = buff;
    }
    else if(dataType == DOUBLE_TYPE)
    {
        EncodeToString(buff,*((double *)dataValue));
        res_str = buff;
    }
    else if(dataType == STRING_TYPE)
    {
        res_str = *((string *)dataValue);
    }
    else if(dataType == BOOL_VECTOR_TYPE)
    {
        const boolVector &vec = AsBoolVector();
            for(size_t i=0;i<vec.size();i++)
            {
                sprintf(buff,"%d ",(int)vec[i]);
                res_str += buff;
            }
    }
    else if(dataType == CHAR_VECTOR_TYPE)
    {
        const charVector &vec = AsCharVector();
        for(size_t i=0;i<vec.size();i++)
        {
            sprintf(buff,"%c ",vec[i]);
            res_str += buff;
        }
    }
    else if(dataType == UNSIGNED_CHAR_VECTOR_TYPE)
    {
        const unsignedCharVector &vec = AsUnsignedCharVector();
        for(size_t i=0;i<vec.size();i++)
        {
            sprintf(buff,"%d ",(int)vec[i]);
            res_str += buff;
        }
    }
    else if(dataType == INT_VECTOR_TYPE)
    {
        const intVector &vec = AsIntVector();
        for(size_t i=0;i<vec.size();i++)
        {
            sprintf(buff,"%d ",(int)vec[i]);
            res_str += buff;
        }
    }
    else if(dataType == LONG_VECTOR_TYPE)
    {
        const longVector &vec = AsLongVector();
        for(size_t i=0;i<vec.size();i++)
        {
            sprintf(buff,"%ld ",vec[i]);
            res_str += buff;
        }
    }
    else if(dataType == FLOAT_VECTOR_TYPE)
    {
        const floatVector &vec = AsFloatVector();
        for(size_t i=0;i<vec.size();i++)
        {
            EncodeToString(buff, vec[i]);
            res_str += buff;
            res_str += " ";
        }
    }
    else if(dataType == DOUBLE_VECTOR_TYPE)
    {
        const doubleVector &vec = AsDoubleVector();
        for(size_t i=0;i<vec.size();i++)
        {
            EncodeToString(buff, vec[i]);
            res_str += buff;
            res_str += " ";
        }
    }
    else if(dataType == STRING_VECTOR_TYPE)
    {
        const stringVector &vec = AsStringVector();
        for(size_t i=0;i<vec.size();i++)
            res_str += string("\"") + 
                       EscapeQuotedString(vec[i]) + 
                       string("\" ");
    }

    node.Text() = res_str;
    return node;
}

// ****************************************************************************
//  Method:  Variant::TypeIDToName
//
//  Purpose:
//     Gets type name from type id.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************

// typename table
static const char *VariantTypeNameLookup[] = 
{ "empty",
  "bool", "char", "unsigned char", "int", "long", "float","double","string",
  "boolVector", "charVector", "unsignedCharVector", "intVector", "longVector", 
  "floatVector", "doubleVector", "stringVector"
};

string
Variant::TypeIDToName(int data_type)
{
    if(data_type < 0 || data_type > 16)
        return VariantTypeNameLookup[0];
    return VariantTypeNameLookup[data_type];
}

// ****************************************************************************
//  Method:  Variant::NameToTypeID
//
//  Purpose:
//     Gets type id from type name.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
int 
Variant::NameToTypeID(const string &name)
{
    for(int i=0;i<17;i++)
    {
        if(name == VariantTypeNameLookup[i])
            return i;
    }
    return EMPTY_TYPE;
}

// ****************************************************************************
//  Method:  Variant::Tokenize
//
//  Purpose:
//     Tokenizes the input string.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
void 
Variant::Tokenize(const string &val,stringVector &tokens)
{
    tokens.clear();
    // read tokens from string
    // this shold work for all cases but the quoted string vector
    istringstream iss;
    iss.str(val);
    while(!iss.eof())
    {
        string s;
        iss >> s;
        iss >> ws;
        tokens.push_back(s);
    }
}

// ****************************************************************************
//  Method:  Variant::TokenizeQuotedString
//
//  Purpose:
//     Tokenizes a vector of strings delimited by quotes.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
void 
Variant::TokenizeQuotedString(const string &val,stringVector &tokens)
{
    tokens.clear();

    int ssize = val.size();
    if(ssize < 1 || val[0] !='\"' )
        return;
        
    string curr = "";
    bool open = true;        
    
    for(int i=1;i<ssize;i++)
    {
        if(val[i] == '"')
        {
            if(val[i-1] == '\\')
            {
                curr.push_back('"');
            }
            else if(open)
            {
                tokens.push_back(curr);
                curr = "";
                open = false;
            }
            else
            {
                open = true;
            }
        }
        else if(val[i] == '\\')
        {
            if(val[i-1] == '\\')
                curr.push_back('\\');
        }
        else if(open)
        {
            curr.push_back(val[i]);
        }
        
    }
}

// ****************************************************************************
//  Method:  Variant::EscapeQuotedString
//
//  Purpose:
//     Returns a new string with all quotes escaped by \.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 17, 2007
//
// ****************************************************************************
string 
Variant::EscapeQuotedString(const string &val)
{
    string res="";
    int ssize = val.size();
    for(int i=0;i<ssize;i++)
    {
        if(val[i] == '"')
        {
            res.push_back('\\');
            res.push_back('"');
        }
        else
        {
            res.push_back(val[i]);
        } 
    }
    return res;
}


// ****************************************************************************
//  Method:  Variant::Init
//
//  Purpose:
//     Releases memory and resets the variant to the empty state.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
void 
Variant::Init(int type)
{
    if(dataType == type)
        return;
    Cleanup();
    if(type == EMPTY_TYPE)
        return;
    dataType = type;
    // allocate proper data
    if(dataType == BOOL_TYPE)
    {
        dataValue = (void *) new bool(false);
    }
    else if(dataType == CHAR_TYPE)
    {   
        dataValue = (void *) new char(0x0);
    }
    else if(dataType == UNSIGNED_CHAR_TYPE)
    {
        dataValue = (void *) new unsigned char(0x0);
    }
    else if(dataType == INT_TYPE)
    {
        dataValue = (void *) new int(0);
    }
    else if(dataType == LONG_TYPE)
    {
        dataValue = (void *) new long(0L);
    }
    else if(dataType == FLOAT_TYPE)
    {
        dataValue = (void *) new float(0.0f);
    }
    else if(dataType == DOUBLE_TYPE)
    {
        dataValue = (void *) new double(0.0);
    }
    else if(dataType == BOOL_TYPE)
    {
        dataValue = (void *) new bool(false);
    }
    else if(dataType == STRING_TYPE)
    {
        dataValue = (void *) new string("");
    }
    else if(dataType == BOOL_VECTOR_TYPE)
    {
        dataValue = (void *) new boolVector;
    }
    if(dataType == CHAR_VECTOR_TYPE)
    {
        dataValue = (void *) new charVector;
    }
    else if(dataType == UNSIGNED_CHAR_VECTOR_TYPE)
    {
        dataValue = (void *) new unsignedCharVector;
    }
    else if(dataType == INT_VECTOR_TYPE)
    {
        dataValue = (void *) new intVector;
    }
    else if(dataType == LONG_VECTOR_TYPE)
    {
        dataValue = (void *) new longVector;
    }
    else if(dataType == FLOAT_VECTOR_TYPE)
    {
        dataValue = (void *) new floatVector;
    }
    else if(dataType == DOUBLE_VECTOR_TYPE)
    {
        dataValue = (void *) new doubleVector;
    }
    else if(dataType == STRING_VECTOR_TYPE)
    {
        dataValue = (void *) new stringVector;
    }
}

// ****************************************************************************
//  Method:  Variant::Cleanup
//
//  Purpose:
//     Releases memory and resets the variant to the Empty state.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 11, 2007
//
// ****************************************************************************
void 
Variant::Cleanup()
{
    if(dataType  == EMPTY_TYPE)
        return;
        
    // clean up previous data
    if(dataType == CHAR_TYPE)
    {
        char *ptr = (char *)dataValue;
        delete ptr;
    }
    else if(dataType == UNSIGNED_CHAR_TYPE)
    {
        unsigned char *ptr = (unsigned char *)dataValue;
        delete ptr;
    }
    else if(dataType == INT_TYPE)
    {
        int *ptr = (int *)dataValue;
        delete ptr;
    }
    else if(dataType == LONG_TYPE)
    {
        long *ptr = (long *)dataValue;
        delete ptr;
    }
    else if(dataType == FLOAT_TYPE)
    {
        float *ptr = (float *)dataValue;
        delete ptr;
    }
    else if(dataType == DOUBLE_TYPE)
    {
        double *ptr = (double *)dataValue;
        delete ptr;
    }
    else if(dataType == BOOL_TYPE)
    {
        bool *ptr = (bool *)dataValue;
        delete ptr;
    }
    else if(dataType == STRING_TYPE)
    {
        string *ptr = (string *)dataValue;
        delete ptr;
    }
    else if(dataType == CHAR_VECTOR_TYPE)
    {
        charVector *ptr = (charVector *)dataValue;
        delete ptr;
    }
    else if(dataType == UNSIGNED_CHAR_VECTOR_TYPE)
    {
        unsignedCharVector *ptr = (unsignedCharVector *)dataValue;
        delete ptr;
    }
    else if(dataType == INT_VECTOR_TYPE)
    {
        intVector *ptr = (intVector *)dataValue;
        delete ptr;
    }
    else if(dataType == LONG_VECTOR_TYPE)
    {
        longVector *ptr = (longVector *)dataValue;
        delete ptr;
    }
    else if(dataType == FLOAT_VECTOR_TYPE)
    {
        floatVector *ptr = (floatVector *)dataValue;
        delete ptr;
    }
    else if(dataType == DOUBLE_VECTOR_TYPE)
    {
        doubleVector *ptr = (doubleVector *)dataValue;
        delete ptr;
    }
    else if(dataType == BOOL_VECTOR_TYPE)
    {
        boolVector *ptr = (boolVector *)dataValue;
        delete ptr;
    }
    else if(dataType == STRING_VECTOR_TYPE)
    {
        stringVector *ptr = (stringVector *)dataValue;
        delete ptr;
    }
    dataType = EMPTY_TYPE;
    dataValue = NULL;
}

// ****************************************************************************
// Method: Variant::operator ==
//
// Purpose: 
//   Comparison operator.
//
// Arguments:
//   obj : The object to be compared.
//
// Returns:    True if the object is equal to this object; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  6 15:17:26 PST 2009
//
// Modifications:
//   
// ****************************************************************************

bool
Variant::operator ==(const Variant &obj) const
{
    // Compare the types
    if(dataType != obj.dataType)
        return false;

    // Compare the data
    bool equal = false;
    if(dataType == BOOL_TYPE)
        equal = AsBool() == obj.AsBool();
    else if(dataType == CHAR_TYPE)
        equal = AsChar() == obj.AsChar();
    else if(dataType == UNSIGNED_CHAR_TYPE)
        equal = AsUnsignedChar() == obj.AsUnsignedChar();
    else if(dataType == INT_TYPE)
        equal = AsInt() == obj.AsInt();
    else if(dataType == LONG_TYPE)
        equal = AsLong() == obj.AsLong();
    else if(dataType == FLOAT_TYPE)
        equal = AsFloat() == obj.AsFloat();
    else if(dataType == DOUBLE_TYPE)
        equal = AsDouble() == obj.AsDouble();
    else if(dataType == STRING_TYPE)
        equal = AsString() == obj.AsString();
    else if(dataType == BOOL_VECTOR_TYPE)
        equal = AsBoolVector() == obj.AsBoolVector();
    else if(dataType == CHAR_VECTOR_TYPE)
        equal = AsCharVector() == obj.AsCharVector();
    else if(dataType == UNSIGNED_CHAR_VECTOR_TYPE)
        equal = AsUnsignedCharVector() == obj.AsUnsignedCharVector();
    else if(dataType == INT_VECTOR_TYPE)
        equal = AsIntVector() == obj.AsIntVector();
    else if(dataType == LONG_VECTOR_TYPE)
        equal = AsLongVector() == obj.AsLongVector();
    else if(dataType == FLOAT_VECTOR_TYPE)
        equal = AsFloatVector() == obj.AsFloatVector();
    else if(dataType == DOUBLE_VECTOR_TYPE)
        equal = AsDoubleVector() == obj.AsDoubleVector();
    else if(dataType == STRING_VECTOR_TYPE)
        equal = AsStringVector() == obj.AsStringVector();
    else if(dataType == EMPTY_TYPE)
        equal = true;
       
    return equal;
}

// ****************************************************************************
// Method: Variant::CalculateMessageSize
//
// Purpose: 
//   Compute the size of the message that contains this object's data.
//
// Arguments:
//   conn : The connection used for the outgoing message.
//
// Returns:    The message size.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  6 15:18:16 PST 2009
//
// Modifications:
//   
// ****************************************************************************

int
Variant::CalculateMessageSize(Connection &conn) const
{
    int messageSize = 0;

    if(dataType == BOOL_TYPE)
        messageSize = conn.CharSize(conn.DEST);
    else if(dataType == CHAR_TYPE)
        messageSize = conn.CharSize(conn.DEST);
    else if(dataType == UNSIGNED_CHAR_TYPE)
        messageSize = conn.CharSize(conn.DEST);
    else if(dataType == INT_TYPE)
        messageSize = conn.IntSize(conn.DEST);
    else if(dataType == LONG_TYPE)
        messageSize = conn.LongSize(conn.DEST);
    else if(dataType == FLOAT_TYPE)
        messageSize = conn.FloatSize(conn.DEST);
    else if(dataType == DOUBLE_TYPE)
        messageSize = conn.DoubleSize(conn.DEST);
    else if(dataType == STRING_TYPE)
        messageSize = conn.CharSize(conn.DEST) * (AsString().size() + 1);
    else if(dataType == BOOL_VECTOR_TYPE)
        messageSize = conn.IntSize(conn.DEST) + 
                      conn.CharSize(conn.DEST) * AsBoolVector().size();
    else if(dataType == CHAR_VECTOR_TYPE)
        messageSize = conn.IntSize(conn.DEST) + 
                      conn.CharSize(conn.DEST) * AsCharVector().size();
    else if(dataType == UNSIGNED_CHAR_VECTOR_TYPE)
        messageSize = conn.IntSize(conn.DEST) + 
                      conn.CharSize(conn.DEST) * AsUnsignedCharVector().size();
    else if(dataType == INT_VECTOR_TYPE)
        messageSize = conn.IntSize(conn.DEST) + 
                      conn.IntSize(conn.DEST) * AsIntVector().size();
    else if(dataType == LONG_VECTOR_TYPE)
        messageSize = conn.IntSize(conn.DEST) + 
                      conn.LongSize(conn.DEST) * AsLongVector().size();
    else if(dataType == FLOAT_VECTOR_TYPE)
        messageSize = conn.IntSize(conn.DEST) + 
                      conn.FloatSize(conn.DEST) * AsFloatVector().size();
    else if(dataType == DOUBLE_VECTOR_TYPE)
        messageSize = conn.IntSize(conn.DEST) + 
                      conn.DoubleSize(conn.DEST) * AsDoubleVector().size();
    else if(dataType == STRING_VECTOR_TYPE)
    {
        messageSize = conn.IntSize(conn.DEST);
        const stringVector &vec = AsStringVector();
        for(size_t i = 0; i < vec.size(); ++i)
            messageSize += (conn.CharSize(conn.DEST) * (vec[i].size() + 1));
    }
       
    return messageSize;
}

// ****************************************************************************
// Method: Variant::Write
//
// Purpose: 
//   Writes this object's data to the connection.
//
// Arguments:
//   conn : The connection to which we're writing the data.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  6 15:19:10 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
Variant::Write(Connection &conn) const
{
    if(dataType == BOOL_TYPE)
    {
        bool value = *((bool *)dataValue);
        conn.WriteChar(value ? 1 : 0);
    }
    else if(dataType == CHAR_TYPE)
        conn.WriteChar(*((char *)dataValue));
    else if(dataType == UNSIGNED_CHAR_TYPE)
        conn.WriteUnsignedChar(*((unsigned char *)dataValue));
    else if(dataType == INT_TYPE)
        conn.WriteInt(*((int *)dataValue));
    else if(dataType == LONG_TYPE)
        conn.WriteLong(*((long *)dataValue));
    else if(dataType == FLOAT_TYPE)
        conn.WriteFloat(*((float *)dataValue));
    else if(dataType == DOUBLE_TYPE)
        conn.WriteDouble(*((double *)dataValue));
    else if(dataType == STRING_TYPE)
    {
        std::string *sptr = (std::string *)dataValue;
        for(size_t i = 0; i < sptr->size(); ++i)
            conn.WriteChar(sptr->at(i));
        conn.WriteChar(0);
    }
    else if(dataType == BOOL_VECTOR_TYPE)
    {
        const boolVector &vec = AsBoolVector();
        conn.WriteInt(vec.size());
        for(size_t i=0;i<vec.size();i++)
            conn.WriteChar(vec[i]?1:0);
    }
    else if(dataType == CHAR_VECTOR_TYPE)
    {
        const charVector &vec = AsCharVector();
        conn.WriteInt(vec.size());
        for(size_t i=0;i<vec.size();i++)
            conn.WriteChar(vec[i]);
    }
    else if(dataType == UNSIGNED_CHAR_VECTOR_TYPE)
    {
        const unsignedCharVector &vec = AsUnsignedCharVector();
        conn.WriteInt(vec.size());
        for(size_t i=0;i<vec.size();i++)
            conn.WriteUnsignedChar(vec[i]);
    }
    else if(dataType == INT_VECTOR_TYPE)
    {
        const intVector &vec = AsIntVector();
        conn.WriteInt(vec.size());
        for(size_t i=0;i<vec.size();i++)
            conn.WriteInt(vec[i]);
    }
    else if(dataType == LONG_VECTOR_TYPE)
    {
        const longVector &vec = AsLongVector();
        conn.WriteInt(vec.size());
        for(size_t i=0;i<vec.size();i++)
            conn.WriteLong(vec[i]);
    }
    else if(dataType == FLOAT_VECTOR_TYPE)
    {
        const floatVector &vec = AsFloatVector();
        conn.WriteInt(vec.size());
        for(size_t i=0;i<vec.size();i++)
            conn.WriteFloat(vec[i]);
    }
    else if(dataType == DOUBLE_VECTOR_TYPE)
    {
        const doubleVector &vec = AsDoubleVector();
        conn.WriteInt(vec.size());
        for(size_t i=0;i<vec.size();i++)
            conn.WriteDouble(vec[i]);
    }
    else if(dataType == STRING_VECTOR_TYPE)
    {
        const stringVector &vec = AsStringVector();
        stringVector::const_iterator spos;

        conn.WriteInt(vec.size());
        for(spos = vec.begin(); spos != vec.end(); ++spos)
            conn.WriteString(*spos);
    }
}

// ****************************************************************************
// Method: Variant::Read
//
// Purpose: 
//   Reads this object's data from the connection.
//
// Arguments:
//   
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  6 15:19:52 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
Variant::Read(Connection &conn)
{
    int s;

    if(dataType == BOOL_TYPE)
    {
        unsigned char tmp;
        conn.ReadChar(&tmp);
        SetValue(tmp==1);
    }
    else if(dataType == CHAR_TYPE)
    {
        char tmp;
        conn.ReadChar((unsigned char *)&tmp);
        SetValue(tmp);
    }
    else if(dataType == UNSIGNED_CHAR_TYPE)
    {
        unsigned char tmp;
        conn.ReadUnsignedChar(&tmp);
        SetValue(tmp);
    }
    else if(dataType == INT_TYPE)
    {
        int tmp;
        conn.ReadInt(&tmp);
        SetValue(tmp);
    }
    else if(dataType == LONG_TYPE)
    {
        long tmp;
        conn.ReadLong(&tmp);
        SetValue(tmp);
    }
    else if(dataType == FLOAT_TYPE)
    {
        float tmp;
        conn.ReadFloat(&tmp);
        SetValue(tmp);
    }
    else if(dataType == DOUBLE_TYPE)
    {
        double tmp;
        conn.ReadDouble(&tmp);
        SetValue(tmp);
    }
    else if(dataType == STRING_TYPE)
    {
        string tmp;
        conn.ReadString(tmp);
        SetValue(tmp);
    }
    else if(dataType == BOOL_VECTOR_TYPE)
    {
        boolVector &vec = AsBoolVector();
        conn.ReadInt(&s);
        vec.clear();
        if(s > 0)
        {
            vec.reserve(s);
            for(int i=0;i<s;i++)
            {
                unsigned char tmp;
                conn.ReadChar(&tmp);
                vec.push_back(tmp==1);
            }
        }
    }
    else if(dataType == CHAR_VECTOR_TYPE)
    {
        charVector &vec = AsCharVector();
        conn.ReadInt(&s);
        vec.clear();
        if(s > 0)
        {
            vec.reserve(s);
            for(int i=0;i<s;i++)
            {
                char tmp;
                conn.ReadChar((unsigned char *)&tmp);
                vec.push_back(tmp);
            }
        }
    }
    else if(dataType == UNSIGNED_CHAR_VECTOR_TYPE)
    {
        unsignedCharVector &vec = AsUnsignedCharVector();
        conn.ReadInt(&s);
        vec.clear();
        if(s > 0)
        {
            vec.reserve(s);
            for(int i=0;i<s;i++)
            {
                unsigned char tmp;
                conn.ReadUnsignedChar(&tmp);
                vec.push_back(tmp);
            }
        }
    }
    else if(dataType == INT_VECTOR_TYPE)
    {
        intVector &vec = AsIntVector();
        conn.ReadInt(&s);
        vec.clear();
        if(s > 0)
        {
            vec.reserve(s);
            for(int i=0;i<s;i++)
            {
                int tmp;
                conn.ReadInt(&tmp);
                vec.push_back(tmp);
            }
        }
    }
    else if(dataType == LONG_VECTOR_TYPE)
    {
        longVector &vec = AsLongVector();
        conn.ReadInt(&s);
        vec.clear();
        if(s > 0)
        {
            vec.reserve(s);
            for(int i=0;i<s;i++)
            {
                long tmp;
                conn.ReadLong(&tmp);
                vec.push_back(tmp);
            }
        }
    }
    else if(dataType == FLOAT_VECTOR_TYPE)
    {
        floatVector &vec = AsFloatVector();
        conn.ReadInt(&s);
        vec.clear();
        if(s > 0)
        {
            vec.reserve(s);
            for(int i=0;i<s;i++)
            {
                float tmp;
                conn.ReadFloat(&tmp);
                vec.push_back(tmp);
            }
        }
    }
    else if(dataType == DOUBLE_VECTOR_TYPE)
    {
        doubleVector &vec = AsDoubleVector();
        conn.ReadInt(&s);
        vec.clear();
        if(s > 0)
        {
            vec.reserve(s);
            for(int i=0;i<s;i++)
            {
                double tmp;
                conn.ReadDouble(&tmp);
                vec.push_back(tmp);
            }
        }
    }
    else if(dataType == STRING_VECTOR_TYPE)
    {
        stringVector &vec = AsStringVector();
        conn.ReadInt(&s);
        vec.clear();
        if(s > 0)
        {
            vec.reserve(s);
            for(int i=0;i<s;i++)
            {
                string tmp;
                conn.ReadString(tmp);
                vec.push_back(tmp);
            }
        }
    }
}
