// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef _GETMETADATA_RPC_H_
#define _GETMETADATA_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <vector>
#include <string>
#include <avtDatabaseMetaData.h>

// ****************************************************************************
// Class: GetMetaDataRPC
//
// Purpose:
//   This class encapsulates a call to get the metadata for a database
//   from a remote file system.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   September  1, 2000
//
// Modifications:
//   Brad Whitlock, Tue May 13 15:18:10 PST 2003
//   I added an optional timestate argument.
//
//   Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//   Added forceReadAllCyclesAndTimes  
//
//   Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//   Added ability to force using a specific plugin when reading
//   the metadata from a file (if it causes the file to be opened).
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added support to treat all databases as time varying
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Added support for controlling creation of MeshQuality and TimeDerivative 
//   expressions.
//
//   Cyrus Harrison, Wed Nov 28 11:22:13 PST 2007
//   Added support for controlling creation of vector magnitude expressions
//
// ****************************************************************************

class MDSERVER_RPC_API GetMetaDataRPC : public BlockingRPC
{
public:
    GetMetaDataRPC();
    virtual ~GetMetaDataRPC();

    virtual const std::string TypeName() const;

    // Invokation method
    const avtDatabaseMetaData *operator()(const std::string&, int timeState=0,
                                 bool forceReadAllCyclesAndTimes=false,
                                 const std::string &forcedFileType="",
                                 bool treatAllDBsAsTimeVarying=false,
                                 bool createMeshQualityExpressions=true,
                                 bool createTimeDerivativeExpressions=true,
                                 bool createVectorMagnitudeExpressions=true);

    // Property setting methods
    void SetFile(const std::string&);
    void SetTimeState(int ts);
    void SetForceReadAllCyclesAndTimes(bool force);
    void SetForcedFileType(const std::string&);
    void SetTreatAllDBsAsTimeVarying(bool set);
    void SetCreateMeshQualityExpressions(bool set);
    void SetCreateTimeDerivativeExpressions(bool set);
    void SetCreateVectorMagnitudeExpressions(bool set);
    

    // Property getting methods
    std::string GetFile() const;
    int GetTimeState() const;
    bool GetForceReadAllCyclesAndTimes() const;
    std::string GetForcedFileType() const;
    bool GetTreatAllDBsAsTimeVarying() const;
    bool GetCreateMeshQualityExpressions() const;
    bool GetCreateTimeDerivativeExpressions() const;
    bool GetCreateVectorMagnitudeExpressions() const;

    // Property selection methods
    virtual void SelectAll();
private:
    avtDatabaseMetaData  metaData;
    std::string          file;
    int                  timeState;
    bool                 forceReadAllCyclesAndTimes;
    std::string          forcedFileType;
    bool                 treatAllDBsAsTimeVarying;
    bool                 createMeshQualityExpressions;
    bool                 createTimeDerivativeExpressions;
    bool                 createVectorMagnitudeExpressions;
};


#endif
