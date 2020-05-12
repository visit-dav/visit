// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtBlueprintWriter.h                               //
// ************************************************************************* //

#ifndef AVT_BLUEPRINT_WRITER_H
#define AVT_BLUEPRINT_WRITER_H

#include <avtDatabaseWriter.h>

#include <ExpressionList.h>

#include <string>
#include <vector>

#include "conduit.hpp"


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
                   avtBlueprintWriter();
    virtual       ~avtBlueprintWriter() {;};

  protected:
    static int            INVALID_CYCLE;
    static double         INVALID_TIME;

    std::string    m_stem;
    std::string    m_meshName;
    std::string    m_mbDirName;
    std::string    m_output_dir;
    double         m_time;
    int            m_cycle;
    int            m_nblocks;
    bool           m_genRoot;
    conduit::Node  n_root_file;
    std::string    m_root_file;

    ExpressionList exprList;

    void           GenRootNode(conduit::Node &mesh,
                               const std::string output_dir,
                               const int ndims);

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


