// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PP_Z_MTSD_FILE_FORMAT_H
#define PP_Z_MTSD_FILE_FORMAT_H
#include <avtMTSDFileFormat.h>
#include <Streaker.h>

class avtFileFormatInterface;
class PP_ZFileReader;
class PDBFileObject;

// ****************************************************************************
// Class: PP_Z_MTSD_FileFormat
//
// Purpose:
//   This subclass of PP_ZFileFormat only works for PPZ files that have
//   multiple time states per file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 13:41:07 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Sep 1 23:52:14 PST 2004
//   Added FreeUpResourcesForTimeStep method, lastTimeState, and 
//   timeFlowsForward members.
//
//   Brad Whitlock, Mon Sep 15 16:24:23 PST 2008
//   Added override for CanCacheVariable so variables read from this file
//   format are never cached in the database. We already do our own low-cost
//   caching.
//
// ****************************************************************************

class PP_Z_MTSD_FileFormat : public avtMTSDFileFormat
{
public:
    static avtFileFormatInterface *CreateInterface(PDBFileObject *pdb,
         const char *const *filenames, int nList);

    PP_Z_MTSD_FileFormat(const char *filename, const char * const *list, int nList);
    PP_Z_MTSD_FileFormat(PDBFileObject *p, const char * const *list, int nList);
    virtual ~PP_Z_MTSD_FileFormat();

    // Mimic PDBReader interface.
    virtual bool Identify();
    void SetOwnsPDBFile(bool);

    // MTSD file format methods.
    virtual const char   *GetType();
    virtual void          GetCycles(std::vector<int> &cycles);
    virtual void          GetTimes(std::vector<double> &times);
    virtual int           GetNTimesteps();
    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual void          GetTimeVaryingInformation(avtDatabaseMetaData *);
    virtual void         *GetAuxiliaryData(const char *var,
                                           int timeState,
                                           const char *type,
                                           void *args,
                                           DestructorFunction &);
    virtual bool          CanCacheVariable(const char *) { return false; }

    virtual vtkDataSet   *GetMesh(int ts, const char *var);
    virtual vtkDataArray *GetVar(int ts, const char *var);

    void FreeUpResourcesForTimeStep(int ts);
private:
    void DetermineTimeFlow(int);
    int  GetReaderIndexAndTimeStep(int ts, int &localTimeState);

    Streaker               streaker;
    int                    nReaders;
    PP_ZFileReader       **readers;
    int                    nTotalTimeSteps;
    int                    lastTimeState;
    bool                   timeFlowsForward;
};

#endif
