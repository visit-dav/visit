// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PDB_READER_H
#define PDB_READER_H
#include <string>
#include <map>
#include <PDBFileObject.h>

// ****************************************************************************
// Class: PDBReader
//
// Purpose:
//   Abstract base class for PDB file format readers.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 09:25:28 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Sep 2 00:06:54 PDT 2004
//   Added the VariableData::FreeData method.
//
//   Brad Whitlock, Thu Nov  6 14:39:36 PST 2008
//   Added method to get the PDB file.
//
//   Mark C. Miller, Tue Apr 28 11:05:54 PDT 2009
//   Changed name of PDB() to PDBfobj() to avoid symbol collision with PDB
//   proper.
// ****************************************************************************

class PDBReader
{
public:
    PDBReader(const char *filename);
    PDBReader(PDBFileObject *p);
    virtual ~PDBReader();
    void Close();
    void SetOwnsPDBFile(bool v);

    bool Identify();
    PDBFileObject *PDBfobj();
protected:
    class VariableData
    {
    public:
        VariableData(const std::string &name);
        ~VariableData();
        bool ReadValues(PDBFileObject *pdb);
        void FreeData();

        std::string  varName;
        void        *data;
        TypeEnum     dataType;
        int         *dims;
        int          nDims;
        int          nTotalElements;
    };

    typedef std::map<std::string, VariableData *> VariableDataMap;

    virtual bool IdentifyFormat() = 0;

    PDBFileObject *pdb;
    bool           ownsPDBFile;
};

#endif
