#ifndef CONNECTION_H
#define CONNECTION_H
#include <comm_exports.h>
#include <TypeRepresentation.h>

// ****************************************************************************
// Class: Connection
//
// Purpose:
//   This is the base class for a stream that converts simple types
//   to the destination format of another machine.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   May 2000
//
// Modifications:
//    Jeremy Meredith, Mon Feb 26 16:05:36 PST 2001
//    Added unsigned chars.
//
//    Brad Whitlock, Wed Mar 20 17:23:43 PST 2002
//    Added Fill, DirectWrite, DirectRead, NeedsRead, GetDescriptor methods.
//
// ****************************************************************************

class COMM_API Connection
{
public:
    static const bool SRC;
    static const bool DEST;

    Connection();
    virtual ~Connection();

    virtual int  Fill() = 0;
    virtual void Flush() = 0;
    virtual long Size() = 0;

    // These read or write a byte to whatever we're using
    // as the connection.
    virtual void Write(unsigned char value) = 0;
    virtual void Read(unsigned char *address) = 0;
    virtual void Append(const unsigned char *buf, int count) = 0;
    virtual long DirectRead(unsigned char *buf, long len) = 0;
    virtual long DirectWrite(const unsigned char *buf, long len) = 0;
    virtual bool NeedsRead(bool = false) const { return true; };
    virtual int  GetDescriptor() const { return -1; };

    // These methods call the Write method after handling conversion issues.
    void WriteChar(unsigned char c);
    void WriteUnsignedChar(unsigned char c);
    void WriteInt(int val);
    void WriteLong(long val);
    void WriteFloat(float val);
    void WriteDouble(double val);

    // We should be able to read into a variable without conversion
    // since conversion takes place on writes to the connection.
    void ReadChar(unsigned char *c);
    void ReadUnsignedChar(unsigned char *c);
    void ReadInt(int *i);
    void ReadLong(long *l);
    void ReadFloat(float *f);
    void ReadDouble(double *d);

    int CharSize(bool = true);
    int UnsignedCharSize(bool = true);
    int IntSize(bool = true);
    int LongSize(bool = true);
    int FloatSize(bool = true);
    int DoubleSize(bool = true);

    int IntFormat(bool = true);
    int LongFormat(bool = true);
    int FloatFormat(bool = true);
    int DoubleFormat(bool = true);

    // Set the destination format.
    void  SetDestinationFormat(const TypeRepresentation &);
    const TypeRepresentation &GetDestinationFormat() const;
    void  EnableConversion(bool val);
protected:
    // Conversion related stuff.
    TypeRepresentation srcFormat, destFormat;
    bool doConversion;
};

//
// Inline these simple read/write methods.
//
inline void Connection::WriteChar(unsigned char c)
{
    Write(c);
}

inline void Connection::WriteUnsignedChar(unsigned char c)
{
    Write(c);
}

inline void Connection::ReadChar(unsigned char *c)
{
    Read(c);
}

inline void Connection::ReadUnsignedChar(unsigned char *c)
{
    Read(c);
}

//
// Functions to return the sizes of the source or destation types.
//

inline int Connection::CharSize(bool dest)
{
    return (dest ? destFormat.CharSize() : srcFormat.CharSize());
}

inline int Connection::IntSize(bool dest)
{
    return (dest ? destFormat.IntSize() : srcFormat.IntSize());
}

inline int Connection::LongSize(bool dest)
{
    return (dest ? destFormat.LongSize() : srcFormat.LongSize());
}

inline int Connection::FloatSize(bool dest)
{
    return (dest ? destFormat.FloatSize() : srcFormat.FloatSize());
}

inline int Connection::DoubleSize(bool dest)
{
    return (dest ? destFormat.DoubleSize() : srcFormat.DoubleSize());
}

//
// Functions to return the format of these types
//

inline int Connection::IntFormat(bool dest)
{
    return (int)(dest ? destFormat.IntFormat : srcFormat.IntFormat);
}

inline int Connection::LongFormat(bool dest)
{
    return (int)(dest ? destFormat.LongFormat : srcFormat.LongFormat);
}

inline int Connection::FloatFormat(bool dest)
{
    return (int)(dest ? destFormat.FloatFormat : srcFormat.FloatFormat);
}

inline int Connection::DoubleFormat(bool dest)
{
    return (int)(dest ? destFormat.DoubleFormat : srcFormat.DoubleFormat);
}
#endif
