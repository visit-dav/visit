#ifndef COMMUNICATION_HEADER_H
#define COMMUNICATION_HEADER_H
#include <comm_exports.h>
#include <TypeRepresentation.h>
#include <string>

class Connection;

// ****************************************************************************
// Class: CommunicationHeader
//
// Purpose:
//   This class contains the information that is exchanged between components
//   when they first connect to each other.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 25 13:57:58 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Apr 27 15:28:07 PDT 2001
//    Added fail bit to WriteHeader.
//
//    Brad Whitlock, Mon Mar 25 14:06:21 PST 2002
//    I made ReadHeader and WriteHeader use Connection objects.
//
//    Brad Whitlock, Mon Dec 16 13:11:30 PST 2002
//    I added a method to return a random key.
//
// ****************************************************************************

class COMM_API CommunicationHeader
{
public:
    CommunicationHeader();
    ~CommunicationHeader();
    void WriteHeader(Connection *conn, const std::string &version,
                     const std::string &key1, const std::string &key2,
                     int failCode = 0);
    void ReadHeader(Connection *conn, const std::string &version,
                    bool checkKeys = true);
    const TypeRepresentation &GetTypeRepresentation() const;

    static std::string CreateRandomKey(int len = 10);
    void SetSecurityKey(const std::string &key) { securityKey = key; }
    const std::string &GetSocketKey() const { return socketKey; }
private:
    TypeRepresentation rep;
    std::string        securityKey, socketKey;
};

#endif
