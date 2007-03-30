#include <visit-config.h>
#include <convert.h>
#include <Connection.h>

const bool Connection::SRC = false;
const bool Connection::DEST = true;

// *******************************************************************
// Method: Connection::Connection
//
// Purpose: 
//   Constructor for the Connection class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 13:14:17 PST 2000
//
// Modifications:
//   
// *******************************************************************

Connection::Connection() : srcFormat(), destFormat()
{
    doConversion = true;
}

// *******************************************************************
// Method: Connection::~Connection
//
// Purpose: 
//   Destructor for the Connection class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 13:14:36 PST 2000
//
// Modifications:
//   
// *******************************************************************

Connection::~Connection()
{
    // nothing.
}

// *******************************************************************
// Method: Connection::WriteInt
//
// Purpose: 
//   Writes an int to the connection.
//
// Arguments:
//   val : The value to write to the connection.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:05:11 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::WriteInt(int val)
{
    if(doConversion)
    {
        unsigned char buffer[8];
        int nbytes = IntConvert(val, buffer, IntFormat());
        Append(buffer, nbytes);
    }
    else
        Append((unsigned char *)&val, SIZEOF_INT);
}

// *******************************************************************
// Method: Connection::WriteLong
//
// Purpose: 
//   Writes a long to the connection.
//
// Arguments:
//   val : The value to write to the connection.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:05:11 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::WriteLong(long val)
{
    if(doConversion)
    {
        unsigned char buffer[8];
        int nbytes = LongConvert(val, buffer, LongFormat());
        Append(buffer, nbytes);
    }
    else
        Append((unsigned char *)&val, SIZEOF_LONG);
}

// *******************************************************************
// Method: Connection::WriteFloat
//
// Purpose: 
//   Writes a float to the connection.
//
// Arguments:
//   val : The value to write to the connection.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:05:11 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::WriteFloat(float val)
{
    if(doConversion)
    {
        unsigned char buffer[8];
        int nbytes = FloatConvert(val, buffer, FloatFormat());
        Append(buffer, nbytes);
    }
    else
        Append((unsigned char *)&val, SIZEOF_FLOAT);
}

// *******************************************************************
// Method: Connection::WriteDouble
//
// Purpose: 
//   Writes a double to the connection.
//
// Arguments:
//   val : The value to write to the connection.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 14:05:11 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::WriteDouble(double val)
{
    if(doConversion)
    {
        unsigned char buffer[8];
        int nbytes = DoubleConvert(val, buffer, DoubleFormat());
        Append(buffer, nbytes);
    }
    else
        Append((unsigned char *)&val, SIZEOF_DOUBLE);
}

// *******************************************************************
// Method: Connection::ReadInt
//
// Purpose: 
//   Reads an int from the connection.
//
// Arguments:
//   i : The memory location of the int.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 13:20:38 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::ReadInt(int *i)
{
    unsigned char *temp = (unsigned char *)i;
#if(SIZEOF_INT == 4)
    Read(temp);
    Read(temp + 1);
    Read(temp + 2);
    Read(temp + 3); 
#elif(SIZEOF_INT == 8)
    Read(temp);
    Read(temp + 1);
    Read(temp + 2);
    Read(temp + 3);
    Read(temp + 4);
    Read(temp + 5);
    Read(temp + 6);
    Read(temp + 7);
#else
#pragma error "Unsupported int size"
#endif
}

// *******************************************************************
// Method: Connection::ReadLong
//
// Purpose: 
//   Reads a long from the connection.
//
// Arguments:
//   l : The memory location of the long.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 13:20:38 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::ReadLong(long *l)
{
    unsigned char *temp = (unsigned char *)l;

#if(SIZEOF_LONG == 4)
    Read(temp);
    Read(temp + 1);
    Read(temp + 2);
    Read(temp + 3); 
#elif(SIZEOF_LONG == 8)
    Read(temp);
    Read(temp + 1);
    Read(temp + 2);
    Read(temp + 3);
    Read(temp + 4);
    Read(temp + 5);
    Read(temp + 6);
    Read(temp + 7);
#else
#pragma error "Unsupported long size"
#endif
}

// *******************************************************************
// Method: Connection::ReadFloat
//
// Purpose: 
//   Reads a float from the connection.
//
// Arguments:
//   f : The memory location of the float.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 13:20:38 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::ReadFloat(float *f)
{
    unsigned char *temp = (unsigned char *)f;

#if(SIZEOF_FLOAT == 4)
    Read(temp);
    Read(temp + 1);
    Read(temp + 2);
    Read(temp + 3); 
#else
#pragma error "Unsupported long size"
#endif
}

// *******************************************************************
// Method: Connection::ReadDouble
//
// Purpose: 
//   Reads a double from the connection.
//
// Arguments:
//   d : The memory location of the double.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 13:20:38 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::ReadDouble(double *d)
{
    unsigned char *temp = (unsigned char *)d;

#if(SIZEOF_DOUBLE == 8)
    Read(temp);
    Read(temp + 1);
    Read(temp + 2);
    Read(temp + 3);
    Read(temp + 4);
    Read(temp + 5);
    Read(temp + 6);
    Read(temp + 7);
#else
#pragma error "Unsupported double size"
#endif
}

// *******************************************************************
// Method: Connection::SetDestinationFormat
//
// Purpose: 
//   Sets the type representation for the destination machine. This
//   information is used to convert data to the destination machine's
//   data formats before sending it.
//
// Arguments:
//   t : The type representation to use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 13:17:56 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::SetDestinationFormat(const TypeRepresentation &t)
{
    destFormat = t;
}

// *******************************************************************
// Method: Connection::GetDestinationFormat
//
// Purpose: 
//   Returns a reference to the destination machine's type
//   representation.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 13:17:56 PST 2000
//
// Modifications:
//   
// *******************************************************************

const TypeRepresentation &
Connection::GetDestinationFormat() const
{
    return destFormat;
}

// *******************************************************************
// Method: Connection::EnableConversion
//
// Purpose: 
//   Indicates whether or not the connection should convert data to
//   the format of the destination machine before writing it.
//
// Arguments:
//   val : Whether or not to do conversion.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 13:16:48 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
Connection::EnableConversion(bool val)
{
    doConversion = val;
}

