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

#include <DataNode.h>
#include "ImproperUseException.h"

#include <cstring>
#include <sstream>
using std::ostringstream;

// Static members. These are returned from the As functions when there
// is no data. It ensures that the references that are returned are safe.
std::string         DataNode::bogusString;
charVector          DataNode::bogusCharVector;
unsignedCharVector  DataNode::bogusUnsignedCharVector;
intVector           DataNode::bogusIntVector;
longVector          DataNode::bogusLongVector;
floatVector         DataNode::bogusFloatVector;
doubleVector        DataNode::bogusDoubleVector;
stringVector        DataNode::bogusStringVector;
MapNode             DataNode::bogusMapNode;

// ****************************************************************************
// Method: DataNode::DataNode
//
// Purpose: 
//   These are the constructors for the DataNode class. There is one
//   constructor for each type of node.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 12:22:58 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Mon Feb 26 16:01:24 PST 2001
//    Added unsigned chars.
//   
// ****************************************************************************

DataNode::DataNode(const std::string &name) : Key(name)
{
    NodeType = INTERNAL_NODE;
    Length = 0;
    Data = 0;
}

DataNode::DataNode(const std::string &name, char val) : Key(name)
{
    NodeType = CHAR_NODE;
    Length = 0;
    Data = (void *)(new char(val));
}

DataNode::DataNode(const std::string &name, unsigned char val) : Key(name)
{
    NodeType = UNSIGNED_CHAR_NODE;
    Length = 0;
    Data = (void *)(new unsigned char(val));
}

DataNode::DataNode(const std::string &name, int val) : Key(name)
{
    NodeType = INT_NODE;
    Length = 0;
    Data = (void *)(new int(val));
}

DataNode::DataNode(const std::string &name, long val) : Key(name)
{
    NodeType = LONG_NODE;
    Length = 0;
    Data = (void *)(new long(val));
}

DataNode::DataNode(const std::string &name, float val) : Key(name)
{
    NodeType = FLOAT_NODE;
    Length = 0;
    Data = (void *)(new float(val));
}

DataNode::DataNode(const std::string &name, double val) : Key(name)
{
    NodeType = DOUBLE_NODE;
    Length = 0;
    Data = (void *)(new double(val));
}

DataNode::DataNode(const std::string &name, const std::string &val) : Key(name)
{
    NodeType = STRING_NODE;
    Length = 0;
    Data = (void *)(new std::string(val));
}

DataNode::DataNode(const std::string &name, bool val) : Key(name)
{
    NodeType = BOOL_NODE;
    Length = 0;
    Data = (void *)(new bool(val));
}

DataNode::DataNode(const std::string &name, const char *vals, int len) : Key(name)
{
    NodeType = CHAR_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new char[len]);
        memcpy(Data, (void *)vals, len * sizeof(char));
    }
    else
        Data = 0;
}

DataNode::DataNode(const std::string &name, const unsigned char *vals, int len) : Key(name)
{
    NodeType = UNSIGNED_CHAR_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new unsigned char[len]);
        memcpy(Data, (void *)vals, len * sizeof(unsigned char));
    }
    else
        Data = 0;
}

DataNode::DataNode(const std::string &name, const int *vals, int len) : Key(name)
{
    NodeType = INT_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new int[len]);
        memcpy(Data, (void *)vals, len * sizeof(int));
    }
    else
        Data = 0;
}

DataNode::DataNode(const std::string &name, const long *vals, int len) : Key(name)
{
    NodeType = LONG_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new long[len]);
        memcpy(Data, (void *)vals, len * sizeof(long));
    }
    else
        Data = 0;
}

DataNode::DataNode(const std::string &name, const float *vals, int len) : Key(name)
{
    NodeType = FLOAT_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new float[len]);
        memcpy(Data, (void *)vals, len * sizeof(float));
    }
    else
        Data = 0;
}

DataNode::DataNode(const std::string &name, const double *vals, int len) : Key(name)
{
    NodeType = DOUBLE_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new double[len]);
        memcpy(Data, (void *)vals, len * sizeof(double));
    }
    else
        Data = 0;
}

DataNode::DataNode(const std::string &name, const std::string *vals, int len) : Key(name)
{
    NodeType = STRING_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        std::string *sptr = new std::string[len];
        Data = (void *)sptr;   
        for(int i = 0; i < len; ++i)
            sptr[i] = vals[i];
    }
    else
        Data = 0;
}

DataNode::DataNode(const std::string &name, const bool *vals, int len) : Key(name)
{
    NodeType = BOOL_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new bool[len]);
        memcpy(Data, (void *)vals, len * sizeof(bool));
    }
    else
        Data = 0;
}

