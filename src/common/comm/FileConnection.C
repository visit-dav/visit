/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <FileConnection.h>

// *******************************************************************
// Method: FileConnection::FileConnection
//
// Purpose: 
//   Constructor for the FileConnection class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:15:46 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 20 17:35:48 PST 2002
//   Moved the contents of the old Open method here.
//
// *******************************************************************

FileConnection::FileConnection(const char *filename, bool writeMode_) :
    Connection()
{
    fileStream = 0;
    fileSize = 0;
    writeMode = writeMode;

    if(writeMode)
    {
        fileStream = fopen(filename, "wb");

        // Write out the type information for this end
        Write(srcFormat.IntFormat);
        Write(srcFormat.LongFormat);
        Write(srcFormat.FloatFormat);
        Write(srcFormat.DoubleFormat);

        fileSize = 0;
    }
    else
    {
        fileStream = fopen(filename, "rb");

        // Read the type information for the other end
        Read(&destFormat.IntFormat);
        Read(&destFormat.LongFormat);
        Read(&destFormat.FloatFormat);
        Read(&destFormat.DoubleFormat);

        fileSize = 100; // Get the size of the input file.
    }
}

// *******************************************************************
// Method: FileConnection::~FileConnection
//
// Purpose: 
//   Destructor for the FileConnection class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:16:22 PST 2000
//
// Modifications:
//   
// *******************************************************************

FileConnection::~FileConnection()
{
    if(fileStream != 0)
    {
        fclose(fileStream);
        fileStream = 0;
    }
}

// ****************************************************************************
// Method: FileConnection::Fill
//
// Purpose: 
//   Tries to fill the connection with data.
//
// Returns:    The number of bytes that were filled.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 20 17:37:11 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
FileConnection::Fill()
{
    return 0;
}

// *******************************************************************
// Method: FileConnection::Flush
//
// Purpose: 
//   Flushes the connection. In this case, it writes remaining
//   characters to the file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:20:07 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
FileConnection::Flush()
{
    if(fileStream == 0)
        return;

    fflush(fileStream);
}

// *******************************************************************
// Method: FileConnection::Write
//
// Purpose: 
//   Writes a character to the file.
//
// Arguments:
//   value : The character to write.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:20:54 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
FileConnection::Write(unsigned char value)
{
    fputc(value, fileStream);
    ++fileSize;
}

// *******************************************************************
// Method: FileConnection::Read
//
// Purpose: 
//   Reads a character from the file.
//
// Arguments:
//   address : The address of the variable into which we'll put the char.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:21:27 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
FileConnection::Read(unsigned char *address)
{
    *address = (unsigned char)fgetc(fileStream);
    --fileSize;
}

// *******************************************************************
// Method: FileConnection::Size
//
// Purpose: 
//   Returns the number of characters that have not been read.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:22:18 PST 2000
//
// Modifications:
//   
// *******************************************************************

long
FileConnection::Size()
{
    return fileSize;
}

// *******************************************************************
// Method: FileConnection::Append
//
// Purpose: 
//   Writes a block of characters to the file.
//
// Arguments:
//   address : The array of characters to write to the file.
//   nChars  : The length of the character array.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:23:09 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
FileConnection::Append(unsigned char *address, int nChars)
{
    if(writeMode)
    {
        fwrite((void *)address, 1, nChars, fileStream);
        fileSize += nChars;
    }
}

// ****************************************************************************
// Method: FileConnection::DirectRead
//
// Purpose: 
//   Reads the specified number of bytes from the connection.
//
// Arguments:
//   buf    : The destination buffer.
//   ntotal : The number of bytes to read.
//
// Returns:    The number of bytes read.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 14:28:39 PST 2002
//
// Modifications:
//   
// ****************************************************************************

long
FileConnection::DirectRead(unsigned char *buf, long len)
{
    long retval = 0;

    if(!writeMode)
    {
         retval = (long)fread((void *)buf, len, 1, fileStream);
    }

    return retval;
}

// ****************************************************************************
// Method: FileConnection::DirectWrite
//
// Purpose: 
//   Writes a buffer to the connection without doing any buffering.
//
// Arguments:
//   buf    : The buffer that we want to write.
//   ntotal : The length of the buffer.
//
// Returns:    The number of bytes written.
//
// Note:       This method was adapted from Jeremy's write_n function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 14:33:40 PST 2002
//
// Modifications:
//   
// ****************************************************************************

long
FileConnection::DirectWrite(const unsigned char *buf, long len)
{
    long retval = 0;

    if(writeMode)
    {
         retval = (long)fwrite((const void *)buf, len, 1, fileStream);
    }

    return retval;
}
