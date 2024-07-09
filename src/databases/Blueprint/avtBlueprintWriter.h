// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtBlueprintWriter.h                               //
// ************************************************************************* //

#ifndef AVT_BLUEPRINT_WRITER_H
#define AVT_BLUEPRINT_WRITER_H

// NOTE: This is from avtblueprint lib
#include "avtConduitBlueprintDataAdaptor.h"

#include <avtDatabaseWriter.h>

#include <ExpressionList.h>

#include <string>
#include <vector>

#include "conduit.hpp"

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtBlueprintWriter
//
//  Purpose:
//      A module that writes out blueprint files.
//
//  Programmer:
//  Creation:
//
//  Modifications:
//
//
// ****************************************************************************

class
avtBlueprintWriter : public virtual avtDatabaseWriter
{
  public:
                   avtBlueprintWriter(DBOptionsAttributes*);
    virtual       ~avtBlueprintWriter() {;};

  protected:
    static int            INVALID_CYCLE;
    static double         INVALID_TIME;

    enum bpMeshOp {
      BP_MESH_OP_NONE,
      BP_MESH_OP_FLATTEN_CSV,
      BP_MESH_OP_FLATTEN_HDF5,
      BP_MESH_OP_PARTITION
    };

    enum bpOutputType {
      HDF5,
      YAML,
      JSON
    };

    std::string     m_stem;
    std::string     m_meshName;
    double          m_time;
    int             m_cycle;
    std::string     m_output_type;

    bpMeshOp        m_op;
    conduit::Node   m_options;
    conduit::Node   m_special_options;
    conduit::Node   m_chunks;

    ExpressionList exprList;

    void           ChunkToBpMesh(vtkDataSet *, int, int, conduit::Node &);

    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &,
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
    virtual void   WriteRootFile();
};


#endif