DataNode::DataNode(const std::string &name, const charVector &vec) : Key(name)
{
    NodeType = CHAR_VECTOR_NODE;
    Length = 0;
    Data = (void *)(new charVector(vec));
}

DataNode::DataNode(const std::string &name, const unsignedCharVector &vec) : Key(name)
{
    NodeType = UNSIGNED_CHAR_VECTOR_NODE;
    Length = 0;
    Data = (void *)(new unsignedCharVector(vec));
}

DataNode::DataNode(const std::string &name, const intVector &vec) : Key(name)
{
    NodeType = INT_VECTOR_NODE;
    Length = 0;
    Data = (void *)(new intVector(vec));
}

DataNode::DataNode(const std::string &name, const longVector &vec) : Key(name)
{
    NodeType = LONG_VECTOR_NODE;
    Length = 0;
    Data = (void *)(new longVector(vec));
}

DataNode::DataNode(const std::string &name, const floatVector &vec) : Key(name)
{
    NodeType = FLOAT_VECTOR_NODE;
    Length = 0;
    Data = (void *)(new floatVector(vec));
}

DataNode::DataNode(const std::string &name, const doubleVector &vec) : Key(name)
{
    NodeType = DOUBLE_VECTOR_NODE;
    Length = 0;
    Data = (void *)(new doubleVector(vec));
}

DataNode::DataNode(const std::string &name, const stringVector &vec) : Key(name)
{
    NodeType = STRING_VECTOR_NODE;
    Length = 0;
    Data = (void *)(new stringVector(vec));
}

DataNode::DataNode(const std::string &name, const MapNode &val) : Key(name)
{
    NodeType = MAP_NODE_NODE;
    Length = 0;
    Data = (void *)(new MapNode(val));
}

// ****************************************************************************
// Method: DataNode::~DataNode
//
// Purpose: 
//   Destructor for the DataNode class. It frees any data that the node
//   has based on the type of the node.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 12:22:11 PDT 2000
//
// Modifications:
//   Jeremy Meredith, Mon Feb 26 16:01:37 PST 2001
//   Added unsigned chars.
//
//   Brad Whitlock, Fri Mar 21 10:48:13 PDT 2003
//   I moved the implementation into FreeData.
//
// ****************************************************************************

DataNode::~DataNode()
{
    FreeData();
}

