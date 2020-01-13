// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef HOSTPROFILELIST_H
#define HOSTPROFILELIST_H
#include <state_exports.h>
#include <AttributeSubject.h>

class MachineProfile;
#include <LaunchProfile.h>

// ****************************************************************************
// Class: HostProfileList
//
// Purpose:
//    This class contains a list of host profiles.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class STATE_API HostProfileList : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    HostProfileList();
    HostProfileList(const HostProfileList &obj);
protected:
    // These constructors are for objects derived from this class
    HostProfileList(private_tmfs_t tmfs);
    HostProfileList(const HostProfileList &obj, private_tmfs_t tmfs);
public:
    virtual ~HostProfileList();

    virtual HostProfileList& operator = (const HostProfileList &obj);
    virtual bool operator == (const HostProfileList &obj) const;
    virtual bool operator != (const HostProfileList &obj) const;
private:
    void Init();
    void Copy(const HostProfileList &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectMachines();

    // Property setting methods

    // Property getting methods
    const AttributeGroupVector &GetMachines() const;
          AttributeGroupVector &GetMachines();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);


    // Attributegroup convenience methods
    void AddMachines(const MachineProfile &);
    void ClearMachines();
    void RemoveMachines(int i);
    int  GetNumMachines() const;
    MachineProfile &GetMachines(int i);
    const MachineProfile &GetMachines(int i) const;

    MachineProfile &operator [] (int i);
    const MachineProfile &operator [] (int i) const;


    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    LaunchProfile *GetActiveLaunchProfileForHost(const std::string &hostName) const;
    MachineProfile *GetMachineProfileForHost(const std::string &hostName) const;

    // IDs that can be used to identify fields in case statements
    enum {
        ID_machines = 0,
        ID__LAST
    };

protected:
    AttributeGroup *CreateSubAttributeGroup(int index);
private:
    AttributeGroupVector machines;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define HOSTPROFILELIST_TMFS "a*"

#endif
