// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SIMV2_WRITER_H
#define AVT_SIMV2_WRITER_H

#include <avtDatabaseWriter.h>
#include <vectortypes.h>
#include <VisItDataInterfaceRuntime.h>

class vtkDataSet;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkUnstructuredGrid;
class vtkPolyData;

// ****************************************************************************
// Class: avtSimV2Writer
//
// Purpose:
//   Writer interface that translates requests to write into data transfers
//   back to the simulation. This lets the simulation provide callbacks that
//   let it accept data back from VisIt.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 30 14:22:32 PST 2006
//
// Modifications:
//    Jeremy Meredith, Tue Mar 27 11:39:24 EDT 2007
//    Added numblocks to the OpenFile method, and save off the actual
//    encountered mesh types, because we cannot trust the metadata.
//   
// ****************************************************************************

class avtSimV2Writer : public virtual avtDatabaseWriter
{
public:
                   avtSimV2Writer();
    virtual       ~avtSimV2Writer();

protected:
    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &,
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

private:
    const avtDatabaseMetaData     *metadata;
    std::string                    objectName;
    stringVector                   varList;
    int                            numblocks;

    void           WriteCurvilinearMesh(vtkStructuredGrid *,
                                       int, visit_handle);
    void           WriteUnstructuredMesh(vtkUnstructuredGrid *,
                                       int, visit_handle);
    void           WriteRectilinearMesh(vtkRectilinearGrid *,
                                       int, visit_handle);
    void           WritePolyDataMesh(vtkPolyData *,
                                       int, visit_handle);

    void           WriteDataArrays(vtkDataSet *ds, int chunk);
    void           WriteOneDataArray(vtkDataArray *, const std::string &, int, VisIt_VarCentering);
    void           WriteDataArraysConditionally(vtkDataSet *, int,
                                                const unsigned char *);
    void           WriteCellDataArrayConditionally(vtkDataArray *,
                                                   const std::string &, int,
                                                   const unsigned char *);
};


#endif