// ****************************************************************************
// Method: DataNode::FreeData
//
// Purpose: 
//   Frees the data in the DataNode and sets the Data pointer to 0.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 10:47:29 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
DataNode::FreeData()
{
    if(Data == 0)
        return;

    // Any node types that have dynamically allocated storage must
    // be freed specially.
    switch(NodeType)
    {
    case INTERNAL_NODE:
        { // new scope
            if(Length == 1)
            {
                // Delete the child.
                DataNode *node = (DataNode *)Data;
                delete node;
            }
            else if(Length > 1)
            {
                // Delete all the children
                DataNode **nodeArray = (DataNode **)Data;
                for(int i = 0; i < Length; ++i)
                    delete nodeArray[i];

                // Delete the children's pointer array
                if(Length > 1)
                    delete [] nodeArray;
            }
        }
        break;
    case CHAR_NODE:
        { // new scope
            char *cptr = (char *)Data;
            delete cptr;
        }
        break;        
    case UNSIGNED_CHAR_NODE:
        { // new scope
            unsigned char *uptr = (unsigned char *)Data;
            delete uptr;
        }
        break;        
    case INT_NODE:
        { // new scope
            int *iptr = (int *)Data;
            delete iptr;
        }
        break;        
    case LONG_NODE:
        { // new scope
            long *lptr = (long *)Data;
            delete lptr;
        }
        break;        
    case FLOAT_NODE:
        { // new scope
            float *fptr = (float *)Data;
            delete fptr;
        }
        break;
    case DOUBLE_NODE:
        { // new scope
            double *dptr = (double *)Data;
            delete dptr;
        }
        break;
    case STRING_NODE:
        { // new scope
            std::string *sptr = (std::string *)Data;
            delete sptr;
        }
        break;
    case BOOL_NODE:
        { // new scope
            bool *bptr = (bool *)Data;
            delete bptr;
        }
        break;        
    case CHAR_ARRAY_NODE:
        { // new scope
            char *cptr = (char *)Data;
            delete [] cptr;
        }
        break;        
    case UNSIGNED_CHAR_ARRAY_NODE:
        { // new scope
            unsigned char *uptr = (unsigned char *)Data;
            delete [] uptr;
        }
        break;        
    case INT_ARRAY_NODE:
        { // new scope
            int *iptr = (int *)Data;
            delete [] iptr;
        }
        break;        
    case LONG_ARRAY_NODE:
        { // new scope
            long *lptr = (long *)Data;
            delete [] lptr;
        }
        break;        
    case FLOAT_ARRAY_NODE:
        { // new scope
            float *fptr = (float *)Data;
            delete [] fptr;
        }
        break;
    case DOUBLE_ARRAY_NODE:
        { // new scope
            double *dptr = (double *)Data;
            delete [] dptr;
        }
        break;
    case STRING_ARRAY_NODE:
        { // new scope
            std::string *sptr = (std::string *)Data;
            delete [] sptr;
        }
        break;
    case BOOL_ARRAY_NODE:
        { // new scope
            bool *bptr = (bool *)Data;
            delete [] bptr;
        }
        break;
    case CHAR_VECTOR_NODE:
        { // new scope
            charVector *cptr = (charVector *)Data;
            delete cptr;
        }
        break;        
    case UNSIGNED_CHAR_VECTOR_NODE:
        { // new scope
            unsignedCharVector *uptr = (unsignedCharVector *)Data;
            delete uptr;
        }
        break;        
    case INT_VECTOR_NODE:
        { // new scope
            intVector *iptr = (intVector *)Data;
            delete iptr;
        }
        break;        
    case LONG_VECTOR_NODE:
        { // new scope
            longVector *lptr = (longVector *)Data;
            delete lptr;
        }
        break;        
    case FLOAT_VECTOR_NODE:
        { // new scope
            floatVector *fptr = (floatVector *)Data;
            delete fptr;
        }
        break;
    case DOUBLE_VECTOR_NODE:
        { // new scope
            doubleVector *dptr = (doubleVector *)Data;
            delete dptr;
        }
        break;
    case STRING_VECTOR_NODE:
        { // new scope
            stringVector *sptr = (stringVector *)Data;
            delete sptr;
        }
        break;
    case BOOL_VECTOR_NODE:
        // Do nothing since it can't be instantiated.
        break;
    case MAP_NODE_NODE:
        { // new scope
            MapNode *mptr = (MapNode *)Data;
            delete mptr;
        }
        break;
    }

    Data = 0;
    Length = 0;
}

//
// Methods to return the data as a value of a certain type.
//

char
DataNode::AsChar() const
{
    return AsValue<char>();
}

unsigned char
DataNode::AsUnsignedChar() const
{
    return AsValue<unsigned char>();
}

int
DataNode::AsInt() const
{
    return AsValue<int>();
}

long
DataNode::AsLong() const
{
    return AsValue<long>();
}

float
DataNode::AsFloat() const
{
    return AsValue<float>();
}

double
DataNode::AsDouble() const
{
    return AsValue<double>();
}

const std::string &
DataNode::AsString() const
{
    return AsClass<std::string>(bogusString);
}

bool
DataNode::AsBool() const
{
    return AsValue<bool>();
}

const char *
DataNode::AsCharArray() const
{
    return AsArray<char>();
}

const unsigned char *
DataNode::AsUnsignedCharArray() const
{
    return AsArray<unsigned char>();
}

const int *
DataNode::AsIntArray() const
{
    return AsArray<int>();
}

const long *
DataNode::AsLongArray() const
{
    return AsArray<long>();
}

const float *
DataNode::AsFloatArray() const
{
    return AsArray<float>();
}

const double *
DataNode::AsDoubleArray() const
{
    return AsArray<double>();
}

const std::string *
DataNode::AsStringArray() const
{
    return AsArray<std::string>();
}

const bool *
DataNode::AsBoolArray() const
{
    return AsArray<bool>();
}

const charVector &
DataNode::AsCharVector() const
{
    return AsClass<charVector>(bogusCharVector);
}

const unsignedCharVector &
DataNode::AsUnsignedCharVector() const
{
    return AsClass<unsignedCharVector>(bogusUnsignedCharVector);
}

const intVector &
DataNode::AsIntVector() const
{
    return AsClass<intVector>(bogusIntVector);
}

const longVector &
DataNode::AsLongVector() const
{
    return AsClass<longVector>(bogusLongVector);
}

const floatVector &
DataNode::AsFloatVector() const
{
    return AsClass<floatVector>(bogusFloatVector);
}

const doubleVector &
DataNode::AsDoubleVector() const
{
    return AsClass<doubleVector>(bogusDoubleVector);
}

