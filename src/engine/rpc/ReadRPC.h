#ifndef READ_RPC_H
#define READ_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>
#include <CompactSILRestrictionAttributes.h>
#include <MaterialAttributes.h>

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
// ****************************************************************************

class ENGINE_RPC_API ReadRPC : public BlockingRPC
{
public:
    ReadRPC();
    virtual ~ReadRPC();

    // Invokation method
    void operator()(const std::string &format, const std::string& filename,
                    const std::string &var, int time,
                    const CompactSILRestrictionAttributes &,
                    const MaterialAttributes &);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetFormat(const std::string&);
    void SetFile(const std::string&);
    void SetVar(const std::string&);
    void SetTime(int);
    void SetCSRAttributes(const CompactSILRestrictionAttributes &);
    void SetMaterialAttributes(const MaterialAttributes &);

    // Property getting methods
    std::string GetVar()  const;
    std::string GetFile() const;
    std::string GetFormat() const;
    int         GetTime() const;
    const CompactSILRestrictionAttributes &GetCSRAttributes() const;
    const MaterialAttributes &GetMaterialAttributes() const;

private:
    std::string file;
    std::string format;
    std::string var;
    int         time;
    CompactSILRestrictionAttributes silr_atts;
    MaterialAttributes materialAtts;
};

#endif
