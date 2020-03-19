// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

// ****************************************************************************
// Class: CommunicationHeader
//
// Purpose:
//   This class contains the initial message that VisIt clients communicate
//   to VisIt. The platform type representations and the version are exchanged.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:55:24 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Aug 19 13:52:09 PST 2002
//   I changed the code that initializes the version so it uses the version
//   from the automatically generated Version class.
//
//   Brad Whitlock, Thu Jan 2 14:12:31 PST 2003
//   I added security keys.
//
//   Brad Whitlock, Fri Jan 28 11:25:36 PST 2011
//   I loosened the test for version equality to support patch releases.
//
//   Brad Whitlock, Wed Nov 28 16:22:51 PST 2012
//   I changed the communication header to match changes in the C++ version.
//
// ****************************************************************************

class CommunicationHeader
{
    public CommunicationHeader()
    {
        // Determine if the machine is big endian and set the conversion
        // formats accordingly. The next 2 lines require Java2 v1.4.0
        // because of the java.nio package.
        bigEndian = (java.nio.ByteOrder.nativeOrder() ==
                     java.nio.ByteOrder.BIG_ENDIAN);

        Format = BINARYFORMAT;

        if(bigEndian)
        {
            IntFormat = B32;
            LongFormat = B32;
            FloatFormat = B32;
            DoubleFormat = B64;
        }
        else
        {
            IntFormat = L32;
            LongFormat = L32;
            FloatFormat = L32;
            DoubleFormat = L64;
        }

        // Set the version number. We copy the version number from the
        // automatically generated Version class.
        version = new byte[10];
        for(int i = 0; i < 10; ++i)
            version[i] = Version.version[i];

        failCode = 0;
        securityKey = new String("");
        socketKey = new String("");
    }

    public void SetSecurityKey(String key)
    {
        securityKey = new String(key);
    }

    public void SetSocketKey(String key)
    {
        socketKey = new String(key);
    }

    public byte[] packHeader()
    {
        int i;
        byte[] buf = new byte[100];

        // Zero out the buffer that we will be sending.
        for(i = 0; i < 100; ++i)
            buf[i] = 0;

        // The first 5 bytes of the header are for the type representation.
        buf[0] = BINARYFORMAT;
        buf[1] = IntFormat;
        buf[2] = LongFormat;
        buf[3] = FloatFormat;
        buf[4] = DoubleFormat;

        // The failure code
        buf[5] = 0;

        // Set the version string into the next 10 bytes.
        for(i = 0; i < 10; ++i)
            buf[i+6] = version[i];

        // Set the security key into the next 21 bytes.
        byte[] securityKeyBytes = securityKey.getBytes();
        for(i = 0; i <= 20; ++i)
        {
            if(i < 20)
                buf[i+6+10] = securityKeyBytes[i];
            else
                buf[i+6+10] = 0; // NULL terminator
        }

        // Set the socket key into the next 21 bytes.
        byte[] socketKeyBytes = socketKey.getBytes();
        for(i = 0; i <= 20; ++i)
        {
            if(i < 20)
                buf[i+6+10+21] = socketKeyBytes[i];
            else
                buf[i+6+10+21] = 0; // NULL terminator
        }

        // The rest of the bytes are reserved for future use.

        return buf;
    }

    public void unpackHeader(byte[] buf)
    {
        // Read the representation from the buffer.
        Format = buf[0];
        IntFormat = buf[1];
        LongFormat = buf[2];
        FloatFormat = buf[3];
        DoubleFormat = buf[4];

        // Do the fail code.
        failCode = (int)buf[5];

        // Do the version number.
        int i;
        for(i = 0; i < 10; ++i)
            version[i] = buf[i+6];

        // Do the security key.
        byte[] securityKeyBytes = new byte[20];
        for(i = 0; i < 20; ++i)
            securityKeyBytes[i] = buf[i+6+10];
        securityKey = new String(securityKeyBytes);

        // Do the socket key.
        byte[] socketKeyBytes = new byte[20];
        for(i = 0; i < 20; ++i)
            socketKeyBytes[i] = buf[i+6+10+21];
        socketKey = new String(socketKeyBytes);
    }

    public boolean isCompatible(CommunicationHeader b) throws CouldNotConnectException,
        IncompatibleVersionException, IncompatibleSecurityTokenException
    {
        //System.out.println("IntFormat="+IntFormat+" remote.IntFormat="+b.IntFormat);
        //System.out.println("LongFormat="+LongFormat+" remote.LongFormat="+b.LongFormat);
        //System.out.println("FloatFormat="+FloatFormat+" remote.FloatFormat="+b.FloatFormat);
        //System.out.println("DoubleFormat="+DoubleFormat+" remote.DoubleFormat="+b.DoubleFormat);

        // Check the version number.
        boolean same = true;
        int dotCount = 0;
        for(int i = 0; i < 10; ++i)
        {
            boolean sameChar = (version[i] == b.version[i]);
            if(sameChar && version[i] == '.')
                dotCount++;
            same &= sameChar;
            // Stop checking versions after the second dot. Our version numbers
            // are now A.B.C and patch releases are supposed to be compatible. So,
            // if the versions have been the same up to the second dot then
            // don't check the patch version. Just assume it is compatible.
            if(same && dotCount >= 2)
                break;
            //System.out.println("ver="+version[i]+" remote.ver="+b.version[i]);
        }
        if(!same)
            throw new IncompatibleVersionException();

        // Check the fail code.
        if(b.failCode == 1)
            throw new IncompatibleVersionException();
        else if(b.failCode == 2)
            throw new IncompatibleSecurityTokenException();
        else if(b.failCode == 3)
            throw new CouldNotConnectException();

        // Check the keys.
        if(!securityKey.equals(b.securityKey) ||
           !socketKey.equals(b.socketKey))
        {
            throw new IncompatibleSecurityTokenException();
        }

        return same;
    }

    public String CreateRandomKey()
    {
        final int keyLen = 20;
        String key = new String("");
        java.util.Random r = new java.util.Random();

        for(int i = 0; i < keyLen; ++i)
        {
            java.math.BigInteger n = new java.math.BigInteger(4, r);
            key = key + n.toString(16);
        }

        return key;
    }

    public static final byte B32 = 0;
    public static final byte B64 = 1;
    public static final byte L32 = 2;
    public static final byte L64 = 3;

    public static final byte ASCIIFORMAT = 0;
    public static final byte BINARYFORMAT = 1;

    public byte Format;
    public byte IntFormat;
    public byte LongFormat;
    public byte FloatFormat;
    public byte DoubleFormat;
    public byte[] version;
    public int    failCode;
    public String securityKey;
    public String socketKey;

    public static boolean bigEndian = true;
}
