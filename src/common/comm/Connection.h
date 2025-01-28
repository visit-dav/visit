// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CONNECTION_H
#define CONNECTION_H
#include <comm_exports.h>
#include <TypeRepresentation.h>
#include <string>

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
//    Brad Whitlock, Tue Jan  6 15:45:23 PST 2009
//    I added convenience methods for reading/writing string.
//
//    Brad Whitlock, Tue Jan 28 09:45:07 PST 2025
//    Port interface to size_t for lenghts
//
// ****************************************************************************

class COMM_API Connection
{
public:
    static const bool SRC;
    static const bool DEST;

    Connection();
    virtual ~Connection();

    virtual size_t Fill() = 0;
    virtual void   Flush() = 0;
    virtual size_t Size() = 0;
    virtual void   Reset() = 0; /// Reset Connection State

    // These read or write a byte to whatever we're using
    // as the connection.
    virtual void Write(unsigned char value) = 0;
    virtual void Read(unsigned char *address) = 0;
    virtual void Append(const unsigned char *buf, size_t count) = 0;
    virtual size_t DirectRead(unsigned char *buf, size_t len) = 0;
    virtual size_t DirectWrite(const unsigned char *buf, size_t len) = 0;
    virtual size_t ReadHeader(unsigned char *buf, size_t len);
    virtual size_t WriteHeader(const unsigned char *buf, size_t len);
    virtual bool NeedsRead(bool = false) const { return true; }
    virtual int  GetDescriptor() const { return -1; }

    // These methods call the Write method after handling conversion issues.
    void WriteChar(unsigned char c);
    void WriteUnsignedChar(unsigned char c);
    void WriteInt(int val);
    void WriteLong(long val);
    void WriteFloat(float val);
    void WriteDouble(double val);
    void WriteString(const std::string &);
    // We should be able to read into a variable without conversion
    // since conversion takes place on writes to the connection.
    void ReadChar(unsigned char *c);
    void ReadUnsignedChar(unsigned char *c);
    void ReadInt(int *i);
    void ReadLong(long *l);
    void ReadFloat(float *f);
    void ReadDouble(double *d);
    void ReadString(std::string &);

    size_t CharSize(bool = true);
    size_t UnsignedCharSize(bool = true);
    size_t IntSize(bool = true);
    size_t LongSize(bool = true);
    size_t FloatSize(bool = true);
    size_t DoubleSize(bool = true);

    size_t IntFormat(bool = true);
    size_t LongFormat(bool = true);
    size_t FloatFormat(bool = true);
    size_t DoubleFormat(bool = true);

    // Set the destination format.
    void  SetDestinationFormat(const TypeRepresentation &);
    const TypeRepresentation &GetDestinationFormat() const;
    const TypeRepresentation &GetSourceFormat() const;
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

inline size_t Connection::CharSize(bool dest)
{
    return (dest ? destFormat.CharSize() : srcFormat.CharSize());
}

inline size_t Connection::IntSize(bool dest)
{
    return (dest ? destFormat.IntSize() : srcFormat.IntSize());
}

inline size_t Connection::LongSize(bool dest)
{
    return (dest ? destFormat.LongSize() : srcFormat.LongSize());
}

inline size_t Connection::FloatSize(bool dest)
{
    return (dest ? destFormat.FloatSize() : srcFormat.FloatSize());
}

inline size_t Connection::DoubleSize(bool dest)
{
    return (dest ? destFormat.DoubleSize() : srcFormat.DoubleSize());
}

//
// Functions to return the format of these types
//

inline size_t Connection::IntFormat(bool dest)
{
    return (dest ? destFormat.IntFormat : srcFormat.IntFormat);
}

inline size_t Connection::LongFormat(bool dest)
{
    return (dest ? destFormat.LongFormat : srcFormat.LongFormat);
}

inline size_t Connection::FloatFormat(bool dest)
{
    return (dest ? destFormat.FloatFormat : srcFormat.FloatFormat);
}

inline size_t Connection::DoubleFormat(bool dest)
{
    return (dest ? destFormat.DoubleFormat : srcFormat.DoubleFormat);
}
#endif
