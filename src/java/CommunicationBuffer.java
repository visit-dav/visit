package llnl.visit;

import java.lang.Float;
import java.lang.Double;
import java.util.Vector;
import java.nio.ByteOrder;
import java.nio.ByteBuffer;

// ****************************************************************************
// Class: CommunicationBuffer
//
// Purpose:
//   This class implements a buffer into which we write variables. It also
//   makes sure that the variables are translated into the format of the 
//   destination platform. We can also read variables out of the buffer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:56:51 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Dec 10 17:02:12 PST 2002
//   I added methods to read/write byte vectors.
//
// ****************************************************************************

public class CommunicationBuffer
{
    public CommunicationBuffer()
    {
        nAlloc = 1000;
        length = 0;
        buf = new byte[nAlloc];
        remoteInfo = new CommunicationHeader();
    }

    public byte[] GetBytes()
    {
        byte[] ret = new byte[length];
        for(int i = 0; i < length; ++i)
            ret[i] = buf[i];

        return ret;
    }

    public void SetRemoteInfo(CommunicationHeader info)
    {
        remoteInfo = info;
    }

    public void Flush()
    {
        length = 0;
    }

    public int Size()
    {
        return length;
    }

    public void WriteByte(byte b)
    {
        AddByte(b);
    }

    public void WriteBool(boolean val)
    {
        AddByte((byte)(val ? 1 : 0));
    }

    public void WriteInt(int i)
    {
        if(remoteInfo.IntFormat == CommunicationHeader.B32)
        {
            // Big endian 32 bit
            AddByte((byte)((i & 0xff000000) >> 24));
            AddByte((byte)((i & 0x00ff0000) >> 16));
            AddByte((byte)((i & 0x0000ff00) >> 8));
            AddByte((byte)(i & 0xff));
        }
        else if(remoteInfo.IntFormat == CommunicationHeader.L32)
        {
            // Little endian 32 bit
            AddByte((byte)(i & 0xff));
            AddByte((byte)((i & 0x0000ff00) >> 8));
            AddByte((byte)((i & 0x00ff0000) >> 16));
            AddByte((byte)((i & 0xff000000) >> 24));
        }
    }

    public void WriteFloat(float f)
    {
        byte[] b = new byte[4];
        if(remoteInfo.IntFormat == CommunicationHeader.B32)
        {
            ByteBuffer.wrap(b).order(ByteOrder.BIG_ENDIAN).putFloat(f);
        }
        else if(remoteInfo.IntFormat == CommunicationHeader.L32)
        {
            ByteBuffer.wrap(b).order(ByteOrder.LITTLE_ENDIAN).putFloat(f);
        }
        for(int i = 0; i < 4; ++i)
            AddByte(b[i]);
    }

    public void WriteLong(long l)
    {
        AddByte((byte)((l & 0xff000000) >> 24));
        AddByte((byte)((l & 0x00ff0000) >> 16));
        AddByte((byte)((l & 0x0000ff00) >> 8));
        AddByte((byte)(l & 0xff));
    }

    public void WriteDouble(double d)
    {
        byte[] b = new byte[8];
        if(remoteInfo.IntFormat == CommunicationHeader.B32)
        {
            ByteBuffer.wrap(b).order(ByteOrder.BIG_ENDIAN).putDouble(d);
        }
        else if(remoteInfo.IntFormat == CommunicationHeader.L32)
        {
            ByteBuffer.wrap(b).order(ByteOrder.LITTLE_ENDIAN).putDouble(d);
        }
        for(int i = 0; i < 8; ++i)
            AddByte(b[i]);
    }

    public void WriteByteArray(byte[] b, boolean writeLength)
    {
        int newSize = length + b.length + 4;

        // See if the byte array needs to be resized.
        if(newSize >= nAlloc)
        {
            int newAlloc = newSize * 3 / 2;
            byte[] newBuf = new byte[newAlloc];
            for(int i = 0; i < length; ++i)
                newBuf[i] = buf[i];
            nAlloc = newAlloc;
            buf = newBuf;
        }
        
        // Add the length if we are writing it.
        if(writeLength)
            WriteInt(b.length);

        // Add the bytes
        for(int i = 0; i < b.length; ++i)
        {
            buf[length] = b[i];
            ++length;
        }
    }

