#ifndef FILECONNECTION_H
#define FILECONNECTION_H
#include <comm_exports.h>
#include <stdio.h>
#include <Connection.h>

// ****************************************************************************
// Class: FileConnection
//
// Purpose:
//   This class is a connection that reads its data from a file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 20 17:38:24 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Mar 25 14:35:49 PST 2002
//   Added DirectRead, DirectWrite methods.
//
// ****************************************************************************

class COMM_API FileConnection : public Connection
{
public:
    FileConnection(const char *filename, bool writeMode_);
    virtual ~FileConnection();

    virtual int Fill();
    virtual void Flush();
    virtual long Size();
    virtual void Append(unsigned char *buf, int count);
    virtual long DirectRead(unsigned char *buf, long len);
    virtual long DirectWrite(const unsigned char *buf, long len);
protected:
    virtual void Write(unsigned char value);
    virtual void Read(unsigned char *address);
private:
    FILE *fileStream;
    long fileSize;
    bool writeMode;
};
#endif