const stringVector &
DataNode::AsStringVector() const
{
    return AsClass<stringVector>(bogusStringVector);
}


const MapNode &
DataNode::AsMapNode() const
{
    return AsClass<MapNode>(bogusMapNode);
}

// traits classes for supported data types
template <typename T> struct DataNodeTraits;
template<> struct DataNodeTraits <char> { static const int Code = CHAR_NODE; };
template<> struct DataNodeTraits <unsigned char> { static const int Code = UNSIGNED_CHAR_NODE; };
template<> struct DataNodeTraits <int> { static const int Code = INT_NODE; };
template<> struct DataNodeTraits <long> { static const int Code = LONG_NODE; };
template<> struct DataNodeTraits <float> { static const int Code = FLOAT_NODE; };
template<> struct DataNodeTraits <double> { static const int Code = DOUBLE_NODE; };
template<> struct DataNodeTraits <std::string> { static const int Code = STRING_NODE; };
template<> struct DataNodeTraits <bool> { static const int Code = BOOL_NODE; };
template<> struct DataNodeTraits <char*> { static const int Code = CHAR_ARRAY_NODE; };
template<> struct DataNodeTraits <unsigned char*> { static const int Code = UNSIGNED_CHAR_ARRAY_NODE; };
template<> struct DataNodeTraits <int*> { static const int Code = INT_ARRAY_NODE; };
template<> struct DataNodeTraits <long*> { static const int Code = LONG_ARRAY_NODE; };
template<> struct DataNodeTraits <float*> { static const int Code = FLOAT_ARRAY_NODE; };
template<> struct DataNodeTraits <double*> { static const int Code = DOUBLE_ARRAY_NODE; };
template<> struct DataNodeTraits <std::string*> { static const int Code = STRING_ARRAY_NODE; };
template<> struct DataNodeTraits <bool*> { static const int Code = BOOL_ARRAY_NODE; };
template<> struct DataNodeTraits <charVector> { static const int Code = CHAR_VECTOR_NODE; };
template<> struct DataNodeTraits <unsignedCharVector> { static const int Code = UNSIGNED_CHAR_VECTOR_NODE; };
template<> struct DataNodeTraits <intVector> { static const int Code = INT_VECTOR_NODE; };
template<> struct DataNodeTraits <longVector> { static const int Code = LONG_VECTOR_NODE; };
template<> struct DataNodeTraits <floatVector> { static const int Code = FLOAT_VECTOR_NODE; };
template<> struct DataNodeTraits <doubleVector> { static const int Code = DOUBLE_VECTOR_NODE; };
template<> struct DataNodeTraits <stringVector> { static const int Code = STRING_VECTOR_NODE; };
template<> struct DataNodeTraits <boolVector> { static const int Code = BOOL_VECTOR_NODE; };
template<> struct DataNodeTraits <MapNode> { static const int Code = MAP_NODE_NODE; };

// ****************************************************************************
// Method: DataNode::AsArray
//
// Purpose:
//   helper for casting from void * to supported static array
//   types. the main point of this method is to throw an exception
//   for the cases that would result in invalid reinterpret
//   cast and thus lead to undefined behavior.
//
// Programmer: Burlen Loring
// Creation:   Wed Jul 16 11:26:26 PDT 2014
//
// Modifications:
//
// ****************************************************************************
template <typename RetType>
const RetType *DataNode::AsArray() const
{
    if (NodeType != DataNodeTraits<RetType*>::Code)
    {
        // invalid cast from void*
        ostringstream oss;
        oss << "Error: DataNode::AsArray Key=" << Key
          << ", cast from " << NodeTypeName(NodeType)
          << " to " << NodeTypeName(DataNodeTraits<RetType*>::Code);
        cerr << oss.str() << endl;
        EXCEPTION1(ImproperUseException, oss.str().c_str());
        return NULL;
    }
    if (!Data || !Length)
    {
        // object has no data to cast or would not
        // cast to a valid array
        ostringstream oss;
        oss << "DataNode::AsArray Key=" << Key
          << ", Acccess to uninitialized object detected";
        cerr << "Error: " << oss.str() << endl;
        EXCEPTION1(ImproperUseException, oss.str().c_str());
        return NULL;
    }
    return reinterpret_cast<RetType*>(Data);
}

