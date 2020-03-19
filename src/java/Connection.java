// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.ServerSocket;
import java.net.SocketException;
import java.net.UnknownHostException;

// ****************************************************************************
// Class: Connection
//
// Purpose:
//   This class sets up socket communication with VisIt's viewer and makes
//   sure that the viewer is compatible.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:53:55 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Jan 2 13:57:01 PST 2003
//   I added a security key.
//
//   Brad Whitlock, Thu Mar 20 10:46:56 PDT 2003
//   I made it use port 5600.
//
//   Kathleen Biagas, Thu Oct 15 18:00:21 PDT 2015
//   Added 'OrderConnections' to match Remote/Parent process.
//
// ****************************************************************************

class Connection
{
    public Connection()
    {
        port = 5600;
        localInfo = new CommunicationHeader();
        remoteInfo = new CommunicationHeader();
        securityKey = localInfo.CreateRandomKey();
    }

    public boolean StartConnection(int p)
    {
        boolean retval = true;
        try
        {
            port = p;
            ss = new ServerSocket(port);
            serverSocketCreated = true;
        }
        catch(Exception e)
        {
            retval = false;
        }

        return retval;
    }

    public boolean CreateSockets()
    {
        boolean success = true;

        try
        {
            // Create a read socket
            readConnection = ss.accept();
            // Create a write socket
            writeConnection = ss.accept();

            readConnection.setTcpNoDelay(true);
            writeConnection.setTcpNoDelay(true);
            socketsCreated = true;
            success = OrderConnections();
            success = ExchangeTypeRepresentations();
        }
        catch(SocketException s)
        {
            // Could not set TCP no delay
            success = false;
        }
        catch(IOException io)
        {
            // Could not create sockets.
            success = false;
        }

        return success;
    }

    public void Close()
    {
        try
        {
            if(serverSocketCreated)
                ss.close();

            if(socketsCreated)
            {
                readConnection.close();
                writeConnection.close();
            }
        }
        catch(IOException e)
        {
            // do nothing.
        }

        serverSocketCreated = false;
        socketsCreated = false;
    }

    private boolean OrderConnections() throws IOException
    {
        boolean retval = true;
        byte[] buf = new byte[4];
        DirectWriteHelper(readConnection.getOutputStream(), buf);
        buf[3] = 1;
        DirectWriteHelper(writeConnection.getOutputStream(), buf);
        return retval;
    }

    private boolean ExchangeTypeRepresentations() throws IOException
    {
        // Create a new key that will be sent over the socket.
        String socketKey = localInfo.CreateRandomKey();

        // Set the keys into the connection objects so that when the
        // type reps are exchanged, we can readily check the keys.
        localInfo.SetSecurityKey(securityKey);
        localInfo.SetSocketKey(socketKey);
        remoteInfo.SetSecurityKey(securityKey);
        remoteInfo.SetSocketKey(socketKey);

        // Send the header. Use the write connection.
        DirectWriteHelper(writeConnection.getOutputStream(),
                          localInfo.packHeader());

        // Get the header from the remote process. Use the read connection.
        boolean retval = true;
        try
        {
            byte[] buf = DirectReadHelper(readConnection.getInputStream(), 100);

            // The buffer we read represents the other process's communication
            // header. We need to check to see if it is compatible with the
            // local communication header.
            remoteInfo.unpackHeader(buf);
            retval = localInfo.isCompatible(remoteInfo);
        }
        catch(LostConnectionException e)
        {
            System.err.println("Lost the connection to the viewer.");
            retval = false;
        }
        catch(CouldNotConnectException e2)
        {
            System.err.println("Could not connect to the viewer.");
            retval = false;
        }
        catch(IncompatibleVersionException e3)
        {
            System.err.println("The viewer had an incompatible version. It can't be used.");
            retval = false;
        }
        catch(IncompatibleSecurityTokenException e4)
        {
            System.err.println("The viewer did not return the proper credentials. It can't be used.");
            retval = false;
        }

        return retval;
    }

    public void DirectWrite(byte[] buf) throws IOException
    {
        // Write to the process. From its point of view, we are writing to the
        // socket from which it reads so write to its read connection.
        DirectWriteHelper(readConnection.getOutputStream(), buf);
    }

    public byte[] DirectRead(int ntotal) throws LostConnectionException, IOException
    {
        // Read from the process. From its point of view, we are reading from
        // the socket to which it writes so read from its write connection.
        return DirectReadHelper(writeConnection.getInputStream(), ntotal);
    }

    public int CanRead()
    {
        int retval;

        try
        {
            retval = writeConnection.getInputStream().available();
        }
        catch(IOException e)
        {
            retval = 0;
            System.out.println("Connection::CanRead: Caught an IOException.");
        }

        return retval;
    }

    public int GetPort()
    {
        return port;
    }

    public String GetLocalHostName()
    {
        String retval;

        if(serverSocketCreated)
        {
            try
            {
                retval = new String(ss.getInetAddress().getLocalHost().getHostName());
            }
            catch(UnknownHostException e)
            {
                retval = new String("localhost");
            }
        }
        else
            retval = new String("localhost");

        return retval;
    }

    public CommunicationHeader getRemoteInfo()
    {
        return remoteInfo;
    }

    public String GetSecurityKey()
    {
        return new String(securityKey);
    }

    private void DirectWriteHelper(OutputStream os, byte[] buf)
    {
        if(buf.length > 0)
        {
            try
            {
                os.write(buf);
                os.flush();
            }
            catch(IOException e)
            {
                System.out.println("Connection::DirectWrite: Caught an IOException.");
            }
        }
    }

    private byte[] DirectReadHelper(InputStream s, int ntotal) throws LostConnectionException
    {
        byte[] b = new byte[ntotal];
        int nread = 0;
        int offset = 0;
        int zeroesRead = 0;

        while(nread < ntotal)
        {
            try
            {
                int n = s.read(b, offset, ntotal - nread);
                if(n == 0)
                {
                    ++zeroesRead;
                    if(zeroesRead > 100)
                        throw new LostConnectionException();
                }
                else if(n > 0)
                {
                    nread += n;
                    offset += n;
                }
            }
            catch(IOException e)
            {
                System.out.println("Connection::DirectReadHelper: Caught an IOException.");
                // Zero out the remaining bytes.
                for(int i = offset; i < ntotal; ++i)
                    b[i] = 0;
            }
        }

        return b;
    }

    private String              localHostName;
    private String              securityKey;
    private int                 port;
    private ServerSocket        ss;
    private Socket              readConnection;
    private Socket              writeConnection;
    private boolean             serverSocketCreated;
    private boolean             socketsCreated;
    private CommunicationHeader localInfo;
    private CommunicationHeader remoteInfo;
}