    public void WriteByteVector(Vector vec)
    {
        WriteInt(vec.size());
        for(int i = 0; i < vec.size(); ++i)
        {
            Byte bv = (Byte)vec.elementAt(i);
            byte b = bv.byteValue();
            WriteByte(b);
        }
    }

    public void WriteBoolArray(boolean[] array)
    {
        WriteInt(array.length);
        for(int i = 0; i < array.length; ++i)
            WriteBool(array[i]);
    }

    public void WriteIntArray(int[] array)
    {
        WriteInt(array.length);
        for(int i = 0; i < array.length; ++i)
            WriteInt(array[i]);
    }

    public void WriteIntVector(Vector vec)
    {
        WriteInt(vec.size());
        for(int i = 0; i < vec.size(); ++i)
        {
            Integer iv = (Integer)vec.elementAt(i);
            WriteInt(iv.intValue());
        }
    }

    public void WriteLongArray(long[] array)
    {
        WriteInt(array.length);
        for(int i = 0; i < array.length; ++i)
            WriteLong(array[i]);
    }

    public void WriteFloatArray(float[] array)
    {
        WriteInt(array.length);
        for(int i = 0; i < array.length; ++i)
            WriteFloat(array[i]);
    }

    public void WriteFloatVector(Vector vec)
    {
        WriteInt(vec.size());
        for(int i = 0; i < vec.size(); ++i)
        {
            Float fv = (Float)vec.elementAt(i);
            WriteFloat(fv.floatValue());
        }
    }

    public void WriteDoubleArray(double[] array)
    {
        WriteInt(array.length);
        for(int i = 0; i < array.length; ++i)
            WriteDouble(array[i]);
    }

    public void WriteDoubleVector(Vector vec)
    {
        WriteInt(vec.size());
        for(int i = 0; i < vec.size(); ++i)
        {
            Double dv = (Double)vec.elementAt(i);
            WriteDouble(dv.doubleValue());
        }
    }

    public void WriteString(String s)
    {
        byte[] str = s.getBytes();
        for(int i = 0; i < str.length; ++i)
            AddByte(str[i]);
        AddByte((byte)0);
    }

    public void WriteStringVector(Vector vec)
    {
        WriteInt(vec.size());
        for(int i = 0; i < vec.size(); ++i)
        {
            String s = (String)vec.elementAt(i);
            WriteString(s);
        }
    }

//
// Read methods
//
    public byte ReadByte()
    {
        byte retval = buf[0];
        Shift(1);
        return retval;
    }

    public byte[] ReadByteArray()
    {
        int len = ReadInt();
        byte[] retval = new byte[len];

        for(int i = 0; i < len; ++i)
            retval[i] = buf[i];
        Shift(len);

        return retval;
    }

    public Vector ReadByteVector()
    {
        int len = ReadInt();
        Vector retval = new Vector();
        for(int i = 0; i < len; ++i)
        {
            retval.addElement(new Byte(ReadByte()));
        }
        return retval;
    }

    public boolean ReadBool()
    {
        boolean retval = (buf[0] == 1);
        Shift(1);
        return retval;
    }

    public int ReadInt()
    {
        int a = 0,b = 0,c = 0,d = 0, retval = 0;

        if(remoteInfo.IntFormat == CommunicationHeader.B32)
        {
            a = (int)buf[0]; b = (int)buf[1]; c = (int)buf[2]; d = (int)buf[3];
        }
        else if(remoteInfo.IntFormat == CommunicationHeader.L32)
        {
            a = (int)buf[3]; b = (int)buf[2]; c = (int)buf[1]; d = (int)buf[0];
        }

        // It's using signed ints when converting from byte to int.
        if(a < 0) a += 256;
        if(b < 0) b += 256;
        if(c < 0) c += 256;
        if(d < 0) d += 256;

        retval = (a << 24) | (b << 16) | (c << 8) | d;

        Shift(4);
        return retval;
    }

