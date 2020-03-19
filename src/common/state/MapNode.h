// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MAP_NODE_H
#define MAP_NODE_H

#include <state_exports.h>
#include <XMLNode.h>
#include <JSONNode.h>
#include <Variant.h>
#include <map>

// ****************************************************************************
//  Class:  MapNode
//
//  Purpose:
//    Provides a nested map type.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
//  Modifications:
//    Brad Whitlock, Tue Jan  6 15:32:50 PST 2009
//    I added methods so the MapNode can read/write itself using Connection.
//
//    Brad Whitlock, Fri Jan 16 11:36:10 PST 2009
//    I added a Merge function.
//
// ****************************************************************************

class STATE_API MapNode : public Variant
{
  public:
    enum {
        MapNodeType = Variant::ID__LAST,
        ID__LAST
    } MapNodeTypeEnum;

    MapNode();
    MapNode(const MapNode&);
    MapNode(const XMLNode&,bool decodeString = true);
    MapNode(const XMLNode*,bool decodeString = true);
    explicit MapNode(const JSONNode&, bool decodeString = true);
    explicit MapNode(const JSONNode*, bool decodeString = true);
    explicit MapNode(const JSONNode&, const JSONNode& metadata, bool decodeString = true);
    explicit MapNode(const JSONNode*,const JSONNode *metadata, bool decodeString = true);
    MapNode  &operator=(const MapNode&);
    MapNode  &operator=(bool);
    MapNode  &operator=(char);
    MapNode  &operator=(unsigned char);
    MapNode  &operator=(const char *); // interp as string
    MapNode  &operator=(int);
    MapNode  &operator=(long);
    MapNode  &operator=(float);
    MapNode  &operator=(double);
    MapNode  &operator=(const std::string &);
    MapNode  &operator=(const boolVector &);
    MapNode  &operator=(const charVector &);
    MapNode  &operator=(const unsignedCharVector &);
    MapNode  &operator=(const intVector &);
    MapNode  &operator=(const longVector &);
    MapNode  &operator=(const floatVector &);
    MapNode  &operator=(const doubleVector &);
    MapNode  &operator=(const stringVector &);
    MapNode  &operator=(const Variant &);
    virtual  ~MapNode();

    bool                 operator ==(const MapNode &obj) const;

    MapNode             &operator[](const std::string &);
    MapNode             *GetEntry(const std::string &);
    const MapNode       *GetEntry(const std::string &) const;

    void                 Merge(const MapNode &);

    void                 RemoveEntry(const std::string &);
    bool                 HasEntry(const std::string &) const;
    bool                 HasNumericEntry(const std::string &) const;
    bool                 HasNumericVectorEntry(const std::string &) const;
    void                 GetEntryNames(stringVector &) const;
    int                  GetNumEntries() const {return (int)entries.size();}
    void                 Reset();

    using Variant::ToXML;
    virtual std::string  ToXML(bool encodeString = true) const;
    virtual XMLNode      ToXMLNode(bool encodeString = true) const;

    using Variant::ToJSON;
    virtual std::string  ToJSON(bool encodeString = true) const;
    virtual JSONNode     ToJSONNode(bool encodeString = true, bool id = true) const;

    int                  CalculateMessageSize(Connection &conn) const;
    int                  CalculateMessageSize(Connection *conn) const;
    void                 Write(Connection &conn) const;
    void                 Write(Connection *conn) const;
    void                 Read(Connection &conn);

 private:
    virtual JSONNode ToJSONNodeData(bool encodeString) const;
    virtual JSONNode ToJSONNodeMetaData(bool id) const;
    void  SetXValue(const XMLNode &, bool decodeString = true);
    void  SetJValue(const JSONNode &, bool decodeString = true);
    void  SetJValue(const JSONNode& data, const JSONNode& metadata,bool decodeString);
    void  SetJValue(const JSONNode* data, const JSONNode* metadata,bool decodeString);
    std::map<std::string,MapNode> entries;
};

#endif

