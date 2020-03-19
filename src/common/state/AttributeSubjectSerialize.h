// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ATTRIBUTE_SUBJECT_SERIALIZE_H
#define ATTRIBUTE_SUBJECT_SERIALIZE_H
#include <state_exports.h>

class Connection;
class MapNode;
class JSONNode;
class AttributeSubject;

// ****************************************************************************
// Class: AttributeSubjectSerialize
//
// Purpose:
//   Serializes an AttributeSubject to a connection in binary or ascii JSON
//   formats.
//
// Notes:
//
// Programmer: Hari Krishnan
// Creation:   Tue Oct 14 13:10:41 PDT 2014
//
// Modifications:
//   Brad Whitlock, Tue Oct 14 13:10:58 PDT 2014
//   Pulled this code out of SocketConnection so SocketConnection can remain
//   simple and the code can operate on multiple connection types.
//
// ****************************************************************************

class STATE_API AttributeSubjectSerialize
{
public:
    AttributeSubjectSerialize();
    ~AttributeSubjectSerialize();
    void SetConnection(Connection *conn);
    int  Fill();
    void Flush(AttributeSubject *);
    int Write(int id,
              JSONNode& node,
              JSONNode& metadata);
private:
    int Write(int id,MapNode *mapnode);
    void WriteToBuffer(MapNode *mapnode,
                       bool write,
                       int id,
                       int& totalLen,
                       int &totalSize);
    void WriteToBuffer(const JSONNode& node,
                       const JSONNode& metadata,
                       bool write,
                       int id,
                       int& totalLen,
                       int &totalSize);

    Connection *conn;
};

#endif
