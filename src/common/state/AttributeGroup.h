// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ATTRIBUTEGROUP_H
#define ATTRIBUTEGROUP_H
#include <state_exports.h>
#include <vector>
#include <string>
#include <visitstream.h>
#include <vectortypes.h>
#include <VisItException.h>
#include <MapNode.h>
#include <JSONNode.h>

// Forward declaration
class AttributeGroup;
class Connection;
class DataNode;

// ****************************************************************************
// Class: AttributeGroup
//
// Purpose:
//   This is the base class for state objects that can be transmitted
//   across a connection.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 4 16:37:48 PST 2000
//
// Modifications:
//    Jeremy Meredith, Mon Feb 26 16:02:31 PST 2001
//    Added unsigned chars.
//
//    Brad Whitlock, Tue Oct 9 15:36:19 PST 2001
//    Added methods for returning the type name and copying objects.
//
//    Jeremy Meredith, Wed May  8 10:56:32 PDT 2002
//    Added methods needed for keyframing, mostly relating
//    to individual fields.
//
//    Eric Brugger, Fri Nov 15 12:48:02 PST 2002
//    Add the method Interpolate.
//
//    Brad Whitlock, Fri Dec 27 15:11:40 PST 2002
//    I added a few more methods to the typeInfo class to get rid of some
//    memory leaks in purify.
//
//    Jeremy Meredith, Thu Jan 16 12:42:22 PST 2003
//    Changed things a bit for keyframing enhancements.
//
//    Brad Whitlock, Fri Mar 21 09:44:44 PDT 2003
//    I added a virtual function that can modify the DataNode that is read
//    from the config file so that we can handle old versions. I also added
//    a static method for version comparison.
//
//    Brad Whitlock, Tue May 20 08:54:38 PDT 2003
//    I added a second argument to CreateNode.
//
//    Brad Whitlock, Wed Dec 17 11:51:40 PDT 2003
//    I added a third argument to CreateNode.
//
//    Brad Whitlock, Thu Dec 9 15:04:41 PST 2004
//    I added FieldType_variablename.
//
//    Hank Childs, Fri Jan 28 15:36:03 PST 2005
//    Re-inherit exceptions from VisItException.
//
//    Brad Whitlock, Tue May 3 11:05:15 PDT 2005
//    I added SelectField, SelectFields and made Select private.
//
//    Kathleen Bonnell, Thu Mar 22 16:43:38 PDT 2007
//    Added FieldType_scalemode.
//
//    Brad Whitlock, Tue Jan  6 13:47:25 PST 2009
//    I added support for including MapNode objects as fields.
//
//    Mark C. Miller, Wed Aug 26 11:11:19 PDT 2009
//    Added _private_tmfs_t type for derived state object constructors.
//
//    Mark C. Miller, Mon Aug 31 14:07:18 PDT 2009
//    Added a generic exception class for state object failures.
//
//    Kathleen Biagas, Wed Dec 21 07:47:58 PST 2016
//    Added glyphtype field.
//
// ****************************************************************************

class STATE_API AttributeGroup
{
public:
    enum FieldType {
        FieldType_unknown,
        FieldType_bool,
        FieldType_boolVector,
        FieldType_char,
        FieldType_charArray,
        FieldType_charVector,
        FieldType_uchar,
        FieldType_ucharArray,
        FieldType_ucharVector,
        FieldType_int,
        FieldType_intArray,
        FieldType_intVector,
        FieldType_long,
        FieldType_longArray,
        FieldType_longVector,
        FieldType_float,
        FieldType_floatArray,
        FieldType_floatVector,
        FieldType_double,
        FieldType_doubleArray,
        FieldType_doubleVector,
        FieldType_string,
        FieldType_stringArray,
        FieldType_stringVector,
        FieldType_colortable,
        FieldType_color,
        FieldType_opacity,
        FieldType_linestyle,
        FieldType_linewidth,
        FieldType_variablename,
        FieldType_att,
        FieldType_attVector,
        FieldType_enum,
        FieldType_scalemode,
        FieldType_MapNode,
        FieldType_glyphtype
    };