// ****************************************************************************
// Method: DataNode::AsClass
//
// Purpose:
//   helper for casting from void * to supported class types
//   the main point of this method is to throw an exception
//   for the cases that would result in invalid reinterpret
//   cast and thus lead to undefined behavior.
//
// Programmer: Burlen Loring
// Creation:   Wed Jul 16 11:26:26 PDT 2014
//
// Modifications:
//
// ****************************************************************************
template <typename RetType>
const RetType &DataNode::AsClass(RetType &substitute) const
{
    if (NodeType != DataNodeTraits<RetType>::Code)
    {
        // invalid cast from void*
        ostringstream oss;
        oss << "Error: DataNode::AsClass Key=" << Key
          << ", no cast from " << NodeTypeName(NodeType)
          << " to " << NodeTypeName(DataNodeTraits<RetType>::Code);
        cerr << oss.str() << endl;
        EXCEPTION1(ImproperUseException, oss.str().c_str());
        return substitute;
    }
    if (!Data)
    {
        // object has no data to cast
        ostringstream oss;
        oss << "Error: DataNode::AsClass Key=" << Key
          << ", Acccess to uninitialized object detected";
        cerr << oss.str() << endl;
        EXCEPTION1(ImproperUseException, oss.str().c_str());
        return substitute;
    }
    return *reinterpret_cast<RetType*>(Data);
}

// ****************************************************************************
// Method: DataNode::AsValue
//
// Purpose:
//   helper for casting from void * to a single value
//   the main point of this method is to throw an exception
//   for the cases that would result in invalid reinterpret
//   cast and thus undefined behavior.
//
// Programmer: Burlen Loring
// Creation:   Wed Jul 16 11:26:26 PDT 2014
//
// Modifications:
//
// ****************************************************************************
template <typename RetType>
RetType DataNode::AsValue() const
{
    RetType value = RetType();
    if (!Data)
    {
        // object has no data to cast
        ostringstream oss;
        oss << "Error: DataNode::AsClass Key=" << Key
          << ", Acccess to uninitialized object detected";
        cerr << oss.str() << endl;
        EXCEPTION1(ImproperUseException, oss.str().c_str());
        return value;
    }
    // cast from our internal void * to a pointer of it's
    // actual type then dereference and cast to the requested
    // type
    switch (NodeType)
    {
        case CHAR_NODE:
            value = static_cast<RetType>(*reinterpret_cast<char*>(Data));
            break;
        case UNSIGNED_CHAR_NODE:
            value = static_cast<RetType>(*reinterpret_cast<unsigned char*>(Data));
            break;
        case INT_NODE:
            value = static_cast<RetType>(*reinterpret_cast<int*>(Data));
            break;
        case LONG_NODE:
            value = static_cast<RetType>(*reinterpret_cast<long*>(Data));
            break;
        case FLOAT_NODE:
            value = static_cast<RetType>(*reinterpret_cast<float*>(Data));
            break;
        case DOUBLE_NODE:
            value = static_cast<RetType>(*reinterpret_cast<double*>(Data));
            break;
        case BOOL_NODE:
            value = static_cast<RetType>(*reinterpret_cast<bool*>(Data));
            break;
        default:
            // invalid cast attempted
            ostringstream oss;
            oss << "Error: DataNode::AsValue Key=" << Key
              << ", no cast from " << NodeTypeName(NodeType)
              << " to " << NodeTypeName(DataNodeTraits<RetType>::Code);
            cerr << oss.str() << endl;
            EXCEPTION1(ImproperUseException, oss.str().c_str());
            break;
    }
    return value;
}

//
// Methods to convert the DataNode to a new type with new data.
//

void
DataNode::SetChar(char val)
{
    FreeData();
    NodeType = CHAR_NODE;
    Data = (void *)(new char(val));
}

void
DataNode::SetUnsignedChar(unsigned char val)
{
    FreeData();
    NodeType = UNSIGNED_CHAR_NODE;
    Data = (void *)(new unsigned char(val));
}

void
DataNode::SetInt(int val)
{
    FreeData();
    NodeType = INT_NODE;
    Data = (void *)(new int(val));
}

void
DataNode::SetLong(long val)
{
    FreeData();
    NodeType = LONG_NODE;
    Data = (void *)(new long(val));
}

void
DataNode::SetFloat(float val)
{
    FreeData();
    NodeType = FLOAT_NODE;
    Data = (void *)(new float(val));
}

void
DataNode::SetDouble(double val)
{
    FreeData();
    NodeType = DOUBLE_NODE;
    Data = (void *)(new double(val));
}

void
DataNode::SetString(const std::string &val)
{
    FreeData();
    NodeType = STRING_NODE;
    Data = (void *)(new std::string(val));
}

void
DataNode::SetBool(bool val)
{
    FreeData();
    NodeType = BOOL_NODE;
    Data = (void *)(new bool(val));
}

