// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <BufferConnection.h>

BufferConnection::BufferConnection() : buffer()
{
}

BufferConnection::~BufferConnection()
{
}

size_t
BufferConnection::Fill()
{
    return 0;
}

void
BufferConnection::Flush()
{
    // Clear the buffer
    buffer.clear();
}

size_t
BufferConnection::Size()
{
    return buffer.empty() ? 0 : buffer.size();
}

void
BufferConnection::Reset()
{
    buffer.clear();
}

void
BufferConnection::Write(unsigned char value)
{
    buffer.push_back(value);
} 

void
BufferConnection::Read(unsigned char *address)
{
   if(buffer.empty())
   {
       *address = 0;
   }
   else
   {
       *address = buffer.front();
       buffer.pop_front();
   }
}

void
BufferConnection::Append(const unsigned char *buf, size_t count)
{
    const unsigned char *temp = buf;
    for(size_t i = 0; i < count; ++i)
        buffer.push_back(*temp++);
}

// ****************************************************************************
// Method: BufferConnection::DirectRead
//
// Purpose: 
//   Reads the contents of the connection into the passed-in buffer.
//
// Arguments:
//    buf : The destination buffer.
//    len : The length of the destination buffer.
//
// Returns:    The number of bytes read.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 14:21:35 PST 2002
//
// Modifications:
//
//     Burlen Loring, Mon Aug  3 13:29:43 PDT 2015
//     Fix a bug where this method did nothing.
//
//     Cyrus Harrison, Tue Jan 28 09:55:51 PST 2025
//     Convert to size_t for buffer sizes
// ****************************************************************************

size_t
BufferConnection::DirectRead(unsigned char *buf, size_t len)
{
    if (!buf)
        return 0;

    size_t n = 0;
    while (buffer.size() && (n < len))
    {
        buf[n] = buffer.front();
        buffer.pop_front();
        ++n;
    }

    return n;
}

// ****************************************************************************
// Method: BufferConnection::DirectWrite
//
// Purpose: 
//   Writes the passed-in buffer into the internal buffer.
//
// Arguments:
//   buf : The buffer to copy.
//   len : The length of the buffer to copy.
//
// Returns:    The number of bytes written.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 14:20:11 PST 2002
//
// Modifications:
//    Cyrus Harrison, Tue Jan 28 09:55:51 PST 2025
//    Convert to size_t for buffer sizes
// ****************************************************************************

size_t
BufferConnection::DirectWrite(const unsigned char *buf, size_t len)
{
    Append(buf, len);
    return len;
}
