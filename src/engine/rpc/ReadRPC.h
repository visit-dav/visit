// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef READ_RPC_H
#define READ_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>
#include <CompactSILRestrictionAttributes.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>

// ****************************************************************************
//  Class: ReadRPC
//
//  Purpose:
//    Initiate an engine computation by reading a database.
//    Sends other attributes needed for computation as well.
//
//  Modifications:
//    Hank Childs, Fri Dec 14 17:30:17 PST 2001
//    Modified all methods to operate on a "compact" sil restriction instead
//    of a normal sil restriction.
//
//    Jeremy Meredith, Thu Oct 24 11:18:20 PDT 2002
//    Added material interface reconstruction attributes.
//
//    Hank Childs, Tue Mar  9 14:27:31 PST 2004
//    Added file format type.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes 
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Tue Jun 10 15:57:15 PDT 2008
//    Added ignoreExtents
//
//    Brad Whitlock, Mon Aug 22 09:50:39 PDT 2011
//    Added selectionName.
//
//    Eric Brugger, Mon Oct 31 09:31:29 PDT 2011
//    Added window id.
//
// ****************************************************************************

class ENGINE_RPC_API ReadRPC : public BlockingRPC
{
public:
    ReadRPC();
    virtual ~ReadRPC();

    virtual const std::string TypeName() const { return "ReadRPC"; }

    // Invokation method
    void operator()(const std::string &format,
                    const std::string &filename,
                    const std::string &var,
                    int time,
                    const CompactSILRestrictionAttributes &,
                    const MaterialAttributes &,
                    const MeshManagementAttributes &,
                    bool  treatAllDBsAsTimeVarying,
                    bool  ignoreExtents,
                    const std::string &selName,
                    int   windowID);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetFormat(const std::string&);
    void SetFile(const std::string&);
    void SetVar(const std::string&);
    void SetTime(int);
    void SetCSRAttributes(const CompactSILRestrictionAttributes &);
    void SetMaterialAttributes(const MaterialAttributes &);
    void SetMeshManagementAttributes(const MeshManagementAttributes &);
    void SetTreatAllDBsAsTimeVarying(bool);
    void SetIgnoreExtents(bool);
    void SetSelectionName(const std::string &selName);
    void SetWindowID(int);

    // Property getting methods
    std::string GetVar()  const;
    std::string GetFile() const;
    std::string GetFormat() const;
    int         GetTime() const;
    const CompactSILRestrictionAttributes &GetCSRAttributes() const;
    const MaterialAttributes &GetMaterialAttributes() const;
    const MeshManagementAttributes &GetMeshManagementAttributes() const;
    bool  GetTreatAllDBsAsTimeVarying() const;
    bool  GetIgnoreExtents() const;
    std::string GetSelectionName() const;
    int GetWindowID() const;

private:
    std::string file;
    std::string format;
    std::string var;
    int         time;
    CompactSILRestrictionAttributes silr_atts;
    MaterialAttributes materialAtts;
    MeshManagementAttributes meshManagementAtts;
    bool treatAllDBsAsTimeVarying;
    bool ignoreExtents;
    std::string selectionName;
    int windowID;
};

#endif