void
DataNode::SetCharArray(const char *vals, int len)
{
    FreeData();
    NodeType = CHAR_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new char[len]);
        memcpy(Data, (void *)vals, len * sizeof(char));
    }
    else
        Data = 0;
}

void
DataNode::SetUnsignedCharArray(const unsigned char *vals, int len)
{
    FreeData();
    NodeType = UNSIGNED_CHAR_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new unsigned char[len]);
        memcpy(Data, (void *)vals, len * sizeof(unsigned char));
    }
    else
        Data = 0;
}

void
DataNode::SetIntArray(const int *vals, int len)
{
    FreeData();
    NodeType = INT_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new int[len]);
        memcpy(Data, (void *)vals, len * sizeof(int));
    }
    else
        Data = 0;
}

void
DataNode::SetLongArray(const long *vals, int len)
{
    FreeData();
    NodeType = LONG_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new long[len]);
        memcpy(Data, (void *)vals, len * sizeof(long));
    }
    else
        Data = 0;
}

void
DataNode::SetFloatArray(const float *vals, int len)
{
    FreeData();
    NodeType = FLOAT_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new float[len]);
        memcpy(Data, (void *)vals, len * sizeof(float));
    }
    else
        Data = 0;
}

void
DataNode::SetDoubleArray(const double *vals, int len)
{
    FreeData();
    NodeType = DOUBLE_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new double[len]);
        memcpy(Data, (void *)vals, len * sizeof(double));
    }
    else
        Data = 0;
}

void
DataNode::SetStringArray(const std::string *vals, int len)
{
    FreeData();
    NodeType = STRING_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        std::string *sptr = new std::string[len];
        Data = (void *)sptr;   
        for(int i = 0; i < len; ++i)
            sptr[i] = vals[i];
    }
    else
        Data = 0;
}

void
DataNode::SetBoolArray(const bool *vals, int len)
{
    FreeData();
    NodeType = BOOL_ARRAY_NODE;
    Length = len;
    if(len > 0)
    {
        Data = (void *)(new bool[len]);
        memcpy(Data, (void *)vals, len * sizeof(bool));
    }
    else
        Data = 0;
}

void
DataNode::SetCharVector(const charVector &vec)
{
    FreeData();
    NodeType = CHAR_VECTOR_NODE;
    Data = (void *)(new charVector(vec));
}

void
DataNode::SetUnsignedCharVector(const unsignedCharVector &vec)
{
    FreeData();
    NodeType = UNSIGNED_CHAR_VECTOR_NODE;
    Data = (void *)(new unsignedCharVector(vec));
}

void
DataNode::SetIntVector(const intVector &vec)
{
    FreeData();
    NodeType = INT_VECTOR_NODE;
    Data = (void *)(new intVector(vec));
}

void
DataNode::SetLongVector(const longVector &vec)
{
    FreeData();
    NodeType = LONG_VECTOR_NODE;
    Data = (void *)(new longVector(vec));
}

void
DataNode::SetFloatVector(const floatVector &vec)
{
    FreeData();
    NodeType = FLOAT_VECTOR_NODE;
    Data = (void *)(new floatVector(vec));
}

void
DataNode::SetDoubleVector(const doubleVector &vec)
{
    FreeData();
    NodeType = DOUBLE_VECTOR_NODE;
    Data = (void *)(new doubleVector(vec));
}

void
DataNode::SetStringVector(const stringVector &vec)
{
    FreeData();
    NodeType = STRING_VECTOR_NODE;
    Data = (void *)(new stringVector(vec));
}

void
DataNode::SetMapNode(const MapNode &val)
{
    FreeData();
    NodeType = MAP_NODE_NODE;
    Data = (void *)(new MapNode(val));
}

// ****************************************************************************
// Method: DataNode::GetNode
//
// Purpose: 
//   Returns a pointer to the node having the specified key. If a
//   parentNode is supplied, then only the children of that node are
//   searched.
//
// Arguments:
//   key : The name of the node to look for.
//
// Returns:    
//   A pointer to the node having the specified key, or 0 if the node
//   is not found.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 12:07:56 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 11:29:24 PDT 2008
//   Removed recursion so we only get the results that we're looking for or
//   we get NULL if the item we're looking for does not exist.
//
// ****************************************************************************