    enum {
        AttributeGroupType = MapNode::ID__LAST,
        AttributeGroupListType,
        AttributeGroupVectorType,
        ID__LAST
    } AttributeGroupEnumType;
    // This type is used only by code-generated state objects
    // which are themselves derived from other code-generated
    // state objects. We define this (silly) type to prevent
    // any possibility of collision between a user-defined
    // constructor requiring a single char* or std::string
    // argument and constructors needed to support derived
    // state objects.
    typedef struct _private_tmfs_t {
        const char *tmfs;
    } private_tmfs_t;

public:
    AttributeGroup(const char *formatString);
    virtual ~AttributeGroup();

    void Write(Connection &conn);
    void Read(Connection &conn);
    int  CalculateMessageSize(Connection &conn);

    void Write(MapNode& map);
    void WriteAPI(MapNode& map);
    void WriteMetaData(MapNode& map);
    //void Read(MapNode& map);

    void Write(JSONNode& map);
    void WriteAPI(JSONNode& map);
    void WriteMetaData(JSONNode& map);
    //void Read(JSONNode& map);

    void SetSendMetaInformation(bool send) { sendMetaInformation = send; }
    bool GetSendMetaInformation() { return sendMetaInformation; }
    int  NumAttributes() const;
    int  NumAttributesSelected() const;
    bool IsSelected(int i) const;

    void SetGuido(int);
    int  GetGuido();

    // Selects all of the attributes in the AttributeGroup
    virtual void SelectAll() = 0;
    void SelectField(int);
    void SelectFields(const std::vector<int> &);
    void UnSelectAll();

    virtual bool CreateNode(DataNode *node, bool, bool);
    virtual void SetFromNode(DataNode *node);
    virtual void ProcessOldVersions(DataNode *node, const char *configVersion);

    STATE_API friend ostream& operator<<(ostream& os, const AttributeGroup&);

    virtual bool CopyAttributes(const AttributeGroup *atts);
    virtual void InterpolateConst(const AttributeGroup *atts1,
                                  const AttributeGroup *atts2, double f);
    virtual void InterpolateLinear(const AttributeGroup *atts1,
                                   const AttributeGroup *atts2, double f);
    virtual bool EqualTo(const AttributeGroup *atts) const;
    virtual const std::string TypeName() const;

    virtual std::string GetFieldName(int index) const;
    int                 FieldNameToIndex(const std::string &fieldName) const;
    virtual FieldType   GetFieldType(int index) const;
    virtual std::string GetFieldTypeName(int index) const;
    virtual bool        FieldsEqual(int index, const AttributeGroup*) const;

    // Generic methods for setting fields.
    virtual bool SetValue(const std::string &name, const char &value);
    virtual bool SetValue(const std::string &name, const unsigned char &value);
    virtual bool SetValue(const std::string &name, const int &value);
    virtual bool SetValue(const std::string &name, const long &value);
    virtual bool SetValue(const std::string &name, const float &value);
    virtual bool SetValue(const std::string &name, const double &value);
    virtual bool SetValue(const std::string &name, const std::string &value);
    virtual bool SetValue(const std::string &name, const bool &value);

    virtual bool SetValue(const std::string &name, const char *value, int len);
    virtual bool SetValue(const std::string &name, const unsigned char *value, int len);
    virtual bool SetValue(const std::string &name, const int *value, int len);
    virtual bool SetValue(const std::string &name, const long *value, int len);
    virtual bool SetValue(const std::string &name, const float *value, int len);
    virtual bool SetValue(const std::string &name, const double *value, int len);
    virtual bool SetValue(const std::string &name, const std::string *value, int len);
    virtual bool SetValue(const std::string &name, const bool *value, int len);

    virtual bool SetValue(const std::string &name, const charVector &value);
    virtual bool SetValue(const std::string &name, const unsignedCharVector &value);
    virtual bool SetValue(const std::string &name, const intVector &value);
    virtual bool SetValue(const std::string &name, const longVector &value);
    virtual bool SetValue(const std::string &name, const floatVector &value);
    virtual bool SetValue(const std::string &name, const doubleVector &value);
    virtual bool SetValue(const std::string &name, const stringVector &value);
    virtual bool SetValue(const std::string &name, const boolVector &value);

    virtual bool SetValue(const std::string &name, const MapNode &value);

