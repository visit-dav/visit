#include <BufferConnection.h>

BufferConnection::BufferConnection() : buffer()
{
}

BufferConnection::~BufferConnection()
{
}

int
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

long
BufferConnection::Size()
{
    return (long)(buffer.empty() ? 0 : buffer.size());
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
BufferConnection::Append(const unsigned char *buf, int count)
{
    const unsigned char *temp = buf;
    for(int i = 0; i < count; ++i)
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
// ****************************************************************************

long
BufferConnection::DirectRead(unsigned char *buf, long len)
{
    long retval = 0;

    if(buffer.size() > 0 && buf != 0)
    {
        for(int i = 0; i < retval && i < len; ++i)
        {
            buf[i] = buffer.front();
            buffer.pop_front();
            ++retval;
        }
    }

    return retval;
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
//   
// ****************************************************************************

long
BufferConnection::DirectWrite(const unsigned char *buf, long len)
{
    Append(buf, len);
    return len;
}
