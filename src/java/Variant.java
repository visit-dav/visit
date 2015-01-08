// ***************************************************************************
//
// Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-442911
// All rights reserved.
//
// This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or other materials provided with the distribution.
//  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
// LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
// DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ***************************************************************************

package llnl.visit;

import java.util.Vector;
import java.lang.*;

// ****************************************************************************
// Class: Variant
//
// Purpose:
//   This class contains data of different types.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb  2 11:53:39 PST 2012
//
// Modifications:
//   
// ****************************************************************************

public class Variant extends java.lang.Object
{
    public final static int VARIANT_EMPTY_TYPE = 0;
    public final static int VARIANT_BOOL_TYPE = 1;
    public final static int VARIANT_CHAR_TYPE = 2;
    public final static int VARIANT_UNSIGNED_CHAR_TYPE = 3;
    public final static int VARIANT_INT_TYPE = 4;
    public final static int VARIANT_LONG_TYPE = 5;
    public final static int VARIANT_FLOAT_TYPE = 6;
    public final static int VARIANT_DOUBLE_TYPE = 7;
    public final static int VARIANT_STRING_TYPE = 8;
    public final static int VARIANT_BOOL_VECTOR_TYPE = 9;
    public final static int VARIANT_CHAR_VECTOR_TYPE = 10;
    public final static int VARIANT_UNSIGNED_CHAR_VECTOR_TYPE = 11;
    public final static int VARIANT_INT_VECTOR_TYPE = 12;
    public final static int VARIANT_LONG_VECTOR_TYPE = 13;
    public final static int VARIANT_FLOAT_VECTOR_TYPE = 14;
    public final static int VARIANT_DOUBLE_VECTOR_TYPE = 15;
    public final static int VARIANT_STRING_VECTOR_TYPE = 16;

    public Variant()
    {
        super();
        dataType = VARIANT_EMPTY_TYPE;
        dataValue = null;
    }

    public Variant(Variant obj)
    {
        super();

        if(obj.dataType == VARIANT_EMPTY_TYPE)
        {
            obj.dataType = VARIANT_EMPTY_TYPE;
            dataValue = null;
        }
        else if(obj.dataType == VARIANT_BOOL_TYPE)
            SetValue(obj.AsBool());
        else if(obj.dataType == VARIANT_CHAR_TYPE)
            SetValue(obj.AsByte());
        else if(obj.dataType == VARIANT_UNSIGNED_CHAR_TYPE)
            SetValue(obj.AsByte());
        else if(obj.dataType == VARIANT_INT_TYPE)
            SetValue(obj.AsInt());
        else if(obj.dataType == VARIANT_LONG_TYPE)
            SetValue(obj.AsLong());
        else if(obj.dataType == VARIANT_FLOAT_TYPE)
            SetValue(obj.AsFloat());
        else if(obj.dataType == VARIANT_DOUBLE_TYPE)
            SetValue(obj.AsDouble());
        else if(obj.dataType == VARIANT_STRING_TYPE)
            SetValue(obj.AsString());
        else
            SetValue(obj.AsVector(), obj.dataType);
    }

    public int Type()
    {
        return dataType;
    }

    public void SetValue(boolean val)
    {
        dataType = VARIANT_BOOL_TYPE;
        dataValue = new Boolean(val);
    }

    public void SetValue(byte val)
    {
        dataType = VARIANT_UNSIGNED_CHAR_TYPE;
        dataValue = new Byte(val);
    }

    public void SetValue(int val)
    {
        dataType = VARIANT_INT_TYPE;
        dataValue = new Integer(val);
    }

    public void SetValue(long val)
    {
        dataType = VARIANT_LONG_TYPE;
        dataValue = new Long(val);
    }

    public void SetValue(float val)
    {
        dataType = VARIANT_FLOAT_TYPE;
        dataValue = new Float(val);
    }

    public void SetValue(double val)
    {
        dataType = VARIANT_DOUBLE_TYPE;
        dataValue = new Double(val);
    }
    public void SetValue(String val)
    {
        dataType = VARIANT_STRING_TYPE;
        dataValue = new String(val);
    }

    public void SetValue(Vector val, int t)
    {
        dataType = t;
        dataValue = new Vector(val);
    }

    public boolean AsBool()
    {
        return ((java.lang.Boolean)dataValue).booleanValue();
    }

    public byte AsByte()
    {
        return ((java.lang.Byte)dataValue).byteValue();
    }

    public int AsInt()
    {
        return ((java.lang.Integer)dataValue).intValue();
    }

    public long AsLong()
    {
        return ((java.lang.Long)dataValue).longValue();
    }

    public float AsFloat()
    {
        return ((java.lang.Float)dataValue).floatValue();
    }

    public double AsDouble()
    {
        return ((java.lang.Double)dataValue).doubleValue();
    }