DataNode *
DataNode::GetNode(const std::string &key)
{
    DataNode *retval = 0;

    if(key == Key)
        retval = this;
    else if(NodeType == INTERNAL_NODE)
    {
        if(Length == 1)
        {
            DataNode *node = (DataNode *)(Data);
            if(key == node->Key)
                retval = node;
        }
        else if(Length > 1)
        {
            DataNode **nodeArray = (DataNode **)(Data);

            for(int i = 0; i < Length; ++i)
            {
                if(key == nodeArray[i]->Key)
                {
                    retval = nodeArray[i];
                    break;
                }
            }
        }        
    }

    return retval;
}

// ****************************************************************************
// Method: DataNode::SearchForNode
//
// Purpose: 
//   Recursively searches for the named node in the tree.
//
// Arguments:
//   key        : The key to search for.
//   parentNode : Optional argument providing the root of the tree to search.
//
// Returns:    A DataNode on success; NULL on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 14 14:51:17 PST 2008
//
// Modifications:
//   
// ****************************************************************************

DataNode *
DataNode::SearchForNode(const std::string &key, DataNode *parentNode)
{
    DataNode *searchNode, *intermediate, *retval = 0;

    // Determine which node's children to search.        
    if(parentNode == 0)
        searchNode = this;
    else
        searchNode = parentNode;

    if(key == searchNode->Key)
        retval = searchNode;
    else if(searchNode->NodeType == INTERNAL_NODE)
    {
        if(searchNode->Length == 1)
        {
            DataNode *nodeArray = (DataNode *)(searchNode->Data);
            intermediate = SearchForNode(key, nodeArray);
            if(intermediate != 0)
            {
                retval = intermediate;
            }
        }
        else if(searchNode->Length > 1)
        {
            DataNode **nodeArray = (DataNode **)(searchNode->Data);

            for(int i = 0; i < searchNode->Length; ++i)
            {
                intermediate = SearchForNode(key, nodeArray[i]);
                if(intermediate != 0)
                {
                    retval = intermediate;
                    break;
                }
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: DataNode::AddNode
//
// Purpose: 
//   Adds a child node to the current node if the current node is of
//   type INTERNAL_NODE.
//
// Arguments:
//   node : A pointer to the node that will be added.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 12:12:38 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 1 13:27:33 PST 2002
//   Fixed a memory leak.
//
// ****************************************************************************

void
DataNode::AddNode(DataNode *node)
{
    if(NodeType != INTERNAL_NODE || node == 0)
        return;

    int i;
    if(Length == 0)
    {
        Length = 1;
        Data = (void *)node;
    }
    else if(Length == 1)
    {
        DataNode **nodeArray = new DataNode*[2];
        nodeArray[0] = (DataNode *)Data;
        nodeArray[1] = node;
        Data = (void *)nodeArray;
        Length = 2;
    }
    else
    {
        DataNode **nodeArray = new DataNode*[Length + 1];
        DataNode **dNptr = (DataNode **)Data;
        for(i = 0; i < Length; ++i)
            nodeArray[i] = dNptr[i];
        nodeArray[i] = node;
        delete [] dNptr;
        Data = (void *)nodeArray;
        ++Length;
    }
}

// ****************************************************************************
// Method: DataNode::RemoveNode
//
// Purpose: 
//   Removes the specified node if it exists under the current node.
//
// Arguments:
//   node : The node to remove.
//
// Programmer: Eric Brugger
// Creation:   Tue Mar 27 15:57:42 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
DataNode::RemoveNode(DataNode *node, bool deleteNode)
{
    if(NodeType != INTERNAL_NODE)
        return;
    if(Length < 1)
        return;

    if(Length == 1)
    {
        if((DataNode *)Data == node)
        {
            if(deleteNode)
                delete node;
            Data = 0;
            Length = 0;
        }
    }
    else
    {
        DataNode **nodeArray = (DataNode **)Data;
        bool start = false;

        for(int i = 0; i < Length; ++i)
        {
            if(!start && nodeArray[i] == node)
            {
                if(deleteNode)
                    delete nodeArray[i];
                start = true;
            }

            if(start && (i < (Length - 1)))
                nodeArray[i] = nodeArray[i + 1];
        }
        if(start)
        {
            --Length;

            // If we're down to 1, convert to a single pointer.
            if(Length == 1)
            {
                DataNode *temp = nodeArray[0];
                delete [] nodeArray;
                Data = (void *)temp;
            } 
        }
    }
}

// ****************************************************************************
// Method: DataNode::RemoveNode
//
// Purpose: 
//   Removes the node with the specified key if it exists under the 
//   current node.
//
// Arguments:
//   key : The key of the node to remove.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 12:14:24 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Feb 2 15:38:05 PST 2004
//   I added an optional argument to delete the node.
//
// ****************************************************************************

void
DataNode::RemoveNode(const std::string &key, bool deleteNode)
{
    if(NodeType != INTERNAL_NODE)
        return;
    if(Length < 1)
        return;

    if(Length == 1)
    {
        DataNode *node = (DataNode *)Data;
        if(node->Key == key)
        {
            if(deleteNode)
                delete node;
            Data = 0;
            Length = 0;
        }
    }
    else
    {
        DataNode **nodeArray = (DataNode **)Data;
        bool start = false;

        for(int i = 0; i < Length; ++i)
        {
            if(!start && nodeArray[i]->Key == key)
            {
                if(deleteNode)
                    delete nodeArray[i];
                start = true;
            }

            if(start && (i < (Length - 1)))
                nodeArray[i] = nodeArray[i + 1];
        }
        if(start)
        {
            --Length;

            // If we're down to 1, convert to a single pointer.
            if(Length == 1)
            {
                DataNode *temp = nodeArray[0];
                delete [] nodeArray;
                Data = (void *)temp;
            } 
        }
    }
}

//
// Methods to get/set some of the private fields.
//

const std::string &
DataNode::GetKey() const
{
    return Key;
}

void
DataNode::SetKey(const std::string &k)
{
    Key = k;
}

NodeTypeEnum
DataNode::GetNodeType() const
{
    return NodeType;
}

int
DataNode::GetLength() const
{
    return Length;
}

int
DataNode::GetNumChildren() const
{
    return Length;
}

// ****************************************************************************
// Method: DataNode::GetNumChildObjects
//
// Purpose: 
//   Return the number of children that are of type INTERNAL_NODE
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:54:21 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
DataNode::GetNumChildObjects() const
{
    int retval = 0;

    if(Length == 1)
    {
        DataNode *child = (DataNode *)Data;
        if(child->NodeType == INTERNAL_NODE)
            retval = 1;
    }
    else if(Length > 0)
    {
        DataNode **children = (DataNode **)Data;
        for(int i = 0; i < Length; ++i)
        {
            if(children[i]->NodeType == INTERNAL_NODE)
                ++retval;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: DataNode::GetChildren
//
// Purpose: 
//   Returns an array of DataNode pointers that point to the node's
//   children. If there are no children, 0 is returned.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 15:11:24 PST 2000
//
// Modifications:
//   
// ****************************************************************************

DataNode **
DataNode::GetChildren()
{
    if(NodeType == INTERNAL_NODE)
    {
        if(Length == 0)
            return 0;
        else if(Length == 1)
            return ((DataNode **)&Data);
        else
            return ((DataNode **)Data);
    }
    else
        return 0;
}

//
// Enum name lookup stuff.
//

const char *NodeTypeName(int e)
{
    static const char *NodeTypeNameLookup[] = {
        "",
        "char", "unsigned char", "int", "long", "float",
        "double", "string", "bool",
        "charArray", "unsignedCharArray", "intArray", "longArray", "floatArray",
        "doubleArray", "stringArray", "boolArray",
        "charVector", "unsignedCharVector", "intVector", "longVector", "floatVector",
        "doubleVector", "stringVector", "boolVector", "MapNode"
        };

    return NodeTypeNameLookup[e];
}

// ****************************************************************************
// Function: GetNodeType
//
// Purpose: 
//   Converts a named type to a NodeTypeEnum.
//
// Returns:    The NodeTypeEnum corresponding to the named type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 18 23:30:52 PST 2001
//
// Modifications:
//   
// ****************************************************************************

NodeTypeEnum GetNodeType(const char *str)
{
    for(int i = 1; i < 24; ++i)
    {
        if (strcmp(str, NodeTypeName(i)) == 0)
        {
            return static_cast<NodeTypeEnum>(i);
        }
    }
    return INTERNAL_NODE;
}

// ****************************************************************************
// Method: DataNode::Print
//
// Purpose: 
//   Print method so we can see the structure of the tree in debug logs.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 22 12:16:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
DataNode::Print(ostream &os, int indent)
{
    std::string space;
    for(int i = 0; i < indent; ++i)
        space += " ";

    os << space.c_str() << this->GetKey().c_str() << endl;
    if(this->GetNodeType() == INTERNAL_NODE)
    {
        os << space.c_str() << "{" << endl;
        for(int j = 0; j < this->GetNumChildren(); ++j)
            if(this->GetChildren()[j] == NULL)
                os << space.c_str() << "   NULL";
            else
                this->GetChildren()[j]->Print(os, indent + 4);
        os << space.c_str() << "}" << endl;
    }
}