    // Generic methods for getting fields.
    virtual bool GetValue(const std::string &name,  char &value);
    virtual bool GetValue(const std::string &name,  unsigned char &value);
    virtual bool GetValue(const std::string &name,  int &value);
    virtual bool GetValue(const std::string &name,  long &value);
    virtual bool GetValue(const std::string &name,  float &value);
    virtual bool GetValue(const std::string &name,  double &value);
    virtual bool GetValue(const std::string &name,  std::string &value);
    virtual bool GetValue(const std::string &name,  bool &value);

    virtual bool GetValue(const std::string &name,  char **value, int &len);
    virtual bool GetValue(const std::string &name,  unsigned char **value, int &len);
    virtual bool GetValue(const std::string &name,  int **value, int &len);
    virtual bool GetValue(const std::string &name,  long **value, int &len);
    virtual bool GetValue(const std::string &name,  float **value, int &len);
    virtual bool GetValue(const std::string &name,  double **value, int &len);
    virtual bool GetValue(const std::string &name,  std::string **value, int &len);
    virtual bool GetValue(const std::string &name,  bool **value, int &len);

    virtual bool GetValue(const std::string &name,  charVector &value);
    virtual bool GetValue(const std::string &name,  unsignedCharVector &value);
    virtual bool GetValue(const std::string &name,  intVector &value);
    virtual bool GetValue(const std::string &name,  longVector &value);
    virtual bool GetValue(const std::string &name,  floatVector &value);
    virtual bool GetValue(const std::string &name,  doubleVector &value);
    virtual bool GetValue(const std::string &name,  stringVector &value);
    virtual bool GetValue(const std::string &name,  boolVector &value);

    virtual bool GetValue(const std::string &name,  MapNode &value);

    static bool VersionLessThan(const char *configVersion, const char *version);
protected:
    void Select(int index, void *address, int length = 0);

    // Used to instantiate a new sub attribute.
    virtual AttributeGroup *CreateSubAttributeGroup(int attrId);

    // These are used to declare the AttributeGroup's component types.
    void DeclareChar();
    void DeclareUnsignedChar();
    void DeclareInt();
    void DeclareLong();
    void DeclareFloat();
    void DeclareDouble();
    void DeclareString();
    void DeclareAttributeGroup();
    void DeclareBool();

    void DeclareListChar();
    void DeclareListUnsignedChar();
    void DeclareListInt();
    void DeclareListLong();
    void DeclareListFloat();
    void DeclareListDouble();
    void DeclareListString();
    void DeclareListAttributeGroup();
    void DeclareListBool();

    void DeclareVectorChar();
    void DeclareVectorUnsignedChar();
    void DeclareVectorInt();
    void DeclareVectorLong();
    void DeclareVectorFloat();
    void DeclareVectorDouble();
    void DeclareVectorString();
    void DeclareVectorAttributeGroup();
    void DeclareVectorBool();

    void DeclareMapNode();
private:
    class STATE_API typeInfo
    {
    public:
        typeInfo();
        typeInfo(const typeInfo &);
        typeInfo(unsigned char tcode);
        virtual ~typeInfo();
        void operator = (const typeInfo &);

        void         *address;
        int           length;
        unsigned char typeCode;
        bool          selected;
    };

    typedef std::vector<typeInfo> typeInfoVector;

    // Support methods
    void CreateTypeMap(const char *formatString);
    void WriteType(Connection &conn, typeInfo &info);
    void ReadType(Connection &conn, int attrId, typeInfo &info);
    void WriteType(MapNode &map, int attrId, typeInfo &info);
    void WriteAPI(MapNode &map, int attrId,typeInfo &info);
    //void ReadType(MapNode &map, int attrId, typeInfo &info);
    void WriteMetaData(MapNode &map, int attrId,typeInfo &info);

    void WriteType(JSONNode &map, int attrId, typeInfo &info);
    void WriteAPI(JSONNode &map, int attrId,typeInfo &info);
    //void ReadType(JSONNode &map, int attrId, typeInfo &info);
    void WriteMetaData(JSONNode &map, int attrId,typeInfo &info);
private:
    typeInfoVector typeMap;  // Holds typemap for the whole class
    int guido;
    bool sendMetaInformation;   //tells connection whether to send meta information
                                //as well.
};

STATE_API ostream& operator<<(ostream& os, const AttributeGroup&);

typedef std::vector<AttributeGroup *> AttributeGroupVector;

// An exception class
class STATE_API BadDeclareFormatString : public VisItException { };
class STATE_API StateObjectException : public VisItException { };

#endif