    public String AsString()
    {
        return dataValue.toString();
    }

    public Vector AsVector()
    {
        return (Vector)dataValue;
    }

    public java.lang.Object DataValue()
    {
        return dataValue;
    }

    // Write and read methods.
    public void Write(CommunicationBuffer buf)
    {
        if(dataType == VARIANT_BOOL_TYPE)
            buf.WriteBool(AsBool());
        else if(dataType == VARIANT_CHAR_TYPE)
            buf.WriteByte(AsByte());
        else if(dataType == VARIANT_UNSIGNED_CHAR_TYPE)
            buf.WriteByte(AsByte());
        else if(dataType == VARIANT_INT_TYPE)
            buf.WriteInt(AsInt());
        else if(dataType == VARIANT_LONG_TYPE)
            buf.WriteLong(AsLong());
        else if(dataType == VARIANT_FLOAT_TYPE)
            buf.WriteFloat(AsFloat());
        else if(dataType == VARIANT_DOUBLE_TYPE)
            buf.WriteDouble(AsDouble());
        else if(dataType == VARIANT_STRING_TYPE)
            buf.WriteString(AsString());

        else if(dataType == VARIANT_BOOL_VECTOR_TYPE)
            buf.WriteByteVector(AsVector());
        else if(dataType == VARIANT_CHAR_VECTOR_TYPE)
            buf.WriteByteVector(AsVector());
        else if(dataType == VARIANT_UNSIGNED_CHAR_VECTOR_TYPE)
            buf.WriteByteVector(AsVector());
        else if(dataType == VARIANT_INT_VECTOR_TYPE)
            buf.WriteIntVector(AsVector());
        else if(dataType == VARIANT_LONG_VECTOR_TYPE)
            buf.WriteLongVector(AsVector());
        else if(dataType == VARIANT_FLOAT_VECTOR_TYPE)
            buf.WriteFloatVector(AsVector());
        else if(dataType == VARIANT_DOUBLE_VECTOR_TYPE)
            buf.WriteDoubleVector(AsVector());
        else if(dataType == VARIANT_STRING_VECTOR_TYPE)
            buf.WriteStringVector(AsVector());
    }

    public void Read(CommunicationBuffer buf)
    {
        if(dataType == VARIANT_BOOL_TYPE)
            SetValue(buf.ReadBool());
        else if(dataType == VARIANT_CHAR_TYPE)
            SetValue(buf.ReadByte());
        else if(dataType == VARIANT_UNSIGNED_CHAR_TYPE)
            SetValue(buf.ReadByte());
        else if(dataType == VARIANT_INT_TYPE)
            SetValue(buf.ReadInt());
        else if(dataType == VARIANT_LONG_TYPE)
            SetValue(buf.ReadLong());
        else if(dataType == VARIANT_FLOAT_TYPE)
            SetValue(buf.ReadFloat());
        else if(dataType == VARIANT_DOUBLE_TYPE)
            SetValue(buf.ReadDouble());
        else if(dataType == VARIANT_STRING_TYPE)
            SetValue(buf.ReadString());

        else if(dataType == VARIANT_BOOL_VECTOR_TYPE)
            SetValue(buf.ReadByteVector(), VARIANT_BOOL_VECTOR_TYPE);
        else if(dataType == VARIANT_CHAR_VECTOR_TYPE)
            SetValue(buf.ReadByteVector(), VARIANT_CHAR_VECTOR_TYPE);
        else if(dataType == VARIANT_UNSIGNED_CHAR_VECTOR_TYPE)
            SetValue(buf.ReadByteVector(), VARIANT_UNSIGNED_CHAR_VECTOR_TYPE);
        else if(dataType == VARIANT_INT_VECTOR_TYPE)
            SetValue(buf.ReadIntVector(), VARIANT_INT_VECTOR_TYPE);
        else if(dataType == VARIANT_LONG_VECTOR_TYPE)
            SetValue(buf.ReadLongVector(), VARIANT_LONG_VECTOR_TYPE);
        else if(dataType == VARIANT_FLOAT_VECTOR_TYPE)
            SetValue(buf.ReadFloatVector(), VARIANT_FLOAT_VECTOR_TYPE);
        else if(dataType == VARIANT_DOUBLE_VECTOR_TYPE)
            SetValue(buf.ReadDoubleVector(), VARIANT_DOUBLE_VECTOR_TYPE);
        else if(dataType == VARIANT_STRING_VECTOR_TYPE)
            SetValue(buf.ReadStringVector(), VARIANT_STRING_VECTOR_TYPE);
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + indent;
        if(dataValue != null)
            str = str + dataValue.toString();
        return str;
    }

    protected int              dataType;
    protected java.lang.Object dataValue;
}