    public int[] ReadIntArray()
    {
        int len = ReadInt();
        int[] retval = new int[len];
        for(int i = 0; i < len; ++i)
            retval[i] = ReadInt();
        return retval;
    }

    public Vector ReadIntVector()
    {
        int len = ReadInt();
        Vector retval = new Vector();
        for(int i = 0; i < len; ++i)
            retval.addElement(new Integer(ReadInt()));
        return retval;
    }

    public float ReadFloat()
    {
        float f = 0.f;
        byte[] b = new byte[4];
        b[0] = buf[0];
        b[1] = buf[1];
        b[2] = buf[2];
        b[3] = buf[3];
        if(remoteInfo.IntFormat == CommunicationHeader.B32)
        {
            f = ByteBuffer.wrap(b).order(ByteOrder.nativeOrder()).getFloat();
        }
        else if(remoteInfo.IntFormat == CommunicationHeader.L32)
        {
            f = ByteBuffer.wrap(b).order(ByteOrder.nativeOrder()).getFloat();
        }

        Shift(4);
        return f;
    }

    public float[] ReadFloatArray()
    {
        int len = ReadInt();
        float[] retval = new float[len];
        for(int i = 0; i < len; ++i)
            retval[i] = ReadFloat();
        return retval;
    }

    public Vector ReadFloatVector()
    {
        int len = ReadInt();
        Vector retval = new Vector();
        for(int i = 0; i < len; ++i)
            retval.addElement(new Float(ReadFloat()));
        return retval;
    }

    public double ReadDouble()
    {
        double d = 0.;
        byte[] b = new byte[8];
        for(int i = 0; i < 8; ++i)
            b[i] = buf[i];
        if(remoteInfo.IntFormat == CommunicationHeader.B32)
        {
            d = ByteBuffer.wrap(b).order(ByteOrder.nativeOrder()).getFloat();
        }
        else if(remoteInfo.IntFormat == CommunicationHeader.L32)
        {
            d = ByteBuffer.wrap(b).order(ByteOrder.nativeOrder()).getFloat();
        }

        Shift(8);
        return d;
    }

    public double[] ReadDoubleArray()
    {
        int len = ReadInt();
        double[] retval = new double[len];
        for(int i = 0; i < len; ++i)
            retval[i] = ReadDouble();
        return retval;
    }

    public Vector ReadDoubleVector()
    {
        int len = ReadInt();
        Vector retval = new Vector();
        for(int i = 0; i < len; ++i)
            retval.addElement(new Double(ReadDouble()));
        return retval;
    }

    public String ReadString()
    {
        String retval = new String("");
        char[] c = new char[1];
        int  index = 0;
        do
        {
           c[0] = (char)buf[index++];
           if(c[0] != '\0')
               retval = retval + String.copyValueOf(c);
        } while(c[0] != '\0');

        Shift(index);

        return retval;
    }

    public Vector ReadStringVector()
    {
        int len = ReadInt();
        Vector retval = new Vector();
        for(int i = 0; i < len; ++i)
            retval.addElement(ReadString());
        return retval;
    }

    public void Shift(int shift)
    {
        if(shift > 0)
        {
            int i, index = 0;
            for(i = shift; i < length; ++i, ++index)
                buf[index] = buf[i];
            length -= shift;
            if(length < 0)
                length = 0;
        }
    }

    private void AddByte(byte b)
    {
        // See if the byte array needs to be resized.
        if(length >= nAlloc - 1)
        {
            int newAlloc = nAlloc*2;
            byte[] newBuf = new byte[newAlloc];
            for(int i = 0; i < nAlloc; ++i)
                newBuf[i] = buf[i];
            nAlloc = newAlloc;
            buf = newBuf;
        }
        
        // Add the byte
        buf[length] = b;
        ++length;
    }

    int                 length;
    int                 nAlloc;
    byte[]              buf;
    CommunicationHeader remoteInfo;
}
